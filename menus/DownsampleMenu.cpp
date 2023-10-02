#include "DownsampleMenu.h"

#include <imgui.h>
#include "spdlog/include/spdlog/spdlog.h"

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
  ImGui::Text("downsample level:");
  ImGui::SliderInt("##downsample_iterations", &downsamepleIters, 0, 5);
  ImGui::EndGroup();

  ImGui::NewLine();

  ImGui::Text("downsample type:");

  ImGui::BeginGroup();

  if(ImGui::RadioButton("Decimate", (operation == MenuOps::Downsample::DECIMATE)))
  {
    operation = MenuOps::Downsample::DECIMATE;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("Nearest", (operation == MenuOps::Downsample::NEAREST)))
  {
    operation = MenuOps::Downsample::NEAREST;
  }

  ImGui::EndGroup();

  ImGui::NewLine();

  ImGui::BeginGroup();
  if (ButtonCenteredOnLine("save image"))
  {
    switch (operation)
    {
      case MenuOps::Downsample::DECIMATE:
        spdlog::info("processing image as decimate");
        break;

      case MenuOps::Downsample::NEAREST:
        spdlog::info("processing image as nearest");
        break;
    }



    processBegin = true;
  }

  ImGui::SameLine();

  if (ButtonCenteredOnLine("add operation") && callback)
  {
    callback();
  }
  ImGui::EndGroup();

  ImGui::End();
}

MenuOps::Downsample DownsampleMenu::CurrentOperation() const
{
  return operation;
}

int32_t DownsampleMenu::DownsampleIterations() const
{
  return downsamepleIters;
}

bool DownsampleMenu::ProcessBegin()
{
  bool should_process = processBegin;
  if (should_process)
  {
    processBegin = false;
  }

  return should_process;
}

void DownsampleMenu::SetCallback(std::function<void()> cb)
{
  callback = cb;
}
