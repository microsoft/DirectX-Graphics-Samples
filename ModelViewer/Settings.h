//=================================================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "pch.h"
#include "SF12_Math.h"

namespace SampleFramework12
{

class FloatSetting;
class IntSetting;
class BoolSetting;
class EnumSetting;
class DirectionSetting;
class OrientationSetting;
class ColorSetting;
class Button;

enum class SettingType
{
    Float = 0,
    Int = 1,
    Bool = 2,
    Enum = 3,
    Direction = 4,
    Orientation = 5,
    Color = 6,
    Button = 7,

    Invalid,
    NumTypes = Invalid
};

enum class ConversionMode
{
    None = 0,
    Square = 1,
    SquareRoot = 2,
    DegToRadians = 3,
};

enum class ColorUnit
{
    None = 0,
    Luminance = 1,
    Illuminance = 2,
    LuminousPower = 3,
    EV100 = 4,
};

enum class DirectionInputMode
{
    Cartesian = 0,
    Spherical
};

// Base class for all setting types
class Setting
{

protected:

    SettingType type = SettingType::Invalid;
    void* data = nullptr;
    std::string name;
    std::string group;
    std::string label;
    std::string helpText;
    bool changed = false;
    bool initialized = false;
    bool visible = true;

    void Initialize(SettingType type, void* data, const char* name,
                    const char* group, const char* label, const char* helpText);

public:

    Setting();

    virtual void Update(const Float4x4& viewMatrix) = 0;

    virtual void SetReadOnly(bool readOnly);
    void SetEditable(bool editable);
    void SetHidden(bool hidden);
    void SetVisible(bool visible);
    void SetLabel(const char* label);

    DirectionSetting& AsDirection();

    bool Changed() const;
    bool Visible() const;
    const std::string& Name() const;
    const std::string& Group() const;
};

// 3D direction setting
class DirectionSetting : public Setting
{

private:

    Float3 val;
    Float3 oldVal;
    Float2 spherical;

    std::string buttonName;
    std::string childName;
    Float2 lastDragDelta;
    bool wasDragged = false;
    bool convertToViewSpace = false;
    DirectionInputMode inputMode = DirectionInputMode::Cartesian;

public:

    DirectionSetting();

    void Initialize(const char* name, const char* group, const char* label, const char* helpText,
                    Float3 initialVal, bool convertToViewSpace);

    virtual void Update(const Float4x4& viewMatrix) override;

    Float3 Value() const;
    void SetValue(Float3 newVal);
    operator Float3();

    template<typename TSerializer> void SerializeValue(TSerializer& serializer)
    {
        Assert_(initialized);
        SerializeItem(serializer, val);
    }
};

}