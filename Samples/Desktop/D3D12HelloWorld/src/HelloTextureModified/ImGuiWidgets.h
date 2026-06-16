#pragma once

#include "imgui.h"
#include <algorithm>

namespace ImGuiWidgets
{

inline float CalcButtonWidth(const char* text)
{
    return ImGui::CalcTextSize(text).x + ImGui::GetStyle().FramePadding.x * 2.0f;
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
    const ImGuiStyle& s = ImGui::GetStyle();
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float btnW = CalcButtonWidth(">");
    const float totalButtons = btnW * 3.0f + s.ItemSpacing.x * 2.0f;
    const float sliderWidth = windowWidth - totalButtons;

    ImGui::PushItemWidth(sliderWidth);
    bool changed = ImGui::SliderFloat(label, value, min, max, format, power);
    ImGui::PopItemWidth();

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("<", ImVec2(btnW, 0.0f)))
    {
        *value = std::clamp(*value - delta, min, max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button(">", ImVec2(btnW, 0.0f)))
    {
        *value = std::clamp(*value + delta, min, max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("|", ImVec2(btnW, 0.0f)))
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
    const ImGuiStyle& s = ImGui::GetStyle();
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float btnW = CalcButtonWidth(">");
    const float totalButtons = btnW * 3.0f + s.ItemSpacing.x * 2.0f;
    const float sliderWidth = windowWidth - totalButtons;

    ImGui::PushItemWidth(sliderWidth);
    bool changed = ImGui::SliderInt(label, value, min, max, format);
    ImGui::PopItemWidth();

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("<", ImVec2(btnW, 0.0f)))
    {
        *value = std::clamp(*value - delta, min, max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button(">", ImVec2(btnW, 0.0f)))
    {
        *value = std::clamp(*value + delta, min, max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("|", ImVec2(btnW, 0.0f)))
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
    const ImGuiStyle& s = ImGui::GetStyle();
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float btnW = CalcButtonWidth(">");
    const float totalButtons = btnW * 3.0f + s.ItemSpacing.x * 2.0f;
    const float sliderWidth = windowWidth - totalButtons;

    ImGui::PushItemWidth(sliderWidth);
    bool changed = ImGui::SliderFloat3(label, v, v_min, v_max, format, power);
    ImGui::PopItemWidth();

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("<", ImVec2(btnW, 0.0f)))
    {
        v[0] = std::clamp(v[0] - delta, v_min, v_max);
        v[1] = std::clamp(v[1] - delta, v_min, v_max);
        v[2] = std::clamp(v[2] - delta, v_min, v_max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button(">", ImVec2(btnW, 0.0f)))
    {
        v[0] = std::clamp(v[0] + delta, v_min, v_max);
        v[1] = std::clamp(v[1] + delta, v_min, v_max);
        v[2] = std::clamp(v[2] + delta, v_min, v_max);
        changed = true;
    }

    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("|", ImVec2(btnW, 0.0f)))
    {
        v[0] = defaultValue[0];
        v[1] = defaultValue[1];
        v[2] = defaultValue[2];
        changed = true;
    }

    return changed;
}

} // namespace ImGuiWidgets
