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

class VariableGroup;

class EngineVar
{
public:
    virtual ~EngineVar() {}

    virtual void Increment() {}	// DPad Right
    virtual void Decrement() {}	// DPad Left
    virtual void Bang() {}		// A Button

    virtual std::wstring ToFormattedString() const { return L""; }
    virtual std::wstring ToString() const { return L""; }
    virtual void SetValue(FILE* file, const std::wstring& setting) = 0; //set value read from file

    EngineVar* NextVar();
    EngineVar* PrevVar();

protected:
    EngineVar(std::function<void(void*)> callback = nullptr, void* args = nullptr);
    EngineVar(const std::wstring& path, std::function<void(void*)> callback = nullptr, void* args = nullptr);

    void Initialize(const std::wstring& path, std::function<void(void*)> callback = nullptr, void* args = nullptr);

	void OnChanged(bool callCallback = true);

	std::function<void(void*)> m_Callback;
	void* m_Arguments;
private:
    friend class VariableGroup;
    VariableGroup* m_GroupPtr;

};

class BoolVar : public EngineVar
{
public:
    BoolVar() : EngineVar() {}
    BoolVar(const std::wstring& path, bool val, std::function<void(void*)> callback = nullptr, void* args = nullptr);
    BoolVar& operator=(bool val) { m_Flag = val; return *this; }
    operator bool() const { return m_Flag; }

    void Initialize(const std::wstring& path, bool val, std::function<void(void*)> callback = nullptr, void* args = nullptr);

	virtual void Increment() override { m_Flag = true; OnChanged(); }
    virtual void Decrement() override { m_Flag = false; OnChanged(); }
    virtual void Bang() override { m_Flag = !m_Flag; OnChanged(); }

    virtual std::wstring ToFormattedString() const override;
    virtual std::wstring ToString() const override;
    virtual void SetValue(FILE* file, const std::wstring& setting) override;
    virtual void SetValue(bool value, bool callCallback = true);

private:
    using EngineVar::Initialize;

    bool m_Flag;
};

class NumVar : public EngineVar
{
public:
    NumVar() : EngineVar() {}
    NumVar(const std::wstring& path, float val, float minValue = -FLT_MAX, float maxValue = FLT_MAX, float stepSize = 1.0f, std::function<void(void*)> callback = nullptr, void* args = nullptr);
    NumVar& operator=(float val) { m_Value = Clamp(val); return *this; }
    operator float() const { return m_Value; }

    void Initialize(const std::wstring& path, float val, float minValue = -FLT_MAX, float maxValue = FLT_MAX, float stepSize = 1.0f, std::function<void(void*)> callback = nullptr, void* args = nullptr);

    virtual void Increment() override { m_Value = Clamp(m_Value + m_StepSize); OnChanged(); }
    virtual void Decrement() override { m_Value = Clamp(m_Value - m_StepSize); OnChanged(); }

    virtual std::wstring ToFormattedString() const override;
    virtual std::wstring ToString() const override;
    virtual void SetValue(FILE* file, const std::wstring& setting)  override;  
    virtual void SetValue(float value, bool callCallback = true);
    void SetMaxValue(float value) { m_MaxValue = value; m_MinValue = std::min(m_MinValue, value); }
    void SetMinValue(float value) { m_MinValue = value; m_MaxValue = std::max(m_MaxValue, value); }
    float MaxValue() const { return m_MaxValue; }
    float MinValue() const { return m_MinValue; }

private:
    using EngineVar::Initialize;

protected:
    float Clamp(float val) { return val > m_MaxValue ? m_MaxValue : val < m_MinValue ? m_MinValue : val; }

    float m_Value;
    float m_MinValue;
    float m_MaxValue;
    float m_StepSize;
};

class ExpVar : public NumVar
{
public:
    ExpVar() : NumVar() {}
    ExpVar(const std::wstring& path, float val, float minExp = -FLT_MAX, float maxExp = FLT_MAX, float expStepSize = 1.0f, std::function<void(void*)> callback = nullptr, void* args = nullptr);
    ExpVar& operator=(float val);	// m_Value = log2(val)
    operator float() const;			// returns exp2(m_Value)

