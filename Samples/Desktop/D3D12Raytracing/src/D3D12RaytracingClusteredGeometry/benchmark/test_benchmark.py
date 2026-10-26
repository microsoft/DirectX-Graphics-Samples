"""Synthetic-only regression tests. No real machine identities or measurements."""

from __future__ import annotations

from dataclasses import asdict, replace
from html.parser import HTMLParser
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import compare
from report import Chart, render_report, script_json
import sweep
from sweep import (AS_COMPONENTS, STAGES, RunConfig, RunResult, TimingNotReady,
                   UnsupportedConfig, build_charts, build_sweep_matrix, load_results,
                   metric, run_one, validate_snapshot, write_json)


def snapshot(cfg: RunConfig = RunConfig()) -> dict:
    memory = {key: 0 for keys in AS_COMPONENTS.values() for key in keys}
    for key in AS_COMPONENTS[cfg.geometry_mode]:
        memory[key] = 100
    memory.update(static_clas_actual=80, animated_pf_clas_actual=80, static_cluster_input=500)
    memory["total_as"] = sum(memory[key] for key in AS_COMPONENTS[cfg.geometry_mode])
    clones = cfg.extra_instances // 5
    triangles = 100 if cfg.animated_detail == "full" else 50
    mesh = 1000 + cfg.extra_instances * 5
    perf = dict(fps=100.0, ms_per_frame=10.0, frames_in_avg_window=100,
                first_sample_frame=10, last_sample_frame=109, avg_window_target_seconds=1.0,
                measured_window_seconds=1.0, publication_interval_seconds=0.25,
                averaging=False, pf_timing_valid=True)
    for key, value in zip(STAGES, (10, 20, 30, 0, 0, 1000)):
        perf[key] = perf[key + "_snap"] = value
    return dict(
        schema_version=1,
        adapter=dict(description="Synthetic adapter", vendor_id=1, device_id=2,
                     driver_version="test-driver", clusters_supported=True,
                     dedicated_vram_bytes=1024 ** 3),
        config={**cfg.expected_config(), "bounce_slider": 3, "reflection_bounces": 3,
                "refraction_bounces": 5, "cluster_tint": 0.5, "tearing_supported": True},
        memory_bytes=memory, build_times_ms=dict(static_clas=1, static_blas=2, tlas=3, total=6),
        frame_perf=perf, scene=dict(total_clusters=10 + cfg.extra_instances,
                                   mesh_input_triangles=mesh,
                                   instanced_triangles=mesh + clones * triangles,
                                   base_objects=4, animated_clusters=10,
                                   animated_triangles=triangles,
                                   animated_instances=clones + 1, anim_clones_pooled=clones),
        bench=dict(frames_rendered=200, actual_seconds=2.0, requested_seconds=2.0))


def result(cfg: RunConfig = RunConfig(), directory: Path = Path(".")) -> RunResult:
    data = snapshot(cfg)
    data["_sweep"] = dict(requested_config=asdict(cfg), status="ok", wall_seconds=3.5,
                          payload_sha256="synthetic",
                          camera_paused=True, camera_seconds=0.0, animation_paused=False)
    return RunResult(cfg, directory / f"{cfg.slug()}.json", data, 3.5)


def save_results(directory: Path, runs: list[RunResult]) -> dict:
    manifest = dict(tool_schema=2, base_config=asdict(RunConfig()),
                    executable="synthetic/sample.exe", payload_sha256="synthetic",
                    runs=[dict(file=r.json_path.name, config=asdict(r.config)) for r in runs])
    for run in runs:
        write_json(directory / run.json_path.name, run.data)
    write_json(directory / "manifest.json", manifest)
    return manifest


