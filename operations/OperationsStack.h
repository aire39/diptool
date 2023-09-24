#pragma once

#include <vector>
#include <memory>
#include "MenuOps.h"
#include "ImageOperation.h"

class OperationsStack : public ImageOperation
{
  public:
    void AddOperation(std::unique_ptr<ImageOperation> && operation, MenuOps::Op op);

    std::vector<uint8_t> ProcessImage(const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations);

  private:

    struct OpInfo
    {
      std::unique_ptr<ImageOperation> operation;
      MenuOps::Op op;
    };

    std::vector<OpInfo> operations;
};
