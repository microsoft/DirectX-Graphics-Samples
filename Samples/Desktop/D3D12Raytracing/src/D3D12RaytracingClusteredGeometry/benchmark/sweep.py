#!/usr/bin/env python3
"""Run reproducible settings sweeps and generate an offline HTML report."""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import asdict, dataclass, replace
from typing import Any

from report import Chart, render_report

HERE = Path(__file__).resolve().parent
DEFAULT_EXE = HERE.parent / "bin" / "x64" / "Release" / "D3D12RaytracingClusteredGeometry.exe"
DEFAULT_RESULTS_ROOT = HERE / "results"
NOMINAL_PRECISION_BITS = tuple(range(2, 17, 2))
BENCHMARK_ENVIRONMENT = {
    "render_width": 1280, "render_height": 720,
    "benchmark_input_locked": True, "benchmark_window_mode": "background-noactivate",
    "camera_paused": True, "camera_seconds": 0.0, "animation_paused": False,
    "debug_layer_active": False,
}
STAGES = ("pf_instantiate_us", "pf_blas_us", "pf_tlas_us",
          "pf_static_blas_us", "pf_static_clas_us", "pf_render_us")
AS_COMPONENTS = {
    "clusters": ("static_clas_alloc", "static_blas_total", "animated_template",
                 "animated_pf_clas_alloc", "animated_blas", "anim_clones_blas_pool", "tlas"),
    "traditional": ("traditional_blas_actual", "animated_trad_blas",
                    "anim_clones_trad_blas_pool", "tlas"),
}
AS_WORK_LABEL = "AS preparation + build/update time per frame (GPU)"
AS_WORK_SCOPE = (
    "Sum of measured GPU stages each frame: CLAS instantiation in clustered mode or vertex "
    "animation compute in traditional mode, animated BLAS builds/refits, TLAS rebuild, and "
    "optional static CLAS/BLAS rebuilds. Excludes untimed clustered vertex-animation compute "
    "and other uninstrumented work. This is not initial setup time or CPU time.")
METRIC_LABELS = {
    ("memory_bytes", "total_as"): "AS memory (resident allocations)",
    ("memory_bytes", "static_clas_alloc"): "Static CLAS memory allocated",
    ("memory_bytes", "static_clas_actual"): "Static CLAS memory used",
    ("memory_bytes", "static_cluster_input"): "Static vertex/index input buffer size",
    ("memory_bytes", "animated_template"): "Animated CLAS template memory allocated",
    ("memory_bytes", "animated_pf_clas_actual"): "Instantiated animated CLAS memory used",
    ("build_times_ms", "total"): "Initial AS setup elapsed time (GPU timestamps)",
    ("build_times_ms", "static_clas"): "Initial static CLAS build time (GPU timestamps)",
    ("build_times_ms", "static_blas"): "Initial static BLAS build time (GPU timestamps)",
    ("frame_perf", "pf_instantiate_us"): "CLAS instantiation time per frame (GPU)",
    ("derived", "timed_as_us"): AS_WORK_LABEL,
    ("frame_perf", "pf_blas_us"): "Animated BLAS build/refit time per frame (GPU)",
    ("frame_perf", "pf_tlas_us"): "TLAS rebuild time per frame (GPU)",
    ("frame_perf", "pf_render_us"): "Raytracing render time per frame (GPU)",
    ("frame_perf", "ms_per_frame"): "Frame time (elapsed per frame)",
    ("frame_perf", "fps"): "Frame rate",
    ("scene", "instanced_triangles"): "Scene triangles (all instances)",
    ("scene", "mesh_input_triangles"): "Input mesh triangles (animated source counted once)",
    ("scene", "total_clusters"): "Static mesh cluster count",
}
METRIC_SCOPES = {
    ("derived", "timed_as_us"): AS_WORK_SCOPE,
    ("frame_perf", "pf_instantiate_us"):
        "GPU time for INSTANTIATE_CLUSTER_TEMPLATES each frame; excludes the vertex-animation "
        "compute pass that precedes instantiation.",
    ("frame_perf", "pf_render_us"):
        "GPU time inside DispatchRays only. Excludes acceleration-structure work, output copy, "
        "overlay, CPU work, and presentation waits.",
    ("frame_perf", "ms_per_frame"):
        "Elapsed time between consecutive completed GPU frames, including queue idle time "
        "from CPU or presentation throttling. Not a CPU-time or GPU-busy-time measurement.",
    ("build_times_ms", "total"):
        "Elapsed GPU-timestamp interval from the first static-AS build timestamp through the "
        "initial TLAS build. Includes animated setup/first builds and intervening queue idle "
        "or synchronization gaps; not just the static CLAS+BLAS+TLAS durations, and not total "
        "application startup time.",
    ("build_times_ms", "static_clas"):
        "GPU-timestamp interval around the initial static CLAS build path, including any "
        "intervening sizing/compaction waits. A startup measurement, not a per-frame cost.",
    ("build_times_ms", "static_blas"):
        "GPU-timestamp interval around the initial static BLAS build path, including any "
        "intervening sizing/compaction waits. A startup measurement, not a per-frame cost.",
    ("memory_bytes", "total_as"):
        "Resident acceleration-structure allocation sizes, including templates and every "
        "independent animated BLAS. Excludes scratch, vertex/index inputs, and other VRAM use.",
    ("memory_bytes", "static_clas_actual"):
        "Reported static CLAS payload bytes, not reserved capacity. Does not include template "
        "or animated CLAS memory.",
    ("memory_bytes", "animated_pf_clas_actual"):
        "Reported payload bytes used by instantiated animated CLAS, not its reserved capacity. "
        "Source vertices are shared by animated copies; their BLASes remain separate.",
    ("memory_bytes", "animated_template"):
        "Resident allocation for prebuilt animated CLAS templates, not per-frame build time.",
    ("memory_bytes", "static_cluster_input"):
        "Static source positions/indices, including packing and alignment. Build input data, "
        "not acceleration-structure memory.",
}
AS_COMPONENT_LABELS = {
    "static_clas_alloc": "Static CLAS",
    "static_blas_total": "Static cluster BLASes",
    "animated_template": "Animated CLAS templates",
    "animated_pf_clas_alloc": "Animated CLAS (instantiated)",
    "animated_blas": "Animated source BLAS",
    "anim_clones_blas_pool": "Animated copy BLASes",
    "traditional_blas_actual": "Static triangle BLASes",
    "animated_trad_blas": "Animated source BLAS",
    "anim_clones_trad_blas_pool": "Animated copy BLASes",
    "tlas": "TLAS",
}
SETTING_LABELS = {
    "geometry_mode": "Geometry path", "vertex_format": "Position format",
    "clas_alloc": "CLAS allocation strategy", "trad_alloc": "Traditional BLAS allocation strategy",
    "trad_anim_mode": "Traditional animated-BLAS update", "static_rebuild": "Static AS rebuild per frame",
    "build_flags": "BVH build preference", "extra_instances": "Extra objects (1 BLAS each)",
    "position_truncate_bits": "PositionTruncateBitCount (bits removed)",
    "compressed_bits": "COMPRESSED1 offset bits per component",
    "rays_per_pixel": "Primary rays per pixel", "animated_detail": "Animated mesh detail",
}
SETTING_VALUES = {
    "geometry_mode": {"clusters": "Clustered", "traditional": "Traditional"},
    "vertex_format": {"float": "FLOAT32_3", "compressed": "COMPRESSED1"},
    "clas_alloc": {"implicit": "Implicit allocation", "get-sizes": "Pre-size (GetSizes)",
                   "compact": "Post-build compaction"},
    "trad_alloc": {"implicit": "Implicit allocation", "compact": "Post-build compaction"},
    "trad_anim_mode": {"rebuild": "Rebuild", "refit": "Refit"},
    "static_rebuild": {"none": "Off", "blas": "BLAS only", "clas-blas": "CLAS + BLAS"},
    "build_flags": {"none": "No preference", "fast-build": "Fast build", "fast-trace": "Fast trace"},
    "animated_detail": {"full": "Full detail", "low": "Low VRAM detail"},
}


