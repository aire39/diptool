#pragma once

#include <vector>
#include <cstdint>
#include "MenuOps.h"

class DownsampleOp
{
  public:
    DownsampleOp() = default;
    ~DownsampleOp() = default;

    std::vector<uint8_t> ProcessImage(MenuOps::Downsample operation
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations);

    [[nodiscard]] const std::vector<uint8_t> & GetImage() const;

    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;

  private:
    int32_t outWidth = 0;
    int32_t outHeight = 0;
    std::vector<uint8_t> result;

    void DecimateAlgorithm(const std::vector<uint8_t> & source_image
                          ,uint32_t width
                          ,uint32_t height
                          ,uint8_t bpp
                          ,uint16_t iterations);

    void NearestAlgorithm(const std::vector<uint8_t> & source_image
                         ,uint32_t width
                         ,uint32_t height
                         ,uint8_t bpp
                         ,uint16_t iterations);
};
