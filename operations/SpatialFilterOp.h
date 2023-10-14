#pragma once

#include <vector>
#include "MenuOps.h"

class SpatialFilterOp
{
  public:
    SpatialFilterOp() = default;
    ~SpatialFilterOp() = default;

    std::vector<uint8_t> ProcessImage(MenuOp_SpatialFilter operation
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations);

    [[nodiscard]] const std::vector<uint8_t> & GetImage() const;

    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;

    void SetKernelSize(int32_t kernel_x, int32_t kernel_y);

  private:

    static float ConvolutionValue(const std::vector<uint8_t> & source
                                 ,int32_t x
                                 ,int32_t y
                                 ,int32_t source_width
                                 ,int32_t source_height
                                 ,int32_t offset
                                 ,int32_t sum_count
                                 ,int32_t bpp
                                 ,const std::vector<float> & kernel
                                 ,int32_t kernel_width
                                 ,int32_t kernel_height
                                 ,float kernel_div);

    void SmoothingFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);

    std::vector<uint8_t> result;
    int32_t outWidth = 0;
    int32_t outHeight = 0;
    int32_t kernelX = 3;
    int32_t kernelY = 3;
};