def setting_value(field: str, value: Any) -> Any:
    return SETTING_VALUES.get(field, {}).get(value, value)


def metric_unit(path: tuple[str, str], unit: str) -> str:
    return unit + "/frame" if unit in ("ms", "us") and path[0] in ("frame_perf", "derived") else unit


FORMAT_METRICS = [
    ("Resident AS", "bytes", ("memory_bytes", "total_as")),
    ("Static CLAS payload", "bytes", ("memory_bytes", "static_clas_actual")),
    ("Static source input", "bytes", ("memory_bytes", "static_cluster_input")),
    ("Animated template", "bytes", ("memory_bytes", "animated_template")),
    ("Instantiated CLAS payload", "bytes", ("memory_bytes", "animated_pf_clas_actual")),
    ("Instantiate", "us", ("frame_perf", "pf_instantiate_us")),
    (AS_WORK_LABEL, "us", ("derived", "timed_as_us")),
    ("DispatchRays", "us", ("frame_perf", "pf_render_us")),
    ("Frame interval", "ms", ("frame_perf", "ms_per_frame")),
    ("Frame rate", "FPS", ("frame_perf", "fps")),
]


@dataclass(frozen=True)
class RunConfig:
    geometry_mode: str = "clusters"
    vertex_format: str = "float"
    clas_alloc: str = "compact"
    trad_alloc: str = "compact"
    trad_anim_mode: str = "rebuild"
    static_rebuild: str = "none"
    build_flags: str = "fast-trace"
    extra_instances: int = 0
    position_truncate_bits: int = 12
    compressed_bits: int = 12
    rays_per_pixel: int = 1
    animated_detail: str = "full"

    def __post_init__(self) -> None:
        choices = dict(geometry_mode=("clusters", "traditional"),
                       vertex_format=("float", "compressed"),
                       clas_alloc=("compact", "implicit", "get-sizes"),
                       trad_alloc=("compact", "implicit"), trad_anim_mode=("rebuild", "refit"),
                       static_rebuild=("none", "blas", "clas-blas"),
                       build_flags=("none", "fast-build", "fast-trace"),
                       animated_detail=("full", "low"))
        for key, values in choices.items():
            if getattr(self, key) not in values:
                raise ValueError(f"invalid {key}: {getattr(self, key)!r}")
        limits = dict(extra_instances=(0, 100, 1000, 10000), position_truncate_bits=range(23),
                      compressed_bits=range(1, 17), rays_per_pixel=(1, 2, 4))
        for key, values in limits.items():
            value = getattr(self, key)
            if type(value) is not int or value not in values:
                raise ValueError(f"invalid {key}: {value!r}")

    def slug(self) -> str:
        identity = json.dumps(asdict(self), sort_keys=True, separators=(",", ":"))
        digest = hashlib.sha256(identity.encode()).hexdigest()[:12]
        return f"{self.geometry_mode}-n{self.extra_instances}-{self.animated_detail}-{digest}"

    def to_cli(self) -> list[str]:
        options = {
            "--geometry-mode": self.geometry_mode, "--vertex-format": self.vertex_format,
            "--clas-alloc": self.clas_alloc, "--trad-alloc": self.trad_alloc,
            "--trad-anim": self.trad_anim_mode, "--rebuild-mode": self.static_rebuild,
            "--build-flags": self.build_flags, "--extra-instances": self.extra_instances,
            "--position-truncate": self.position_truncate_bits,
            "--compressed-bits": self.compressed_bits,
            "--rays-per-pixel": self.rays_per_pixel, "--animated-detail": self.animated_detail,
            "--vsync": "off", "--camera-seconds": 0,
        }
        return [part for key, value in options.items() for part in (key, str(value))] + [
            "--camera-paused"]

    def expected_config(self) -> dict[str, Any]:
        fields = asdict(self)
        fields["compressed_bits_per_component"] = fields.pop("compressed_bits")
        fields["aa_samples_per_pixel"] = fields.pop("rays_per_pixel")
        return {**fields, "vsync": False, "wireframe": False}


@dataclass
class RunResult:
    config: RunConfig
    json_path: Path
    data: dict[str, Any]
    wall_seconds: float = 0.0

    @property
    def status(self) -> str:
        return self.data.get("_sweep", {}).get("status", "failed" if "_error" in self.data else "ok")


class TimingNotReady(ValueError):
    pass


class UnsupportedConfig(ValueError):
    pass


class BenchmarkContractError(ValueError):
    pass


def number(data: dict, group: str, name: str) -> float:
    section = data.get(group)
    if not isinstance(section, dict):
        raise ValueError(f"missing/object field required: {group}")
    value = section.get(name)
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
        raise ValueError(f"missing/nonfinite numeric field: {group}.{name}")
    if value < 0:
        raise ValueError(f"negative measurement: {group}.{name}")
    return value


def reject_constant(value: str) -> None:
    raise ValueError(f"nonfinite JSON number: {value}")


def read_json(path: Path) -> dict:
    value = json.loads(path.read_text(encoding="utf-8"), parse_constant=reject_constant)
    if not isinstance(value, dict):
        raise ValueError(f"{path.name}: expected a JSON object")
    return value


