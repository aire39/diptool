#pragma once

#include <cstdint>
#include <memory>

class ImageOperation;

namespace MenuOps {

  enum class OpType : uint16_t {
    NONE = 0,
    DOWNSAMPLE,
    UPSAMPLE,
    VARYINGBITS
  };

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
    HIGHBOOST,
    ARITH_MEAN,
    GEO_MEAN,
    MIN,
    MAX,
    MIDPOINT,
    HARMONIC_MEAN,
    CONTRA_HARMONIC_MEAN,
    ALPHA_TRIM_MEAN
  };

  union Op {
    uint16_t op = 0;
    Downsample downsample_op;
    Upsample upsample_op;

    Op & operator=(const Downsample& ds)
    {
      this->downsample_op = ds;
      return *this;
    }

    Op & operator=(const Upsample& us)
    {
      this->upsample_op = us;
      return *this;
    }

    explicit operator Downsample() const
    {
      return downsample_op;
    }

    explicit operator Upsample() const
    {
      return upsample_op;
    }
  };

  struct OpInfo
  {
    std::unique_ptr<ImageOperation> operation;
    MenuOps::Op op;
  };

}
