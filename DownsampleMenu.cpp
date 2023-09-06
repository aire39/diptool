#include "DownsampleMenu.h"

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

void DownsampleMenu::RenderMenu()
{
  ImGui::Begin("Downsample Operation", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::BeginGroup();

  if (ImGui::RadioButton("Nearest", (operation == MenuOp_Downsample::NEAREST)))
  {
    operation = MenuOp_Downsample::NEAREST;
  }

  ImGui::SameLine();

  if(ImGui::RadioButton("Bilinear", (operation == MenuOp_Downsample::BILINEAR)))
  {
    operation = MenuOp_Downsample::BILINEAR;
  }

  ImGui::EndGroup();

  if (ButtonCenteredOnLine("process image"))
  {
    switch (operation)
    {
      case MenuOp_Downsample::NEAREST:
        spdlog::info("processing image as nearest");
        //TODO: do nearest neighbor downsampling
        break;

      case MenuOp_Downsample::BILINEAR:
        spdlog::info("processing image as bilinear");
        //TODO: do bilinear downsampling
        break;
    }
  }

  ImGui::End();
}
