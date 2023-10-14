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

  const std::vector<const char*> items_list = {"Smoothing", "Median", "Sharpening (Laplacian)", "High-Boosting"};
  ImGui::Combo("##operations", &currentItem, items_list.data(), static_cast<int32_t>(items_list.size()));
  ImGui::EndGroup();

  ImGui::NewLine();

  if(ButtonCenteredOnLine("Process"))
  {
    processBegin = true;
  }

  ImGui::End();
}

MenuOp_SpatialFilter SpatialFilterMenu::CurrentOperation()
{
  switch (currentItem)
  {
    case 0:
      operation = MenuOp_SpatialFilter::SMOOTHING;
      break;

    case 1:
      operation = MenuOp_SpatialFilter::MEDIAN;
      break;

    case 2:
      operation = MenuOp_SpatialFilter::SHARPENING;
      break;

    case 3:
      operation = MenuOp_SpatialFilter::HIGHBOOST;
      break;

    default:
      operation = MenuOp_SpatialFilter::SMOOTHING;
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
