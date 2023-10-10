#pragma once

#include <map>
#include "HistogramOp.h"

class HistogramEqualizationOp : public HistogramOp
{
  public:
    HistogramEqualizationOp() = default;
    ~HistogramEqualizationOp() = default;

    [[nodiscard]] const std::map<int32_t, float> & GetHistogramRemap() override;
    [[nodiscard]] const std::map<int32_t, float> & GetHistogramRemapRed() override;
    [[nodiscard]] const std::map<int32_t, float> & GetHistogramRemapGreen() override;
    [[nodiscard]] const std::map<int32_t, float> & GetHistogramRemapBlue() override;

    [[nodiscard]] const MenuOp_HistogramColor & HistogramColorType() const;

    void SetHistogramColorType(MenuOp_HistogramColor color_type);
    void SetLocalizeKernelSize(int32_t x, int32_t y);

  protected:
    void ProcessHistogram(MenuOp_HistogramMethod operation
                         ,const std::vector<uint8_t> & source_image
                         ,uint8_t bpp) override;

  private:
    std::map<int, int> remappedValuesGray;
    std::map<int, int> remappedValuesRed;
    std::map<int, int> remappedValuesGreen;
    std::map<int, int> remappedValuesBlue;
    std::map<int32_t, float> remappedValuesNormalized;
    std::map<int32_t, float> remappedValuesNormalizedRed;
    std::map<int32_t, float> remappedValuesNormalizedGreen;
    std::map<int32_t, float> remappedValuesNormalizedBlue;

    MenuOp_HistogramColor inputColorType = MenuOp_HistogramColor::RGBA;
    MenuOp_HistogramMethod histogramMethod = MenuOp_HistogramMethod::GLOBAL;

    int32_t kernelSizeX = 3;
    int32_t kernelSizeY = 3;

    void GlobalProcess(const std::vector<uint8_t> & source_image
                      ,uint8_t bpp);

    void LocalizeProcess(const std::vector<uint8_t> & source_image
                        ,uint8_t bpp);
};
