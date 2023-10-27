#pragma once

#include <string>
#include <cstdint>
#include <functional>

#include "operations/MenuOps.h"

class DownsampleMenu
{
  public:
    DownsampleMenu() = default;

    void RenderMenu();
    [[nodiscard]] MenuOps::Downsample CurrentOperation() const;
    [[nodiscard]] int32_t DownsampleIterations() const;
    bool ProcessBegin();

    void SetCallback(std::function<void()> cb);

  private:
    bool processBegin = false;
    MenuOps::Downsample operation = MenuOps::Downsample::DECIMATE;
    int32_t downsamepleIters = 0;
    std::function<void()> callback;
};
