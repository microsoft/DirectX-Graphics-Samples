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

"""Exercise the paused menu action through Win32 input, then inspect GPU state."""
from pathlib import Path
import ctypes as c
from ctypes import wintypes as w
import struct
import subprocess
import sys
import tempfile
import time

sample = (Path(sys.argv[1]) if len(sys.argv) > 1 else
          Path(__file__).resolve().parents[1] / "bin/x64/Release/dxr_partitioned_tlas.exe").resolve()
startup = subprocess.STARTUPINFO()
startup.dwFlags = subprocess.STARTF_USESHOWWINDOW
startup.wShowWindow = 0
user32 = c.windll.user32
user32.GetWindowThreadProcessId.argtypes = [w.HWND, c.POINTER(w.DWORD)]
user32.GetWindowTextW.argtypes = [w.HWND, w.LPWSTR, c.c_int]
user32.SendMessageW.argtypes = [w.HWND, w.UINT, w.WPARAM, w.LPARAM]
user32.SendMessageW.restype = w.LPARAM

with tempfile.TemporaryDirectory(prefix="ptlas_topple_") as temporary:
    directory = Path(temporary).resolve()
    assert directory.parent == Path(tempfile.gettempdir()).resolve()
    # Keep the full physics scene; use a small render target so pixel shading
    # does not consume the timeout intended for the menu/physics regression.
    process = subprocess.Popen([str(sample), "--full", "--paused", "--self-topple-count", "0",
        "--frames", "4000", "--width", "320", "--height", "240", "--render-scale", "1",
        "--no-vsync", "--debug", "--state-dump", "state.bin"],
        cwd=directory, startupinfo=startup)
    try:
        window = [None]

        @c.WINFUNCTYPE(w.BOOL, w.HWND, w.LPARAM)
        def find_window(hwnd, parameter):
            pid = w.DWORD()
            user32.GetWindowThreadProcessId(hwnd, c.byref(pid))
            title = c.create_unicode_buffer(512)
            user32.GetWindowTextW(hwnd, title, len(title))
            if pid.value == process.pid and "GPU" in title.value:
                window[0] = hwnd
            return True

        for _ in range(200):
            user32.EnumWindows(find_window, 0)
            if window[0]:
                break
            assert process.poll() is None, "Sample exited before accepting input"
            time.sleep(.05)
        assert window[0], "Sample window was not ready"

        def key(code):
            user32.SendMessageW(window[0], 0x100, code, 0)
            time.sleep(.05)
            user32.SendMessageW(window[0], 0x101, code, 0)
            time.sleep(.05)

        key(0x71)  # F2 opens the PTLAS/animation menu.
        for _ in range(11):
            key(0x28)  # Select "Topple selected domino" (index defaults to zero).
        key(0x0d)
        key(0x71)
        # No resume/step key: the action itself must start physics.
        assert process.wait(timeout=60) == 0
        state = (directory / "state.bin").read_bytes()
        assert struct.unpack_from("<I", state, 84)[0] != 0xffffffff, "Topple request was lost"
        assert "errors=0" in (directory / "ptlas_debug.log").read_text()
        print("Paused menu topple: simulation resumed, contact recorded, zero debug errors")
    finally:
        if process.poll() is None:
            process.terminate()
            process.wait()
