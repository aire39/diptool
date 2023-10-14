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
    [[nodiscard]] float GetSharpenConstant() const;
    [[nodiscard]] bool IsSharpenFullUse() const;
    [[nodiscard]] bool ShowSharpenFilter() const;
    [[nodiscard]] bool ShowSharpenFilterScaling() const;

  private:
    bool processBegin = false;
    MenuOp_SpatialFilter operation = MenuOp_SpatialFilter::SMOOTHING;
    int32_t currentItem = 0;
    int32_t kernelX = 3;
    int32_t kernelY = 3;
    float sharpenConstant = -1.0f;
    bool isKernelUniform = true;
    bool isSharpUsingFullKernel = false;
    bool showSharpenFilter = false;
    bool showSharpenFilterScaling = false;
};

