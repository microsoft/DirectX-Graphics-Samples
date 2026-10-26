# Clustered Geometry settings sweeps

Reusable, standard-library-only Python tooling for running the sample across
settings and comparing **matched workloads** between machines or builds.
Reports embed the included MIT-licensed Chart.js library and work offline.
`LICENSE.Chart.js.txt` preserves the [Chart.js 4.4.1 license](https://github.com/chartjs/Chart.js/blob/v4.4.1/LICENSE.md);
the complete notice is also included in each generated report.

## Requirements

- Python 3.10 or later; no Python packages need to be installed. The CMD wrappers
  find a compatible `python.exe` on PATH (including an active virtual environment)
  or fall back to the Windows `py.exe` launcher. You do not invoke Python directly.
- A current, working `D3D12RaytracingClusteredGeometry.exe` and its deployed
  runtime DLLs and assets. Use an optimized **Release** build for measurements.
  Debug builds are useful for functional smoke tests, not comparable performance.
- The sample's normal OS, driver, and preview-feature prerequisites.
  The scripts do not install drivers or change global graphics settings.

Run the following from this `benchmark` directory. The default executable is
`..\bin\x64\Release\D3D12RaytracingClusteredGeometry.exe`; use `--exe` for an
isolated build or another location.
Wrappers locate their scripts relative to themselves, so they can also be invoked
by full path from another directory. Relative input/output arguments remain
relative to your current directory. All arguments and the tool's exit code are
forwarded; missing/outdated Python produces a clear error without installing it.

```powershell
# Show the exact configurations without launching anything.
.\sweep.cmd --list

# Shorter matrix: excludes 10K and the precision sweeps.
.\sweep.cmd --quick

# Full matrix, including traditional rebuild and refit at 10K.
.\sweep.cmd --exe ..\bin\x64\Release\D3D12RaytracingClusteredGeometry.exe

# Explicit low-VRAM baseline. This is reported, never selected secretly.
.\sweep.cmd --animated-detail low

# Skip expensive configurations when needed.
.\sweep.cmd --no-10k
.\sweep.cmd --no-10k-trad

# Longer measurement window; increase the retry ceiling for very slow frames.
.\sweep.cmd --bench-seconds 10 --max-bench-seconds 120
```

The matrix covers scene scaling, clustered/traditional rebuild/traditional
refit, vertex formats, CLAS/traditional allocation, build preferences, position
precision, primary rays per pixel, animated Full/Low detail, and per-frame
static rebuilding. `--list` is authoritative; counts are not hardcoded in
this document.

## Measurement contract

Every invocation pins its settings rather than relying on changing sample
defaults. Vsync is requested **off**, the camera is paused at its initial
position, and animation continues. The benchmark baseline has **zero extra
copies**, independent of the interactive default of 100.

Both geometry modes receive the same selected mesh and extra-object count.
Every animated copy must have an independent BLAS; capped or shared-BLAS
substitutions are rejected. Static curved copies use the sample's
generation-time distance LOD. Animated Full/Low is a separate, explicit setting.

The current sample publishes FPS, frame interval, and GPU stage costs from the
**same settled, roughly one-second rolling window**. These are the last
published window's means, **not** averages over the entire process lifetime.
The report includes the measured duration, sample count, and first/last frame.
Legacy snapshots with independent FPS/EMA statistics are rejected.

`--bench-seconds` starts after the first rendered frame. At low FPS, the
timestamp-readback pipeline can outlast a short run. Only an unsettled case is
retried, with a longer duration, up to `--max-bench-seconds` (default 60).
Successful cases are not repeated. An unsuccessful retry remains an explicit
failure; the tool never substitutes zero or an EMA for an unavailable mean.
The process timeout includes `--init-timeout` (default 120 seconds), the current
measurement duration, and an additional 15 seconds. Increase the allowance
when initialization or individual frames are unusually long.

Driver/compositor policy can still limit presentation, particularly under
remote desktop. Close other GPU-heavy applications, use matching viewport and
desktop conditions, and use matching optimized binaries when comparing
hardware. For publication, repeat measurements and investigate variability;
a single sweep is an observation, not a statistical performance guarantee.

## Reports and private data

Each sweep creates an independent `results\<UTC timestamp>\` directory:

- `report.html`: offline charts, settings, failures, and local provenance.
- `manifest.json`: exact case identities and executable/runtime fingerprint.
- One final JSON per case: sample snapshot plus `_sweep` provenance.
- Per-attempt JSON and sample/stdout/stderr logs, including unsuccessful attempts.

**Do not commit generated results, reports, logs, or machine details.**
They can include adapter/driver identity, local executable paths and commands,
and timing/memory data. Default results and generated HTML/JSON/log/CSV files
under this directory are ignored by Git. If you choose a custom output location,
keep it outside version control too. Only reusable code, documentation, the
offline library, and synthetic tests belong in the repository.

The tool records an executable plus deployed-runtime SHA-256 fingerprint.
Different fingerprints are disclosed, not automatically assumed equivalent.
It checks actual reported settings, resident-memory totals, geometry counts,
and paired timing arithmetic before accepting a measurement.

Missing/unsupported/failed cases are gaps, not zero-cost points. Unsupported
cluster hardware is labeled as skipped rather than plotting the sample's
traditional fallback under a cluster label. Any unexpected failed case produces
a nonzero exit status, while the report and evidence remain available.

### Regenerate a report without running the executable

```powershell
.\sweep.cmd --input-dir .\results\<sweep-directory>

# Or select the most recently modified manifest under --results-root.
.\sweep.cmd --no-run

.\sweep.cmd --input-dir .\results\<sweep-directory> --out .\results\revised-report.html
```

No executable is needed for report-only mode. It loads exactly the manifest's
cases, not guessed filenames or whichever adapter directory sorts last.
Use `--input-dir` for an unambiguous selection. Original process durations and
failure status are preserved.

### Add missing configurations without repeating recorded runs

```powershell
.\sweep.cmd --extend .\results\<sweep-directory>
```

This adds only configurations missing from the current matrix, then regenerates
the report. Existing successful, failed, and skipped records are retained,
not overwritten or rerun. The saved baseline is preserved. The saved executable
is used unless `--exe` is supplied; its executable/runtime fingerprint must still
match. Adapter/driver identity must match too. Start a new sweep for a different
machine, driver, or build rather than mixing it into an existing directory.
Without `--quick`, an older full sweep gains the additional matched precision
points; extending a quick sweep also adds its previously omitted full-matrix cases.

## Compare machines or builds

Run the same command on each machine, then copy each **whole sweep directory**
to a private location. Copying only `report.html` is sufficient for viewing,
but not for rebuilding comparisons.

```powershell
.\compare.cmd .\results\<machine-A-sweep> .\results\<machine-B-sweep> `
    --label "Machine A" --label "Machine B" --out .\results\comparison.html
```

Repeat `--label` in input order for additional machines. Duplicate default
adapter/driver labels are automatically distinguished.

By default, only successful configurations present on **every** input enter
comparison charts. `--partial` additionally shows unmatched configurations as
gaps. Matching uses the complete pinned configuration, actual geometry counts,
camera settings, and timing semantics, never a filename's spelling.
Different geometry for the same settings is rejected rather than presented
as an apples-to-apples comparison.

Percentage deltas always use the **first input** as the reference. A missing
reference remains missing; another machine is not silently substituted.
Hardware/driver metadata stays in the local report. Select and review any
shareable findings separately before including them in a public article.

## Reading the charts

- FLOAT32_3 precision plots use **PositionTruncateBitCount** (`--position-truncate`)
  directly on the x-axis. Zero disables truncation; increasing the count clears
  more low mantissa bits and reduces precision. It is not bits retained and is
  not the COMPRESSED1 bits/component control.
  Tooltips, comparison labels, and the run table also show the **explicit
  mantissa bits kept: 23 minus PositionTruncateBitCount**. Count 12 keeps 11;
  normal values have an additional implicit leading bit. Source float components
  still occupy 32 bits.
- **COMPRESSED1 vs FLOAT32_3 precision** overlays line graphs across nominal
  precision bits 2, 4, 6, 8, 10, 12, 14, and 16. For normal FLOAT32_3 values the
  axis is retained significand bits, including the implicit leading bit:
  **24 minus PositionTruncateBitCount**. For COMPRESSED1 it is cluster-local
  offset bits/component. The full matrix includes both formats at every target.
  Tooltips show the actual controls and retained fraction bits.
  This is a rough nominal-bit alignment, **not measured equal geometric error**:
  per-value floating-point precision and cluster-local fixed-point grids have
  different ranges and scales. Workload, rays, detail, and other settings stay fixed.
  Older result sets show gaps for missing precision cases; `--extend` fills them.
  A separate table retains the selected baseline comparison.
  In particular, truncate count 12 versus COMPRESSED1 12 offset bits/component
  is not an equal-precision match: one discards FLOAT32_3 mantissa bits, while
  the other encodes cluster-local fixed-point offsets using an anchor and scale.
- Each axis has a single unit; all linear y-axes start at zero.
- Resident AS allocation components sum to `memory_bytes.total_as`, including
  cluster templates, allocated instantiated CLAS capacity, all animated clone
  BLAS pools, and TLAS. Payload bytes, scratch, and source inputs are distinct.
- Static setup times are GPU-timestamp measurements, not CPU initialization time.
- Render time covers `DispatchRays` only.
- Frame-budget stacks use the same frame samples: timed AS stages plus render
  plus the residual equal the completion interval. The residual is other work
  and queue idle time, **not** a measured CPU duration.
- Traditional rebuild and refit are labeled separately. Precision and Full/Low
  comparisons intentionally change geometry; a speedup alone does not establish
  equal visual quality.

## Tests

```powershell
.\test.cmd -v
```

Fixtures are synthetic and contain no real machine identities or measurements.
They cover settings verification, missing/invalid timing, slow/fast cadence,
resident-memory accounting, retries and process failures, report-only loading,
comparison fairness, safe HTML/JSON embedding, and offline report behavior.
