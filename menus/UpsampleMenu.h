#pragma once

#include <string>
#include <cstdint>

#include "operations/MenuOps.h"

class UpsampleMenu
{
  public:
    UpsampleMenu() = default;

    void RenderMenu();
    [[nodiscard]] MenuOps::Upsample CurrentOperation() const;
    [[nodiscard]] int32_t UpsampleIterations() const;
    bool ProcessBegin();

  private:
    bool processBegin = false;
    MenuOps::Upsample operation = MenuOps::Upsample::NEAREST;
    int32_t upsamepleIters = 0;
};
