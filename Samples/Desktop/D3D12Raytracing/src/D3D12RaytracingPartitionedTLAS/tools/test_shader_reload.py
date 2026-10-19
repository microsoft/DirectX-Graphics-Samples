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

"""Check source discovery, failed reload rollback, restart, and device recovery.

Run after a Release build. Invalid pipeline creation intentionally emits debug
errors, so those cases use Release without --debug. All mutations are confined
to a temporary copy of the application and its shaders.
"""
from pathlib import Path
import hashlib
import shutil
import subprocess
import sys
import tempfile

sample = (Path(sys.argv[1]) if len(sys.argv) > 1 else
          Path(__file__).resolve().parents[1] / "bin/x64/Release/dxr_partitioned_tlas.exe").resolve()
startup = subprocess.STARTUPINFO()
startup.dwFlags = subprocess.STARTF_USESHOWWINDOW
startup.wShowWindow = 0

with tempfile.TemporaryDirectory(prefix="ptlas_reload_") as temporary:
    root = Path(temporary).resolve()
    assert root.parent == Path(tempfile.gettempdir()).resolve()
    binary = root / "bin/x64/Release"
    binary.mkdir(parents=True)
    for source in sample.parent.iterdir():
        if source.is_file() and source.suffix in (".exe", ".dll", ".dxil", ".spritefont"):
            shutil.copy2(source, binary / source.name)
    for name in ("D3D12", "shaders"):
        shutil.copytree(sample.parent / name, binary / name)
    shutil.copytree(binary / "shaders", root / "shaders")
    project = root / "D3D12RaytracingPartitionedTLAS.vcxproj"
    project.touch()
    original = {p.name: p.read_bytes() for p in (root / "shaders").iterdir()}
    dxil = {p.name: hashlib.sha256(p.read_bytes()).digest() for p in binary.glob("*.dxil")}

    def run(name, options=(), expect_reload=None):
        directory = root / name
        directory.mkdir()
        result = subprocess.run([str(binary / sample.name), "--frames", "8", "--no-vsync",
                                 "--width", "320", "--height", "240", "--image", "image.dds",
                                 *options], cwd=directory, startupinfo=startup, timeout=45)
        assert result.returncode == 0, (directory / "dxr2_error.log").read_text()
        if expect_reload is not None:
            log = (directory / "dxr2_shader_reload.log").read_text()
            assert ("reload successful" in log) == expect_reload, log
        if "--test-device-loss-frame" in options:
            assert "Device restored" in (directory / "device_recovery.log").read_text()
        assert dxil == {p.name: hashlib.sha256(p.read_bytes()).digest() for p in binary.glob("*.dxil")}
        print(name + ": passed", flush=True)
        return (directory / "image.dds").read_bytes()

    run("baseline")
    shader = root / "shaders/raytracing.hlsl"
    shader.write_bytes(original[shader.name] + b"\nINVALID_TEST_HLSL\n")
    run("project_compile_failure", ["--reload-shaders", "--debug"], False)
    shader.write_bytes(original[shader.name].replace(b"void RayGen()", b"void MissingRayGen()"))
    run("pipeline_failure", ["--reload-shaders"], False)
    run("restart_after_failure")
    run("recover_after_failure", ["--reload-shaders", "--test-device-loss-frame", "3"], False)
    shader.write_bytes(original[shader.name])

    # A visible valid change must survive reconstruction of the GPU resources.
    composite = root / "shaders/compositing.hlsl"
    marker = b"int2 maximum = int2(width - 1, height - 1);"
    assert marker in original[composite.name]
    composite.write_bytes(original[composite.name].replace(marker, marker +
        b"\nif(PassId == 3) { Output[coord] = float4(1, 0, 0, 1); return; }"))
    accepted = run("valid_project_reload", ["--reload-shaders", "--debug"], True)
    recovered = run("recover_accepted_reload", ["--reload-shaders", "--debug", "--test-device-loss-frame", "3"], True)
    assert accepted == recovered, "Device recovery lost the accepted shader change"
    assert accepted != run("restart_uses_build_shaders"), "Reload overwrote the build shaders"

    project.unlink()
    packaged = binary / "shaders/raytracing.hlsl"
    packaged.write_bytes(original[packaged.name] + b"\nINVALID_PACKAGED_HLSL\n")
    run("standalone_compile_failure", ["--reload-shaders", "--debug"], False)
