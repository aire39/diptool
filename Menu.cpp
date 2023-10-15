#include "Menu.h"

#include <imgui.h>
#include <imgui-SFML.h>
#include <spdlog/spdlog.h>

#include <tinyfiledialogs/tinyfiledialogs.h>

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

void Menu::SetImagePath(std::string image_path)
{
  imageFilePath = std::move(image_path);
}

void Menu::RenderMenu(sf::Image & image, sf::Texture & texture, sf::Sprite & sprite)
{
  ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::SeparatorText("source");
  if(ImGui::Button("Load Image"))
  {
    char const * file_filter[2]={"*.png","*.jpg"};
    auto selected_file = tinyfd_openFileDialog("Load Image"
        ,nullptr
        ,2
        ,file_filter
        ,"image files"
        ,0
    );

    if (selected_file)
    {
      imageFilePath = std::string(selected_file);

      if(image.loadFromFile(imageFilePath))
      {
        spdlog::info("New image loaded");
        texture.loadFromImage(image);
        sprite.setTexture(texture, true);
        sprite.setPosition(8, 8);
      }
      else
      {
        spdlog::warn("Unable to load image");
      }
    }
  }

  ImGui::SameLine();
  ImGui::Text(imageFilePath.c_str());

  ImGui::NewLine();

  ImGui::SeparatorText("options");
  ImGui::Checkbox("use output as source", &outputAsSource);

  ImGui::NewLine();

  ImGui::SeparatorText("operations");

  const std::vector<const char*> items_list = {"None", "Downsample", "Upsample", "Varying Bits", "Histogram Equalization", "Spatial Filtering"};

  ImGui::Combo("##operations", &currentItem, items_list.data(), static_cast<int32_t>(items_list.size()));

  ImGui::NewLine();

  if (ButtonCenteredOnLine("Save Output"))
  {
    saveOutput = true;
  }

  ImGui::End();
}

bool Menu::IsDownSampleSet() const
{
  return (currentItem == 1);
}

bool Menu::IsUpSampleSet() const
{
  return (currentItem == 2);
}

bool Menu::IsVaryingBitsSet() const
{
  return (currentItem == 3);
}

bool Menu::IsHistogramEqualizationSet() const
{
  return (currentItem == 4);
}

bool Menu::IsSpatialFiltering() const
{
  return (currentItem == 5);
}

bool Menu::IsOutputAsSourceSet() const
{
  return outputAsSource;
}

bool Menu::IsSavingOutput()
{
  bool tmp_save = saveOutput;
  saveOutput = false;
  return tmp_save;
}
