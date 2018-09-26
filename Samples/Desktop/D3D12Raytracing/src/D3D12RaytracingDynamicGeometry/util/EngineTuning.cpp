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

#include "stdafx.h"
#include "EngineTuning.h"
#include "GameInput.h"
using namespace std;

wstring FormattedString(const wchar_t* format, ...)
{
	wchar_t buffer[256];
	va_list ap;
	va_start(ap, format);
	vswprintf(buffer, 256, format, ap);
	return wstring(buffer);
}

namespace EngineTuning
{
    // For delayed registration.  Some objects are constructed before we can add them to the graph (due
    // to unreliable order of initialization.)
    enum { kMaxUnregisteredTweaks = 1024 };
    WCHAR s_UnregisteredPath[kMaxUnregisteredTweaks][128];
    EngineVar* s_UnregisteredVariable[kMaxUnregisteredTweaks] = { nullptr };
    int32_t s_UnregisteredCount = 0;

    float s_ScrollOffset = 0.0f;
    float s_ScrollTopTrigger = 1080.0f * 0.2f;
    float s_ScrollBottomTrigger = 1080.0f * 0.8f;

    // Internal functions
    void AddToVariableGraph( const wstring& path, EngineVar& var );
    void RegisterVariable( const wstring& path, EngineVar& var );

    EngineVar* sm_SelectedVariable = nullptr;
    bool sm_IsVisible = true;
}

// Not open to the public.  Groups are auto-created when a tweaker's path includes the group name.
class VariableGroup : public EngineVar
{
public:
    VariableGroup() : m_IsExpanded(false) {}

    EngineVar* FindChild( const wstring& name )
    {
        auto iter = m_Children.find(name);
        return iter == m_Children.end() ? nullptr : iter->second;
    }
     
    void AddChild( const wstring& name, EngineVar& child )
    {
        m_Children[name] = &child;
        child.m_GroupPtr = this;
    }

    void Display(wstringstream* renderText, UINT leftMargin, EngineVar* highlightedTweak );

    void SaveToFile( FILE* file, int fileMargin );
    void LoadSettingsFromFile( FILE* file );

    EngineVar* NextVariable( EngineVar* currentVariable );
    EngineVar* PrevVariable( EngineVar* currentVariable );
    EngineVar* FirstVariable( void );
    EngineVar* LastVariable( void );

    bool IsExpanded( void ) const { return m_IsExpanded; }

    virtual void Increment( void ) override { m_IsExpanded = true; }
    virtual void Decrement( void ) override { m_IsExpanded = false; }
    virtual void Bang( void ) override { m_IsExpanded = !m_IsExpanded; }

    virtual void SetValue( FILE*, const wstring& ) override {}
    
    static VariableGroup sm_RootGroup;

private:
    bool m_IsExpanded;
    map<wstring, EngineVar*> m_Children;
};

VariableGroup VariableGroup::sm_RootGroup;

//=====================================================================================================================
// VariableGroup implementation

wstring Indent(UINT spaces)
{
	wstringstream indent;
	for (UINT i = 0; i < spaces; i++)
	{
		indent << L" ";
	}
	return indent.str();
}

void VariableGroup::Display( wstringstream* renderText, UINT leftMargin, EngineVar* highlightedTweak )
{
    for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
    {
		*renderText << ((iter->second == highlightedTweak) ? L"[x] " : L"[] ");
	
        VariableGroup* subGroup = dynamic_cast<VariableGroup*>(iter->second);
        if (subGroup != nullptr)
        {
            if (subGroup->IsExpanded())
            {
                *renderText << Indent(leftMargin) << L"- ";
            }
            else
            {
                *renderText << L"+ ";				
            }
            *renderText << iter->first;
            *renderText << L"/...\n";

            if (subGroup->IsExpanded())
            {
                subGroup->Display(renderText, leftMargin + 1, highlightedTweak);
            }
        }
        else
        {
			*renderText << Indent(leftMargin) 
						<< iter->first << L": " << iter->second->ToFormattedString()
						<< L"\n";
        }
        
    }
}

