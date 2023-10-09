#pragma once

#include <map>
#include "HistogramOp.h"

class HistogramEqualizationOp : public HistogramOp
{
  public:
    HistogramEqualizationOp() = default;
    ~HistogramEqualizationOp() = default;

    [[nodiscard]] const std::map<int32_t, float> & GetHistogramRemap() override;

  protected:
    void ProcessHistogram(MenuOp_Downsample operation
                         ,const std::vector<uint8_t> & source_image
                         ,uint8_t bpp) override;

  private:
    std::map<int, int> remappedValuesGray;
    std::map<int, int> remappedValuesRed;
    std::map<int, int> remappedValuesGreen;
    std::map<int, int> remappedValuesBlue;
    std::map<int32_t, float> remappedValuesNormalized;

    MenuOp_HistogramColor inputColorType = MenuOp_HistogramColor::RGBA;
    MenuOp_HistogramMethod histogramMethod = MenuOp_HistogramMethod::GLOBAL;

    void GlobalProcess(MenuOp_Downsample operation
                      ,const std::vector<uint8_t> & source_image
                      ,uint8_t bpp);
};
