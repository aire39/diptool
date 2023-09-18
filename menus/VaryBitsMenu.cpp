#include "VaryBitsMenu.h"

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

void VaryBitsMenu::RenderMenu()
{
  ImGui::Begin("Varying Bits Operation", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::BeginGroup();
  ImGui::Text("Bits Level:");
  ImGui::SliderInt("##bit_level", &setBit, 8, 1);
  ImGui::EndGroup();

  ImGui::NewLine();

  ImGui::Checkbox("contrast bit shift", &shiftBitsContrast);

  if (ButtonCenteredOnLine("save image"))
  {
    processBegin = true;
  }

  ImGui::End();
}

int32_t VaryBitsMenu::BitScale() const
{
  return setBit;
}

bool VaryBitsMenu::ShiftBitsForContrast() const
{
  return shiftBitsContrast;
}

bool VaryBitsMenu::ProcessBegin()
{
  bool should_process = processBegin;
  if (should_process)
  {
    processBegin = false;
  }

  return should_process;
}
