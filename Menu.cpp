#include "Menu.h"

#include <imgui.h>
#include <imgui-SFML.h>
#include <spdlog/spdlog.h>

#include <tinyfiledialogs/tinyfiledialogs.h>

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

  ImGui::SameLine();
  ImGui::Text(imageFilePath.c_str());

  ImGui::NewLine();

  ImGui::SeparatorText("options");
  ImGui::Checkbox("use output as source", &outputAsSource);

  ImGui::NewLine();

  ImGui::SeparatorText("operations");

  const std::vector<const char*> items_list = {"None", "Downsample", "Upsample", "Varying Bits", "Histogram Equalization"};

  ImGui::Combo("##operations", &currentItem, items_list.data(), static_cast<int32_t>(items_list.size()));

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

bool Menu::IsOutputAsSourceSet() const
{
  return outputAsSource;
}