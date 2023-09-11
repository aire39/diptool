#pragma once

#include <string>
#include <cstdint>

#include "MenuOps.h"

class DownsampleMenu
{
  public:
    DownsampleMenu() = default;

    void RenderMenu();
    [[nodiscard]] MenuOp_Downsample CurrentOperation() const;
    [[nodiscard]] int32_t DownsampleIterations() const;
    bool ProcessBegin();

  private:
    bool processBegin = false;
    MenuOp_Downsample operation = MenuOp_Downsample::DECIMATE;
    int32_t downsamepleIters = 0;
};
