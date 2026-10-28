"""Windows wrapper tests use temporary scripts, never real measurements."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


@unittest.skipUnless(os.name == "nt", "CMD wrappers require Windows")
class CmdWrappers(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.tools = self.root / "tools with spaces & bang!"
        self.caller = self.root / "caller"
        self.tools.mkdir()
        self.caller.mkdir()
        source = Path(__file__).parent
        for name in ("sweep.cmd", "compare.cmd", "run-python.cmd", "test.cmd"):
            shutil.copy2(source / name, self.tools / name)
        stub = (
            "import json, os, sys\n"
            "print(json.dumps({'args': sys.argv[1:], 'cwd': os.getcwd()}))\n"
            "sys.exit(37 if '--fail' in sys.argv else 0)\n"
        )
        for name in ("sweep.py", "compare.py"):
            (self.tools / name).write_text(stub, encoding="utf-8")

    def invoke(self, name, args=(), environment=None):
        comspec = os.environ.get("COMSPEC", str(Path(os.environ["SystemRoot"]) / "System32" / "cmd.exe"))
        # Quote every argument for CMD, including shell metacharacters without spaces.
        command = f'"{comspec}" /d /v:off /s /c ""{self.tools / name}"'
        command += "".join(f' "{arg}"' for arg in args) + '"'
        return subprocess.run(command, cwd=self.caller,
                              env=environment if environment is not None else os.environ.copy(),
                              capture_output=True, text=True, encoding="utf-8", timeout=20)

    def test_arguments_special_paths_and_caller_directory(self):
        args = ["--out", "folder with spaces & bang!/report.html", "--label", "A&B!",
                "--label", "", "--label", "Unicode \u03a9"]
        for name in ("sweep.cmd", "compare.cmd"):
            with self.subTest(name=name):
                completed = self.invoke(name, args)
                self.assertEqual(completed.returncode, 0, completed.stderr)
                value = json.loads(completed.stdout)
                self.assertEqual(value["args"], args)
                self.assertEqual(Path(value["cwd"]).resolve(), self.caller.resolve())

    def test_python_error_code_is_preserved(self):
        completed = self.invoke("sweep.cmd", ["--fail"])
        self.assertEqual(completed.returncode, 37, completed.stderr)

    def test_python_on_path_without_launcher(self):
        environment = os.environ.copy()
        environment["PATH"] = str(Path(sys.executable).parent)
        completed = self.invoke("sweep.cmd", ["--list"], environment)
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertEqual(json.loads(completed.stdout)["args"], ["--list"])

    def test_launcher_fallback_without_python_on_path(self):
        launcher = shutil.which("py.exe")
        if not launcher or (Path(launcher).parent / "python.exe").exists():
            self.skipTest("an isolated Windows Python launcher is not available")
        environment = os.environ.copy()
        environment["PATH"] = str(Path(launcher).parent)
        completed = self.invoke("compare.cmd", ["--help"], environment)
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertEqual(json.loads(completed.stdout)["args"], ["--help"])

    def test_missing_python_is_an_explicit_failure(self):
        environment = os.environ.copy()
        environment["PATH"] = str(self.caller)
        completed = self.invoke("sweep.cmd", environment=environment)
        self.assertEqual(completed.returncode, 9009, completed.stderr)
        self.assertIn("Python 3.10 or later is required", completed.stderr)
        self.assertEqual(completed.stdout, "")

    def test_test_wrapper_is_anchored_to_its_own_directory(self):
        completed = self.invoke("test.cmd", ["--help"])
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertIn("unittest", completed.stdout)


if __name__ == "__main__":
    unittest.main()