class Measurements(unittest.TestCase):
    def test_invalid_configuration_values_fail(self):
        for change in (dict(extra_instances=7), dict(rays_per_pixel=True),
                       dict(position_truncate_bits=23), dict(geometry_mode="unknown")):
            with self.subTest(change=change), self.assertRaises(ValueError):
                replace(RunConfig(), **change)

    def test_all_matrix_configs_are_unique_and_explicit(self):
        for quick in (True, False):
            matrix = build_sweep_matrix(quick, True)
            self.assertEqual(len(matrix), len(set(matrix)))
            self.assertEqual(len(matrix), len({c.slug() for c in matrix}))
            for cfg in matrix:
                cli = cfg.to_cli()
                for flag in ("--extra-instances", "--animated-detail", "--vsync",
                             "--rays-per-pixel", "--position-truncate", "--compressed-bits",
                             "--camera-paused", "--camera-seconds"):
                    self.assertIn(flag, cli)
                self.assertEqual(cli[cli.index("--vsync") + 1], "off")
                validate_snapshot(snapshot(cfg), cfg)
            if quick:
                self.assertNotIn(10000, {c.extra_instances for c in matrix})

    def test_requested_mode_cannot_silently_fall_back(self):
        data = snapshot()
        data["config"]["geometry_mode"] = "traditional"
        with self.assertRaisesRegex(ValueError, "geometry_mode"):
            validate_snapshot(data, RunConfig())
        data["adapter"]["clusters_supported"] = False
        with self.assertRaises(UnsupportedConfig):
            validate_snapshot(data, RunConfig())

    def test_clamped_or_ignored_settings_fail(self):
        for key, value in (("animated_detail", "low"), ("extra_instances", 100),
                           ("aa_samples_per_pixel", 4), ("vsync", True)):
            with self.subTest(key=key):
                data = snapshot()
                data["config"][key] = value
                with self.assertRaisesRegex(ValueError, "requested"):
                    validate_snapshot(data, RunConfig())

    def test_no_legacy_ema_or_unsettled_timing_fallback(self):
        data = snapshot()
        del data["frame_perf"]["avg_window_target_seconds"]
        with self.assertRaisesRegex(ValueError, "legacy"):
            validate_snapshot(data, RunConfig())
        for key, value in (("pf_timing_valid", False), ("averaging", True)):
            data = snapshot()
            data["frame_perf"][key] = value
            with self.assertRaises(TimingNotReady):
                validate_snapshot(data, RunConfig())

    def test_sub_one_and_high_fps_are_not_clamped(self):
        for fps, samples, coverage in ((0.05, 1, 20.0), (1000.0, 1000, 1.0)):
            data = snapshot()
            data["frame_perf"].update(fps=fps, ms_per_frame=1000 / fps,
                                      frames_in_avg_window=samples,
                                      last_sample_frame=10 + samples - 1,
                                      measured_window_seconds=coverage)
            if fps == 1000:
                data["frame_perf"]["pf_render_us"] = data["frame_perf"]["pf_render_us_snap"] = 500
            validate_snapshot(data, RunConfig())

    def test_bad_numeric_data_is_rejected(self):
        for value in (float("nan"), float("inf"), -1, None, True, "100"):
            data = snapshot()
            data["frame_perf"]["fps"] = value
            with self.subTest(value=value), self.assertRaises(ValueError):
                validate_snapshot(data, RunConfig())

    def test_caps_memory_errors_and_mismatched_windows_fail(self):
        cfg = RunConfig(extra_instances=10000)
        for group, key, value in (
                ("scene", "anim_clones_pooled", 200),
                ("memory_bytes", "total_as", 1),
                ("frame_perf", "last_sample_frame", 108),
                ("frame_perf", "measured_window_seconds", 0.5),
                ("frame_perf", "pf_render_us", 20000)):
            data = snapshot(cfg)
            data[group][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                validate_snapshot(data, cfg)

    def test_chart_totals_and_missing_values(self):
        good = result()
        failed = result(replace(RunConfig(), extra_instances=100))
        failed.data["_error"] = "synthetic failure"
        failed.data["_sweep"]["status"] = "failed"
        charts = build_charts([good, failed])
        scaling = next(c for c in charts if c.title == "Resident AS storage")
        self.assertEqual(scaling.series["clustered"][-1], (100, None))
        storage = next(c for c in charts if c.title == "Resident allocation components")
        self.assertEqual(sum(v for _, v in storage.series["clustered"]),
                         good.data["memory_bytes"]["total_as"])
        budget = next(c for c in charts if c.title == "Frame interval breakdown: clustered")
        self.assertEqual(sum(points[0][1] for points in budget.series.values()), 10000)
        self.assertIsNone(metric(failed, ("frame_perf", "fps")))

    def test_precision_charts_increase_precision_to_the_right(self):
        runs = [result(replace(RunConfig(), position_truncate_bits=n)) for n in (0, 12, 20)]
        chart = next(c for c in build_charts(runs)
                     if c.category == "FLOAT32_3 precision")
        self.assertEqual([x for x, _ in chart.series["clustered"]], [3, 11, 23])

    def test_all_failed_measurements_do_not_make_empty_charts(self):
        run = result()
        run.data["_sweep"]["status"] = "failed"
        run.data["_error"] = "synthetic failure"
        self.assertEqual(build_charts([run]), [])


class Execution(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.exe = self.root / "sample.exe"
        self.exe.write_bytes(b"synthetic executable fixture, never executed")
        self.output = self.root / "results"

    def process(self, command, **kwargs):
        write_json(Path(command[-1]), snapshot())
        return subprocess.CompletedProcess(command, 0)

    def test_cwd_logs_and_provenance_and_no_repeated_success(self):
        with patch("sweep.subprocess.run", side_effect=self.process) as process:
            run = run_one(self.exe, RunConfig(), self.output, 2, 1)
        self.assertEqual(run.status, "ok")
        self.assertEqual(process.call_count, 1)
        self.assertEqual(process.call_args.kwargs["cwd"], self.exe.parent.resolve())
        self.assertIn("SAMPLE_LOG", process.call_args.kwargs["env"])
        self.assertTrue(run.data["_sweep"]["payload_sha256"])
        self.assertTrue(run.json_path.is_file())

    def test_only_unsettled_case_retries_and_attempts_are_preserved(self):
        calls = []
        def process(command, **kwargs):
            data = snapshot()
            if not calls:
                data["frame_perf"]["averaging"] = True
            calls.append(command)
            write_json(Path(command[-1]), data)
            return subprocess.CompletedProcess(command, 0)
        with patch("sweep.subprocess.run", side_effect=process):
            run = run_one(self.exe, RunConfig(), self.output, 2, 1, max_bench_seconds=10)
        self.assertEqual(run.status, "ok")
        self.assertEqual(len(calls), 2)
        self.assertEqual(len(list(self.output.glob("*.attempt-*.json"))), 2)
        self.assertGreater(float(calls[1][calls[1].index("--bench-seconds") + 1]), 2)

    def test_timeout_failure_and_missing_json_are_persisted(self):
        for index, behavior in enumerate((
                subprocess.TimeoutExpired(["synthetic"], 1),
                subprocess.CompletedProcess(["synthetic"], 9),
                subprocess.CompletedProcess(["synthetic"], 0))):
            with self.subTest(index=index), patch("sweep.subprocess.run", side_effect=(
                    behavior if isinstance(behavior, Exception) else None),
                    return_value=behavior):
                run = run_one(self.exe, RunConfig(), self.root / str(index), 2, 1)
            self.assertEqual(run.status, "failed")
            self.assertIn("_error", sweep.read_json(run.json_path))

    def test_load_uses_manifest_not_attempt_or_filename_guessing(self):
        run = result(directory=self.root)
        manifest = save_results(self.root, [run])
        run.json_path.rename(self.root / "renamed-measurement.json")
        manifest["runs"][0]["file"] = "renamed-measurement.json"
        write_json(self.root / "manifest.json", manifest)
        write_json(self.root / "unrelated.json", {"not": "a run"})
        loaded, _ = load_results(self.root)
        self.assertEqual(len(loaded), 1)
        self.assertEqual(loaded[0].wall_seconds, 3.5)
        self.assertEqual(loaded[0].config, run.config)

    def test_report_only_does_not_require_executable(self):
        save_results(self.root, [result(directory=self.root)])
        code = sweep.main(["--input-dir", str(self.root), "--exe", str(self.root / "absent.exe")])
        self.assertEqual(code, 0)
        self.assertTrue((self.root / "report.html").is_file())

    def test_mixed_payload_results_and_stale_attempts_fail(self):
        run = result(directory=self.root)
        manifest = save_results(self.root, [run])
        manifest["payload_sha256"] = "different-synthetic-build"
        write_json(self.root / "manifest.json", manifest)
        with self.assertRaisesRegex(ValueError, "fingerprint"):
            load_results(self.root)
        self.output.mkdir()
        stale = self.output / f"{RunConfig().slug()}.attempt-1.json"
        write_json(stale, snapshot())
        with patch("sweep.subprocess.run") as process, self.assertRaisesRegex(ValueError, "stale"):
            run_one(self.exe, RunConfig(), self.output, 2, 1)
        process.assert_not_called()

    def test_changed_payload_is_not_mislabeled_with_old_fingerprint(self):
        with patch("sweep.subprocess.run") as process, self.assertRaisesRegex(ValueError, "payload"):
            run_one(self.exe, RunConfig(), self.output, 2, 1, fingerprint="not-the-fixture")
        process.assert_not_called()


class Comparison(unittest.TestCase):
    def machine(self, label, runs):
        return compare.MachineReport(label, Path("synthetic"),
                                     runs, {"base_config": asdict(RunConfig())})

    def test_intersection_default_and_partial_gaps(self):
        base = result()
        expanded = result(replace(RunConfig(), extra_instances=100))
        machines = [self.machine("A", [base, expanded]), self.machine("B", [result()])]
        charts, shared = compare.comparison_charts(machines)
        self.assertEqual(shared, 1)
        chart = next(c for c in charts if c.title == "Resident AS storage")
        self.assertEqual(chart.series["A / clustered"], [(0, 700)])
        charts, _ = compare.comparison_charts(machines, partial=True)
        chart = next(c for c in charts if c.title == "Resident AS storage")
        self.assertEqual([x for x, _ in chart.series["A / clustered"]], [0, 100])
        self.assertEqual([x for x, _ in chart.series["B / clustered"]], [0])

    def test_matching_flags_but_different_geometry_is_rejected(self):
        other = result()
        other.data["scene"]["mesh_input_triangles"] += 10
        other.data["scene"]["instanced_triangles"] += 10
        validate_snapshot(other.data, other.config)
        with self.assertRaisesRegex(ValueError, "differ"):
            compare.comparison_charts([self.machine("A", [result()]),
                                       self.machine("B", [other])])

    def test_first_machine_remains_reference_when_missing(self):
        machines = [self.machine("A", []), self.machine("B", [result()])]
        _, _, rows = compare.headline_table(machines, RunConfig())
        self.assertEqual(rows[0][1], "not measured")
        self.assertNotIn("vs first", rows[0][2])

    def test_frame_budget_stacks_do_not_add_machines_together(self):
        charts, _ = compare.comparison_charts([
            self.machine("A", [result()]), self.machine("B", [result()])])
        chart = next(c for c in charts if c.stacked)
        self.assertEqual(set(chart.stacks.values()), {"A", "B"})


class Report(unittest.TestCase):
    def test_escaping_offline_theme_zero_axes_and_tooltip_keys(self):
        attack = "</script><script>synthetic_bad()</script>"
        chart = Chart(attack, "synthetic category", attack, "extra BLASes", "us",
                      {attack: [(0, 0), (100, None), (1000, 20)]})
        report = render_report(attack, attack, [chart], [(attack, ["column"], [[attack]])], [attack])
        self.assertNotIn(attack, report)
        self.assertIn("\\u003c/script", script_json(attack))
        self.assertIn("beginAtZero: true", report)
        self.assertIn("stack: spec.stacked ? series.stack : undefined", report)
        self.assertIn("Permission is hereby granted", report)
        self.assertIn("item.raw.x", report.replace("items[0].raw.x", "item.raw.x"))
        self.assertIn("scoutTheme", report)
        self.assertIn("--cp-bg", report)
        self.assertIn("[100, null]", report)
        class Scripts(HTMLParser):
            external = []
            def handle_starttag(self, tag, attrs):
                if tag == "script" and "src" in dict(attrs):
                    self.external.append(dict(attrs)["src"])
        parser = Scripts()
        parser.feed(report)
        self.assertEqual(parser.external, [])


if __name__ == "__main__":
    unittest.main()
