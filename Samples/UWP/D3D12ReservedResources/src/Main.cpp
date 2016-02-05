//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "ViewProvider.h"
#include "D3D12ReservedResources.h"

[Platform::MTAThread]
int WINAPIV main(Platform::Array<Platform::String^>^ /*params*/)
{
	D3D12ReservedResources sample(1200, 900, L"Use arrow keys to cycle through resources");
	auto viewProvider = ref new ViewProvider(reinterpret_cast<UINT_PTR>(&sample));

	Windows::ApplicationModel::Core::CoreApplication::Run(viewProvider);
	return 0;
}
