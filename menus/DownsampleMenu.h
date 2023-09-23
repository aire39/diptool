#pragma once

#include <string>
#include <cstdint>

#include "operations/MenuOps.h"

class DownsampleMenu
{
  public:
    DownsampleMenu() = default;

    void RenderMenu();
    [[nodiscard]] MenuOps::Downsample CurrentOperation() const;
    [[nodiscard]] int32_t DownsampleIterations() const;
    bool ProcessBegin();

  private:
    bool processBegin = false;
    MenuOps::Downsample operation = MenuOps::Downsample::DECIMATE;
    int32_t downsamepleIters = 0;
};
