#pragma once

#include <vector>
#include <cstdint>
#include "MenuOps.h"
#include "ImageOperation.h"

class DownsampleOp : public ImageOperation
{
  public:
    DownsampleOp();
    ~DownsampleOp() = default;

    std::vector<uint8_t> ProcessImage(MenuOps::Op op
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations) override;

    std::vector<uint8_t> ProcessImage(MenuOps::Downsample operation
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations);

  private:
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
