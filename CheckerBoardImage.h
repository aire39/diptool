#pragma once

#include <cstdint>
#include <vector>

class CheckerBoardImage
{
  public:
    CheckerBoardImage();
    CheckerBoardImage(uint32_t pixel_width, uint32_t pixel_height, uint32_t checker_tile_repeat);
    ~CheckerBoardImage() = default;

    void Generate(uint32_t pixel_width, uint32_t pixel_height, uint32_t repeat_checkers_per_line);
    const std::vector<uint8_t> & GetImage();

  private:
    std::vector<uint8_t> image;
    uint32_t pixelWidth = 800;
    uint32_t pixelHeight = 600;
    static constexpr uint32_t bytesPerPixel = 4;
    static constexpr uint32_t defaultCheckerTileRepeat = 32;

    void SetPixelColor(uint32_t x, uint32_t y, const uint8_t rgb[3]);
};