void VariableGroup::SaveToFile( FILE* file, int fileMargin )
{
    for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
    {
        const WCHAR* buffer = (iter->first).c_str();

        VariableGroup* subGroup = dynamic_cast<VariableGroup*>(iter->second);
        if (subGroup != nullptr)
        {		
            fwprintf(file, L"%*c + %s ...\r\n", fileMargin, L' ', buffer);
            subGroup->SaveToFile(file, fileMargin + 5);
        }
        else if (dynamic_cast<CallbackTrigger*>(iter->second) == nullptr)
        {
            fwprintf(file, L"%*c %s:  %s\r\n", fileMargin, L' ', buffer, iter->second->ToString().c_str());
        }		
    }
}

void VariableGroup::LoadSettingsFromFile( FILE* file )
{
    for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
    {
        VariableGroup* subGroup = dynamic_cast<VariableGroup*>(iter->second);
        if (subGroup != nullptr)
        {
            WCHAR skippedLines[100];
            fwscanf_s(file, L"%*s %[^\n]", skippedLines, (int)_countof(skippedLines));
            subGroup->LoadSettingsFromFile(file);
        }
        else
        {	
            iter->second->SetValue(file, iter->first);
        }
    }
}

EngineVar* VariableGroup::FirstVariable( void )
{
    return m_Children.size() == 0 ? nullptr : m_Children.begin()->second;
}

EngineVar* VariableGroup::LastVariable( void )
{
    if (m_Children.size() == 0)
        return this;

    auto LastVariable = m_Children.end();
    --LastVariable;

    VariableGroup* isGroup = dynamic_cast<VariableGroup*>(LastVariable->second);
    if (isGroup && isGroup->IsExpanded())
        return isGroup->LastVariable();

    return LastVariable->second;
}

EngineVar* VariableGroup::NextVariable( EngineVar* curVar )
{
    auto iter = m_Children.begin();
    for (; iter != m_Children.end(); ++iter)
    {
        if (curVar == iter->second)
            break;
    }

    ThrowIfFailed( iter != m_Children.end(), L"Did not find engine variable in its designated group" );

    auto nextIter = iter;
    ++nextIter;

    if (nextIter == m_Children.end())
        return m_GroupPtr ? m_GroupPtr->NextVariable(this) : nullptr;
    else
        return nextIter->second;
}

EngineVar* VariableGroup::PrevVariable( EngineVar* curVar )
{
    auto iter = m_Children.begin();
    for (; iter != m_Children.end(); ++iter)
    {
        if (curVar == iter->second)
            break;
    }

    ThrowIfFailed( iter != m_Children.end(), L"Did not find engine variable in its designated group" );

    if (iter == m_Children.begin())
        return this;

    auto prevIter = iter;
    --prevIter;

    VariableGroup* isGroup = dynamic_cast<VariableGroup*>(prevIter->second);
    if (isGroup && isGroup->IsExpanded())
        return isGroup->LastVariable();

    return prevIter->second;
}

//=====================================================================================================================
// EngineVar implementations

EngineVar::EngineVar(function<void(void*)> callback, void* args) :
	m_GroupPtr(nullptr),
	m_Callback(callback),
	m_Arguments(args)
{
}

EngineVar::EngineVar( const wstring& path, function<void(void*)> callback, void* args) : 
	m_GroupPtr(nullptr),
	m_Callback(callback),
	m_Arguments(args)
{
    EngineTuning::RegisterVariable(path, *this);
}

EngineVar* EngineVar::NextVar( void )
{
    EngineVar* next = nullptr;
    VariableGroup* isGroup = dynamic_cast<VariableGroup*>(this);
    if (isGroup != nullptr && isGroup->IsExpanded())
        next = isGroup->FirstVariable();

    if (next == nullptr)
        next = m_GroupPtr->NextVariable(this);

    return next != nullptr ? next : this;
}

EngineVar* EngineVar::PrevVar( void )
{
    EngineVar* prev = m_GroupPtr->PrevVariable(this);
    if (prev != nullptr && prev != m_GroupPtr)
    {
        VariableGroup* isGroup = dynamic_cast<VariableGroup*>(prev);
        if (isGroup != nullptr && isGroup->IsExpanded())
            prev = isGroup->LastVariable();
    }
    return prev != nullptr ? prev : this;
}

