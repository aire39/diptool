#pragma once

#include "MenuOps.h"

class SpatialFilterMenu
{
  public:
    SpatialFilterMenu() = default;
    ~SpatialFilterMenu() = default;

    void RenderMenu();
    [[nodiscard]] MenuOp_SpatialFilter CurrentOperation();
    [[nodiscard]] bool ProcessBegin();

    [[nodiscard]] int32_t GetKernelX() const;
    [[nodiscard]] int32_t GetKernelY() const;

  private:
    bool processBegin = false;
    MenuOp_SpatialFilter operation = MenuOp_SpatialFilter::SMOOTHING;
    int32_t currentItem = 0;
    int32_t kernelX = 3;
    int32_t kernelY = 3;
    bool isKernelUniform = true;
};

