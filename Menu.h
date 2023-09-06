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

  private:
    int32_t currentItem = 0;
    std::string imageFilePath;
};
