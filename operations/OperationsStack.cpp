#include "OperationsStack.h"

#include "ImageOperation.h"

void OperationsStack::AddOperation(std::unique_ptr<ImageOperation> && operation, MenuOps::Op op)
{
  operations.emplace_back(std::move(operation), op);
}

std::vector<uint8_t> OperationsStack::ProcessImage(const std::vector<uint8_t> & source_image
                                                  ,uint32_t width
                                                  ,uint32_t height
                                                  ,uint8_t bpp
                                                  ,uint16_t iterations)
{
  auto processed_source = source_image;
  auto processed_width = static_cast<int32_t>(width);
  auto processed_height = static_cast<int32_t>(height);

  for(auto & op : operations)
  {
    op.operation->ProcessImage(op.op, processed_source, processed_width, processed_height, bpp, iterations);
    processed_source = op.operation->GetImage();
    processed_width = op.operation->GetWidth();
    processed_height = op.operation->GetHeight();
  }

  result = processed_source;
  outWidth = processed_width;
  outHeight = processed_height;

  return result;
}
