#!/usr/bin/env python3
"""Compare verified sweep directories without matching measurements by filename."""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
from pathlib import Path
import sys

from report import Chart, render_report
from sweep import (REPORT_NOTES, RunConfig, RunResult, build_charts, format_metric,
                   load_results, metric, report_tables)


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
            raise ValueError(f"{cfg.slug()}: geometry, camera, or timing semantics differ")
    base = RunConfig(**machines[0].manifest["base_config"])
    merged: dict[tuple, Chart] = {}
    for machine in machines:
        available = [r for r in machine.runs if r.config in selected]
        for chart in build_charts(available, base):
            key = (chart.category, chart.title, chart.unit)
            if key not in merged:
                merged[key] = Chart(chart.title, chart.category, chart.description,
                                    chart.x_label, chart.unit, chart_type=chart.chart_type,
                                    numeric_x=chart.numeric_x, stacked=chart.stacked)
            target = merged[key]
            target.x_tooltips.update(chart.x_tooltips)
            for label, points in chart.series.items():
                name = f"{machine.label} / {label}"
                target.series[name] = points
                target.color_groups[name] = label if chart.stacked else machine.label
                target.stacks[name] = machine.label
    return list(merged.values()), len(shared)


def headline_table(machines: list[MachineReport], base: RunConfig) -> tuple:
    rows = []
    for title, path, unit, higher_better in (
            ("Baseline frame rate", ("frame_perf", "fps"), "FPS", True),
            ("Baseline frame interval", ("frame_perf", "ms_per_frame"), "ms", False),
            ("Baseline resident AS", ("memory_bytes", "total_as"), "bytes", False),
            ("Baseline DispatchRays", ("frame_perf", "pf_render_us"), "us", False),
            ("Baseline timed AS", ("derived", "timed_as_us"), "us", False)):
        values = []
        for machine in machines:
            match = next((r for r in machine.runs if r.config == base and r.status == "ok"), None)
            values.append(metric(match, path) if match else None)
        # The reference is always the first machine, even if its value is absent.
        reference = values[0]
        cells = []
        for index, value in enumerate(values):
            text = format_metric(value, unit)
            if index and value is not None and reference is not None and reference > 0:
                text += f" ({(value / reference - 1) * 100:+.2f}% vs first)"
            cells.append(text)
        rows.append([title + (" (higher is better)" if higher_better else " (lower is better)"),
                     *cells])
    return ("Baseline measurements; percentage change, not an automatic winner",
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
            label = args.label[index] if index < len(args.label) else (
                f"{adapter['description']} [{adapter.get('driver_version', 'unknown driver')}]")
            original = label
            suffix = 2
            while label in labels:
                label = f"{original} ({suffix})"
                suffix += 1
            labels.add(label)
            machines.append(MachineReport(label, directory, runs, manifest))
        charts, shared = comparison_charts(machines, args.partial)
        base = RunConfig(**machines[0].manifest["base_config"])
        tables = [headline_table(machines, base)]
        for machine in machines:
            for title, columns, rows in report_tables(machine.runs, machine.manifest):
                tables.append((f"{machine.label}: {title}", columns, rows))
        notes = REPORT_NOTES + [
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
                                         "Matched workloads; local/private machine provenance.",
                                         charts, tables, notes), encoding="utf-8")
        print(f"Report: {args.out.resolve()}")
        return 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
