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
      sprite.setTexture(texture);
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

  ImGui::Text("operations:");

  const char * items_list[] = {"None", "Downsample"};

  ImGui::Combo("##operations", &currentItem, items_list, 2);

  ImGui::End();
}

bool Menu::IsDownSampleSet() const
{
  return (currentItem == 1);
}