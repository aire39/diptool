#pragma once

#include <cstdint>

namespace MenuOps {
enum class Downsample : uint16_t {
  DECIMATE = 0,
  NEAREST
};

enum class Upsample : uint16_t {
  NEAREST = 0,
  LINEAR,
  BILINEAR
};

enum class HistogramColor : uint16_t {
  GRAY = 0,
  RGBA
};

enum class HistogramMethod : uint16_t {
  GLOBAL = 0,
  LOCALIZE,
  LOCALIZE_ENCHANCEMENT
};

enum class SpatialFilter : uint16_t {
  SMOOTHING = 0,
  MEDIAN,
  SHARPENING,
  HIGHBOOST
};

}

