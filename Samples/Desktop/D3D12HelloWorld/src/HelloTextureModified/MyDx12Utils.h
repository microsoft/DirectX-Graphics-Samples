#pragma once

namespace MyDx12Util {

	void CreateUploadBuffer(ComPtr<ID3D12Device>& device, size_t size, ComPtr<ID3D12Resource>& ppResource)
    {
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(size),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&ppResource)));

	}
	




}