#pragma once

#include <cstdint>

enum class MenuOp_Downsample : uint16_t {
  DECIMATE = 0,
  NEAREST
};

enum class MenuOp_Upsample : uint16_t {
  NEAREST = 0,
  LINEAR,
  BILINEAR
};