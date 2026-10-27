"""Synthetic-only regression tests. No real machine identities or measurements."""

from __future__ import annotations

from dataclasses import asdict, replace
from html.parser import HTMLParser
import json
import os
from pathlib import Path
import subprocess
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import compare
from report import Chart, render_report, script_json
import sweep
from sweep import (AS_COMPONENTS, STAGES, RunConfig, RunResult, TimingNotReady,
                   UnsupportedConfig, build_charts, build_sweep_matrix, load_results,
                   metric, run_one, validate_snapshot, write_json)

AS_MEMORY = sweep.METRIC_LABELS[("memory_bytes", "total_as")]
FORMAT_MEMORY_TITLE = f"COMPRESSED1 vs FLOAT32_3: {AS_MEMORY}"
FORMAT_RENDER_TITLE = f"COMPRESSED1 vs FLOAT32_3: {sweep.METRIC_LABELS[('frame_perf', 'pf_render_us')]}"
FLOAT_SERIES = "FLOAT32_3 (bits retained, including leading 1)"
COMPRESSED_SERIES = "COMPRESSED1 (offset bits/component)"


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
        config={**cfg.expected_config(), **sweep.BENCHMARK_ENVIRONMENT,
                "bounce_slider": 3, "reflection_bounces": 3,
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
            else:
                for bits in sweep.NOMINAL_PRECISION_BITS:
                    for cfg in sweep.precision_pair_configs(RunConfig(), bits):
                        self.assertIn(cfg, matrix)

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

    def test_new_runs_require_fixed_resolution_input_lock_and_actual_animation_state(self):
        for key, value in (("render_width", 1920), ("render_height", 1080),
                           ("benchmark_input_locked", False), ("benchmark_window_mode", "interactive"),
                           ("camera_paused", False), ("camera_seconds", 1.0),
                           ("animation_paused", True), ("debug_layer_active", True)):
            data = snapshot()
            data["config"][key] = value
            with self.subTest(key=key), self.assertRaises(sweep.BenchmarkContractError):
                validate_snapshot(data, RunConfig(), require_environment=True)
        legacy = snapshot()
        for key in sweep.BENCHMARK_ENVIRONMENT:
            del legacy["config"][key]
        validate_snapshot(legacy, RunConfig())
        with self.assertRaises(sweep.BenchmarkContractError):
            validate_snapshot(legacy, RunConfig(), require_environment=True)

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
        scaling = next(c for c in charts if c.title == AS_MEMORY)
        self.assertEqual(scaling.series["clustered"][-1], (100, None))
        storage = next(c for c in charts if c.title == "AS memory by buffer (resident allocations)")
        self.assertEqual(sum(v for _, v in storage.series["clustered"]),
                         good.data["memory_bytes"]["total_as"])
        budget = next(c for c in charts if c.title == "Frame time breakdown: clustered")
        self.assertEqual(sum(points[0][1] for points in budget.series.values()), 10000)
        self.assertIsNone(metric(failed, ("frame_perf", "fps")))

    def test_float_precision_charts_show_actual_position_truncate_bitcount(self):
        runs = [result(replace(RunConfig(), position_truncate_bits=n)) for n in (0, 12, 20)]
        chart = next(c for c in build_charts(runs)
                     if c.category == "FLOAT32_3 precision")
        self.assertEqual(chart.x_label, "PositionTruncateBitCount (bits removed)")
        self.assertEqual([x for x, _ in chart.series["clustered"]], [0, 12, 20])
        self.assertIn("PositionTruncateBitCount", chart.title)
        self.assertIn("larger values reduce precision", chart.description)
        self.assertIn("--position-truncate", chart.description)
        self.assertIn("11 explicit mantissa bits kept", chart.x_tooltips["12"])
        self.assertIn("23 explicit mantissa bits kept", chart.x_tooltips["0"])

    def test_direct_format_comparison_labels_both_controls_and_uses_matched_cases(self):
        base = RunConfig()
        float_run = result(base)
        compressed = result(replace(base, vertex_format="compressed"))
        unrelated = result(replace(base, vertex_format="compressed", extra_instances=100))
        compressed.data["memory_bytes"]["static_clas_alloc"] = 50
        compressed.data["memory_bytes"]["total_as"] = 650
        more_precision = [result(cfg) for cfg in sweep.precision_pair_configs(base, 8)]
        chart = next(c for c in build_charts([float_run, compressed, unrelated, *more_precision])
                     if c.title == FORMAT_MEMORY_TITLE)
        self.assertEqual(chart.chart_type, "line")
        self.assertTrue(chart.numeric_x)
        self.assertEqual(dict(chart.series[FLOAT_SERIES])[12], 700)
        self.assertEqual(dict(chart.series[COMPRESSED_SERIES])[12], 650)
        self.assertIn("not an equal-error comparison", chart.description)
        self.assertIn("PositionTruncateBitCount=12", chart.x_tooltips["12"])
        tables = sweep.report_tables([float_run, compressed], {"base_config": asdict(base)})
        _, columns, rows = next(table for table in tables if table[0].startswith("Direct format"))
        self.assertIn("11 mantissa bits kept", columns[1])
        self.assertIn("PositionTruncateBitCount=12", rows[0][1])
        self.assertEqual(rows[0][-1], "Not an equal-error comparison")
        self.assertEqual(next(row for row in rows if row[0] == AS_MEMORY)[-1], "-7.14%")
        _, columns, rows = next(table for table in tables if table[0] == "All requested cases")
        self.assertEqual(rows[0][columns.index("PositionTruncateBitCount")], 12)
        self.assertEqual(rows[0][columns.index("FLOAT32_3 mantissa bits kept")], 11)
        self.assertEqual(rows[1][columns.index("COMPRESSED1 offset bits/component")], 12)

    def test_direct_format_comparison_keeps_missing_or_failed_format_as_gap(self):
        compressed = result(replace(RunConfig(), vertex_format="compressed"))
        compressed.data["_sweep"]["status"] = "failed"
        more_precision = [result(cfg) for bits in (8, 14)
                          for cfg in sweep.precision_pair_configs(RunConfig(), bits)]
        chart = next(c for c in build_charts([result(), compressed, *more_precision])
                     if c.title == FORMAT_MEMORY_TITLE)
        self.assertIsNone(dict(chart.series[COMPRESSED_SERIES])[12])

    def test_precision_overlay_uses_aligned_pairs_and_exposes_missing_runs(self):
        runs = []
        for bits in sweep.NOMINAL_PRECISION_BITS:
            for cfg in sweep.precision_pair_configs(RunConfig(), bits):
                runs.append(result(cfg))
        chart = next(c for c in build_charts(runs)
                     if c.title == FORMAT_RENDER_TITLE)
        for points in chart.series.values():
            self.assertEqual([x for x, _ in points], list(sweep.NOMINAL_PRECISION_BITS))
            self.assertTrue(all(value is not None for _, value in points))
        self.assertIn("PositionTruncateBitCount=22", chart.x_tooltips["2"])
        self.assertIn("PositionTruncateBitCount=8", chart.x_tooltips["16"])
        missing = sweep.precision_pair_configs(RunConfig(), 6)[0]
        chart = next(c for c in build_charts([r for r in runs if r.config != missing])
                     if c.title == FORMAT_RENDER_TITLE)
        self.assertIsNone(dict(chart.series[FLOAT_SERIES])[6])

    def test_float_retained_bit_annotation_at_truncation_limit(self):
        runs = [result(replace(RunConfig(), position_truncate_bits=count)) for count in (0, 22)]
        chart = next(c for c in build_charts(runs) if c.category == "FLOAT32_3 precision")
        self.assertIn("1 explicit mantissa bits kept", chart.x_tooltips["22"])

    def test_all_failed_measurements_do_not_make_empty_charts(self):
        run = result()
        run.data["_sweep"]["status"] = "failed"
        run.data["_error"] = "synthetic failure"
        self.assertEqual(build_charts([run]), [])

    def test_flag_matrix_covers_all_preferences_for_every_animation_strategy(self):
        for quick in (True, False):
            matrix = build_sweep_matrix(quick, True)
            for mode, strategy in (("clusters", "rebuild"), ("traditional", "rebuild"),
                                   ("traditional", "refit")):
                for flag in ("none", "fast-build", "fast-trace"):
                    self.assertIn(replace(RunConfig(), geometry_mode=mode,
                                          trad_anim_mode=strategy, build_flags=flag), matrix)

    def test_old_sweep_does_not_show_single_flag_refit_study(self):
        matrix = build_sweep_matrix(True, True)
        old = [cfg for cfg in matrix if not (
            cfg.geometry_mode == "traditional" and cfg.trad_anim_mode == "refit" and
            cfg.build_flags != "fast-trace")]
        studies = [chart for chart in build_charts([result(cfg) for cfg in old])
                   if chart.category == "Build preference"]
        self.assertTrue(studies)
        self.assertTrue(all("traditional refit" not in chart.series for chart in studies))
        complete = [chart for chart in build_charts([result(cfg) for cfg in matrix])
                    if chart.category == "Build preference"]
        for chart in complete:
            self.assertEqual([x for x, _ in chart.series["traditional refit"]],
                             ["No preference", "Fast build", "Fast trace"])

    def test_quick_sweep_does_not_present_single_precision_point_as_a_curve(self):
        charts = build_charts([result(cfg) for cfg in build_sweep_matrix(True, True)])
        self.assertFalse(any(chart.category == "COMPRESSED1 vs FLOAT32_3 precision" for chart in charts))

    def test_discrete_studies_are_grouped_bars_with_clear_setting_names(self):
        charts = build_charts([result(cfg) for cfg in build_sweep_matrix(True, True)])
        chart = next(chart for chart in charts if chart.category == "CLAS allocation")
        self.assertEqual(chart.chart_type, "bar")
        self.assertFalse(chart.stacked)
        self.assertEqual(chart.x_label, "CLAS allocation strategy")
        self.assertEqual([x for x, _ in chart.series["clustered"]],
                         ["Implicit allocation", "Pre-size (GetSizes)", "Post-build compaction"])
        self.assertNotIn("image quality", chart.description)

    def test_time_labels_distinguish_initial_setup_render_and_measured_per_frame_work(self):
        charts = build_charts([result(cfg) for cfg in build_sweep_matrix(True, True)])
        work = next(chart for chart in charts if chart.title == sweep.AS_WORK_LABEL)
        self.assertEqual(work.unit, "us/frame")
        self.assertIn("Excludes untimed clustered vertex-animation", work.description)
        self.assertIn("not initial setup time or CPU time", work.description)
        setup = next(chart for chart in charts
                     if chart.title == "Initial AS setup elapsed time (GPU timestamps)")
        self.assertEqual(setup.unit, "ms")
        self.assertIn("Includes animated setup", setup.description)
        render = next(chart for chart in charts
                      if chart.title == "Raytracing render time per frame (GPU)")
        self.assertEqual(render.unit, "us/frame")
        self.assertIn("DispatchRays only", render.description)
        self.assertTrue(all("Timed AS" not in chart.title for chart in charts))

    def test_memory_components_are_human_names_and_reporting_does_not_modify_raw_snapshots(self):
        runs = [result(cfg) for cfg in build_sweep_matrix(True, True)]
        before = json.dumps([run.data for run in runs], sort_keys=True)
        charts = build_charts(runs)
        memory = next(chart for chart in charts if chart.category == "AS memory breakdown")
        self.assertTrue(all("_" not in x for points in memory.series.values() for x, _ in points))
        self.assertIn("0 extra objects", memory.description)
        tables = sweep.report_tables(runs, {"base_config": asdict(RunConfig())})
        _, columns, rows = next(table for table in tables if table[0] == "All requested cases")
        self.assertIn("Frame time (ms/frame)", columns)
        self.assertIn("Frames in average", columns)
        self.assertEqual(rows[0][columns.index("Animated BLAS update")], "Not applicable")
        self.assertEqual(json.dumps([run.data for run in runs], sort_keys=True), before)


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

    def test_invalidated_sweep_cannot_be_loaded_for_comparison_or_extension(self):
        manifest = save_results(self.root, [result(directory=self.root)])
        manifest["invalidated"] = "Input changed settings during measurement"
        write_json(self.root / "manifest.json", manifest)
        with self.assertRaisesRegex(ValueError, "sweep invalidated"):
            load_results(self.root)

    def test_old_executable_contract_failure_stops_the_sweep_after_one_case(self):
        def process(command, **kwargs):
            data = snapshot()
            for key in sweep.BENCHMARK_ENVIRONMENT:
                del data["config"][key]
            write_json(Path(command[-1]), data)
            return subprocess.CompletedProcess(command, 0)
        args = SimpleNamespace(bench_seconds=2, init_timeout=1, verbose=False, max_bench_seconds=10)
        manifest = dict(payload_sha256=sweep.payload_fingerprint(self.exe), runs=[])
        runs = []
        with patch("sweep.subprocess.run", side_effect=process) as native:
            sweep.execute_cases(self.exe, [RunConfig(), replace(RunConfig(), extra_instances=100)],
                                self.output, manifest, runs, args)
        self.assertEqual(native.call_count, 1)
        self.assertEqual(runs[0].status, "failed")
        self.assertTrue(runs[0].data["_sweep"]["stop_sweep"])

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

    def test_extension_only_runs_missing_cases_and_preserves_existing_result(self):
        first = result(directory=self.root)
        fingerprint = sweep.payload_fingerprint(self.exe)
        first.data["_sweep"]["payload_sha256"] = fingerprint
        manifest = save_results(self.root, [first])
        manifest["payload_sha256"] = fingerprint
        write_json(self.root / "manifest.json", manifest)
        original = first.json_path.read_bytes()
        extra = replace(RunConfig(), position_truncate_bits=22)
        def run(exe, cfg, directory, *args):
            self.assertEqual(cfg, extra)
            measured = result(cfg, directory)
            measured.data["_sweep"]["payload_sha256"] = fingerprint
            write_json(measured.json_path, measured.data)
            return measured
        args = SimpleNamespace(bench_seconds=2, init_timeout=1, verbose=False,
                               max_bench_seconds=10)
        runs = [first]
        with patch("sweep.run_one", side_effect=run) as process:
            sweep.execute_cases(self.exe, [first.config, extra], self.root, manifest, runs, args)
        self.assertEqual(process.call_count, 1)
        self.assertEqual(first.json_path.read_bytes(), original)
        loaded, _ = load_results(self.root)
        self.assertEqual(len(loaded), 2)
        with patch("sweep.run_one") as process:
            sweep.execute_cases(self.exe, [first.config, extra], self.root, manifest, loaded, args)
        process.assert_not_called()

    def test_extension_rejects_changed_payload_without_running(self):
        run = result(directory=self.root)
        manifest = save_results(self.root, [run])
        args = SimpleNamespace(bench_seconds=2, init_timeout=1, verbose=False,
                               max_bench_seconds=10)
        with patch("sweep.run_one") as process, self.assertRaisesRegex(ValueError, "runtime differs"):
            sweep.execute_cases(self.exe, [replace(RunConfig(), position_truncate_bits=22)],
                                self.root, manifest, [run], args)
        process.assert_not_called()

    def test_extension_rejects_another_adapter(self):
        expected = list(sweep.adapter_identity(snapshot()))
        expected[0] = "Different synthetic adapter"
        with patch("sweep.subprocess.run", side_effect=self.process):
            run = run_one(self.exe, RunConfig(), self.output, 2, 1,
                          expected_adapter=tuple(expected))
        self.assertEqual(run.status, "failed")
        self.assertIn("adapter/driver changed", run.data["_error"])

    @unittest.skipUnless(os.name == "nt", "native Windows path limit")
    def test_long_results_path_uses_short_native_paths_and_preserves_artifacts(self):
        destination = self.root / ("long-" + "x" * 100) / ("nested-" + "y" * 100)
        paths = []
        def process(command, **kwargs):
            paths.append(Path(command[-1]))
            self.assertLess(len(command[-1]), 250)
            self.assertLess(len(kwargs["env"]["SAMPLE_LOG"]), 250)
            return self.process(command, **kwargs)
        with patch("sweep.subprocess.run", side_effect=process):
            run = run_one(self.exe, RunConfig(), destination, 2, 1)
        self.assertEqual(run.status, "ok")
        self.assertEqual(run.json_path.parent, destination.resolve())
        self.assertTrue(run.json_path.is_file())
        self.assertTrue(list(destination.glob("*.attempt-1.json")))
        self.assertTrue(list(destination.glob("*.stdout.log")))
        self.assertFalse(paths[0].parent.exists())


class ResultsDirectoryNames(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def pending(self, name="pending"):
        directory = self.root / name
        directory.mkdir()
        run = result(directory=directory)
        manifest = save_results(directory, [run])
        manifest["created_local"] = "2026-01-02T03:04:05-07:00"
        write_json(directory / "manifest.json", manifest)
        return directory, run, manifest

    def test_readable_local_date_adapter_and_driver_and_loadable_data(self):
        directory, run, manifest = self.pending()
        final = sweep.finalize_results_directory(directory, [run], manifest)
        self.assertEqual(final.name, "2026-01-02_03-04-05_Synthetic-adapter_driver-test-driver")
        self.assertFalse(directory.exists())
        self.assertTrue(run.json_path.is_file())
        loaded, _ = load_results(final)
        self.assertEqual(loaded[0].config, run.config)

    def test_unsafe_characters_long_labels_and_collisions(self):
        directory, run, manifest = self.pending("first")
        run.data["adapter"].update(description='GPU: / invalid? * \\\\ "<>| ' + "x" * 100,
                                   driver_version="1:2/3? " + "y" * 100)
        final = sweep.finalize_results_directory(directory, [run], manifest)
        self.assertFalse(any(char in final.name for char in '<>:"/\\\\|?*'))
        self.assertLessEqual(len(final.name), 101)
        directory2, run2, manifest2 = self.pending("second")
        run2.data["adapter"] = dict(run.data["adapter"])
        final2 = sweep.finalize_results_directory(directory2, [run2], manifest2)
        self.assertEqual(final2.name, final.name + "_2")
        self.assertTrue(final.is_dir())
        self.assertTrue(final2.is_dir())

    def test_missing_adapter_and_failed_rename_preserve_evidence(self):
        directory, run, manifest = self.pending()
        with patch.object(Path, "rename", side_effect=PermissionError("synthetic access denial")):
            with self.assertRaises(PermissionError):
                sweep.finalize_results_directory(directory, [run], manifest)
        self.assertTrue(run.json_path.exists())
        final = sweep.finalize_results_directory(directory, [], manifest)
        self.assertIn("unknown-adapter_driver-unknown", final.name)


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
        chart = next(c for c in charts if c.title == f"{AS_MEMORY}: clustered")
        self.assertEqual(chart.series["A"], [(0, 700)])
        charts, _ = compare.comparison_charts(machines, partial=True)
        chart = next(c for c in charts if c.title == f"{AS_MEMORY}: clustered")
        self.assertEqual([x for x, _ in chart.series["A"]], [0, 100])
        self.assertEqual([x for x, _ in chart.series["B"]], [0])

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

    def test_retained_bit_annotations_survive_machine_comparison(self):
        runs = [result(), result(replace(RunConfig(), position_truncate_bits=20))]
        charts, _ = compare.comparison_charts([self.machine("A", runs), self.machine("B", runs)])
        chart = next(c for c in charts if c.category == "FLOAT32_3 precision")
        self.assertIn("11 explicit mantissa bits kept", chart.x_tooltips["12"])
        self.assertIn("3 explicit mantissa bits kept", chart.x_tooltips["20"])

    def test_three_machine_scaling_is_faceted_and_precision_formats_have_distinct_styles(self):
        configs = build_sweep_matrix(False, True)
        runs = [result(config) for config in configs]
        charts, _ = compare.comparison_charts([self.machine(label, runs) for label in ("A", "B", "C")])
        scaling = [chart for chart in charts if chart.title.startswith(f"{AS_MEMORY}: ")
                   and chart.category.startswith("Scene scaling /")]
        self.assertEqual(len(scaling), 3)
        for chart in scaling:
            self.assertEqual(set(chart.series), {"A", "B", "C"})
        precision = next(chart for chart in charts
                         if chart.title == FORMAT_RENDER_TITLE)
        self.assertEqual(precision.line_dashes[f"A / {FLOAT_SERIES}"], [])
        self.assertEqual(precision.line_dashes[f"A / {COMPRESSED_SERIES}"], [6, 3])
        self.assertNotEqual(precision.point_styles[f"A / {FLOAT_SERIES}"],
                            precision.point_styles[f"A / {COMPRESSED_SERIES}"])
        self.assertEqual(precision.color_groups[f"A / {FLOAT_SERIES}"], "A")
        self.assertEqual(precision.color_groups[f"A / {COMPRESSED_SERIES}"], "A")

    def test_within_gpu_ratios_are_computed_from_that_gpu_only(self):
        configs = [RunConfig(), replace(RunConfig(), geometry_mode="traditional"),
                   replace(RunConfig(), geometry_mode="traditional", trad_anim_mode="refit")]
        a = [result(config) for config in configs]
        b = [result(config) for config in configs]
        a[0].data["frame_perf"]["fps"] = 200
        b[0].data["frame_perf"]["fps"] = 50
        charts, _ = compare.comparison_charts([self.machine("A", a), self.machine("B", b)])
        fps = next(chart for chart in charts if chart.title == "Frame-rate ratio: clustered / traditional (rebuild)")
        self.assertEqual(fps.series["A"], [(0, 2)])
        self.assertEqual(fps.series["B"], [(0, 0.5)])
        memory = next(chart for chart in charts
                      if chart.title == "AS memory ratio: traditional / clustered (rebuild)")
        self.assertEqual(memory.series["A"], [(0, 400 / 700)])

    def test_statistical_cautions_are_prominent_and_data_driven(self):
        a, b = self.machine("A", [result()]), self.machine("B", [result()])
        a.manifest["payload_sha256"], b.manifest["payload_sha256"] = "build-A", "build-B"
        b.runs[0].data["frame_perf"]["frames_in_avg_window"] = 2
        notes = compare.comparison_warnings([a, b])
        self.assertTrue(any("BUILD CAUTION" in note and "differ" in note for note in notes))
        self.assertTrue(any("SAMPLE CAUTION: B" in note and "minimum 2" in note for note in notes))
        self.assertTrue(any("1280x720" in note for note in notes))
        self.assertTrue(any("No variance" in note for note in notes))
        a.manifest["payload_sha256"] = b.manifest["payload_sha256"]
        b.runs[0].data["frame_perf"]["frames_in_avg_window"] = 100
        notes = compare.comparison_warnings([a, b])
        self.assertFalse(any("BUILD CAUTION" in note for note in notes))
        self.assertFalse(any("SAMPLE CAUTION" in note for note in notes))

    def test_legacy_and_fixed_window_results_are_not_silently_combined(self):
        legacy = result()
        for key in sweep.BENCHMARK_ENVIRONMENT:
            del legacy.data["config"][key]
        a = self.machine("A", [legacy])
        b = self.machine("B", [result()])
        self.assertTrue(any("LEGACY INPUT" in note for note in compare.comparison_warnings([a])))
        with self.assertRaisesRegex(ValueError, "render/input conditions"):
            compare.comparison_charts([a, b])

    def test_largest_workload_summary_uses_common_supported_scale(self):
        configs = build_sweep_matrix(True, True)
        first = self.machine("A", [result(cfg) for cfg in configs])
        second = self.machine("B", [result(cfg) for cfg in configs if cfg.extra_instances != 1000])
        tables = compare.largest_workload_tables([first, second])
        self.assertIn("100 extra BLASes", tables[0][0])
        self.assertEqual(len(tables[0][2]), 2)
        self.assertEqual(tables[0][2][0][-3:-1], ["1.00x", "1.00x"])
        self.assertIn("not total VRAM", tables[1][0])

    def test_ratio_comparisons_pin_one_baseline_across_all_machines(self):
        configs = [RunConfig(), replace(RunConfig(), geometry_mode="traditional"),
                   replace(RunConfig(), geometry_mode="traditional", trad_anim_mode="refit")]
        full = [result(cfg) for cfg in configs]
        low = [result(replace(cfg, animated_detail="low")) for cfg in configs]
        for run in low:
            if run.config.geometry_mode == "clusters":
                run.data["frame_perf"]["fps"] = 500
        first = self.machine("A", full + low)
        second = self.machine("B", [result(cfg) for cfg in configs] + low)
        second.manifest["base_config"]["animated_detail"] = "low"
        charts, _ = compare.comparison_charts([first, second])
        ratio = next(chart for chart in charts
                     if chart.title == "Frame-rate ratio: clustered / traditional (rebuild)")
        self.assertEqual(ratio.series["A"], [(0, 1)])
        self.assertEqual(ratio.series["B"], [(0, 1)])
        self.assertTrue(any("BASELINE CAUTION" in note
                            for note in compare.comparison_warnings([first, second])))


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
        self.assertIn("Math.ceil(chart.legend.height + 280)", report)
        self.assertIn("chart.height - chart.chartArea.height + 240", report)
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

    def test_comparison_summary_precedes_charts_and_series_styles_are_embedded(self):
        chart = Chart("Synthetic plot", "Category", "Description", "x", "ms",
                      {"A / FLOAT32_3": [(0, 1)], "A / COMPRESSED1": [(0, 2)]})
        chart.line_dashes["A / COMPRESSED1"] = [6, 3]
        chart.point_styles["A / COMPRESSED1"] = "triangle"
        rendered = render_report("Test", "Subtitle", [chart], [], [],
                                 overview_tables=[("Coverage first", ["GPU"], [["Synthetic"]])])
        self.assertLess(rendered.index("Coverage first"), rendered.index("<canvas"))
        self.assertIn('"line_dash": [6, 3]', rendered)
        self.assertIn('"point_style": "triangle"', rendered)
        self.assertIn("href='#study-0'", rendered)
        self.assertIn("id='study-0'", rendered)

    def test_exact_values_distinguish_zero_small_and_missing_allocations(self):
        chart = Chart("AS memory by buffer", "AS memory breakdown / traditional",
                      "Description", "AS buffer", "bytes",
                      {"A": [("Animated copy BLASes", 0), ("TLAS", 1024), ("Unknown", None)],
                       "B": [("Animated copy BLASes", 0), ("TLAS", 2048), ("Unknown", None)]},
                      chart_type="bar")
        rendered = render_report("Test", "Subtitle", [chart], [], [])
        self.assertIn("Zero in every plotted series: Animated copy BLASes.", rendered)
        self.assertIn("<td>0 B</td>", rendered)
        self.assertIn("<td>1.00 KiB</td>", rendered)
        self.assertIn("<td>Not measured</td>", rendered)
        self.assertIn("class='chart-values' open", rendered)
        self.assertIn("mode: 'index', intersect: false", rendered)
        self.assertIn("pointBackgroundColor: css('--cp-point-fill')", rendered)

    def test_reports_use_plain_provenance_labels_without_privacy_callouts(self):
        run = result()
        machine = compare.MachineReport("Synthetic GPU", Path("synthetic"), [run],
                                        {"base_config": asdict(RunConfig())})
        titles = [table[0] for table in sweep.report_tables([run], machine.manifest)]
        self.assertIn("Run provenance", titles)
        self.assertEqual(compare.summary_table([machine])[0], "Input coverage and build identity")
        for text in [*titles, *sweep.REPORT_NOTES]:
            self.assertNotIn("private", text.lower())


if __name__ == "__main__":
    unittest.main()
