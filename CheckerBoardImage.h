#pragma once

#include <cstdint>
#include <vector>

class CheckerBoardImage
{
  public:
    CheckerBoardImage() = default;
    CheckerBoardImage(uint32_t pixel_width, uint32_t pixel_height, uint32_t checker_tile_repeat);
    ~CheckerBoardImage() = default;

    bool IsImageGenerated();
    void Generate(uint32_t pixel_width, uint32_t pixel_height, uint32_t repeat_checkers_per_line);
    const std::vector<uint8_t> & GetImage();

    [[maybe_unused,nodiscard]] uint32_t GetImageWidth() const;
    [[maybe_unused,nodiscard]] uint32_t GetImageHeight() const;

  private:
    std::vector<uint8_t> image;
    uint32_t pixelWidth = 0;
    uint32_t pixelHeight = 0;
    static constexpr uint32_t bytesPerPixel = 4;

    void SetPixelColor(uint32_t x, uint32_t y, const uint8_t rgb[3]);
};