void EngineVar::OnChanged() 
{
	if (m_Callback)
	{
		m_Callback(m_Arguments);
	}
}


BoolVar::BoolVar( const wstring& path, bool val, function<void(void*)> callback, void* args)
    : EngineVar(path, callback, args)
{
    m_Flag = val;
}

wstring BoolVar::ToFormattedString() const
{
    return ToString();
}

wstring BoolVar::ToString( void ) const
{
    return m_Flag ? L"on" : L"off";
} 

void BoolVar::SetValue(FILE* file, const wstring& setting)
{	
    wstring pattern = L"\n L" + setting + L": %s";
    WCHAR valstr[6];

    // Search through the file for an entry that matches this setting's name
    fwscanf_s(file, pattern.c_str(), valstr, _countof(valstr));

    // Look for one of the many affirmations
    m_Flag = (
        0 == _wcsicmp(valstr, L"1") ||
        0 == _wcsicmp(valstr, L"on") ||
        0 == _wcsicmp(valstr, L"yes") ||
        0 == _wcsicmp(valstr, L"true") );
}

NumVar::NumVar( const wstring& path, float val, float minVal, float maxVal, float stepSize, function<void(void*)> callback, void* args)
    : EngineVar(path, callback, args)
{
    ThrowIfFailed(minVal <= maxVal);
    m_MinValue = minVal;
    m_MaxValue = maxVal;
    m_Value = Clamp(val);
    m_StepSize = stepSize;
}

wstring NumVar::ToFormattedString() const
{
    return FormattedString(L"%-11f", m_Value);
}

wstring NumVar::ToString( void ) const
{
    WCHAR buf[128];
    swprintf_s(buf, L"%f", m_Value);
    return buf;
} 

void NumVar::SetValue(FILE* file, const wstring& setting) 
{
    wstring scanString = L"\n" + setting + L": %f";
    float valueRead;
    
    //If we haven't read correctly, just keep m_Value at default value
    if (fwscanf_s(file, scanString.c_str(), &valueRead))
        *this = valueRead; 
}

#if _MSC_VER < 1800
__forceinline float log2( float x ) { return log(x) / log(2.0f); }
__forceinline float exp2( float x ) { return pow(2.0f, x); }
#endif

ExpVar::ExpVar( const wstring& path, float val, float minExp, float maxExp, float expStepSize, function<void(void*)> callback, void* args)
    : NumVar(path, log2(val), minExp, maxExp, expStepSize)
{
}

ExpVar& ExpVar::operator=( float val )
{
    m_Value = Clamp(log2(val));
    return *this;
}

ExpVar::operator float() const
{
    return exp2(m_Value);
}

wstring ExpVar::ToFormattedString() const
{
    return FormattedString(L"%-11f", (float)*this);
}

wstring ExpVar::ToString( void ) const
{
    WCHAR buf[128];
    swprintf_s(buf, L"%f", (float)*this);
    return buf;
} 

void ExpVar::SetValue(FILE* file, const wstring& setting) 
{
    wstring scanString = L"\n" + setting + L": %f";
    float valueRead;
    
    //If we haven't read correctly, just keep m_Value at default value
    if (fwscanf_s(file, scanString.c_str(), &valueRead))
        *this = valueRead;
}

IntVar::IntVar( const wstring& path, int32_t val, int32_t minVal, int32_t maxVal, int32_t stepSize, function<void(void*)> callback, void* args)
    : EngineVar(path, callback, args)
{
    ThrowIfFailed(minVal <= maxVal);
    m_MinValue = minVal;
    m_MaxValue = maxVal;
    m_Value = Clamp(val);
    m_StepSize = stepSize;
}

wstring IntVar::ToFormattedString() const
{
    return FormattedString(L"%-11d", m_Value);
}

wstring IntVar::ToString( void ) const
{
    WCHAR buf[128];
    swprintf_s(buf, L"%d", m_Value);
    return buf;
} 

void IntVar::SetValue(FILE* file, const wstring& setting) 
{
    wstring scanString = L"\n" + setting + L": %d";
    int32_t valueRead;
    
    if (fwscanf_s(file, scanString.c_str(), &valueRead))
        *this = valueRead;
}


