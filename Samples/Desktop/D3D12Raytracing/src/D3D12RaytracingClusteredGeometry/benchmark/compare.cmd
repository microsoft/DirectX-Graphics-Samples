@echo off
setlocal EnableExtensions DisableDelayedExpansion
"%~dp0run-python.cmd" "%~dp0compare.py" %*
