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
#include "D3D12Fullscreen.h"

[Platform::MTAThread]
int WINAPIV main(Platform::Array<Platform::String^>^ /*params*/)
{
	D3D12Fullscreen sample(1200, 900, L"Press the SPACE bar to toggle fullscreen mode and use the left and right arrow keys to change the rendering resolution");
	auto viewProvider = ref new ViewProvider(reinterpret_cast<UINT_PTR>(&sample));

	Windows::ApplicationModel::Core::CoreApplication::Run(viewProvider);
	return 0;
}
