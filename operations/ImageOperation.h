#pragma once

#include <cstdint>
#include <vector>

#include "MenuOps.h"

class ImageOperation
{
  public:
    virtual std::vector<uint8_t> ProcessImage(MenuOps::Op op
                                             ,const std::vector<uint8_t> & source_image
                                             ,uint32_t width
                                             ,uint32_t height
                                             ,uint8_t bpp
                                             ,uint16_t iterations) = 0;

    [[nodiscard]] const std::vector<uint8_t> & GetImage() const;

    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;

    [[nodiscard]] MenuOps::Op GetOperation() const;
    [[nodiscard]] MenuOps::OpType GetOperationType() const;

  protected:
    MenuOps::Op operation;
    MenuOps::OpType opType = MenuOps::OpType::NONE;
    int32_t outWidth = 0;
    int32_t outHeight = 0;
    std::vector<uint8_t> result;
};
