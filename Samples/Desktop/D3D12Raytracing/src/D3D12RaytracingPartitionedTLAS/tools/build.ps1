#*********************************************************
#
# Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# This code is licensed under the MIT License (MIT).
# THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
# IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
# PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
#
#*********************************************************

param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [string]$Dxr2SdkRoot,
    [string]$ArtifactsRoot
)
$ErrorActionPreference = 'Stop'
$sampleRoot = Split-Path $PSScriptRoot -Parent
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (!$msbuild) { throw 'Install Visual Studio 2022 with Desktop development with C++.' }
$arguments = @((Join-Path $sampleRoot 'D3D12RaytracingPartitionedTLAS.sln'), '/restore', '/m',
    '/p:RestorePackagesConfig=true', "/p:Configuration=$Configuration", '/p:Platform=x64')
if ($Dxr2SdkRoot) { $arguments += "/p:Dxr2SdkRoot=$Dxr2SdkRoot" }
if ($ArtifactsRoot) { $arguments += "/p:Dxr2ArtifactsRoot=$([IO.Path]::GetFullPath($ArtifactsRoot))" }
& $msbuild @arguments
exit $LASTEXITCODE
