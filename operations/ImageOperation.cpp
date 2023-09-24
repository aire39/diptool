#include "ImageOperation.h"

const std::vector<uint8_t> & ImageOperation::GetImage() const
{
  return result;
}

int32_t ImageOperation::GetWidth() const
{
  return outWidth;
}

int32_t ImageOperation::GetHeight() const
{
  return outHeight;
}

MenuOps::Op ImageOperation::GetOperation() const
{
  return operation;
}

MenuOps::OpType ImageOperation::GetOperationType() const
{
  return opType;
}
