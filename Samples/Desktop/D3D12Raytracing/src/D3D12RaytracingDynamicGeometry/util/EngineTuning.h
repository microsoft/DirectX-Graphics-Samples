//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once

#include <string>
#include <stdint.h>
#include <float.h>
#include <map>
#include <set>

class VariableGroup;
class TextContext;

class EngineVar
{
public:

    virtual ~EngineVar() {}

    virtual void Increment( void ) {}	// DPad Right
    virtual void Decrement( void ) {}	// DPad Left
    virtual void Bang( void ) {}		// A Button

    virtual void DisplayValue( TextContext& ) const {}
    virtual std::string ToString( void ) const { return ""; }
    virtual void SetValue( FILE* file, const std::string& setting) = 0; //set value read from file

    EngineVar* NextVar( void );
    EngineVar* PrevVar( void );

protected:
    EngineVar( void );
    EngineVar( const std::string& path );

private:
    friend class VariableGroup;
    VariableGroup* m_GroupPtr;
};

class BoolVar : public EngineVar
{
public:
    BoolVar( const std::string& path, bool val );
    BoolVar& operator=( bool val ) { m_Flag = val; return *this; }
    operator bool() const { return m_Flag; }

    virtual void Increment( void ) override { m_Flag = true; }
    virtual void Decrement( void ) override { m_Flag = false; }
    virtual void Bang( void ) override { m_Flag = !m_Flag; }

    virtual void DisplayValue( TextContext& Text ) const override;
    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting) override;

private:
    bool m_Flag;
};

class NumVar : public EngineVar
{
public:
    NumVar( const std::string& path, float val, float minValue = -FLT_MAX, float maxValue = FLT_MAX, float stepSize = 1.0f );
    NumVar& operator=( float val ) { m_Value = Clamp(val); return *this; }
    operator float() const { return m_Value; }

    virtual void Increment( void ) override { m_Value = Clamp(m_Value + m_StepSize); }
    virtual void Decrement( void ) override { m_Value = Clamp(m_Value - m_StepSize); }

    virtual void DisplayValue( TextContext& Text ) const override;
    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting)  override;

protected:
    float Clamp( float val ) { return val > m_MaxValue ? m_MaxValue : val < m_MinValue ? m_MinValue : val; }

    float m_Value;
    float m_MinValue;
    float m_MaxValue;
    float m_StepSize;
};

class ExpVar : public NumVar
{
public:
    ExpVar( const std::string& path, float val, float minExp = -FLT_MAX, float maxExp = FLT_MAX, float expStepSize = 1.0f );
    ExpVar& operator=( float val );	// m_Value = log2(val)
    operator float() const;			// returns exp2(m_Value)

    virtual void DisplayValue( TextContext& Text ) const override;
    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting ) override;

};

class IntVar : public EngineVar
{
public:
    IntVar( const std::string& path, int32_t val, int32_t minValue = 0, int32_t maxValue = (1 << 24) - 1, int32_t stepSize = 1 );
    IntVar& operator=( int32_t val ) { m_Value = Clamp(val); return *this; }
    operator int32_t() const { return m_Value; }

    virtual void Increment( void ) override { m_Value = Clamp(m_Value + m_StepSize); }
    virtual void Decrement( void ) override { m_Value = Clamp(m_Value - m_StepSize); }

    virtual void DisplayValue( TextContext& Text ) const override;
    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting ) override;

protected:
    int32_t Clamp( int32_t val ) { return val > m_MaxValue ? m_MaxValue : val < m_MinValue ? m_MinValue : val; }

    int32_t m_Value;
    int32_t m_MinValue;
    int32_t m_MaxValue;
    int32_t m_StepSize;
};

class EnumVar : public EngineVar
{
public:
    EnumVar( const std::string& path, int32_t initialVal, int32_t listLength, const char** listLabels );
    EnumVar& operator=( int32_t val ) { m_Value = Clamp(val); return *this; }
    operator int32_t() const { return m_Value; }

    virtual void Increment( void ) override { m_Value = (m_Value + 1) % m_EnumLength; }
    virtual void Decrement( void ) override { m_Value = (m_Value + m_EnumLength - 1) % m_EnumLength; }

    virtual void DisplayValue( TextContext& Text ) const override;
    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting ) override;

    void SetListLength(int32_t listLength) { m_EnumLength = listLength; m_Value = Clamp(m_Value); }

private:
    int32_t Clamp( int32_t val ) { return val < 0 ? 0 : val >= m_EnumLength ? m_EnumLength - 1 : val; }

    int32_t m_Value;
    int32_t m_EnumLength;
    const char** m_EnumLabels;
};

class CallbackTrigger : public EngineVar
{
public:
    CallbackTrigger( const std::string& path, std::function<void (void*)> callback, void* args = nullptr );

    virtual void Bang( void ) override { m_Callback(m_Arguments); m_BangDisplay = 64; }

    virtual void DisplayValue( TextContext& Text ) const override;
    virtual void SetValue( FILE* file, const std::string& setting ) override;

private:
    std::function<void (void*)> m_Callback;
    void* m_Arguments;
    mutable uint32_t m_BangDisplay;
};

class GraphicsContext;

namespace EngineTuning
{
    void Initialize( void );
    void Update( float frameTime );
    void Display( GraphicsContext& Context, float x, float y, float w, float h );
    bool IsFocused( void );

} // namespace EngineTuning
