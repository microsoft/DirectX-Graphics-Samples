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
echo Renaming dxc->fxc so that Visual Studio can natively work with HLSL files and still compile them to SM 6.1
copy dxc.exe fxc.exe

if %FILENOTFOUND% NEQ 0 (
	echo Please go to https://github.com/Microsoft/DirectX-Graphics-Samples/releases and download "DirectX Raytracing Binary Release v1.0". 
	echo Copy all the binaries to Samples\Desktop\D3D12Raytracing\tools\x64"
	exit /b 1
)
