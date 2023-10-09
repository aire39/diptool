#include "HistogramEqualizationMenu.h"

#include <imgui.h>
#include <implot/implot.h>
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

    float max_h_value = 0.0f;
    for (const auto & [key, value] : histogramNormalized)
    {
      histogramSourceValues[key] = value;
      max_h_value = (max_h_value < value) ? value : max_h_value;
    }

    float max_h_remap_value = 0.0f;
    for (const auto & [key, value] : histogramNormalizedRemap)
    {
      histogramRemapValues[key] = value;
      max_h_remap_value = (max_h_remap_value < value) ? value : max_h_remap_value;
    }

    if (ImPlot::BeginPlot("##histogram_eq", ImVec2(512, 0), ImPlotFlags_::ImPlotFlags_None))
    {
      ImPlot::SetupAxes("pixel values", "pixel ratio", ImPlotAxisFlags_::ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_::ImPlotAxisFlags_Opposite | ImPlotAxisFlags_::ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_::ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_::ImPlotAxisFlags_AutoFit);
      ImPlot::SetupAxisLimits(ImAxis_::ImAxis_X1, 0, 255);
      ImPlot::SetupAxisLimits(ImAxis_::ImAxis_Y1, 0, max_h_value * 2.0);

      ImPlot::PlotBars("source histogram", histogramSourceValues.data(), static_cast<int32_t>(histogramSourceValues.size()), max_h_value * 2.0f, 2.0f, ImPlotBarsFlags_::ImPlotBarsFlags_None, 0);
      ImPlot::SetNextFillStyle(ImVec4(1.f,0.75f,0.25f,1));
      ImPlot::PlotBars("remapped histogram", histogramRemapValues.data(), static_cast<int32_t>(histogramRemapValues.size()), max_h_value * 2.0f, 2.0f, ImPlotBarsFlags_::ImPlotBarsFlags_None, 0);

      ImPlot::EndPlot();
    }
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