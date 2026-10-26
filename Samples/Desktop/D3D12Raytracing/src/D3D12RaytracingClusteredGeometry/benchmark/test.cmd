@echo off
setlocal EnableExtensions DisableDelayedExpansion
"%~dp0run-python.cmd" -m unittest discover -s "%~dp0." -p "test_*.py" %*
