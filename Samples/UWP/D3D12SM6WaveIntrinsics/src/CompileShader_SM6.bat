@ECHO OFF
:: Search dxc.exe on current PATH. 
where dxc.exe >nul 2>nul
IF %ERRORLEVEL%==0 (
    SET dxcCmd=dxc.exe
	GOTO COMPILE_SHADER
)

:: Search dxc.exe on 15063 SDK installtion path.
dir "%PROGRAMFILES(x86)%\Windows Kits\10\bin\10.0.15063.0\x86\dxc.exe" >nul 2>nul
IF %ERRORLEVEL%==0 (
	SET dxcCmd="%PROGRAMFILES(x86)%\Windows Kits\10\bin\10.0.15063.0\x86\dxc.exe"
	GOTO COMPILE_SHADER
)

:DXC_NOT_FOUND
ECHO Error: dxc.exe does not exist somewhere on PATH or on %PROGRAMFILES(x86)%\Windows Kits\10\bin\10.0.15063.0\x86\
EXIT /b 1

:COMPILE_SHADER
ECHO DXC Path: %dxcCmd%
ECHO Start compiling shaders...
ECHO ON
%dxcCmd% /Zi /E"VSMain" /Vn"g_Wave_VS" /Tvs_6_0 /Fh"wave_vs.hlsl.h" /nologo wave.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /D_WAVE_OP /Zi /E"PSMain" /Vn"g_Wave_PS" /Tps_6_0 /Fh"wave_ps.hlsl.h" /nologo wave.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"VSMain" /Vn"g_Magnify_VS" /Tvs_6_0 /Fh"magnify_vs.hlsl.h" /nologo magnify.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /D_WAVE_OP /Zi /E"PSMain" /Vn"g_Magnify_PS" /Tps_6_0 /Fh"magnify_ps.hlsl.h" /nologo magnify.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
ECHO Done.
