#include "StackMenu.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "operations/ImageOperation.h"
#include "operations/OperationsStack.h"

namespace {

  bool ButtonAlignOnLine(const char* label, float alignment)
  {

    alignment = std::clamp(alignment, 0.0f, 1.0f);

    ImGuiStyle& style = ImGui::GetStyle();

    float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;

    float off = (avail - size) * alignment;
    if (off > 0.0f)
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    return ImGui::Button(label);
  }

  bool ArrowButtonAlignOnLine(const char* label, float alignment, ImGuiDir_ dir)
  {

    alignment = std::clamp(alignment, 0.0f, 1.0f);

    ImGuiStyle& style = ImGui::GetStyle();

    float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;

    float off = (avail - size) * alignment;
    if (off > 0.0f)
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    return ImGui::ArrowButton(label, dir);
  }

}

StackMenu::StackMenu(OperationsStack& operation_stack)
  : operationsStack(operation_stack)
{
}

void StackMenu::RenderMenu()
{

  ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_FrameRounding, 5.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowRounding, 5.0f);
  ImGui::Begin("operation stack", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_Modal);

  ImGui::BeginListBox("##bb");

  for (size_t i=0; i<operationsStack.GetStack().size(); i++)
  {
    auto & stack_item = operationsStack.GetStack()[i];
    MenuItem(stack_item.operation.get(), i);
  }

  ImGui::EndListBox();


  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();

}

bool StackMenu::ProcessBegin()
{
  return false;
}

void StackMenu::MenuItem(ImageOperation * image_operation, size_t index)
{
  std::string operation_name;

  switch (image_operation->GetOperationType())
  {
    case MenuOps::OpType::DOWNSAMPLE:
      operation_name = "downsample";
    break;

    case MenuOps::OpType::UPSAMPLE:
      operation_name = "upsample";
    break;

    case MenuOps::OpType::VARYINGBITS:
      operation_name = "varyingbits";
    break;

    case MenuOps::OpType::NONE:
    default:
      operation_name = "unknown";
    break;
  }

  ImGui::PushID((int32_t)index);
  auto pos = ImGui::GetCursorPos();
  bool is_selected = false;

  ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_HeaderHovered, IM_COL32(255, 0, 255, 255));
  ImGui::Selectable("##aaa", &is_selected, ImGuiSelectableFlags_::ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_::ImGuiSelectableFlags_AllowOverlap, ImVec2(0,80));
  ImGui::SetItemAllowOverlap();

  ImGui::SetCursorPos(pos);

  ImGui::BeginGroup();

  ImGui::SeparatorText(std::to_string(index).c_str());

  ImGui::BeginGroup();
  ImGui::Text(operation_name.c_str());
  ImGui::SameLine();
  if(ButtonAlignOnLine("X", 0.45f))
  {
    //TODO: index of what operation to remove
    spdlog::info("removing item: {} - {}", operation_name, index);
  }
  ImGui::SameLine();
  ArrowButtonAlignOnLine("down", 0.45f, ImGuiDir_::ImGuiDir_Down);
  ImGui::SameLine();
  if(ArrowButtonAlignOnLine("up", 0.45f, ImGuiDir_::ImGuiDir_Up))
  {
    spdlog::info("test up");
  }

  ImGui::EndGroup();


  ImGui::SeparatorText("##end");

  ImGui::EndGroup();
  ImGui::SetCursorPos(ImVec2(pos.x, pos.y+88));

  /*
  if (((xx > 0) && (xx < xx_width)) && ((yy > 0) && (yy < ImGui::GetItemRectSize().y)))
  {
    spdlog::info("xx: {} -- {} - {} - {} -> idx: {}", xx, ImGui::GetWindowPos().x, ImGui::GetMousePos().x, ImGui::GetItemRectSize().x, index);
    spdlog::info("yy: {} -- {} - {} - {}", yy, ImGui::GetWindowPos().y, ImGui::GetMousePos().y, ImGui::GetItemRectSize().y);
  }
  */

  ImGui::PopStyleColor();
  ImGui::PopID();
}