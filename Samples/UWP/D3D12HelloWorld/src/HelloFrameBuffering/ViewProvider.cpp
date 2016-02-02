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
#include "View.h"

ViewProvider::ViewProvider(UINT_PTR pSample) :
	m_pSample(pSample)
{
}

Windows::ApplicationModel::Core::IFrameworkView^ ViewProvider::CreateView()
{
	return ref new View(m_pSample);
}
