#pragma once

#include <string>
#include <cstdint>

#include "operations/MenuOps.h"

class ImageOperation;
class OperationsStack;

class StackMenu
{
  public:
    StackMenu(OperationsStack& operation_stack);

    void RenderMenu();
    bool ProcessBegin();

  private:
    OperationsStack& operationsStack;
    bool processBegin = false;

    void MenuItem(ImageOperation * image_operation, size_t index);
};
