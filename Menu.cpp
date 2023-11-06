#include "Menu.h"

#include <fstream>
#include <filesystem>
#include <cstring>

#include <imgui.h>
#include <imgui-SFML.h>
#include <spdlog/spdlog.h>

#include <tinyfiledialogs/tinyfiledialogs.h>

#include "operations/RunLengthCodec.h"

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
    char const * file_filter[3] = {"*.png","*.jpg", "*.encode"};
    auto selected_file = tinyfd_openFileDialog("Load Image"
        ,nullptr
        ,3
        ,file_filter
        ,"image files"
        ,0
    );

    if (selected_file)
    {
      imageFilePath = std::string(selected_file);

      std::string image_file_ext = imageFilePath.substr(imageFilePath.find_last_of('.') + 1);
      for (auto & c : image_file_ext)
      {
        c = static_cast<char>(std::tolower(static_cast<int32_t>(c)));
      }

      if (image_file_ext == "encode")
      {
        std::filesystem::path file_path(selected_file);
        size_t file_size = std::filesystem::file_size(file_path);

        if (file_size)
        {
          std::vector<uint8_t> data(file_size);
          std::ifstream file_encode(selected_file, std::ifstream::binary);
          file_encode.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(file_size));
          file_encode.close();

          uint32_t width = *reinterpret_cast<uint32_t*>(&data[0]);
          uint32_t height = *reinterpret_cast<uint32_t*>(&data[4]);

          auto image_data = RunLengthCodec::Decode(data);
          image.create(width, height, &image_data[0]);
          texture.loadFromImage(image);
          sprite.setTexture(texture, true);
          sprite.setPosition(8, 8);
        }
        else
        {
          spdlog::warn("Unable to load image {} because no data", imageFilePath);
        }
      }
      else if(image.loadFromFile(imageFilePath))
      {
        spdlog::info("New image loaded");
        texture.loadFromImage(image);
        sprite.setTexture(texture, true);
        sprite.setPosition(8, 8);
      }
      else
      {
        spdlog::warn("Unable to load image {}", imageFilePath);
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

  ImGui::SeparatorText("output");

  ImGui::Text("file name");

  ImGui::SameLine();

  ImGui::InputText("##save file name ", &filepath[0], sizeof(filepath));

  ImGui::SameLine();

  if (ImGui::RadioButton("png", (fileType == 0)))
  {
    fileType = 0;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("jpg", (fileType == 1)))
  {
    fileType = 1;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("rle", (fileType == 2)))
  {
    fileType = 2;
  }

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

bool Menu::IsOutputPNG() const
{
  return (fileType == 0);
}

bool Menu::IsOutputJPG() const
{
  return (fileType == 1);
}

bool Menu::IsOutputRLE() const
{
  return (fileType == 2);
}

std::string Menu::FileOutputPath()
{
  std::string file_path (filepath);
  return file_path;
}

bool Menu::IsSavingOutput()
{
  bool tmp_save = saveOutput;
  saveOutput = false;
  return tmp_save;
}
