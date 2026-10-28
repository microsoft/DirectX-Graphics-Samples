@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "PYLAUNCHER_ALLOW_INSTALL="
set "PYTHON_MANAGER_AUTOMATIC_INSTALL=0"

"%SystemRoot%\System32\where.exe" /q python.exe
if errorlevel 1 goto check_launcher
python.exe -X utf8 -c "import sys; sys.exit(sys.version_info < (3, 10))" >nul 2>&1
if not errorlevel 1 goto use_python

:check_launcher
"%SystemRoot%\System32\where.exe" /q py.exe
if errorlevel 1 goto missing_python
py.exe -3 -X utf8 -c "import sys; sys.exit(sys.version_info < (3, 10))" >nul 2>&1
if not errorlevel 1 goto use_launcher

:missing_python
echo ERROR: Python 3.10 or later is required to run the benchmark tools. 1>&2
echo Install Python and make python.exe or the py.exe launcher available, then retry. 1>&2
exit /b 9009

:use_python
python.exe -X utf8 %*
exit /b %errorlevel%

:use_launcher
py.exe -3 -X utf8 %*
exit /b %errorlevel%
