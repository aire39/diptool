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
    [[nodiscard]] bool IsOutputAsSourceSet() const;

  private:
    int32_t currentItem = 0;
    bool outputAsSource = false;
    std::string imageFilePath;
};