EnumVar::EnumVar( const wstring& path, int32_t initialVal, int32_t listLength, const WCHAR** listLabels, function<void(void*)> callback, void* args)
    : EngineVar(path, callback, args)
{
    ThrowIfFailed(listLength > 0);
    m_EnumLength = listLength;
    m_EnumLabels = listLabels;
    m_Value = Clamp(initialVal);
}

wstring EnumVar::ToFormattedString() const
{
    return m_EnumLabels[m_Value];
}

wstring EnumVar::ToString( void ) const
{
    return m_EnumLabels[m_Value];
} 

void EnumVar::SetValue(FILE* file, const wstring& setting) 
{
    wstring scanString = L"\n" + setting + L": %[^\n]";
    WCHAR valueRead[14];
        
    if (fwscanf_s(file, scanString.c_str(), valueRead, _countof(valueRead)) == 1)
    {
        wstring valueReadStr = valueRead;
        valueReadStr = valueReadStr.substr(0, valueReadStr.length() - 1);

        //if we don't find the wstring, then leave m_EnumLabes[m_Value] as default
        for(int32_t i = 0; i < m_EnumLength; ++i)
        {
            if (m_EnumLabels[i] == valueReadStr)
            {
                m_Value = i;
                break;
            }
        }
    }

}

CallbackTrigger::CallbackTrigger( const wstring& path, function<void (void*)> callback, void* args )
    : EngineVar(path, callback, args)
{
    m_BangDisplay = 0;
}

wstring CallbackTrigger::ToFormattedString() const
{
	if (m_BangDisplay > 0)
		--m_BangDisplay;

    static const WCHAR s_animation[] = { L'-', L'\\', L'|', L'/' };
    return FormattedString(L"[%c]", s_animation[(m_BangDisplay >> 3) & 3]);
}

void CallbackTrigger::SetValue(FILE* file, const wstring& setting) 
{
    //Skip over setting without reading anything
    wstring scanString = L"\n" + setting + L": %[^\n]";
    WCHAR skippedLines[100];
    fwscanf_s(file, scanString.c_str(), skippedLines, _countof(skippedLines));
}

//=====================================================================================================================
// EngineTuning namespace methods

void EngineTuning::Initialize( void )
{
    for (int32_t i = 0; i < s_UnregisteredCount; ++i)
    {
        ThrowIfFailed(wcslen(s_UnregisteredPath[i]) > 0, L"Register = %d\n", i);
        ThrowIfFailed(s_UnregisteredVariable[i] != nullptr);
        AddToVariableGraph(s_UnregisteredPath[i], *s_UnregisteredVariable[i]);
    }
    s_UnregisteredCount = -1;
}

void HandleDigitalButtonPress( GameInput::DigitalInput button, float timeDelta, function<void ()> action )
{
    if (!GameInput::IsPressed(button))
        return;

    float durationHeld = GameInput::GetDurationPressed(button);

    // Tick on the first press
    if (durationHeld == 0.0f)
    {
        action();
        return;
    }

    // After ward, tick at fixed intervals
    float oldDuration = durationHeld - timeDelta;

    // Before 2 seconds, use slow scale (200ms/tick), afterward use fast scale (50ms/tick).
    float timeStretch = durationHeld < 2.0f ? 5.0f : 20.0f;

    if (floor(durationHeld * timeStretch) > floor(oldDuration * timeStretch))
        action();
}

