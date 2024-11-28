//=================================================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#include "pch.h"
#include "Settings.h"
#include "Utility.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "ImGuiHelper.h"

#define ArraySize_(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

namespace SampleFramework12
{

static Float2 CanvasTransform(Float2 pos, Float2 canvasStart, Float2 canvasSize)
{
    return canvasStart + canvasSize * (pos * Float2(0.5f, -0.5f) + Float2(0.5f));
}

// == Setting =====================================================================================

Setting::Setting()
{
}

void Setting::Initialize(SettingType type_, void* data_, const char* name_,
                         const char* group_, const char* label_, const char* helpText_)
{
    type = type_;
    data = data_;
    name = name_;
    group = group_;
    label = label_;
    helpText = helpText_;
    changed = false;

    initialized = true;
}

void Setting::SetReadOnly(bool readOnly)
{
    /*Assert_(initialized);
    TwHelper::SetReadOnly(tweakBar, name.c_str(), readOnly);*/
}

void Setting::SetEditable(bool editable)
{
    SetReadOnly(!editable);
}

void Setting::SetHidden(bool hidden)
{
    visible = !hidden;
}

void Setting::SetVisible(bool visible_)
{
    visible = visible_;
}

void Setting::SetLabel(const char* newLabel)
{
    //Assert_(newLabel != nullptr);
    label = newLabel;
}

DirectionSetting& Setting::AsDirection()
{
    //Assert_(type == SettingType::Direction);
    return *(static_cast<DirectionSetting*>(this));
}

bool Setting::Changed() const
{
    return changed;
}

bool Setting::Visible() const
{
    return visible;
}

const std::string& Setting::Name() const
{
    return name;
}

const std::string& Setting::Group() const
{
    return group;
}

// == DirectionSetting ============================================================================

DirectionSetting::DirectionSetting()
{
}

void DirectionSetting::Initialize(const char* name_, const char* group_, const char* label_,
                                  const char* helpText_, Float3 initialVal, bool convertToViewSpace_)
{
    val = Float3::Normalize(initialVal);
    oldVal = val;
    spherical = CartesianToSpherical(val);
    convertToViewSpace = convertToViewSpace_;
    Setting::Initialize(SettingType::Direction, &val, name_, group_, label_, helpText_);

    buttonName = name + "_CanvasButton";
    childName = name + "_Child";
}

void DirectionSetting::Update(const Float4x4& viewMatrix)
{
    const float WidgetSize = 75.0f;

    ImVec2 textSize = ImGui::CalcTextSize(label.c_str());

    ImGui::BeginChild(childName.c_str(), ImVec2(0.0f, WidgetSize + textSize.y * 4.5f), true);
    ImGui::BeginGroup();

    ImGui::Text(label.c_str());

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, WidgetSize + 35.0f);

    ImGui::InvisibleButton(buttonName.c_str(), ImVec2(WidgetSize, WidgetSize));

    if(ImGui::IsItemActive())
    {
        Float2 dragDelta = ToFloat2(ImGui::GetMouseDragDelta());
        Float2 rotAmt = dragDelta;

        if(wasDragged)
            rotAmt -= lastDragDelta;

        rotAmt *= 0.01f;
        Float3x3 rotation = Float3x3::RotationEuler(rotAmt.y, rotAmt.x, 0.0f);        
        if(convertToViewSpace)
        {
            Float3 dirVS = Float3::TransformDirection(val, viewMatrix);
            dirVS = Float3::Transform(dirVS, rotation);
            val = Float3::TransformDirection(dirVS, Float4x4::Transpose(viewMatrix));
        }
        else
            val = Float3::Transform(val, rotation);

        spherical = CartesianToSpherical(val);

        wasDragged = true;
        lastDragDelta = dragDelta;
    }
    else
        wasDragged = false;

    Float2 canvasStart = ToFloat2(ImGui::GetItemRectMin());
    Float2 canvasSize = ToFloat2(ImGui::GetItemRectSize());
    Float2 canvasEnd = ToFloat2(ImGui::GetItemRectMax());

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddCircleFilled(ToImVec2(canvasStart + WidgetSize * 0.5f), WidgetSize * 0.5f,  ImColor(0.5f, 0.5f, 0.5f, 0.5f), 32);

    Float3 drawDir = val;
    if(convertToViewSpace)
        drawDir = Float3::TransformDirection(val, viewMatrix);

