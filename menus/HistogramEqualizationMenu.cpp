#include "HistogramEqualizationMenu.h"

#include <limits>
#include <algorithm>

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

[[nodiscard]] MenuOp_HistogramMethod HistogramEqualizationMenu::CurrentOperation() const
{
  return operation;
}

void HistogramEqualizationMenu::RenderMenu()
{
  ImGui::Begin("Histogram Equalization Operation", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  {
    std::lock_guard<decltype(histogramMtx)> lock(histogramMtx);

    float rgb_color_source[][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
    float rgb_color_remap[][3] = {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}};

    if(ImPlot::BeginPlot("##histogram_eq", ImVec2(512, 0), ImPlotFlags_::ImPlotFlags_None))
    {
      ImPlot::SetupAxes("pixel values", "pixel ratio",
                        ImPlotAxisFlags_::ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_::ImPlotAxisFlags_Opposite |
                        ImPlotAxisFlags_::ImPlotAxisFlags_AutoFit,
                        ImPlotAxisFlags_::ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_::ImPlotAxisFlags_AutoFit);
      ImPlot::SetupAxisLimits(ImAxis_::ImAxis_X1, 0, 255);
      ImPlot::SetupAxisLimits(ImAxis_::ImAxis_Y1, 0, 1.0);

      for (size_t i=0; i<histogramNormalized.size(); i++)
      {
        float max_h_value = 0.0f;
        for (const auto &[key, value]: histogramNormalized[i])
        {
          histogramSourceValues[i][key] = value;
          max_h_value = (max_h_value < value) ? value : max_h_value;
        }

        float max_h_remap_value = 0.0f;
        for (const auto &[key, value]: histogramNormalizedRemap[i])
        {
          histogramRemapValues[i][key] = value;
          max_h_remap_value = (max_h_remap_value < value) ? value : max_h_remap_value;
        }

        if (histogramNormalized.size() == 1)
        {
          ImPlot::PlotBars("source histogram", histogramSourceValues[i].data(),
                           static_cast<int32_t>(histogramSourceValues[i].size()), max_h_value * 2.0f, 2.0f,
                           ImPlotBarsFlags_::ImPlotBarsFlags_None, 0);
          ImPlot::SetNextFillStyle(ImVec4(1.f, 0.75f, 0.25f, 1));
          ImPlot::PlotBars("remapped histogram", histogramRemapValues[i].data(),
                           static_cast<int32_t>(histogramRemapValues[i].size()), max_h_value * 2.0f, 2.0f,
                           ImPlotBarsFlags_::ImPlotBarsFlags_None, 0);
        }
        else if (histogramNormalized.size() > 1)
        {
          ImPlot::SetNextFillStyle(ImVec4(rgb_color_source[i][0], rgb_color_source[i][1], rgb_color_source[i][2], 1));
          ImPlot::PlotBars(("source histogram " + std::to_string(i)).c_str(), histogramSourceValues[i].data(),
                           static_cast<int32_t>(histogramSourceValues[i].size()), max_h_value * 2.0f, 2.0f,
                           ImPlotBarsFlags_::ImPlotBarsFlags_None, 0);
          ImPlot::SetNextFillStyle(ImVec4(rgb_color_remap[i][0], rgb_color_remap[i][1], rgb_color_remap[i][2], 1));
          ImPlot::PlotBars(("remapped histogram " + std::to_string(i)).c_str(), histogramRemapValues[i].data(),
                           static_cast<int32_t>(histogramRemapValues[i].size()), max_h_value * 2.0f, 2.0f,
                           ImPlotBarsFlags_::ImPlotBarsFlags_None, 0);
        }
      }

      if (histogramNormalized.empty())
      {
        ImPlot::SetupAxes("pixel values", "pixel ratio",
                          ImPlotAxisFlags_::ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_::ImPlotAxisFlags_Opposite |
                          ImPlotAxisFlags_::ImPlotAxisFlags_AutoFit,
                          ImPlotAxisFlags_::ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_::ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_::ImAxis_X1, 0, 255);
        ImPlot::SetupAxisLimits(ImAxis_::ImAxis_Y1, 0, 1);

        ImPlot::PlotBars("source histogram", (float*)nullptr, 0, 0, 2.0f, ImPlotBarsFlags_::ImPlotBarsFlags_None, 0);
        ImPlot::PlotBars("remapped histogram", (float*)nullptr,0, 0, 2.0f,ImPlotBarsFlags_::ImPlotBarsFlags_None, 0);
      }

      ImPlot::EndPlot();
    }
  }

  ImGui::NewLine();

  ImGui::Text(("Process Time: " + std::to_string(processTimeSecs) + " secs").c_str());

  ImGui::NewLine();

  ImGui::Text("Set Method Type:");

  ImGui::BeginGroup();

  if(ImGui::RadioButton("Global", (setMethodType == 0)))
  {
    setMethodType = 0;
    operation = MenuOp_HistogramMethod::GLOBAL;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("Localize", (setMethodType == 1)))
  {
    setMethodType = 1;
    operation = MenuOp_HistogramMethod::LOCALIZE;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("Localize Enhancement", (setMethodType == 2)))
  {
    setMethodType = 2;
    operation = MenuOp_HistogramMethod::LOCALIZE_ENCHANCEMENT;
  }

  ImGui::EndGroup();

  if (setMethodType == 1)
  {
    ImGui::BeginGroup();

    ImGui::InputInt("##kernel_x", &localizeKernelX, 1, 100, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
    ImGui::SameLine();
    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::InputInt("##kernel_y", &localizeKernelY, 1, 100, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);

    ImGui::EndGroup();
  }

  if (setMethodType == 2)
  {
    ImGui::BeginGroup();

    ImGui::InputInt("##kernel_x_2", &localizeKernelX, 1, 100, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
    ImGui::SameLine();
    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::InputInt("##kernel_y_2", &localizeKernelY, 1, 100, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);

    ImGui::NewLine();

    ImGui::InputFloat("K_0", &localizeKernelK0, 0.01f, 0.1f, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
    ImGui::InputFloat("K_1", &localizeKernelK1, 0.01f, 0.1f, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);

    ImGui::NewLine();

    ImGui::InputFloat("K_2", &localizeKernelK2, 0.01f, 0.1f, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
    ImGui::InputFloat("K_3", &localizeKernelK3, 0.01f, 0.1f, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);

    ImGui::NewLine();

    ImGui::InputFloat("C", &localizeKernelEnhanceConst, 0.01f, 0.1f, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);

    ImGui::EndGroup();
  }

  localizeKernelX = std::clamp(localizeKernelX, 1, imagePixelWidth);
  localizeKernelY = std::clamp(localizeKernelY, 1, imagePixelHeight);

  ImGui::NewLine();

  ImGui::Text("Set Color Type:");

  ImGui::BeginGroup();

  if(ImGui::RadioButton("Gray", (setColorType == 0)))
  {
    setColorType = 0;
  }

  ImGui::SameLine();

  if(ImGui::RadioButton("RGBA", (setColorType == 1)))
  {
    setColorType = 1;
  }

  ImGui::EndGroup();

  if (ButtonCenteredOnLine("Generate Histogram Equalization"))
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

void HistogramEqualizationMenu::SetHistogramData(std::vector<std::map<int32_t, float>> & histogram_data)
{
  std::lock_guard<decltype(histogramMtx)> lock(histogramMtx);
  histogramNormalized = histogram_data;
}

void HistogramEqualizationMenu::SetHistogramRemapData(std::vector<std::map<int32_t, float>> & histogram_data)
{
  std::lock_guard<decltype(histogramMtx)> lock(histogramMtx);
  histogramNormalizedRemap = histogram_data;
}

void HistogramEqualizationMenu::SetProcessTime(float process_time)
{
  processTimeSecs = process_time;
}

void HistogramEqualizationMenu::SetSizeOfImage(int32_t pixel_width, int32_t pixel_height)
{
  imagePixelWidth = pixel_width;
  imagePixelHeight = pixel_height;
}

bool HistogramEqualizationMenu::IsHistogramColorTypeGray() const
{
  return (setColorType == 0);
}

bool HistogramEqualizationMenu::IsHistogramColorTypeRGBA() const
{
  return (setColorType == 1);
}

bool HistogramEqualizationMenu::IsGlobalMethodType() const
{
  return (setMethodType == 0);
}

bool HistogramEqualizationMenu::IsLocalizeMethodType() const
{
  return (setMethodType == 1);
}

bool HistogramEqualizationMenu::IsLocalizeEnchancementMethodType() const
{
  return (setMethodType == 2);
}

int32_t HistogramEqualizationMenu::GetKernelX() const
{
  return localizeKernelX;
}

int32_t HistogramEqualizationMenu::GetKernelY() const
{
  return localizeKernelY;
}

float HistogramEqualizationMenu::GetKernelK0() const
{
  return localizeKernelK0;
}

float HistogramEqualizationMenu::GetKernelK1() const
{
  return localizeKernelK1;
}

float HistogramEqualizationMenu::GetKernelK2() const
{
  return localizeKernelK2;
}

float HistogramEqualizationMenu::GetKernelK3() const
{
  return localizeKernelK3;
}

float HistogramEqualizationMenu::GetKernelEnhanceConst() const
{
  return localizeKernelEnhanceConst;
}

void HistogramEqualizationMenu::ClearData()
{
  histogramNormalized.clear();
  histogramNormalizedRemap.clear();
  histogramSourceValues = {std::vector<float>(256), std::vector<float>(256), std::vector<float>(256)};
  histogramRemapValues = {std::vector<float>(256), std::vector<float>(256), std::vector<float>(256)};
}
