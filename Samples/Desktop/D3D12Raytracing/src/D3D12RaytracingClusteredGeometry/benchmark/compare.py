#!/usr/bin/env python3
"""Compare verified sweep directories without matching measurements by filename."""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass, replace
from pathlib import Path
import re
import sys

from report import Chart, render_report
from sweep import (REPORT_NOTES, AS_WORK_LABEL, METRIC_LABELS, SETTING_LABELS, BENCHMARK_ENVIRONMENT,
                   RunConfig, RunResult, build_charts, format_metric,
                   load_results, metric, report_tables, setting_value, metric_unit)


@dataclass
class MachineReport:
    label: str
    directory: Path
    runs: list[RunResult]
    manifest: dict


def workload_signature(run: RunResult) -> tuple:
    """Hardware may differ, but selected geometry and measurement semantics may not."""
    scene = run.data["scene"]
    config = run.data["config"]
    perf = run.data["frame_perf"]
    provenance = run.data["_sweep"]
    return (
        tuple(scene[key] for key in ("total_clusters", "mesh_input_triangles",
                                    "instanced_triangles", "base_objects", "animated_clusters",
                                    "animated_triangles", "animated_instances", "anim_clones_pooled")),
        tuple(config[key] for key in ("bounce_slider", "reflection_bounces", "refraction_bounces",
                                      "cluster_tint", "vsync", "wireframe")),
        tuple(perf[key] for key in ("avg_window_target_seconds", "publication_interval_seconds")),
        tuple(provenance[key] for key in ("camera_paused", "camera_seconds", "animation_paused")),
        tuple(config.get(key) for key in BENCHMARK_ENVIRONMENT),
    )


def comparison_charts(machines: list[MachineReport], partial: bool = False) -> tuple[list[Chart], int]:
    identities = [{r.config for r in m.runs if r.status == "ok"} for m in machines]
    shared = set.intersection(*identities)
    selected = set.union(*identities) if partial else shared
    if not selected:
        raise ValueError("no successful, matching configurations across the input sweeps")
    for cfg in selected:
        signatures = {workload_signature(r) for m in machines for r in m.runs
                      if r.config == cfg and r.status == "ok"}
        if len(signatures) != 1:
            raise ValueError(f"{cfg.slug()}: geometry, camera, timing, or render/input conditions differ")
    base = RunConfig(**machines[0].manifest["base_config"])
    merged: dict[tuple, Chart] = {}
    split_categories = {"Scene scaling", "Build preference", "Animated detail", "AS memory breakdown"}
    for machine_index, machine in enumerate(machines):
        available = [r for r in machine.runs if r.config in selected]
        for chart in build_charts(available, base):
            for label, points in chart.series.items():
                # Facet algorithms instead of cramming GPU x mode into nine lines.
                split = chart.category in split_categories
                title = f"{chart.title}: {label}" if split else chart.title
                category = f"{chart.category} / {label}" if split else chart.category
                key = (category, title, chart.unit)
                if key not in merged:
                    merged[key] = Chart(title, category, chart.description,
                                        chart.x_label, chart.unit, chart_type=chart.chart_type,
                                        numeric_x=chart.numeric_x, stacked=chart.stacked)
                target = merged[key]
                target.x_tooltips.update(chart.x_tooltips)
                name = machine.label if split or len(chart.series) == 1 else f"{machine.label} / {label}"
                target.series[name] = points
                target.color_groups[name] = label if chart.stacked else machine.label
                target.stacks[name] = machine.label
                target.point_radii[name] = max(3, 7 - (machine_index % 3) * 2)
                if label.startswith("COMPRESSED1"):
                    target.line_dashes[name] = [6, 3]
                    target.point_styles[name] = "triangle"
                elif label.startswith("FLOAT32_3"):
                    target.line_dashes[name] = []
                    target.point_styles[name] = "circle"
                else:
                    target.line_dashes[name] = ([], [8, 3], [2, 3])[machine_index % 3]
                    target.point_styles[name] = ("circle", "rectRot", "triangle")[machine_index % 3]
    # Keep faceted categories contiguous when series are added machine by machine.
    categories = list(dict.fromkeys(chart.category for chart in merged.values()))
    charts = sorted(merged.values(), key=lambda chart: categories.index(chart.category))
    return ratio_charts(machines, selected) + charts, len(shared)


