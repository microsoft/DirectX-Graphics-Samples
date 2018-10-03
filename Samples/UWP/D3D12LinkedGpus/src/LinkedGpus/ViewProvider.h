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

#pragma once

using namespace Windows::ApplicationModel::Core;

ref class ViewProvider sealed : IFrameworkViewSource
{
public:
    ViewProvider(UINT_PTR pSample);
    virtual IFrameworkView^ CreateView();

private:
    UINT_PTR m_pSample;
};
