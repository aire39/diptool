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

enum class MenuOp_HistogramColor : uint16_t {
  GRAY = 0,
  RGBA
};

enum class MenuOp_HistogramMethod : uint16_t {
  GLOBAL = 0,
  LOCAL3X3,
  LOCAL5X5,
  LOCAL9X9
};