def ratio_charts(machines: list[MachineReport], selected: set[RunConfig]) -> list[Chart]:
    charts = []
    base = RunConfig(**machines[0].manifest["base_config"])
    for strategy in ("rebuild", "refit"):
        for title, path, numerator_cluster in (
                ("Frame-rate ratio: clustered / traditional", ("frame_perf", "fps"), True),
                ("AS memory ratio: traditional / clustered", ("memory_bytes", "total_as"), False)):
            chart = Chart(f"{title} ({strategy})", "Within-GPU ratios",
                          "Ratio of matched settings on the same GPU. Above 1 favors clustered. "
                          "Frame-rate ratios include rendering and all frame work; resident AS "
                          "excludes scratch and inputs. These are single-run observations, "
                          "not image-quality equivalence or confidence intervals.",
                          SETTING_LABELS["extra_instances"], "x")
            for machine_index, machine in enumerate(machines):
                by_config = {run.config: run for run in machine.runs if run.status == "ok"}
                points = []
                for count in (0, 100, 1000, 10000):
                    cluster = replace(base, extra_instances=count, geometry_mode="clusters",
                                      trad_anim_mode="rebuild")
                    traditional = replace(cluster, geometry_mode="traditional",
                                          trad_anim_mode=strategy)
                    if cluster not in selected or traditional not in selected:
                        continue
                    c, t = by_config.get(cluster), by_config.get(traditional)
                    cv, tv = metric(c, path) if c else None, metric(t, path) if t else None
                    num, denom = (cv, tv) if numerator_cluster else (tv, cv)
                    value = num / denom if num is not None and denom is not None and denom > 0 else None
                    points.append((count, value))
                if points:
                    chart.series[machine.label] = points
                    chart.color_groups[machine.label] = machine.label
                    chart.line_dashes[machine.label] = ([], [8, 3], [2, 3])[machine_index % 3]
                    chart.point_styles[machine.label] = ("circle", "rectRot", "triangle")[machine_index % 3]
                    chart.point_radii[machine.label] = max(3, 7 - (machine_index % 3) * 2)
            if any(value is not None for points in chart.series.values() for _, value in points):
                charts.append(chart)
    return charts


def comparison_warnings(machines: list[MachineReport]) -> list[str]:
    warnings = []
    fingerprints = {m.manifest.get("payload_sha256") for m in machines}
    if len(fingerprints) > 1:
        warnings.append("BUILD CAUTION: the executable/runtime fingerprints differ between inputs. "
                        "Matching flags and geometry do not prove identical source, optimization, or "
                        "runtime versions; verify the builds before making hardware performance claims.")
    if any(m.manifest.get("payload_sha256") is None for m in machines):
        warnings.append("BUILD CAUTION: at least one input lacks an executable/runtime fingerprint.")
    if any(machine.manifest["base_config"] != machines[0].manifest["base_config"]
           for machine in machines[1:]):
        warnings.append("BASELINE CAUTION: sweep baselines differ. Charts and ratio summaries are "
                        "pinned to the first input's baseline and use only exact matching cases, "
                        "never the other machines' different defaults.")
    for machine in machines:
        valid = [r for r in machine.runs if r.status == "ok"]
        sparse = [r for r in valid if r.data["frame_perf"]["frames_in_avg_window"] < 10]
        if sparse:
            fewest = min(r.data["frame_perf"]["frames_in_avg_window"] for r in sparse)
            warnings.append(f"SAMPLE CAUTION: {machine.label} has {len(sparse)} cases with fewer "
                            f"than 10 frames in the published window (minimum {fewest}). "
                            "A settled time window is not a statistical-confidence guarantee.")
        if any(not all(key in run.data["config"] for key in BENCHMARK_ENVIRONMENT)
               for run in valid):
            warnings.append(f"LEGACY INPUT: {machine.label} does not record verified render size, "
                            "input isolation, or validation state. Treat it as unverified for "
                            "controlled comparison; collect a fresh sweep with the current sample.")
        failed = sum(r.status == "failed" for r in machine.runs)
        skipped = sum(r.status == "skipped" for r in machine.runs)
        if failed or skipped:
            warnings.append(f"COVERAGE: {machine.label}: {failed} failed and {skipped} skipped cases. "
                            "Excluded cases must not be interpreted as zero cost.")
    warnings += [
        "REPRODUCIBILITY: current samples verify a fixed 1280x720 physical-pixel render size, "
        "blocked user input, and disabled validation. Legacy inputs may lack this proof. "
        "Power state, clocks, and desktop/presentation conditions still need separate control.",
        "STATISTICS: each point is one final rolling-window mean, not the entire process average "
        "and not a repeated-trial estimate. No variance or error bars can be recovered from it.",
        "VISUALS: non-stacked chart colors identify GPUs; frame-budget stack colors identify "
        "stages. Separate charts identify geometry mode. Precision "
        "overlays use solid circles for FLOAT32_3 and dashed triangles for COMPRESSED1. "
        "Hover at any x value to read every GPU's value, even when curves overlap; click legend "
        "entries to hide/show series. Discrete settings use grouped bars. Ratios are computed "
        "within each GPU, not by mixing measurements from different machines.",
    ]
    return warnings