void EngineTuning::Update( float frameTime )
{
    if (GameInput::IsFirstPressed( GameInput::kBackButton )
        || GameInput::IsFirstPressed( GameInput::kKey_back ))
        sm_IsVisible = !sm_IsVisible;

    if (!sm_IsVisible)
        return;

    if (sm_SelectedVariable == nullptr || sm_SelectedVariable == &VariableGroup::sm_RootGroup)
        sm_SelectedVariable = VariableGroup::sm_RootGroup.FirstVariable();

    if (sm_SelectedVariable == nullptr)
        return;

    // Detect a DPad button press
    HandleDigitalButtonPress(GameInput::kDPadRight, frameTime, []{ sm_SelectedVariable->Increment(); } );
    HandleDigitalButtonPress(GameInput::kDPadLeft,	frameTime, []{ sm_SelectedVariable->Decrement(); } );
    HandleDigitalButtonPress(GameInput::kDPadDown,	frameTime, []{ sm_SelectedVariable = sm_SelectedVariable->NextVar(); } );
    HandleDigitalButtonPress(GameInput::kDPadUp,	frameTime, []{ sm_SelectedVariable = sm_SelectedVariable->PrevVar(); } );

    HandleDigitalButtonPress(GameInput::kKey_right, frameTime, []{ sm_SelectedVariable->Increment(); } );
    HandleDigitalButtonPress(GameInput::kKey_left,	frameTime, []{ sm_SelectedVariable->Decrement(); } );
    HandleDigitalButtonPress(GameInput::kKey_down,	frameTime, []{ sm_SelectedVariable = sm_SelectedVariable->NextVar(); } );
    HandleDigitalButtonPress(GameInput::kKey_up,	frameTime, []{ sm_SelectedVariable = sm_SelectedVariable->PrevVar(); } );

    if (GameInput::IsFirstPressed( GameInput::kAButton )
        || GameInput::IsFirstPressed( GameInput::kKey_return ))
    {
        sm_SelectedVariable->Bang();
    }
}

void StartSave(void*)
{
    FILE* settingsFile;
    _wfopen_s(&settingsFile, L"engineTuning.txt", L"wb");
    if (settingsFile != nullptr)
    {
        VariableGroup::sm_RootGroup.SaveToFile(settingsFile, 2 );
        fclose(settingsFile);
    }
}
function<void(void*)> StartSaveFunc = StartSave;
//ToDo static CallbackTrigger Save(L"Save Settings", StartSaveFunc, nullptr); 

void StartLoad(void*)
{
    FILE* settingsFile;
    _wfopen_s(&settingsFile, L"engineTuning.txt", L"rb");
    if (settingsFile != nullptr)
    {
        VariableGroup::sm_RootGroup.LoadSettingsFromFile(settingsFile);
        fclose(settingsFile);
    }
}
function<void(void*)> StartLoadFunc = StartLoad;
//ToDo static CallbackTrigger Load(L"Load Settings", StartLoadFunc, nullptr); 


void EngineTuning::Display( wstringstream* renderText)
{
	*renderText << L"Engine Tuning (use arrow keys)\n";
    VariableGroup::sm_RootGroup.Display( renderText, 0, sm_SelectedVariable );    
}

void EngineTuning::AddToVariableGraph( const wstring& path, EngineVar& var )
{
    vector<wstring> separatedPath;
    wstring leafName;
    size_t start = 0, end = 0;

    while (1)
    {
        end = path.find('/', start);
        if (end == wstring::npos)
        {
            leafName = path.substr(start);
            break;
        }
        else
        {
            separatedPath.push_back(path.substr(start, end - start));
            start = end + 1;
        }
    }

    VariableGroup* group = &VariableGroup::sm_RootGroup;

    for (auto iter = separatedPath.begin(); iter != separatedPath.end(); ++iter )
    {
        VariableGroup* nextGroup;
        EngineVar* node = group->FindChild(*iter);
        if (node == nullptr)
        {
            nextGroup = new VariableGroup();
            group->AddChild(*iter, *nextGroup);
            group = nextGroup;
        }
        else
        {
            nextGroup = dynamic_cast<VariableGroup*>(node);
            ThrowIfFailed(nextGroup != nullptr, L"Attempted to trash the tweak graph");
            group = nextGroup;
        }
    }

    group->AddChild(leafName, var);
}

void EngineTuning::RegisterVariable( const wstring& path, EngineVar& var )
{
    if (s_UnregisteredCount >= 0)
    {
        int32_t Idx = s_UnregisteredCount++;
        wcscpy_s(s_UnregisteredPath[Idx], path.c_str());
        s_UnregisteredVariable[Idx] = &var;
    }
    else
    {
        AddToVariableGraph( path, var );
    }
}

bool EngineTuning::IsFocused( void )
{
    return sm_IsVisible;
}