def write_json(path: Path, value: Any) -> None:
    path.write_text(json.dumps(value, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def adapter_identity(data: dict) -> tuple:
    adapter = data.get("adapter", {})
    return tuple(adapter.get(key) for key in
                 ("description", "vendor_id", "device_id", "driver_version", "clusters_supported"))


def validate_snapshot(data: dict, config: RunConfig, require_environment: bool = False) -> None:
    if data.get("schema_version") != 1:
        raise ValueError("unsupported sample JSON schema")
    adapter = data.get("adapter", {})
    if not isinstance(adapter, dict) or not isinstance(adapter.get("description"), str):
        raise ValueError("missing adapter identity")
    if config.geometry_mode == "clusters" and adapter.get("clusters_supported") is False:
        raise UnsupportedConfig("cluster support unavailable; the sample fell back to traditional")
    actual = data.get("config", {})
    recorded_environment = any(key in actual for key in BENCHMARK_ENVIRONMENT)
    if require_environment or recorded_environment:
        for key, expected in BENCHMARK_ENVIRONMENT.items():
            observed = actual.get(key)
            if type(observed) is not type(expected) or observed != expected:
                raise BenchmarkContractError(
                    f"benchmark requires {key}={expected!r}, reported {observed!r}; "
                    "rebuild the current sample and check validation/input/viewport settings")
    for key, expected in config.expected_config().items():
        if actual.get(key) != expected:
            raise ValueError(f"requested {key}={expected!r}, sample reported {actual.get(key)!r}")
    total = number(data, "memory_bytes", "total_as")
    components = sum(number(data, "memory_bytes", key) for key in AS_COMPONENTS[config.geometry_mode])
    if total != components:
        raise ValueError(f"resident AS total {total} does not equal its components {components}")
    instances = number(data, "scene", "animated_instances")
    clones = number(data, "scene", "anim_clones_pooled")
    if clones != config.extra_instances // 5 or instances != clones + 1:
        raise ValueError("animated copies do not all have independent BLASes")
    mesh_triangles = number(data, "scene", "mesh_input_triangles")
    instanced_triangles = number(data, "scene", "instanced_triangles")
    animated_triangles = number(data, "scene", "animated_triangles")
    if instanced_triangles != mesh_triangles + clones * animated_triangles:
        raise ValueError("instanced triangle count does not match the workload")
    for key in ("static_clas", "static_blas", "tlas", "total"):
        number(data, "build_times_ms", key)
    if number(data, "bench", "frames_rendered") < 2 or number(data, "bench", "actual_seconds") <= 0:
        raise ValueError("benchmark elapsed-time/frame-count evidence is missing")
    perf = data.get("frame_perf", {})
    if "avg_window_target_seconds" not in perf:
        raise ValueError("legacy independent FPS/EMA statistics; rebuild the current sample")
    if perf.get("pf_timing_valid") is not True or perf.get("averaging") is not False:
        raise TimingNotReady("the shared timing window has not settled")
    samples = number(data, "frame_perf", "frames_in_avg_window")
    first = number(data, "frame_perf", "first_sample_frame")
    last = number(data, "frame_perf", "last_sample_frame")
    if samples < 1 or samples != int(samples) or last - first + 1 != samples:
        raise ValueError("inconsistent shared timing sample range")
    fps = number(data, "frame_perf", "fps")
    ms = number(data, "frame_perf", "ms_per_frame")
    coverage = number(data, "frame_perf", "measured_window_seconds")
    target = number(data, "frame_perf", "avg_window_target_seconds")
    if fps <= 0 or ms <= 0 or target <= 0:
        raise ValueError("zero frame cadence or window duration")
    if not math.isclose(fps * ms, 1000, rel_tol=2e-5, abs_tol=0.001):
        raise ValueError("FPS and ms/frame do not describe the same cadence")
    if not math.isclose(coverage, samples * ms / 1000, abs_tol=2e-6):
        raise ValueError("timing window duration does not match its samples")
    if coverage + 2e-6 < target:
        raise ValueError("settled timing window is shorter than its target")
    costs = [number(data, "frame_perf", key) for key in STAGES]
    if sum(costs) > ms * 1000 + max(1.0, ms * 0.01):
        raise ValueError("GPU stage costs exceed the paired frame interval")
    for key, cost in zip(STAGES, costs):
        if not math.isclose(cost, number(data, "frame_perf", key + "_snap"), abs_tol=1e-4):
            raise ValueError(f"{key} is not the paired snapshot mean")


def build_sweep_matrix(quick: bool, include_10k_trad: bool,
                       base: RunConfig = RunConfig()) -> list[RunConfig]:
    runs = []
    scales = (0, 100, 1000) if quick else (0, 100, 1000, 10000)
    for count in scales:
        runs.append(replace(base, extra_instances=count))
        if count != 10000 or include_10k_trad:
            for strategy in ("rebuild", "refit"):
                runs.append(replace(base, extra_instances=count, geometry_mode="traditional",
                                    trad_anim_mode=strategy))
    runs += [replace(base, vertex_format="compressed")]
    runs += [replace(base, clas_alloc=value) for value in ("implicit", "get-sizes")]
    runs += [replace(base, geometry_mode=mode, trad_anim_mode=strategy, build_flags=value)
             for mode, strategy in (("clusters", "rebuild"), ("traditional", "rebuild"),
                                    ("traditional", "refit"))
             for value in ("none", "fast-build")]
    runs += [replace(base, geometry_mode="traditional", trad_alloc="implicit")]
    runs += [replace(base, rays_per_pixel=value) for value in (1, 2, 4)]
    alternate_detail = "low" if base.animated_detail == "full" else "full"
    runs += [replace(base, extra_instances=100, animated_detail=alternate_detail)]
    runs += [replace(base, extra_instances=100, animated_detail=alternate_detail,
                     geometry_mode="traditional", trad_anim_mode=strategy)
             for strategy in ("rebuild", "refit")]
    runs += [replace(base, clas_alloc="implicit", static_rebuild=mode)
             for mode in ("blas", "clas-blas")]
    if not quick:
        runs += [replace(base, position_truncate_bits=bits) for bits in (0, 4, 8, 12, 16, 20)]
        for bits in NOMINAL_PRECISION_BITS:
            runs += list(precision_pair_configs(base, bits))
    return list(dict.fromkeys(runs))


def payload_fingerprint(exe: Path) -> str:
    digest = hashlib.sha256()
    files = [exe, *exe.parent.glob("*.dll"), *(exe.parent / "D3D12").glob("*.dll")]
    for path in sorted(set(files)):
        digest.update(path.relative_to(exe.parent).as_posix().encode())
        with path.open("rb") as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(block)
    return digest.hexdigest()


def run_one(exe: Path, cfg: RunConfig, out_dir: Path, bench_seconds: float,
            init_timeout: float, verbose: bool = False, max_bench_seconds: float = 60.0,
            fingerprint: str | None = None, expected_adapter: tuple | None = None) -> RunResult:
    """Keep native snapshot/log paths within the sample's Windows path limits."""
    out_dir = out_dir.resolve()
    arguments = (exe, cfg, out_dir, bench_seconds, init_timeout, verbose,
                 max_bench_seconds, fingerprint, expected_adapter)
    longest_name = cfg.slug() + ".attempt-100.sample.log"
    if os.name != "nt" or len(str(out_dir / longest_name)) < 250:
        return _run_one(*arguments)
    out_dir.mkdir(parents=True, exist_ok=True)
    if any(out_dir.glob(cfg.slug() + ".*")):
        raise ValueError(f"refusing to overwrite an existing run or attempt: {cfg.slug()}")
    staging = Path(tempfile.mkdtemp(prefix="cluster-bench-"))
    if len(str(staging / longest_name)) >= 250:
        staging.rmdir()
        raise ValueError("TEMP is too long for native benchmark output; use a shorter TEMP path")
    try:
        result = _run_one(exe, cfg, staging, bench_seconds, init_timeout, verbose,
                          max_bench_seconds, fingerprint, expected_adapter)
        files = list(staging.iterdir())
        if any((out_dir / path.name).exists() for path in files):
            raise FileExistsError("benchmark output appeared while the run was executing")
        for path in files:
            shutil.move(str(path), str(out_dir / path.name))
        result.json_path = out_dir / result.json_path.name
        staging.rmdir()
        return result
    except (OSError, ValueError) as error:
        # Keep any unmoved evidence rather than deleting a partially saved run.
        if not any(staging.iterdir()):
            staging.rmdir()
            raise
        raise ValueError(f"{error}; additional run evidence remains in {staging}") from error


def _run_one(exe: Path, cfg: RunConfig, out_dir: Path, bench_seconds: float,
             init_timeout: float, verbose: bool = False, max_bench_seconds: float = 60.0,
             fingerprint: str | None = None, expected_adapter: tuple | None = None) -> RunResult:
    """Only unsettled measurements retry; successful runs are never repeated."""
    exe, out_dir = exe.resolve(), out_dir.resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    output = out_dir / f"{cfg.slug()}.json"
    if output.exists():
        raise ValueError(f"refusing to overwrite an existing run: {output}")
    actual_fingerprint = payload_fingerprint(exe)
    if fingerprint is not None and fingerprint != actual_fingerprint:
        raise ValueError("executable/runtime payload changed during the sweep")
    metadata = dict(requested_config=asdict(cfg), executable=str(exe),
                    payload_sha256=actual_fingerprint, attempts=[],
                    camera_paused=True, camera_seconds=0.0, animation_paused=False)
    start = time.monotonic()
    duration = bench_seconds
    data: dict[str, Any] = {}
    status = "failed"
    while True:
        attempt = len(metadata["attempts"]) + 1
        prefix = out_dir / f"{cfg.slug()}.attempt-{attempt}"
        sample_json = prefix.with_suffix(prefix.suffix + ".json")
        if sample_json.exists():
            raise ValueError(f"refusing to reuse a stale attempt: {sample_json}")
        command = [str(exe), *cfg.to_cli(), "--bench-seconds", str(duration),
                   "--bench-out", str(sample_json)]
        environment = {**os.environ, "SAMPLE_LOG": str(prefix) + ".sample.log"}
        attempt_info: dict[str, Any] = dict(command=command, bench_seconds=duration)
        metadata["attempts"].append(attempt_info)
        if verbose:
            print("  " + subprocess.list2cmdline(command), flush=True)
        try:
            with Path(str(prefix) + ".stdout.log").open("wb") as stdout, \
                    Path(str(prefix) + ".stderr.log").open("wb") as stderr:
                completed = subprocess.run(command, cwd=exe.parent, env=environment,
                                           stdout=stdout, stderr=stderr,
                                           timeout=init_timeout + duration + 15)
            if completed.returncode:
                raise ValueError(f"sample exited with code {completed.returncode}; see attempt logs")
            data = read_json(sample_json)
            if expected_adapter is not None and adapter_identity(data) != expected_adapter:
                raise ValueError("adapter/driver changed; start a new sweep instead of extending")
            validate_snapshot(data, cfg, require_environment=True)
            status = "ok"
            break
        except TimingNotReady as error:
            attempt_info["error"] = str(error)
            bench = data.get("bench", {})
            frames = bench.get("frames_rendered", 0)
            elapsed = bench.get("actual_seconds", 0)
            cadence = elapsed / (frames - 1) if frames > 1 else duration
            next_duration = min(max_bench_seconds, max(duration * 2, cadence * 5 + 1))
            if next_duration <= duration:
                data["_error"] = f"{error}; reached --max-bench-seconds={max_bench_seconds:g}"
                break
            print(f"  timing not ready; retrying this case for {next_duration:.1f}s", flush=True)
            duration = next_duration
        except UnsupportedConfig as error:
            status = "skipped"
            data["_error"] = str(error)
            break
        except BenchmarkContractError as error:
            data["_error"] = str(error)
            attempt_info["error"] = str(error)
            metadata["stop_sweep"] = True
            break
        except (OSError, ValueError, subprocess.TimeoutExpired) as error:
            data["_error"] = str(error)
            attempt_info["error"] = str(error)
            break
    wall_seconds = time.monotonic() - start
    metadata.update(status=status, wall_seconds=wall_seconds)
    data["_sweep"] = metadata
    write_json(output, data)
    return RunResult(cfg, output, data, wall_seconds)


def load_results(directory: Path) -> tuple[list[RunResult], dict]:
    manifest = read_json(directory / "manifest.json")
    if manifest.get("invalidated"):
        raise ValueError(f"sweep invalidated: {manifest['invalidated']}")
    if manifest.get("tool_schema") != 2:
        raise ValueError("unsupported or missing sweep manifest; legacy EMA results are not comparable")
    results = []
    for entry in manifest["runs"]:
        path = directory / entry["file"]
        if path.name != entry["file"]:
            raise ValueError("manifest run filenames must not contain directory components")
        data = read_json(path)
        metadata = data.get("_sweep")
        if not isinstance(metadata, dict) or metadata.get("status") not in {"ok", "failed", "skipped"}:
            raise ValueError(f"{path.name}: invalid run provenance/status")
        cfg = RunConfig(**metadata["requested_config"])
        if cfg != RunConfig(**entry["config"]):
            raise ValueError(f"{path.name}: manifest/config identity mismatch")
        if data["_sweep"]["status"] == "ok":
            if "_error" in data:
                raise ValueError(f"{path.name}: successful run contains an error")
            validate_snapshot(data, cfg)
            if metadata.get("payload_sha256") != manifest.get("payload_sha256"):
                raise ValueError(f"{path.name}: executable/runtime fingerprint differs from the sweep")
        results.append(RunResult(cfg, path, data, data["_sweep"]["wall_seconds"]))
    if len({run.config for run in results}) != len(results):
        raise ValueError("duplicate run configurations")
    identities = {adapter_identity(r.data) for r in results if r.status == "ok"}
    if len(identities) > 1:
        raise ValueError("results mix different adapters or driver versions")
    return results, manifest


def finalize_results_directory(directory: Path, runs: list[RunResult], manifest: dict) -> Path:
    """Use discovered identity without an extra adapter-probe benchmark run."""
    identified = next((run for run in runs if run.status == "ok"), None)
    if identified is None:
        identified = next((run for run in runs if isinstance(run.data.get("adapter"), dict)), None)
    adapter = identified.data["adapter"] if identified else {}

    def component(value: Any, fallback: str, limit: int) -> str:
        cleaned = re.sub(r'[<>:"/\\|?*\x00-\x1f]', "-", str(value or fallback))
        cleaned = re.sub(r"\s+", "-", cleaned)
        cleaned = re.sub(r"-+", "-", cleaned).strip(" .-_")
        return cleaned[:limit].rstrip(" .-_") or fallback

    timestamp = dt.datetime.fromisoformat(manifest["created_local"]).strftime("%Y-%m-%d_%H-%M-%S")
    name = (f"{timestamp}_{component(adapter.get('description'), 'unknown-adapter', 48)}"
            f"_driver-{component(adapter.get('driver_version'), 'unknown', 24)}")
    suffix = 1
    while True:
        target = directory.with_name(name if suffix == 1 else f"{name}_{suffix}")
        if target.exists():
            suffix += 1
            continue
        try:
            directory.rename(target)
            break
        except FileExistsError:
            suffix += 1
    for run in runs:
        run.json_path = target / run.json_path.name
    return target


def metric(run: RunResult, path: tuple[str, str]) -> float | None:
    if run.status != "ok":
        return None
    if path == ("derived", "timed_as_us"):
        return sum(number(run.data, "frame_perf", key) for key in STAGES[:-1])
    if path == ("derived", "unattributed_us"):
        interval = number(run.data, "frame_perf", "ms_per_frame") * 1000
        # Serialization can round the sum a fraction above the interval.
        return max(0.0, interval - sum(number(run.data, "frame_perf", key) for key in STAGES))
    return number(run.data, *path)


def format_comparison(runs: list[RunResult], base: RunConfig) -> dict[str, RunResult | None]:
    formats = {
        f"FLOAT32_3 (trunc={base.position_truncate_bits}; "
        f"{23 - base.position_truncate_bits} mantissa bits kept)":
            replace(base, vertex_format="float"),
        f"COMPRESSED1 ({base.compressed_bits} offset bits/component)":
            replace(base, vertex_format="compressed"),
    }
    return {label: next((run for run in runs if run.config == cfg), None)
            for label, cfg in formats.items()}


def precision_pair_configs(base: RunConfig, bits: int) -> tuple[RunConfig, RunConfig]:
    # FLOAT32 normal values have 23 fraction bits plus the implicit leading 1.
    return (replace(base, vertex_format="float", position_truncate_bits=24 - bits),
            replace(base, vertex_format="compressed", compressed_bits=bits))


def build_charts(runs: list[RunResult], base: RunConfig = RunConfig()) -> list[Chart]:
    charts: list[Chart] = []

    def matches(run: RunResult, reference: RunConfig, vary: set[str]) -> bool:
        return all(value == getattr(reference, key)
                   for key, value in asdict(run.config).items() if key not in vary)

    def axis(title: str, category: str, description: str, x_field: str,
             unit: str, path: tuple[str, str],
             references: dict[str, RunConfig], numeric_x: bool = False,
             minimum_values: int = 1) -> None:
        title = METRIC_LABELS.get(path, title)
        if x_field == "position_truncate_bits":
            title += " vs PositionTruncateBitCount"
        scope = METRIC_SCOPES.get(path, "")
        chart = Chart(title, category, (scope + " " + description).strip(),
                      SETTING_LABELS[x_field], metric_unit(path, unit),
                      numeric_x=numeric_x, chart_type="line" if x_field in {
                          "extra_instances", "position_truncate_bits", "compressed_bits",
                          "rays_per_pixel"} else "bar")
        def x_value(run: RunResult) -> Any:
            return setting_value(x_field, getattr(run.config, x_field))
        category_order = {
            "vertex_format": ("float", "compressed"),
            "clas_alloc": ("implicit", "get-sizes", "compact"),
            "trad_alloc": ("implicit", "compact"),
            "build_flags": ("none", "fast-build", "fast-trace"),
            "animated_detail": ("full", "low"),
            "static_rebuild": ("none", "blas", "clas-blas"),
        }.get(x_field)
        for label, reference in references.items():
            selected = [r for r in runs if matches(r, reference, {x_field})]
            selected.sort(key=lambda r: category_order.index(getattr(r.config, x_field))
                          if category_order else getattr(r.config, x_field))
            if sum(metric(run, path) is not None for run in selected) >= minimum_values:
                chart.series[label] = [(x_value(r), metric(r, path)) for r in selected]
                if x_field == "position_truncate_bits":
                    for run in selected:
                        count = run.config.position_truncate_bits
                        chart.x_tooltips[str(count)] = (
                            f"PositionTruncateBitCount={count}\n{23 - count} explicit mantissa "
                            f"bits kept\n{24 - count} significant bits for normal values")
        if any(value is not None for points in chart.series.values() for _, value in points):
            charts.append(chart)

    modes = {
        "clustered": base,
        "traditional rebuild": replace(base, geometry_mode="traditional"),
        "traditional refit": replace(base, geometry_mode="traditional", trad_anim_mode="refit"),
    }
    scaling = [
        ("Resident AS storage", "bytes", ("memory_bytes", "total_as"),
         "Resident BVH allocations, including templates and all independent BLASes; excludes scratch and inputs."),
        ("Static AS setup", "ms", ("build_times_ms", "total"),
         "Initial setup, recorded once before the benchmark's per-frame measurements."),
        ("Frame rate", "FPS", ("frame_perf", "fps"),
         "Completed-frame cadence from the same rolling sample window as every GPU stage."),
        ("Frame interval", "ms", ("frame_perf", "ms_per_frame"),
         "Includes GPU work and queue idle time; not just DispatchRays or GPU busy time."),
        (AS_WORK_LABEL, "us", ("derived", "timed_as_us"),
         "The measured preparation/build/update stages are listed here; the sum is not a full GPU-frame time."),
        ("Animated BLAS build/update", "us", ("frame_perf", "pf_blas_us"),
         "Separate BLAS for every animated copy. Refit preserves topology; rebuild and clustered builds do not."),
        ("TLAS rebuild", "us", ("frame_perf", "pf_tlas_us"),
         "TLAS cost for the selected workload, using the same sample set as FPS."),
        ("Scene DispatchRays", "us", ("frame_perf", "pf_render_us"),
         "Ray dispatch only; excludes output copy, overlay, and presentation."),
        ("Instanced scene triangles", "triangles", ("scene", "instanced_triangles"),
         "Counts triangle occurrences in every instance, including all animated copies."),
        ("Mesh input triangles", "triangles", ("scene", "mesh_input_triangles"),
         "Static copies plus one animated source mesh. This is not the full instanced triangle count."),
        ("Static input clusters", "clusters", ("scene", "total_clusters"),
         "Static geometry only. Curved static copies use generation-time distance LOD."),
    ]
    for title, unit, path, description in scaling:
        axis(title, "Scene scaling", description, "extra_instances", unit, path, modes)

    for label, reference in modes.items():
        chart = Chart(f"Frame time breakdown: {label}", "Frame time breakdown",
                      "AS preparation/build/update + raytracing render + other work/waits "
                      "sum to elapsed time per frame, from the same averaged frames. "
                      "Other work/waits is calculated by subtraction, not measured CPU time. "
                      + AS_WORK_SCOPE,
                      SETTING_LABELS["extra_instances"], "us/frame", chart_type="bar", stacked=True)
        selected = sorted((r for r in runs if matches(r, reference, {"extra_instances"})),
                          key=lambda r: r.config.extra_instances)
        for name, path in (
                ("AS preparation + build/update", ("derived", "timed_as_us")),
                ("Raytracing render (DispatchRays)", ("frame_perf", "pf_render_us")),
                ("Other GPU work + waits (remainder)", ("derived", "unattributed_us"))):
            chart.series[name] = [(r.config.extra_instances, metric(r, path)) for r in selected]
        if any(value is not None for points in chart.series.values() for _, value in points):
            charts.append(chart)

    studies = [
        ("CLAS allocation", "clas_alloc", {"clustered": base},
         [("Reserved static CLAS", "bytes", ("memory_bytes", "static_clas_alloc")),
          ("Static CLAS payload", "bytes", ("memory_bytes", "static_clas_actual")),
          ("GPU static CLAS setup", "ms", ("build_times_ms", "static_clas"))]),
        ("Build preference", "build_flags", modes,
         [("Resident AS", "bytes", ("memory_bytes", "total_as")),
          ("GPU static BLAS setup", "ms", ("build_times_ms", "static_blas")),
          (AS_WORK_LABEL, "us", ("derived", "timed_as_us")),
          ("DispatchRays", "us", ("frame_perf", "pf_render_us")),
          ("Frame rate", "FPS", ("frame_perf", "fps"))]),
        ("Traditional allocation", "trad_alloc", {"traditional": modes["traditional rebuild"]},
         [("Resident AS", "bytes", ("memory_bytes", "total_as")),
          ("GPU static BLAS setup", "ms", ("build_times_ms", "static_blas"))]),
        ("Primary rays", "rays_per_pixel", {"clustered": base},
         [("DispatchRays", "us", ("frame_perf", "pf_render_us")),
          ("Frame rate", "FPS", ("frame_perf", "fps"))]),
        ("Animated detail", "animated_detail",
         {label: replace(reference, extra_instances=100) for label, reference in modes.items()},
         [("Resident AS", "bytes", ("memory_bytes", "total_as")),
          ("Animated BLAS", "us", ("frame_perf", "pf_blas_us")),
          ("Instanced triangles", "triangles", ("scene", "instanced_triangles")),
          ("Frame rate", "FPS", ("frame_perf", "fps"))]),
        ("Static rebuild", "static_rebuild", {"clustered": replace(base, clas_alloc="implicit")},
         [(AS_WORK_LABEL, "us", ("derived", "timed_as_us")),
          ("Frame interval", "ms", ("frame_perf", "ms_per_frame"))]),
        ("FLOAT32_3 precision", "position_truncate_bits", {"clustered": base},
         [("Static CLAS payload", "bytes", ("memory_bytes", "static_clas_actual")),
          ("Animated template", "bytes", ("memory_bytes", "animated_template")),
          ("Instantiated CLAS payload", "bytes", ("memory_bytes", "animated_pf_clas_actual")),
          ("Instantiate", "us", ("frame_perf", "pf_instantiate_us")),
          ("DispatchRays", "us", ("frame_perf", "pf_render_us")),
          ("Frame rate", "FPS", ("frame_perf", "fps"))]),
        ("COMPRESSED1 precision", "compressed_bits",
         {"clustered": replace(base, vertex_format="compressed")},
         [("Static CLAS payload", "bytes", ("memory_bytes", "static_clas_actual")),
          ("Animated template", "bytes", ("memory_bytes", "animated_template")),
          ("Instantiated CLAS payload", "bytes", ("memory_bytes", "animated_pf_clas_actual")),
          ("Instantiate", "us", ("frame_perf", "pf_instantiate_us")),
          ("DispatchRays", "us", ("frame_perf", "pf_render_us")),
          ("Frame rate", "FPS", ("frame_perf", "fps"))]),
    ]
    by_config = {run.config: run for run in runs}
    for title, unit, path in FORMAT_METRICS:
        chart = Chart(f"COMPRESSED1 vs FLOAT32_3: {METRIC_LABELS[path]}",
                      "COMPRESSED1 vs FLOAT32_3 precision",
                      METRIC_SCOPES.get(path, "") + " "
                      "Overlaid curves across aligned nominal precision bits. FLOAT32_3 uses "
                      "24-PositionTruncateBitCount significant bits for normal values "
                      "(23-N explicit fraction bits plus the implicit leading 1). COMPRESSED1 "
                      "uses offset bits/component with a cluster-local anchor and scale. "
                      "This is an approximate bit-count alignment, not an equal-error comparison. "
                      "Hover a point for the actual controls. Missing runs are gaps; use --extend.",
                      "Precision bits (approximate alignment, not equal error)",
                      metric_unit(path, unit), numeric_x=True)
        chart.series = {"FLOAT32_3 (bits retained, including leading 1)": [],
                        "COMPRESSED1 (offset bits/component)": []}
        for bits in NOMINAL_PRECISION_BITS:
            configs = precision_pair_configs(base, bits)
            for label, cfg in zip(chart.series, configs):
                run = by_config.get(cfg)
                chart.series[label].append((bits, metric(run, path) if run else None))
            chart.x_tooltips[str(bits)] = (
                f"Nominal bits={bits} (not equal geometric error)\n"
                f"FLOAT32_3 PositionTruncateBitCount={24 - bits}\n"
                f"{bits - 1} explicit mantissa bits + implicit leading 1 (normal)\n"
                f"COMPRESSED1 {bits} offset bits/component")
        if all(sum(value is not None for _, value in points) >= 2
               for points in chart.series.values()):
            charts.append(chart)
    for category, field_name, references, metrics in studies:
        if len({getattr(r.config, field_name) for r in runs
                if any(matches(r, ref, {field_name}) for ref in references.values())}) < 2:
            continue
        for title, unit, path in metrics:
            description = {
                "clas_alloc": "Only the CLAS allocation strategy changes; source geometry, detail, "
                              "and ray settings stay fixed.",
                "trad_alloc": "Only traditional BLAS allocation changes; source geometry, detail, "
                              "and ray settings stay fixed.",
                "build_flags": "Compare No preference, Fast build, and Fast trace BVH preferences. "
                               "These change AS construction, not source geometry or ray settings.",
                "rays_per_pixel": "Number of primary rays per pixel. Shadow/reflection/refraction "
                                  "rays are additional. Geometry and AS settings stay fixed.",
                "animated_detail": "Full detail versus the explicit Low VRAM animated mesh. "
                                   "This intentionally changes triangle counts, not object/BLAS counts.",
                "static_rebuild": "Choose which static AS stages rebuild every frame. This is "
                                  "separate from initial AS setup; CLAS rebuilding uses implicit allocation.",
                "compressed_bits": "COMPRESSED1 offset bits per coordinate component. Lower values "
                                   "reduce precision and can change geometry and image quality.",
            }.get(field_name, "")
            if field_name == "position_truncate_bits":
                description = (
                    "FLOAT32_3 uses PositionTruncateBitCount, set by --position-truncate. "
                    "The x value is the number of low position-mantissa bits cleared: 0 disables "
                    "truncation, and larger values reduce precision. Of 23 explicit mantissa bits, "
                    "23 minus the count remain: count 12 keeps 11. Normal values also have an "
                    "implicit leading bit. Source components still occupy 32 bits. "
                    "It is not a COMPRESSED1 offset-bits/component setting. "
                    "Geometry and image quality may change.")
            axis(title, category, description,
                 field_name, unit, path, references,
                 numeric_x=field_name in {"position_truncate_bits", "compressed_bits", "rays_per_pixel"},
                 minimum_values=2)
    storage = Chart("AS memory by buffer (resident allocations)", "AS memory breakdown",
                    "All bars are allocated resident BVH storage, not payload estimates. "
                    "Components sum to total resident AS memory; scratch and source inputs are excluded. "
                    f"Baseline: {base.extra_instances:,} extra objects. Zero or very small buffers "
                    "are listed explicitly in the exact-values table.",
                    "AS buffer", "bytes", chart_type="bar")
    for label, reference in modes.items():
        matching = [r for r in runs if r.config == reference]
        if matching:
            run = matching[0]
            storage.series[label] = [(AS_COMPONENT_LABELS[key], metric(run, ("memory_bytes", key)))
                                     for key in AS_COMPONENTS[reference.geometry_mode]]
    if any(value is not None for points in storage.series.values() for _, value in points):
        charts.append(storage)
    return charts


def format_metric(value: float | None, unit: str = "") -> str:
    if value is None:
        return "not measured"
    if unit == "bytes":
        return f"{value / 1024 ** 2:.3f} MiB"
    return f"{value:.6g} {unit}".strip()


def report_tables(runs: list[RunResult], manifest: dict) -> list[tuple[str, list[str], list[list]]]:
    successful = [r for r in runs if r.status == "ok"]
    adapter = successful[0].data["adapter"] if successful else {}
    adapter_labels = {
        "description": "GPU adapter", "driver_version": "Driver version",
        "vendor_id": "PCI vendor ID", "device_id": "PCI device ID",
        "dedicated_vram_bytes": "Dedicated GPU memory", "clusters_supported": "Clustered geometry supported",
    }
    facts = []
    for key, value in adapter.items():
        if key == "dedicated_vram_bytes":
            value = format_metric(value, "bytes")
        elif key in ("vendor_id", "device_id"):
            value = f"0x{value:04X}"
        elif isinstance(value, bool):
            value = "Yes" if value else "No"
        facts.append([adapter_labels.get(key, key.replace("_", " ")), value])
    if successful:
        tearing = successful[0].data["config"].get("tearing_supported")
        facts += [["Tearing supported", "Yes" if tearing else "No" if tearing is False else "Not recorded"]]
        actual = successful[0].data["config"]
        if "render_width" in actual:
            facts += [["Render size (physical pixels)", f"{actual['render_width']}x{actual['render_height']}"],
                      ["Benchmark window", actual["benchmark_window_mode"]],
                      ["User input", "Blocked" if actual["benchmark_input_locked"] else "Not blocked"],
                      ["D3D debug layer", "On" if actual["debug_layer_active"] else "Off"]]
        else:
            facts += [["Render/input isolation", "Not recorded by this older sample"]]
    facts += [["Executable", manifest.get("executable", "unknown")],
              ["Executable + runtime SHA-256", manifest.get("payload_sha256", "unknown")],
              ["Sweep start time (UTC)", manifest.get("created_utc", "unknown")],
              ["Sweep start time (local)", manifest.get("created_local", "not recorded")]]
    rows = []
    for run in runs:
        perf = run.data.get("frame_perf", {}) if run.status == "ok" else {}
        rows.append([run.config.slug(), run.status, setting_value("geometry_mode", run.config.geometry_mode),
                     setting_value("trad_anim_mode", run.config.trad_anim_mode)
                     if run.config.geometry_mode == "traditional" else "Not applicable",
                     setting_value("vertex_format", run.config.vertex_format),
                     run.config.position_truncate_bits if run.config.vertex_format == "float" else "-",
                     23 - run.config.position_truncate_bits if run.config.vertex_format == "float" else "-",
                     run.config.compressed_bits if run.config.vertex_format == "compressed" else "-",
                     run.config.extra_instances, setting_value("animated_detail", run.config.animated_detail),
                     run.config.rays_per_pixel,
                     format_metric(metric(run, ("frame_perf", "fps"))),
                     format_metric(metric(run, ("frame_perf", "ms_per_frame")), "ms"),
                     format_metric(metric(run, ("memory_bytes", "total_as")), "bytes"),
                     perf.get("frames_in_avg_window", "-"),
                     perf.get("measured_window_seconds", "-"),
                     f"{perf.get('first_sample_frame', '-')}..{perf.get('last_sample_frame', '-')}",
                     f"{run.wall_seconds:.2f}", run.data.get("_error", "")])
    base = RunConfig(**manifest.get("base_config", {}))
    formats = format_comparison(runs, base)
    labels = list(formats)
    comparison_rows = [
        ["Configured precision",
         f"PositionTruncateBitCount={base.position_truncate_bits}; "
         f"{23 - base.position_truncate_bits} of 23 explicit mantissa bits kept",
         f"{base.compressed_bits} cluster-local offset bits/component",
         "Not an equal-error comparison"],
        ["Normal-value significand",
         f"{24 - base.position_truncate_bits} bits including the implicit leading bit",
         "Fixed-point offsets with cluster anchor/scale",
         "Different precision models"],
    ]
    for title, unit, path in FORMAT_METRICS:
        values = [metric(run, path) if run else None for run in formats.values()]
        delta = ("not measured" if any(value is None for value in values) else
                 f"{(values[1] / values[0] - 1) * 100:+.2f}%" if values[0] > 0 else
                 "n/a (FLOAT32_3 reference is zero)")
        comparison_rows.append([METRIC_LABELS[path], *(format_metric(value, metric_unit(path, unit))
                                                       for value in values), delta])
    by_config = {run.config: run for run in runs}
    precision_rows = []
    for bits in NOMINAL_PRECISION_BITS:
        configs = precision_pair_configs(base, bits)
        states = [by_config[cfg].status if cfg in by_config else "missing (use --extend)"
                  for cfg in configs]
        precision_rows.append([bits, 24 - bits, bits - 1, bits, *states])
    return [
        ("Run provenance", ["Property", "Value"], facts),
        ("Baseline settings", ["Setting", "Value"],
         [[SETTING_LABELS[key], setting_value(key, value)] for key, value in asdict(base).items()]),
        ("Precision alignment and coverage (nominal bits, not equal error)",
         ["Nominal bits", "FLOAT32_3 PositionTruncateBitCount", "FLOAT32_3 explicit mantissa bits",
          "COMPRESSED1 offset bits/component", "FLOAT32_3 run", "COMPRESSED1 run"],
         precision_rows),
        ("Direct format comparison (matched benchmark baseline)",
         ["Metric", *labels, "COMPRESSED1 change vs FLOAT32_3"], comparison_rows),
        ("All requested cases", ["Configuration ID", "Status", "Geometry path", "Animated BLAS update",
                                "Position format",
                                "PositionTruncateBitCount", "FLOAT32_3 mantissa bits kept",
                                "COMPRESSED1 offset bits/component",
                                "Extra objects (1 BLAS each)", "Animated mesh detail",
                                "Primary rays/pixel", "Frame rate (FPS)", "Frame time (ms/frame)",
                                "AS memory (resident allocations)", "Frames in average",
                                "Averaged duration (s)", "Averaged frame IDs",
                                "Total run time incl. retries (s)", "Error / skip reason"], rows),
    ]


REPORT_NOTES = [
    "Current benchmark runs use a non-activating background window at a verified 1280x720 "
    "physical-pixel render size. User keyboard/mouse input cannot change settings. "
    "New runs require this contract and disabled D3D validation; older saved data may lack it.",
    "AS = acceleration structure; BLAS = bottom-level AS; TLAS = top-level AS; "
    "CLAS = cluster-level AS. Time charts distinguish one-time initial setup from GPU work per frame.",
    AS_WORK_SCOPE,
    METRIC_SCOPES[("build_times_ms", "total")],
    "Memory allocated means resident buffer capacity. Memory used means the driver's reported "
    "AS payload size; it can be smaller than allocated capacity. Vertex/index inputs and scratch "
    "are separate from AS memory. Exact-value tables distinguish a true zero from Not measured.",
    "Discrete settings use grouped bars. For overlaid curves, hover anywhere at an x position "
    "to see all series' values, including coincident points. Legend entries toggle visibility. "
    "A one-setting result is not shown as a multi-setting study.",
    "All frame costs and FPS use the same settled, roughly one-second rolling window. "
    "They are not an average over the entire benchmark process lifetime.",
    "The benchmark baseline explicitly has zero extra copies; the interactive sample defaults to 100. "
    "Every added copy has an independent BLAS in both geometry modes.",
    "Static curved copies use distance LOD selected at generation time. Animated copies retain "
    "the selected Full/Low mesh; their CLAS is shared in cluster mode, but their BLASes are not.",
    "Camera position is fixed and vsync is requested off. Animation and per-frame AS work continue. "
    "Driver/compositor settings may still throttle presentation.",
    "Rebuild and refit are different algorithms, not interchangeable quality guarantees. "
    "Precision and Full/Low comparisons intentionally change geometry.",
    "Overlaid format curves align nominal normal-FLOAT32 significand bits (24 minus "
    "PositionTruncateBitCount) with COMPRESSED1 offset bits/component. This is not matched "
    "quantization error. Missing precision points need additional runs; --extend adds only "
    "missing configurations to a compatible saved sweep.",
    "Missing, failed, or unsupported measurements are gaps, never zero-cost results. "
    "Each axis has one unit and a zero baseline. These are single-run observations, not statistical claims.",
]


def positive_float(value: str) -> float:
    parsed = float(value)
    if not math.isfinite(parsed) or parsed <= 0:
        raise argparse.ArgumentTypeError("must be finite and greater than zero")
    return parsed


def execute_cases(exe: Path, matrix: list[RunConfig], directory: Path, manifest: dict,
                  runs: list[RunResult], args: argparse.Namespace) -> None:
    completed = {run.config for run in runs}
    pending = [cfg for cfg in matrix if cfg not in completed]
    print(f"Reusing {len(runs)} recorded cases; {len(pending)} missing configurations.", flush=True)
    if not pending:
        return
    if not exe.is_file():
        raise ValueError(f"executable not found: {exe}; build Release or pass --exe")
    fingerprint = manifest["payload_sha256"]
    if payload_fingerprint(exe) != fingerprint:
        raise ValueError("executable/runtime differs from saved results; start a new sweep")
    for asset in ("chart.umd.min.js", "LICENSE.Chart.js.txt"):
        if not HERE.joinpath(asset).is_file():
            raise ValueError(f"missing offline report asset: {asset}")
    successful = [run for run in runs if run.status == "ok"]
    identity = adapter_identity(successful[0].data) if successful else None
    known = (successful[0].data["adapter"] if successful else
             next((run.data["adapter"] for run in runs if "adapter" in run.data), {}))
    cluster_support = known.get("clusters_supported")
    prior_wall = manifest.get("wall_seconds", 0.0)
    started = time.monotonic()
    for index, cfg in enumerate(pending, 1):
        print(f"[{index}/{len(pending)}] {cfg.slug()}", flush=True)
        if cfg.geometry_mode == "clusters" and cluster_support is False:
            data = {"_error": "cluster support unavailable",
                    "_sweep": dict(requested_config=asdict(cfg), status="skipped",
                                   wall_seconds=0.0, payload_sha256=fingerprint)}
            path = directory / f"{cfg.slug()}.json"
            write_json(path, data)
            run = RunResult(cfg, path, data)
        else:
            run = run_one(exe, cfg, directory, args.bench_seconds, args.init_timeout,
                          args.verbose, args.max_bench_seconds, fingerprint, identity)
            if "adapter" in run.data:
                cluster_support = run.data["adapter"].get("clusters_supported")
            if run.status == "ok":
                identity = adapter_identity(run.data)
        runs.append(run)
        manifest["runs"].append(dict(file=run.json_path.name, config=asdict(cfg)))
        manifest["wall_seconds"] = prior_wall + time.monotonic() - started
        write_json(directory / "manifest.json", manifest)
        print(f"  {run.status}: " + (run.data.get("_error") or
              format_metric(metric(run, ("frame_perf", "fps")), "FPS")), flush=True)
        if run.data["_sweep"].get("stop_sweep"):
            break
        if identity is not None and "adapter" in run.data and adapter_identity(run.data) != identity:
            break


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, help="default: current Release; saved executable when extending")
    parser.add_argument("--out", type=Path)
    parser.add_argument("--results-root", type=Path, default=DEFAULT_RESULTS_ROOT)
    parser.add_argument("--quick", action="store_true", help="omit 10K and precision sweeps")
    parser.add_argument("--no-10k", action="store_true")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--no-10k-trad", action="store_true")
    group.add_argument("--include-10k-trad", action="store_true", help="included by default")
    parser.add_argument("--animated-detail", choices=("full", "low"), help="default: full for a new sweep")
    parser.add_argument("--bench-seconds", type=positive_float, default=5.0)
    parser.add_argument("--max-bench-seconds", type=positive_float, default=60.0)
    parser.add_argument("--init-timeout", type=positive_float, default=120.0,
                        help="additional per-process time allowance for initialization/slow frames")
    parser.add_argument("--no-run", action="store_true")
    parser.add_argument("--input-dir", type=Path, help="exact sweep directory to render without running")
    parser.add_argument("--extend", type=Path, help="reuse recorded cases and run only missing configurations")
    parser.add_argument("--list", action="store_true", help="print configs without requiring an executable")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args(argv)
    if args.max_bench_seconds < args.bench_seconds:
        parser.error("--max-bench-seconds must be at least --bench-seconds")
    if args.extend and (args.no_run or args.input_dir):
        parser.error("--extend cannot be combined with report-only arguments")
    if args.extend and args.list:
        parser.error("--list describes a new matrix; do not combine it with --extend")
    base = RunConfig(animated_detail=args.animated_detail or "full")
    matrix = build_sweep_matrix(args.quick, not args.no_10k_trad, base)
    if args.no_10k:
        matrix = [cfg for cfg in matrix if cfg.extra_instances != 10000]
    if args.list:
        for cfg in matrix:
            print(cfg.slug(), subprocess.list2cmdline(cfg.to_cli()))
        return 0
    try:
        if args.no_run or args.input_dir:
            directory = args.input_dir
            if directory is None:
                manifests = [path for path in args.results_root.glob("*/manifest.json")
                             if not read_json(path).get("invalidated")]
                if not manifests:
                    raise ValueError("no prior sweep manifests; supply --input-dir")
                directory = max(manifests, key=lambda p: p.stat().st_mtime_ns).parent
            runs, manifest = load_results(directory)
            base = RunConfig(**manifest["base_config"])
        elif args.extend:
            directory = args.extend.resolve()
            runs, manifest = load_results(directory)
            base = RunConfig(**manifest["base_config"])
            if args.animated_detail is not None and args.animated_detail != base.animated_detail:
                raise ValueError("cannot change an existing sweep's baseline; start a new sweep")
            matrix = build_sweep_matrix(args.quick, not args.no_10k_trad, base)
            if args.no_10k:
                matrix = [cfg for cfg in matrix if cfg.extra_instances != 10000]
            exe = (args.exe or Path(manifest["executable"])).resolve()
            execute_cases(exe, matrix, directory, manifest, runs, args)
        else:
            exe = (args.exe or DEFAULT_EXE).resolve()
            if not exe.is_file():
                raise ValueError(f"executable not found: {exe}; build Release or pass --exe")
            # Require the offline asset before spending time running benchmarks.
            for asset in ("chart.umd.min.js", "LICENSE.Chart.js.txt"):
                if not HERE.joinpath(asset).is_file():
                    raise ValueError(f"missing offline report asset: {asset}")
            fingerprint = payload_fingerprint(exe)
            now = dt.datetime.now(dt.timezone.utc)
            stamp = now.strftime("%Y%m%dT%H%M%S.%fZ")
            directory = args.results_root.resolve() / f"_pending-{stamp}"
            directory.mkdir(parents=True, exist_ok=False)
            manifest = dict(tool_schema=2, created_utc=stamp, executable=str(exe),
                            created_local=now.astimezone().isoformat(timespec="seconds"),
                            payload_sha256=fingerprint, base_config=asdict(base), runs=[])
            runs = []
            execute_cases(exe, matrix, directory, manifest, runs, args)
            directory = finalize_results_directory(directory, runs, manifest)
        charts = build_charts(runs, base)
        output = args.out.resolve() if args.out else directory / "report.html"
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(render_report("Clustered Geometry settings sweep",
                                       "Offline report; hover for values and toggle legend entries.",
                                       charts, report_tables(runs, manifest), REPORT_NOTES),
                          encoding="utf-8")
        print(f"Report: {output}")
        print(f"Measurements: {directory.resolve()}")
        return 1 if any(r.status == "failed" for r in runs) or not any(
            r.status == "ok" for r in runs) else 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
