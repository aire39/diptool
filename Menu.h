#pragma once

#include <string>
#include <cstdint>

#include <SFML/Graphics.hpp>

class Menu {
  public:
    Menu() = default;

    void SetImagePath(std::string image_path);
    void RenderMenu(sf::Image & image, sf::Texture & texture, sf::Sprite & sprite);

    [[nodiscard]] bool IsDownSampleSet() const;
    [[nodiscard]] bool IsUpSampleSet() const;
    [[nodiscard]] bool IsVaryingBitsSet() const;
    [[nodiscard]] bool IsHistogramEqualizationSet() const;
    [[nodiscard]] bool IsSpatialFiltering() const;
    [[nodiscard]] bool IsOutputAsSourceSet() const;
    [[nodiscard]] bool IsOutputPNG() const;
    [[nodiscard]] bool IsOutputJPG() const;
    [[nodiscard]] bool IsOutputRLE() const;
    [[nodiscard]] std::string FileOutputPath();
    bool IsSavingOutput();

  private:
    int32_t currentItem = 0;
    bool outputAsSource = true;
    std::string imageFilePath;
    int32_t fileType = 0;
    char filepath[64] = "output";
    bool saveOutput = false;
};
