dxc.exe /D_WAVE_OP /Zi /E"main" /Vn"g_pModelViewerPS_SM6" /Tps_6_0 /Fh"ModelViewerPS_SM6.h" /nologo Shaders/ModelViewerPS.hlsl

copy ModelViewerPS_SM6.h ..\Build_VS14\x64\Debug\Output\ModelViewer\CompiledShaders
copy ModelViewerPS_SM6.h ..\Build_VS14\x64\Profile\Output\ModelViewer\CompiledShaders
copy ModelViewerPS_SM6.h ..\Build_VS14\x64\Release\Output\ModelViewer\CompiledShaders

dxc.exe /Zi /E"main" /Vn"g_pModelViewerVS_SM6" /Tvs_6_0 /Fh"ModelViewerVS_SM6.h" /nologo Shaders/ModelViewerVS.hlsl

copy ModelViewerVS_SM6.h ..\Build_VS14\x64\Debug\Output\ModelViewer\CompiledShaders
copy ModelViewerVS_SM6.h ..\Build_VS14\x64\Profile\Output\ModelViewer\CompiledShaders
copy ModelViewerVS_SM6.h ..\Build_VS14\x64\Release\Output\ModelViewer\CompiledShaders

dxc.exe /Zi /E"main" /Vn"g_pDepthViewerVS_SM6" /Tvs_6_0 /Fh"DepthViewerVS_SM6.h" /nologo Shaders/DepthViewerVS.hlsl

copy DepthViewerVS_SM6.h ..\Build_VS14\x64\Debug\Output\ModelViewer\CompiledShaders
copy DepthViewerVS_SM6.h ..\Build_VS14\x64\Profile\Output\ModelViewer\CompiledShaders
copy DepthViewerVS_SM6.h ..\Build_VS14\x64\Release\Output\ModelViewer\CompiledShaders