def summary_table(machines: list[MachineReport]) -> tuple:
    rows = []
    for machine in machines:
        valid = [run for run in machine.runs if run.status == "ok"]
        adapter = valid[0].data["adapter"]
        counts = [run.data["frame_perf"]["frames_in_avg_window"] for run in valid]
        rows.append([machine.label, adapter["description"], adapter.get("driver_version", "unknown"),
                     len(valid), len(machine.runs), min(counts),
                     (f"{valid[0].data['config']['render_width']}x"
                      f"{valid[0].data['config']['render_height']}")
                     if "render_width" in valid[0].data["config"] else "Not recorded",
                     "Locked" if valid[0].data["config"].get("benchmark_input_locked") else "Not verified",
                     machine.manifest.get("payload_sha256", "not recorded")])
    return ("Input coverage and build identity",
            ["GPU label", "Adapter", "Driver version", "Valid configurations",
             "Requested configurations", "Fewest frames averaged", "Render size (pixels)",
             "User input", "Executable/runtime SHA-256"], rows)


def largest_workload_tables(machines: list[MachineReport]) -> list[tuple]:
    """Headline the largest common three-strategy workload, not just the easy baseline."""
    by_machine = [{run.config: run for run in machine.runs if run.status == "ok"}
                  for machine in machines]
    strategies = ("clusters", "rebuild", "refit")
    base = RunConfig(**machines[0].manifest["base_config"])
    def config(count: int, strategy: str) -> RunConfig:
        return replace(base, extra_instances=count,
                       geometry_mode="clusters" if strategy == "clusters" else "traditional",
                       trad_anim_mode="refit" if strategy == "refit" else "rebuild")
    counts = [count for count in (0, 100, 1000, 10000)
              if all(config(count, strategy) in runs
                     for runs in by_machine
                     for strategy in strategies)]
    if not counts:
        return []
    count = max(counts)
    frame_rows, memory_rows = [], []
    for machine, runs in zip(machines, by_machine):
        case = [runs[config(count, strategy)] for strategy in strategies]
        fps = [metric(run, ("frame_perf", "fps")) for run in case]
        memory = [metric(run, ("memory_bytes", "total_as")) for run in case]
        frame_rows.append([machine.label, *(format_metric(value, "FPS") for value in fps),
                           f"{fps[0] / fps[1]:.2f}x", f"{fps[0] / fps[2]:.2f}x",
                           " / ".join(str(run.data["frame_perf"]["frames_in_avg_window"])
                                      for run in case)])
        memory_rows.append([machine.label, *(format_metric(value, "bytes") for value in memory),
                            f"{memory[1] / memory[0]:.2f}x",
                            setting_value("animated_detail", case[0].config.animated_detail),
                            case[0].data["scene"]["instanced_triangles"]])
    return [
        (f"Largest shared workload: {count:,} extra BLASes (single-run frame rate)",
         ["GPU", "Clustered FPS", "Traditional rebuild FPS", "Traditional refit FPS",
          "FPS ratio: clustered / rebuild", "FPS ratio: clustered / refit",
          "Frames averaged (clustered / rebuild / refit)"],
         frame_rows),
        (f"Largest shared workload: {count:,} extra BLASes (resident AS, not total VRAM)",
         ["GPU", "Clustered AS memory", "Traditional rebuild AS memory", "Traditional refit AS memory",
          "AS memory ratio: rebuild / clustered", "Animated mesh detail",
          "Scene triangles (all instances)"], memory_rows),
    ]


