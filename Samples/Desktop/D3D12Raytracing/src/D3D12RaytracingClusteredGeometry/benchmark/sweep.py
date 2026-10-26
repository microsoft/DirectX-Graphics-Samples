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
import subprocess
import sys
import time
from dataclasses import asdict, dataclass, replace
from typing import Any

from report import Chart, render_report

HERE = Path(__file__).resolve().parent
DEFAULT_EXE = HERE.parent / "bin" / "x64" / "Release" / "D3D12RaytracingClusteredGeometry.exe"
DEFAULT_RESULTS_ROOT = HERE / "results"
NOMINAL_PRECISION_BITS = tuple(range(2, 17, 2))
STAGES = ("pf_instantiate_us", "pf_blas_us", "pf_tlas_us",
          "pf_static_blas_us", "pf_static_clas_us", "pf_render_us")
AS_COMPONENTS = {
    "clusters": ("static_clas_alloc", "static_blas_total", "animated_template",
                 "animated_pf_clas_alloc", "animated_blas", "anim_clones_blas_pool", "tlas"),
    "traditional": ("traditional_blas_actual", "animated_trad_blas",
                    "anim_clones_trad_blas_pool", "tlas"),
}
FORMAT_METRICS = [
    ("Resident AS", "bytes", ("memory_bytes", "total_as")),
    ("Static CLAS payload", "bytes", ("memory_bytes", "static_clas_actual")),
    ("Static source input", "bytes", ("memory_bytes", "static_cluster_input")),
    ("Animated template", "bytes", ("memory_bytes", "animated_template")),
    ("Instantiated CLAS payload", "bytes", ("memory_bytes", "animated_pf_clas_actual")),
    ("Instantiate", "us", ("frame_perf", "pf_instantiate_us")),
    ("Timed AS", "us", ("derived", "timed_as_us")),
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


def validate_snapshot(data: dict, config: RunConfig) -> None:
    if data.get("schema_version") != 1:
        raise ValueError("unsupported sample JSON schema")
    adapter = data.get("adapter", {})
    if not isinstance(adapter, dict) or not isinstance(adapter.get("description"), str):
        raise ValueError("missing adapter identity")
    if config.geometry_mode == "clusters" and adapter.get("clusters_supported") is False:
        raise UnsupportedConfig("cluster support unavailable; the sample fell back to traditional")
    actual = data.get("config", {})
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
    runs += [replace(base, geometry_mode=mode, build_flags=value)
             for mode in ("clusters", "traditional") for value in ("none", "fast-build")]
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
            validate_snapshot(data, cfg)
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
             references: dict[str, RunConfig], numeric_x: bool = False) -> None:
        x_label = ("PositionTruncateBitCount"
                   if x_field == "position_truncate_bits" else x_field.replace("_", " "))
        chart = Chart(title, category, description, x_label, unit,
                      numeric_x=numeric_x)
        def x_value(run: RunResult) -> Any:
            return getattr(run.config, x_field)
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
            selected.sort(key=lambda r: category_order.index(x_value(r))
                          if category_order else x_value(r))
            if selected:
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
         "GPU-timestamp static setup time, not CPU initialization or total process startup."),
        ("Frame rate", "FPS", ("frame_perf", "fps"),
         "Completed-frame cadence from the same rolling sample window as every GPU stage."),
        ("Frame interval", "ms", ("frame_perf", "ms_per_frame"),
         "Includes GPU work and queue idle time; not just DispatchRays or GPU busy time."),
        ("Timed AS stages", "us", ("derived", "timed_as_us"),
         "Sum of the instrumented AS stages. Clustered deformation compute is not timed by the instantiate pair."),
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
        chart = Chart(f"Frame interval breakdown: {label}", "Frame budget",
                      "Same-window timed AS + DispatchRays + residual equals the frame interval. "
                      "The residual is uninstrumented work and queue idle, not a CPU-time measurement.",
                      "extra BLASes", "us", chart_type="bar", stacked=True)
        selected = sorted((r for r in runs if matches(r, reference, {"extra_instances"})),
                          key=lambda r: r.config.extra_instances)
        for name, path in (
                ("Timed AS", ("derived", "timed_as_us")),
                ("DispatchRays", ("frame_perf", "pf_render_us")),
                ("Other work / queue idle", ("derived", "unattributed_us"))):
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
          ("Timed AS", "us", ("derived", "timed_as_us")),
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
         [("Timed AS", "us", ("derived", "timed_as_us")),
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
        chart = Chart(f"COMPRESSED1 vs FLOAT32_3: {title}", "COMPRESSED1 vs FLOAT32_3 precision",
                      "Overlaid curves across aligned nominal precision bits. FLOAT32_3 uses "
                      "24-PositionTruncateBitCount significant bits for normal values "
                      "(23-N explicit fraction bits plus the implicit leading 1). COMPRESSED1 "
                      "uses offset bits/component with a cluster-local anchor and scale. "
                      "This is an approximate bit-count alignment, not an equal-error comparison. "
                      "Hover a point for the actual controls. Missing runs are gaps; use --extend.",
                      "Nominal precision bits (not matched error)", unit, numeric_x=True)
        chart.series = {"FLOAT32_3 (normal significand)": [],
                        "COMPRESSED1 (cluster offset)": []}
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
        if any(value is not None for points in chart.series.values() for _, value in points):
            charts.append(chart)
    for category, field_name, references, metrics in studies:
        if len({getattr(r.config, field_name) for r in runs
                if any(matches(r, ref, {field_name}) for ref in references.values())}) < 2:
            continue
        for title, unit, path in metrics:
            description = ("Only the named setting varies; other settings are pinned to the benchmark "
                           "baseline. Precision changes can alter geometry, so faster is not proof "
                           "of equal visual quality.")
            if field_name == "position_truncate_bits":
                title = f"{title} vs PositionTruncateBitCount"
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
                 numeric_x=field_name in {"position_truncate_bits", "compressed_bits", "rays_per_pixel"})
    storage = Chart("Resident allocation components", "Resident storage",
                    "All bars are allocated resident BVH storage, not payload estimates. "
                    "Components sum exactly to total_as; scratch and source inputs are excluded.",
                    "allocation", "bytes", chart_type="bar")
    for label, reference in modes.items():
        matching = [r for r in runs if r.config == reference]
        if matching:
            run = matching[0]
            storage.series[label] = [(key, metric(run, ("memory_bytes", key)))
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
    facts = [[key, value] for key, value in adapter.items()]
    if successful:
        facts += [["Tearing supported", successful[0].data["config"].get("tearing_supported", "unknown")]]
    facts += [["Executable", manifest.get("executable", "unknown")],
              ["Executable + runtime SHA-256", manifest.get("payload_sha256", "unknown")],
              ["Generated UTC", manifest.get("created_utc", "unknown")],
              ["Benchmark baseline", json.dumps(manifest.get("base_config", {}), sort_keys=True)]]
    rows = []
    for run in runs:
        perf = run.data.get("frame_perf", {}) if run.status == "ok" else {}
        rows.append([run.config.slug(), run.status, run.config.geometry_mode,
                     run.config.trad_anim_mode, run.config.vertex_format,
                     run.config.position_truncate_bits if run.config.vertex_format == "float" else "-",
                     23 - run.config.position_truncate_bits if run.config.vertex_format == "float" else "-",
                     run.config.compressed_bits if run.config.vertex_format == "compressed" else "-",
                     run.config.extra_instances, run.config.animated_detail,
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
        comparison_rows.append([title, *(format_metric(value, unit) for value in values), delta])
    by_config = {run.config: run for run in runs}
    precision_rows = []
    for bits in NOMINAL_PRECISION_BITS:
        configs = precision_pair_configs(base, bits)
        states = [by_config[cfg].status if cfg in by_config else "missing (use --extend)"
                  for cfg in configs]
        precision_rows.append([bits, 24 - bits, bits - 1, bits, *states])
    return [
        ("Run provenance (local/private)", ["Property", "Value"], facts),
        ("Precision alignment and coverage (nominal bits, not equal error)",
         ["Nominal bits", "FLOAT32_3 PositionTruncateBitCount", "FLOAT32_3 explicit mantissa bits",
          "COMPRESSED1 offset bits/component", "FLOAT32_3 run", "COMPRESSED1 run"],
         precision_rows),
        ("Direct format comparison (matched benchmark baseline)",
         ["Metric", *labels, "COMPRESSED1 change vs FLOAT32_3"], comparison_rows),
        ("All requested cases", ["JSON id", "Status", "Geometry", "Trad animation", "Vertex",
                                "PositionTruncateBitCount", "FLOAT32_3 mantissa bits kept",
                                "COMPRESSED1 offset bits/component",
                                "Extra BLASes", "Detail", "Rays/pixel", "FPS", "Frame interval",
                                "Resident AS", "Samples", "Window seconds", "Frame range",
                                "Process wall seconds", "Error / skip reason"], rows),
    ]


REPORT_NOTES = [
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
    "Reports, logs, commands, and raw JSON contain machine-specific information. Keep them private; "
    "review any aggregate findings before publishing.",
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
                manifests = list(args.results_root.glob("*/manifest.json"))
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
            stamp = dt.datetime.now(dt.timezone.utc).strftime("%Y%m%dT%H%M%S.%fZ")
            directory = args.results_root.resolve() / stamp
            directory.mkdir(parents=True, exist_ok=False)
            manifest = dict(tool_schema=2, created_utc=stamp, executable=str(exe),
                            payload_sha256=fingerprint, base_config=asdict(base), runs=[])
            runs = []
            execute_cases(exe, matrix, directory, manifest, runs, args)
        charts = build_charts(runs, base)
        output = args.out.resolve() if args.out else directory / "report.html"
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(render_report("Clustered Geometry settings sweep",
                                       "Offline report; hover for values and toggle legend entries.",
                                       charts, report_tables(runs, manifest), REPORT_NOTES),
                          encoding="utf-8")
        print(f"Report: {output}")
        print(f"Private measurements: {directory.resolve()}")
        return 1 if any(r.status == "failed" for r in runs) or not any(
            r.status == "ok" for r in runs) else 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
