#pragma once

#include <string>
#include <cstdint>

#include "MenuOps.h"

class UpsampleMenu
{
  public:
    UpsampleMenu() = default;

    void RenderMenu();
    [[nodiscard]] MenuOp_Upsample CurrentOperation() const;
    [[nodiscard]] int32_t UpsampleIterations() const;
    bool ProcessBegin();

  private:
    bool processBegin = false;
    MenuOp_Upsample operation = MenuOp_Upsample::NEAREST;
    int32_t upsamepleIters = 0;
};
