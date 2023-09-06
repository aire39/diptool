#pragma once

#include <string>
#include <cstdint>

#include "MenuOps.h"

class DownsampleMenu
{
  public:
    DownsampleMenu() = default;

    void RenderMenu();

  private:
    MenuOp_Downsample operation = MenuOp_Downsample::NEAREST;

};
