#include "HistogramEqualizationMenu.h"

#include <imgui.h>
#include <spdlog/spdlog.h>

namespace {
  bool ButtonCenteredOnLine(const char* label)
  {
    constexpr float alignment = 0.5f;

    ImGuiStyle& style = ImGui::GetStyle();

    float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;

    float off = (avail - size) * alignment;
    if (off > 0.0f)
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    return ImGui::Button(label);
  }
}

[[nodiscard]] MenuOp_Upsample HistogramEqualizationMenu::CurrentOperation() const
{
  return MenuOp_Upsample::LINEAR;
}

void HistogramEqualizationMenu::RenderMenu()
{
  ImGui::Begin("Histogram Equalization Operation", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  {
    std::lock_guard<decltype(histogramMtx)> lock(histogramMtx);
    static  std::vector<float> h_values(256);
    float max_h_value = 0.0f;
    for (const auto & [key, value] : histogramNormalized)
    {
      h_values[key] = value;
      max_h_value = (max_h_value < value) ? value : max_h_value;
    }

    ImGui::PlotHistogram("##ohistogram", h_values.data(), h_values.size(), 2.0f, "source histogram", 0.0f, max_h_value * 2.0f, ImVec2(256, 80));

    ImGui::NewLine();

    static std::vector<float> h_remap_values(256);
    float max_h_remap_value = 0.0f;
    for (const auto & [key, value] : histogramNormalizedRemap)
    {
      h_remap_values[key] = value;
      max_h_remap_value = (max_h_remap_value < value) ? value : max_h_remap_value;
    }

    ImGui::PlotHistogram("##ohistogram_remap", h_remap_values.data(), h_remap_values.size(), 2.0f, "remapped histogram", 0.0f, max_h_remap_value * 2.0f, ImVec2(256, 80));
    ImGui::NewLine();
  }

  ImGui::BeginGroup();

  ImGui::NewLine();

  if(ImGui::RadioButton("Global", (operation == MenuOp_Upsample::NEAREST)))
  {
    operation = MenuOp_Upsample::NEAREST;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("3x3", (operation == MenuOp_Upsample::LINEAR)))
  {
    operation = MenuOp_Upsample::LINEAR;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("5x5", (operation == MenuOp_Upsample::BILINEAR)))
  {
    operation = MenuOp_Upsample::BILINEAR;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("9x9", (operation == MenuOp_Upsample::BILINEAR)))
  {
    operation = MenuOp_Upsample::BILINEAR;
  }

  ImGui::EndGroup();

  if (ButtonCenteredOnLine("process"))
  {
    processBegin = true;
  }

  ImGui::End();
}

bool HistogramEqualizationMenu::ProcessBegin()
{
  bool tmp = processBegin;
  processBegin = false;
  return tmp;
}

void HistogramEqualizationMenu::SetHistogramData(const std::map<int32_t, float> & histogram_data)
{
  std::lock_guard<decltype(histogramMtx)> lock(histogramMtx);
  histogramNormalized = histogram_data;
}

void HistogramEqualizationMenu::SetHistogramRemapData(const std::map<int32_t, float> & histogram_data)
{
  std::lock_guard<decltype(histogramMtx)> lock(histogramMtx);
  histogramNormalizedRemap = histogram_data;
}