    Float3x3 basis;
    basis.SetZBasis(drawDir);

    Float3 up = Float3(0.0f, 1.0f, 0.0f);
    if(std::abs(drawDir.y) >= 0.999f)
        up = Float3(0.0f, 0.0f, -1.0f);

    basis.SetXBasis(Float3::Normalize(Float3::Cross(up, drawDir)));
    basis.SetYBasis(Float3::Cross(drawDir, basis.Right()));

    const float arrowHeadSize = 0.2f;
    Float3 arrowHeadPoints[] =
    {
        Float3(0.0f, 0.0f, 1.0f) + Float3(1.0f, 0.0f, -1.0f) * arrowHeadSize,
        Float3(0.0f, 0.0f, 1.0f) + Float3(0.0f, 1.0f, -1.0f) * arrowHeadSize,
        Float3(0.0f, 0.0f, 1.0f) + Float3(-1.0f, 0.0f, -1.0f) * arrowHeadSize,
        Float3(0.0f, 0.0f, 1.0f) + Float3(0.0f, -1.0f, -1.0f) * arrowHeadSize,
    };

    Float2 startPoint = CanvasTransform(0.0f, canvasStart, canvasSize);
    ImColor color = ImColor(1.0f, 1.0f, 0.0f);

    Float2 endPoint = CanvasTransform(drawDir.To2D(), canvasStart, canvasSize);
    drawList->AddLine(ToImVec2(startPoint), ToImVec2(endPoint), color);

    for(uint64_t  i = 0; i < ArraySize_(arrowHeadPoints); ++i)
    {
        Float2 headPoint = Float3::Transform(arrowHeadPoints[i], basis).To2D();
        headPoint = CanvasTransform(headPoint, canvasStart, canvasSize);
        drawList->AddLine(ToImVec2(headPoint), ToImVec2(endPoint), color);

        uint64_t  nextHeadIdx = (i + 1) % ArraySize_(arrowHeadPoints);
        Float2 nextHeadPoint = Float3::Transform(arrowHeadPoints[nextHeadIdx], basis).To2D();
        nextHeadPoint = CanvasTransform(nextHeadPoint, canvasStart, canvasSize);
        drawList->AddLine(ToImVec2(headPoint), ToImVec2(nextHeadPoint), color);
    }

    ImGui::NextColumn();

    if(inputMode == DirectionInputMode::Cartesian)
    {
        ImGui::SliderFloat("x", &val.x, -1.0f, 1.0f);
        ImGui::SliderFloat("y", &val.y, -1.0f, 1.0f);
        ImGui::SliderFloat("z", &val.z, -1.0f, 1.0f);
        spherical = CartesianToSpherical(val);
    }
    else if(inputMode == DirectionInputMode::Spherical)
    {
        Float2 degrees = Float2(RadToDeg(spherical.x), RadToDeg(spherical.y));
        bool sliderChanged = ImGui::SliderFloat("azimuth", &degrees.x, 0.0f, 360.0f);
        sliderChanged |= ImGui::SliderFloat("elevation", &degrees.y, -90.0f, 90.0f);
        if(sliderChanged)
        {
            spherical = Float2(DegToRad(degrees.x), DegToRad(degrees.y));        
            val = SphericalToCartesian(spherical.x, spherical.y);
        }
    }

    bool cartesianButton = ImGui::RadioButton("Cartesian", inputMode == DirectionInputMode::Cartesian);
    ImGui::SameLine();
    bool sphericalButton = ImGui::RadioButton("Spherical", inputMode == DirectionInputMode::Spherical);

    if(cartesianButton)
        inputMode = DirectionInputMode::Cartesian;
    else if(sphericalButton)
        inputMode = DirectionInputMode::Spherical;

    ImGui::EndGroup();
    if(ImGui::IsItemHovered() && helpText.length() > 0)
        ImGui::SetTooltip("%s", helpText.c_str());

    ImGui::EndChild();

    val = Float3::Normalize(val);
    changed = oldVal != val;
    oldVal = val;
}

Float3 DirectionSetting::Value() const
{
    return val;
}

void DirectionSetting::SetValue(Float3 newVal)
{
    val = Float3::Normalize(newVal);
}

DirectionSetting::operator Float3()
{
    return val;
}

}