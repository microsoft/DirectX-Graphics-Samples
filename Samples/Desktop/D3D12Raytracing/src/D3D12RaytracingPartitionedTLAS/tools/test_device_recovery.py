#*********************************************************
#
# Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# This code is licensed under the MIT License (MIT).
# THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
# IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
# PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
#
#*********************************************************

"""Verify real application-local device removal against a fresh simulation.

Run after building: python tools/test_device_recovery.py [path/to/sample.exe]
ID3D12Device5::RemoveDevice affects this process only, without a system TDR.
"""
from pathlib import Path
import csv
import subprocess
import sys
import tempfile

sample = (Path(sys.argv[1]) if len(sys.argv) > 1 else
          Path(__file__).resolve().parents[1] / "bin/x64/Release/dxr_partitioned_tlas.exe").resolve()
startup = subprocess.STARTUPINFO()
startup.dwFlags = subprocess.STARTF_USESHOWWINDOW
startup.wShowWindow = 0


def run(directory, frames, options, removal=None):
    directory.mkdir()
    arguments = [str(sample), "--frames", str(frames), "--debug", "--no-vsync",
                 "--state-dump", "state.bin", "--csv", "frames.csv", *options]
    if removal is not None:
        arguments += ["--test-device-loss-frame", str(removal)]
    result = subprocess.run(arguments, cwd=directory, startupinfo=startup, timeout=60)
    assert result.returncode == 0, (directory / "ptlas_error.log").read_text()
    assert "errors=0" in (directory / "ptlas_debug.log").read_text()
    rows = list(csv.DictReader((directory / "frames.csv").open()))
    assert int(rows[-1]["frame"]) == frames
    if removal is not None:
        log = (directory / "device_recovery.log").read_text()
        assert "Device restored; PTLAS rebuilt" in log
    return (directory / "state.bin").read_bytes()


with tempfile.TemporaryDirectory(prefix="ptlas_recovery_") as temporary:
    root = Path(temporary)
    for name, removal, options in [
        ("ptlas_before_first_frame", 0, ["--mode", "3"]),
        ("ptlas_in_flight", 13, ["--mode", "3", "--mark-all"]),
        ("regular_tlas_in_flight", 13, ["--tlas"]),
        ("paused_ptlas", 13, ["--mode", "2", "--paused"]),
    ]:
        actual = run(root / name, 60, options, removal)
        expected = run(root / (name + "_baseline"), 60 - removal, options)
        assert actual == expected, f"{name}: restored simulation differs from a fresh run"
        print(name + ": restored, finished, zero debug errors, physics matches fresh run")
