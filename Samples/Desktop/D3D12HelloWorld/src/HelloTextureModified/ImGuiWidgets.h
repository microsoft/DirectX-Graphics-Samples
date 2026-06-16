#pragma once

#include "imgui.h"
#include <algorithm>
#include <cstring>

namespace ImGuiWidgets
{

struct SliderStyle
{
    float buttonWidth = 18.0f;
    float buttonHeight = 0.0f; // 0 = use default
};

inline SliderStyle& GetStyle()
{
    static SliderStyle style;
    return style;
}

inline bool SliderFloatWithControls(const char* label,
                                    float* value,
                                    float min,
                                    float max,
                                    float delta,
                                    float defaultValue,
                                    const char* format = "%.3f",
                                    float power = 1.0f)
{
    const SliderStyle& s = GetStyle();
    const ImGuiStyle& imguiStyle = ImGui::GetStyle();
    const float spacing = imguiStyle.ItemSpacing.x;
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float totalButtonWidth = s.buttonWidth * 3.0f + spacing * 2.0f;
    const float sliderWidth = windowWidth - totalButtonWidth - spacing;

    ImGui::PushItemWidth(sliderWidth);
    bool changed = ImGui::SliderFloat(label, value, min, max, format, power);
    ImGui::PopItemWidth();

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("<", ImVec2(s.buttonWidth, s.buttonHeight)))
    {
        *value = std::clamp(*value - delta, min, max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button(">", ImVec2(s.buttonWidth, s.buttonHeight)))
    {
        *value = std::clamp(*value + delta, min, max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("|", ImVec2(s.buttonWidth, s.buttonHeight)))
    {
        *value = defaultValue;
        changed = true;
    }

    return changed;
}

inline bool SliderIntWithControls(const char* label,
                                   int* value,
                                   int min,
                                   int max,
                                   int delta,
                                   int defaultValue,
                                   const char* format = "%d")
{
    const SliderStyle& s = GetStyle();
    const ImGuiStyle& imguiStyle = ImGui::GetStyle();
    const float spacing = imguiStyle.ItemSpacing.x;
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float totalButtonWidth = s.buttonWidth * 3.0f + spacing * 2.0f;
    const float sliderWidth = windowWidth - totalButtonWidth - spacing;

    ImGui::PushItemWidth(sliderWidth);
    bool changed = ImGui::SliderInt(label, value, min, max, format);
    ImGui::PopItemWidth();

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("<", ImVec2(s.buttonWidth, s.buttonHeight)))
    {
        *value = std::clamp(*value - delta, min, max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button(">", ImVec2(s.buttonWidth, s.buttonHeight)))
    {
        *value = std::clamp(*value + delta, min, max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("|", ImVec2(s.buttonWidth, s.buttonHeight)))
    {
        *value = defaultValue;
        changed = true;
    }

    return changed;
}

inline bool SliderFloat3WithControls(const char* label,
                                     float* v,
                                     float v_min,
                                     float v_max,
                                     float delta,
                                     const float* defaultValue,
                                     const char* format = "%.3f",
                                     float power = 1.0f)
{
    const SliderStyle& s = GetStyle();
    const ImGuiStyle& imguiStyle = ImGui::GetStyle();
    const float spacing = imguiStyle.ItemSpacing.x;
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float totalButtonWidth = s.buttonWidth * 3.0f + spacing * 2.0f;
    const float sliderWidth = windowWidth - totalButtonWidth - spacing;

    ImGui::PushItemWidth(sliderWidth);
    bool changed = ImGui::SliderFloat3(label, v, v_min, v_max, format, power);
    ImGui::PopItemWidth();

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("<", ImVec2(s.buttonWidth, s.buttonHeight)))
    {
        v[0] = std::clamp(v[0] - delta, v_min, v_max);
        v[1] = std::clamp(v[1] - delta, v_min, v_max);
        v[2] = std::clamp(v[2] - delta, v_min, v_max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button(">", ImVec2(s.buttonWidth, s.buttonHeight)))
    {
        v[0] = std::clamp(v[0] + delta, v_min, v_max);
        v[1] = std::clamp(v[1] + delta, v_min, v_max);
        v[2] = std::clamp(v[2] + delta, v_min, v_max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("|", ImVec2(s.buttonWidth, s.buttonHeight)))
    {
        v[0] = defaultValue[0];
        v[1] = defaultValue[1];
        v[2] = defaultValue[2];
        changed = true;
    }

    return changed;
}

} // namespace ImGuiWidgets