    void Initialize(const std::wstring& path, float val, float minExp = -FLT_MAX, float maxExp = FLT_MAX, float expStepSize = 1.0f, std::function<void(void*)> callback = nullptr, void* args = nullptr);

    virtual std::wstring ToFormattedString() const override;
    virtual std::wstring ToString() const override;
    virtual void SetValue(FILE* file, const std::wstring& setting) override;

private:
    using NumVar::Initialize;
};

class IntVar : public EngineVar
{
public:
    IntVar() : EngineVar() {}
    IntVar(const std::wstring& path, int val, int minValue = 0, int maxValue = (1 << 24) - 1, int stepSize = 1, std::function<void(void*)> callback = nullptr, void* args = nullptr);
    IntVar& operator=(int val) { m_Value = Clamp(val); return *this; }
    operator int() const { return m_Value; }

    void Initialize(const std::wstring& path, int val, int minValue = 0, int maxValue = (1 << 24) - 1, int stepSize = 1, std::function<void(void*)> callback = nullptr, void* args = nullptr);

    virtual void Increment() override { m_Value = Clamp(m_Value + m_StepSize); OnChanged(); }
	virtual void Decrement() override { m_Value = Clamp(m_Value - m_StepSize); OnChanged(); }

    virtual std::wstring ToFormattedString() const override;
    virtual std::wstring ToString() const override;
    virtual void SetValue(FILE* file, const std::wstring& setting) override;
    virtual void SetValue(int value, bool callCallback = true);
    void SetMaxValue(int value) { m_MaxValue = value; m_MinValue = std::min(m_MinValue, value); }
    void SetMinValue(int value) { m_MinValue = value; m_MaxValue = std::max(m_MaxValue, value); }
    int MaxValue() const { return m_MaxValue; }
    int MinValue() const { return m_MinValue; }

private:
    using EngineVar::Initialize;

protected:
    int Clamp(int val) { return val > m_MaxValue ? m_MaxValue : val < m_MinValue ? m_MinValue : val; }

    int m_Value;
    int m_MinValue;
    int m_MaxValue;
    int m_StepSize;
};

class EnumVar : public EngineVar
{
public:
    EnumVar() : EngineVar() {}
    EnumVar(const std::wstring& path, int initialVal, int listLength, const WCHAR** listLabels, std::function<void(void*)> callback = nullptr, void* args = nullptr);
    EnumVar& operator=(int val) { m_Value = Clamp(val); return *this; }
    operator int() const { return m_Value; }

    void Initialize(const std::wstring& path, int initialVal, int listLength, const WCHAR** listLabels, std::function<void(void*)> callback = nullptr, void* args = nullptr);

    virtual void Increment() override { m_Value = (m_Value + 1) % m_EnumLength; OnChanged(); }
    virtual void Decrement() override { m_Value = (m_Value + m_EnumLength - 1) % m_EnumLength; OnChanged();	}

    virtual std::wstring ToFormattedString() const override;
    virtual std::wstring ToString() const override;
    virtual void SetValue(FILE* file, const std::wstring& setting) override;
    virtual void SetValue(int value, bool callCallback = true);

    void SetListLength(int listLength) { m_EnumLength = listLength; m_Value = Clamp(m_Value); }

private:
    using EngineVar::Initialize;
    int Clamp(int val) { return val < 0 ? 0 : val >= m_EnumLength ? m_EnumLength - 1 : val; }

    int m_Value;
    int m_EnumLength;
    const WCHAR** m_EnumLabels;
};

class CallbackTrigger : public EngineVar
{
public:
    CallbackTrigger(const std::wstring& path, std::function<void (void*)> callback, void* args = nullptr);

    virtual void Bang() override { OnChanged(); m_BangDisplay = 64; }

    virtual std::wstring ToFormattedString() const override;
    virtual void SetValue(FILE* file, const std::wstring& setting) override;

private:
    mutable UINT m_BangDisplay;
};

namespace EngineTuning
{
    void Initialize();
    void Update(float frameTime);
    void Display(std::wstringstream* renderText, bool expandAllNodes = false);
    bool IsFocused();
    void SetIsVisible(bool isVisible);

} // namespace EngineTuning
