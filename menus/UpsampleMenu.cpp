#include "UpsampleMenu.h"

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

void UpsampleMenu::RenderMenu()
{
  ImGui::Begin("Upsample Operation", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::BeginGroup();
  ImGui::Text("upsample level:");
  ImGui::SliderInt("##upsample_iterations", &upsamepleIters, 0, 5);
  ImGui::EndGroup();

  ImGui::NewLine();

  ImGui::Text("upsample type:");

  ImGui::BeginGroup();

  if(ImGui::RadioButton("Nearest", (operation == MenuOps::Upsample::NEAREST)))
  {
    operation = MenuOps::Upsample::NEAREST;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("Linear", (operation == MenuOps::Upsample::LINEAR)))
  {
    operation = MenuOps::Upsample::LINEAR;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("Bilinear", (operation == MenuOps::Upsample::BILINEAR)))
  {
    operation = MenuOps::Upsample::BILINEAR;
  }

  ImGui::EndGroup();

  ImGui::NewLine();

  if (ButtonCenteredOnLine("save image"))
  {
    switch (operation)
    {
      case MenuOps::Upsample::NEAREST:
        spdlog::info("processing image as nearest");
        break;

      case MenuOps::Upsample::LINEAR:
        spdlog::info("processing image as linear");
        break;

      case MenuOps::Upsample::BILINEAR:
        spdlog::info("processing image as bilinear");
        break;
    }

    processBegin = true;
  }

  ImGui::End();
}

MenuOps::Upsample UpsampleMenu::CurrentOperation() const
{
  return operation;
}

int32_t UpsampleMenu::UpsampleIterations() const
{
  return upsamepleIters;
}

bool UpsampleMenu::ProcessBegin()
{
  bool should_process = processBegin;
  if (should_process)
  {
    processBegin = false;
  }

  return should_process;
}
