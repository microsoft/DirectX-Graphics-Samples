rem Get the Windows 10 version and trim everything off except the number itself
SET "OS="
for /F "skip=1 tokens=*" %%a IN ('WMIC OS GET VERSION') DO IF NOT DEFINED OS SET OS=%%a
for /F "tokens=3* delims=." %%f in ("%OS%") do ( set OS=%%f )
set BuildVersion=%OS%
rem Remove whitespace
set BuildVersion=%BuildVersion: =%
set UseRS4Binaries=0

rem If the user is running on any "recent" RS4 build, copy DXR-enabled D3D12 binaries
rem RS3 and older can only use the Fallback layer and RS5 and newer already has DXR built-in
if %BuildVersion% GEQ 17093 if %BuildVersion% LEQ 17623 set UseRS4Binaries=1

rem Copy Dxcompiler, needed regardless of whether it's using native DXR or the Fallback layer
set outputDirectory=%1
echo %1
copy dxrfallbackcompiler.dll %outputDirectory%

if %UseRS4Binaries% NEQ 0 ( 
	echo RS4-DXR compatible build detected, copying DXR D3D12 binaries
   copy d3d12.dll %outputDirectory%
   copy dxgidebug.dll %outputDirectory%
   copy d3d12sdklayers.dll %outputDirectory%
)
