#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{

void CreateRootSignature(ID3D12Device* device,
                         UINT textureSrvCount,
                         UINT gbufferSrvCount,
                         Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature);

} // namespace Engine
