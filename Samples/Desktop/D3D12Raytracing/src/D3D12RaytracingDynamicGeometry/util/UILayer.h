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

class UILayer
{
public:
    UILayer(UINT frameCount, ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue);

    void UpdateLabels(const std::wstring& uiText);
    void Render(UINT frameIndex);
    void ReleaseResources();
    void Resize(Microsoft::WRL::ComPtr<ID3D12Resource>* ppRenderTargets, UINT width, UINT height);

private:
    UINT FrameCount() { return static_cast<UINT>(m_wrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue);

    // Render target dimensions
    float m_width;
    float m_height;

    struct TextBlock
    {
        std::wstring text;
        D2D1_RECT_F layout;
        IDWriteTextFormat* pFormat;
    };

    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
    Microsoft::WRL::ComPtr<ID3D11On12Device> m_d3d11On12Device;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
    Microsoft::WRL::ComPtr<ID2D1Factory3> m_d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1Device2> m_d2dDevice;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
    std::vector<Microsoft::WRL::ComPtr<ID3D11Resource>> m_wrappedRenderTargets;
    std::vector<Microsoft::WRL::ComPtr<ID2D1Bitmap1>> m_d2dRenderTargets;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    std::vector<TextBlock> m_textBlocks;
};
/*
template <class T>
class UIParameter
{
public:
	UIParameter() {}
	virtual void Update(bool bIncrease) = 0;
	const T& GetValue() = 0;
};

template <class T>
class UIParameterRange : public UIParameter<T>
{
	T  m_value;
	T  m_minValue;
	T  m_maxValue;
	T  m_increment;
public:
	UIParameterRange(const T& initialValue, const T& minValue, const T& maxValue, const T& increment) :
		m_value(initialValue),
		m_minValue(minValue),
		m_maxValue(maxValue),
		m_increment(increment)
	{}

	void Initialize(const T& initialValue, const T& minValue, const T& maxValue, const T& increment)
	{
		m_value = initialValue;
		m_minValue = minValue;
		m_maxValue = maxValue;
		m_increment = increment;
	}

	const T& GetValue() { return m_value; }
	const T& GetMinValue() { return m_minValue; }
	const T& GetMaxValue() { return m_maxValue; }

	void SetValue(const T& value) { m_value = value; }
	void SetMinValue(const T& minValue) { m_minValue = minValue; }
	void SetMaxValue(const T& maxValue) { m_maxValue = maxValue; }

	void Update(bool bIncrease)
	{
		if (bIncrease)
		{
			if (m_maxValue - *m_pParameter > increment)
			{
				*m_pParameter += increment;
			}
			else
			{
				*m_pParameter = m_maxValue;
			}
		}
		else // decrease
		{
			if (*m_pParameter - m_minValue > increment)
			{
				*m_pParameter -= increment;
			}
			else
			{
				*m_pParameter = m_minValue;
			}
		}
	}
};

template<class T>
class UIParameterList : public UIParameter<T>
{
	std::vector<T> m_values;
	UIParameterRange<UINT> m_currentIndex;
public:
	UIParameterList() {}
	void SetValue(const T& value) 
	{
		T* p = std::find(m_values, m_values.end(), value);
		
		assert(p != m_values.end());
		UIParameterRange.SetValue(p - m_values);
	}
	void push_back(const T& parameter)
	{
		m_values.push_back(parameter);
		m_currentIndex.SetMaxValue(m_values.size() - 1);
	}
	void Update(bool bIncrease)
	{
		m_currentIndex.Update(bIncrease);
	}
};

*/