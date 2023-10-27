#pragma once

#include <vector>
#include <cstdint>
#include "MenuOps.h"
#include "ImageOperation.h"

class UpsampleOp : public ImageOperation
{
  public:
    UpsampleOp();
    ~UpsampleOp() = default;

    std::vector<uint8_t> ProcessImage(MenuOps::Op op
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations) override;

    std::vector<uint8_t> ProcessImage(MenuOps::Upsample operation
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations);

  private:
    void NearestAlgorithm(const std::vector<uint8_t> & source_image
                         ,uint32_t width
                         ,uint32_t height
                         ,uint8_t bpp
                         ,uint16_t iterations);

    void LinearAlgorithm(const std::vector<uint8_t> & source_image
                          ,uint32_t width
                          ,uint32_t height
                          ,uint8_t bpp
                          ,uint16_t iterations);

    void BilinearAlgorithm(const std::vector<uint8_t> & source_image
                          ,uint32_t width
                          ,uint32_t height
                          ,uint8_t bpp
                          ,uint16_t iterations);
};
