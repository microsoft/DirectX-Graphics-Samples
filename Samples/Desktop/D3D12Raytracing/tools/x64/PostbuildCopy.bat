rem Get the Windows 10 version and trim everything off except the number itself
SET "OS="
for /F "skip=1 tokens=*" %%a IN ('WMIC OS GET VERSION') DO IF NOT DEFINED OS SET OS=%%a
for /F "tokens=3* delims=." %%f in ("%OS%") do ( set OS=%%f )
set BuildVersion=%OS%
rem Remove whitespace

rem Copy dxrfallbackcompiler.dll needed regardless of whether it's using native DXR or the Fallback layer
set outputDirectory=%1
echo %1
copy dxrfallbackcompiler.dll %outputDirectory%
