set FILENOTFOUND=0

if not exist dxcompiler.dll (
   echo No dxcompiler.dll found!
   set FILENOTFOUND=1
)

if not exist d3d12.dll (
   echo No d3d12.dll found!
   set FILENOTFOUND=1
)

if not exist d3d12sdklayers.dll (
   echo No d3d12sdklayers.dll found!
   set FILENOTFOUND=1
)

if not exist dxgidebug.dll (
   echo No dxgidebug.dll found!
   set FILENOTFOUND=1
)

if not exist dxc.exe (
   echo No dxc.exe found!
   set FILENOTFOUND=1
)

if not exist dxrfallbackcompiler.dll (
   echo No dxrfallbackcompiler.dll found!
   echo Note that if you were using an older version of the Raytracing samples, the Fallback Layer has been changed to require dxrfallbackcompiler.dll instead of dxcompiler.dll. This can be pulled from the below linked release
   set FILENOTFOUND=1
)

echo Renaming dxc->fxc so that Visual Studio can natively work with HLSL files and still compile them to SM 6.1
copy dxc.exe fxc.exe

if %FILENOTFOUND% NEQ 0 (
	goto :mismatch
)

if not exist version.txt (
   goto :mismatch
) 

set /p version=< version.txt
if not "%version%"=="1.3-dxr" (
	goto :mismatch
)

exit

:mismatch
	echo Stale binaries detected, please grab the v1.3-dxr binaries from https://github.com/Microsoft/DirectX-Graphics-Samples/releases/tag/v1.3-dxr 
	echo Copy all the binaries to Samples\Desktop\D3D12Raytracing\tools\x64"
	exit /b 1
)