def headline_table(machines: list[MachineReport], base: RunConfig) -> tuple:
    rows = []
    for title, path, unit, higher_better in (
            (METRIC_LABELS[("frame_perf", "fps")], ("frame_perf", "fps"), "FPS", True),
            (METRIC_LABELS[("frame_perf", "ms_per_frame")], ("frame_perf", "ms_per_frame"), "ms", False),
            (METRIC_LABELS[("memory_bytes", "total_as")], ("memory_bytes", "total_as"), "bytes", False),
            (METRIC_LABELS[("frame_perf", "pf_render_us")], ("frame_perf", "pf_render_us"), "us", False),
            (AS_WORK_LABEL, ("derived", "timed_as_us"), "us", False)):
        values = []
        for machine in machines:
            match = next((r for r in machine.runs if r.config == base and r.status == "ok"), None)
            values.append(metric(match, path) if match else None)
        # The reference is always the first machine, even if its value is absent.
        reference = values[0]
        cells = []
        for index, value in enumerate(values):
            text = format_metric(value, metric_unit(path, unit))
            if index and value is not None and reference is not None and reference > 0:
                text += f" ({(value / reference - 1) * 100:+.2f}% vs first)"
            cells.append(text)
        rows.append([title + (" (higher is better)" if higher_better else " (lower is better)"),
                     *cells])
    return ("Baseline comparison (percentage change uses the first GPU as reference)",
            ["Metric", *[m.label for m in machines]], rows)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dirs", nargs="+", type=Path)
    parser.add_argument("--label", action="append", default=[])
    parser.add_argument("--out", type=Path, default=Path("compare.html"))
    parser.add_argument("--partial", action="store_true",
                        help="include unmatched cases as explicit gaps; default uses common cases")
    args = parser.parse_args(argv)
    if len(args.dirs) < 2:
        parser.error("at least two sweep directories are required")
    if len(args.label) > len(args.dirs):
        parser.error("more labels than input directories")
    try:
        machines = []
        labels = set()
        for index, directory in enumerate(args.dirs):
            runs, manifest = load_results(directory)
            valid = [r for r in runs if r.status == "ok"]
            if not valid:
                raise ValueError(f"{directory}: no successful measurements")
            adapter = valid[0].data["adapter"]
            gpu = re.sub(r"^(NVIDIA GeForce|AMD Radeon)\s+", "", adapter["description"])
            label = args.label[index] if index < len(args.label) else (
                f"{gpu} [{adapter.get('driver_version', 'unknown driver')}]")
            original = label
            suffix = 2
            while label in labels:
                label = f"{original} ({suffix})"
                suffix += 1
            labels.add(label)
            machines.append(MachineReport(label, directory, runs, manifest))
        charts, shared = comparison_charts(machines, args.partial)
        base = RunConfig(**machines[0].manifest["base_config"])
        tables = []
        for machine in machines:
            for title, columns, rows in report_tables(machine.runs, machine.manifest):
                tables.append((f"{machine.label}: {title}", columns, rows))
        notes = comparison_warnings(machines) + REPORT_NOTES + [
            f"{shared} successful configurations are present on every input. "
            + ("Partial mode includes unmatched settings as gaps."
               if args.partial else "Only common successful settings enter the charts."),
            "Configs are matched by all pinned settings and checked against actual geometry counts, "
            "camera settings, and timing-window semantics. Different executable hashes are disclosed, "
            "not assumed equivalent. Use matching optimized builds for hardware comparisons.",
            "Percentage changes always use the first input as the reference. "
            "An absent reference stays absent; another machine is never silently substituted.",
        ]
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(render_report("Clustered Geometry cross-machine comparison",
                                         "Matched workloads across adapters and drivers.",
                                         charts, tables, notes,
                                         overview_tables=[summary_table(machines),
                                                          *largest_workload_tables(machines),
                                                          headline_table(machines, base)]),
                            encoding="utf-8")
        print(f"Report: {args.out.resolve()}")
        return 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
