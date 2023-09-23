#include "SpatialFilterMenu.h"

#include <algorithm>
#include <limits>
#include <imgui.h>
#include <imgui-SFML.h>
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

void SpatialFilterMenu::RenderMenu()
{
  ImGui::Begin("spatial filters", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::BeginGroup();

  ImGui::Text("Kernel Size:");


  float check_valid_kernel_x = (static_cast<float>(kernelX) - 1.0f) / 2.0f;
  float check_valid_kernel_x_f = (check_valid_kernel_x - static_cast<float>(static_cast<int32_t>(check_valid_kernel_x)));
  if (check_valid_kernel_x_f > 0.0f)
  {
    kernelX = static_cast<int32_t>(std::ceil(check_valid_kernel_x)) * 2;
    kernelX += 1;
  }

  float check_valid_kernel_y = (static_cast<float>(kernelY) - 1.0f) / 2.0f;
  float check_valid_kernel_y_f = (check_valid_kernel_y - static_cast<float>(static_cast<int32_t>(check_valid_kernel_y)));
  if (check_valid_kernel_y_f > 0.0f)
  {
    kernelY = static_cast<int32_t>(std::ceil(check_valid_kernel_y)) * 2;
    kernelY += 1;
  }

  if (isKernelUniform)
  {
    ImGui::InputInt("##kernel_x", &kernelX, 2, 100, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
    ImGui::SameLine();
    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::InputInt("##kernel_y", &kernelX, 2, 100, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);

    kernelY = kernelX;
  }
  else
  {
    ImGui::InputInt("##kernel_x", &kernelX, 2, 100, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
    ImGui::SameLine();
    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::InputInt("##kernel_y", &kernelY, 2, 100, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
  }

  kernelX = std::clamp(kernelX, 3, std::numeric_limits<int32_t>::max());
  kernelY = std::clamp(kernelY, 3, std::numeric_limits<int32_t>::max());

  ImGui::Checkbox("uniform kernel", &isKernelUniform);

  ImGui::EndGroup();

  ImGui::NewLine();

  ImGui::BeginGroup();
  ImGui::Text("Filter Type:");

  const std::vector<const char*> items_list = {"Smoothing", "Median", "Sharpening (Laplacian)", "High-Boosting"
                                              ,"Arithmetic Mean", "Geometric Mean", "Min", "Max", "Midpoint"
                                              ,"Harmonic Mean", "Contra-Harmonic Mean", "Alpha-Trimmed Mean"};
  ImGui::Combo("##operations", &currentItem, items_list.data(), static_cast<int32_t>(items_list.size()));
  ImGui::EndGroup();

  ImGui::NewLine();

  if (CurrentOperation() == MenuOps::SpatialFilter::SHARPENING)
  {
    ImGui::TextColored(ImVec4(0.75, 0.5, 0.9, 1.0f), "filter options:");

    ImGui::Text("kernel full use:");
    ImGui::SameLine();
    ImGui::Checkbox("##sharp_full_kernel", &isSharpUsingFullKernel);
    ImGui::Text("show sharp filter:");
    ImGui::SameLine();
    ImGui::Checkbox("##sharp_show_filter", &showSharpenFilter);

    if (showSharpenFilter)
    {
      ImGui::Text("show sharp filter with scaling:");
      ImGui::SameLine();
      ImGui::Checkbox("##sharp_show_filter_scaling", &showSharpenFilterScaling);

      if (showSharpenFilterScaling)
      {
        ImGui::Text("invert filter scaling:");
        ImGui::SameLine();
        ImGui::Checkbox("##sharp_invert_scaling", &invertSharpenFilterScaling);
      }
    }

    ImGui::NewLine();

    ImGui::TextColored(ImVec4(0.75, 0.5, 0.9, 1.0f), "sharpen constant (C):");
    ImGui::InputFloat("##sharp_const", &sharpenConstant, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
  }

  if (CurrentOperation() == MenuOps::SpatialFilter::HIGHBOOST)
  {
    ImGui::TextColored(ImVec4(0.75, 0.5, 0.9, 1.0f), "filter options:");
    ImGui::Text("show unsharp mask filter:");
    ImGui::SameLine();
    ImGui::Checkbox("##unsharp_show_filter", &showUnSharpenFilter);

    if (showUnSharpenFilter)
    {
      ImGui::Text("show unsharp mask filter with scaling:");
      ImGui::SameLine();
      ImGui::Checkbox("##unsharp_show_filter_scaling", &showUnSharpenFilterScaling);
    }

    ImGui::TextColored(ImVec4(0.75, 0.5, 0.9, 1.0f), "unsharp constant (K):");
    ImGui::InputFloat("##unsharp_const", &unsharpConstant, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
  }

  if (CurrentOperation() == MenuOp_SpatialFilter::CONTRA_HARMONIC_MEAN)
  {
    ImGui::TextColored(ImVec4(0.75, 0.5, 0.9, 1.0f), "filter options:");

    ImGui::TextColored(ImVec4(0.75, 0.5, 0.9, 1.0f), "contra-harmonic constant (Q):");
    ImGui::InputFloat("##contra_hormonic_const", &contraHarminocConstant, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);
  }

  if (CurrentOperation() == MenuOp_SpatialFilter::ALPHA_TRIM_MEAN)
  {
    ImGui::TextColored(ImVec4(0.75, 0.5, 0.9, 1.0f), "filter options:");

    ImGui::TextColored(ImVec4(0.75, 0.5, 0.9, 1.0f), "trim constant (d):");
    ImGui::InputInt("##alpha_trim_const", &alphaTrimConstant, 1, 10, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll);

    alphaTrimConstant = std::clamp(alphaTrimConstant, 0, (kernelX * kernelY) - 1);
  }

  ImGui::NewLine();

  if(ButtonCenteredOnLine("Process"))
  {
    processBegin = true;
  }

  ImGui::End();
}

MenuOps::SpatialFilter SpatialFilterMenu::CurrentOperation()
{
  switch (currentItem)
  {
    case 0:
      operation = MenuOps::SpatialFilter::SMOOTHING;
      break;

    case 1:
      operation = MenuOps::SpatialFilter::MEDIAN;
      break;

    case 2:
      operation = MenuOps::SpatialFilter::SHARPENING;
      break;

    case 3:
      operation = MenuOps::SpatialFilter::HIGHBOOST;
      break;

    case 4:
      operation = MenuOp_SpatialFilter::ARITH_MEAN;
      break;

    case 5:
      operation = MenuOp_SpatialFilter::GEO_MEAN;
      break;

    case 6:
      operation = MenuOp_SpatialFilter::MIN;
      break;

    case 7:
      operation = MenuOp_SpatialFilter::MAX;
      break;

    case 8:
      operation = MenuOp_SpatialFilter::MIDPOINT;
      break;

    case 9:
      operation = MenuOp_SpatialFilter::HARMONIC_MEAN;
      break;

    case 10:
      operation = MenuOp_SpatialFilter::CONTRA_HARMONIC_MEAN;
      break;

    case 11:
      operation = MenuOp_SpatialFilter::ALPHA_TRIM_MEAN;
      break;

    default:
      operation = MenuOps::SpatialFilter::SMOOTHING;
      break;
  }

  return operation;
}

bool SpatialFilterMenu::ProcessBegin()
{
  bool tmp = processBegin;
  processBegin = false;

  return tmp;
}

int32_t SpatialFilterMenu::GetKernelX() const
{
  return kernelX;
}

int32_t SpatialFilterMenu::GetKernelY() const
{
  return kernelY;
}

float SpatialFilterMenu::GetSharpenConstant() const
{
  return sharpenConstant;
}

float SpatialFilterMenu::GetUnsharpConstant() const
{
  return unsharpConstant;
}

float SpatialFilterMenu::GetContraHarminocConstant() const
{
  return contraHarminocConstant;
}

int32_t SpatialFilterMenu::GetAlphaTrimConstant() const
{
  return alphaTrimConstant;
}

bool SpatialFilterMenu::IsSharpenFullUse() const
{
  return isSharpUsingFullKernel;
}

bool SpatialFilterMenu::ShowSharpenFilter() const
{
  return showSharpenFilter;
}

bool SpatialFilterMenu::ShowSharpenFilterScaling() const
{
  return showSharpenFilterScaling;
}

bool SpatialFilterMenu::InvertSharpenFilterScaling() const
{
  return invertSharpenFilterScaling;
}

bool SpatialFilterMenu::ShowUnSharpenFilter() const
{
  return showUnSharpenFilter;
}

bool SpatialFilterMenu::ShowUnSharpenFilterScaling() const
{
  return showUnSharpenFilterScaling;
}
