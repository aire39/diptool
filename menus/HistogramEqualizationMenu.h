#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include <mutex>

#include "MenuOps.h"

class HistogramEqualizationMenu
{
  public:
    HistogramEqualizationMenu() = default;
    ~HistogramEqualizationMenu() = default;

    void RenderMenu();
    [[nodiscard]] MenuOp_Upsample CurrentOperation() const;
    bool ProcessBegin();

    void SetHistogramData(const std::map<int32_t, float> & histogram_data);
    void SetHistogramRemapData(const std::map<int32_t, float> & histogram_data);

  private:
    bool processBegin = false;
    MenuOp_Upsample operation = MenuOp_Upsample::NEAREST;
    std::map<int32_t, float> histogramNormalized;
    std::map<int32_t, float> histogramNormalizedRemap;
    std::vector<float> histogramSourceValues = std::vector<float>(256);
    std::vector<float> histogramRemapValues = std::vector<float>(256);
    std::mutex histogramMtx;
};
