@ECHO OFF
REM This batch file generates a new MiniEngine library project for Visual Studio
REM 2015 and 2017.  It is expected that python.exe is on your path and is version
REM 3.0 or above.  If not, we recommend installing the latest version of Anaconda.
python.exe Tools\Scripts\CreateNewProject.py LIB %1