#pragma once

#include <map>
#include <vector>
#include <cstdint>
#include "MenuOps.h"

class HistogramOp
{
  public:
    HistogramOp() = default;
    ~HistogramOp() = default;

    std::vector<uint8_t> ProcessImage(MenuOp_Downsample operation
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations);

    [[nodiscard]] const std::vector<uint8_t> & GetImage() const;
    [[nodiscard]] const std::map<int32_t, float> & GetHistogram() const;
    [[nodiscard]] virtual const std::map<int32_t, float> & GetHistogramRemap();

    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;

  protected:
    virtual void ProcessHistogram(MenuOp_Downsample operation
                                 ,const std::vector<uint8_t> & source_image
                                 ,uint8_t bpp);
    int32_t outWidth = 0;
    int32_t outHeight = 0;
    int32_t minPixelValueGray = 0;
    int32_t maxPixelValueGray = 0;
    int32_t minPixelValueRed = 0;
    int32_t maxPixelValueRed = 0;
    int32_t minPixelValueGreen = 0;
    int32_t maxPixelValueGreen = 0;
    int32_t minPixelValueBlue = 0;
    int32_t maxPixelValueBlue = 0;
    std::map<int32_t, std::vector<int32_t>> histogramPixelValuesGray; // key => pixel value, value => pixel index
    std::map<int32_t, float> histogramNormalizedGray; // key => pixel value, value => normalized amount of pixels for the key value
    std::map<int32_t, std::vector<int32_t>> histogramPixelValuesRed; // key => pixel value, value => pixel index
    std::map<int32_t, float> histogramNormalizedRed; // key => pixel value, value => normalized amount of pixels for the key value
    std::map<int32_t, std::vector<int32_t>> histogramPixelValuesGreen; // key => pixel value, value => pixel index
    std::map<int32_t, float> histogramNormalizedGreen; // key => pixel value, value => normalized amount of pixels for the key value
    std::map<int32_t, std::vector<int32_t>> histogramPixelValuesBlue; // key => pixel value, value => pixel index
    std::map<int32_t, float> histogramNormalizedBlue; // key => pixel value, value => normalized amount of pixels for the key value

    static constexpr int32_t maxBppValue = 255;

    std::vector<uint8_t> result;
};
