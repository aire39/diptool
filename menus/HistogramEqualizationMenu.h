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

    void SetHistogramData(std::vector<std::map<int32_t, float>> & histogram_data);
    void SetHistogramRemapData(std::vector<std::map<int32_t, float>> & histogram_data);

    void SetProcessTime(float process_time);

    bool IsHistogramColorTypeGray() const;
    bool IsHistogramColorTypeRGBA() const;
    bool IsGlobalMethodType() const;
    bool IsLocalizeMethodType() const;

    int32_t GetKernelX() const;
    int32_t GetKernelY() const;

    void ClearData();

  private:
    bool processBegin = false;
    MenuOp_Upsample operation = MenuOp_Upsample::NEAREST;
    std::vector<std::map<int32_t, float>> histogramNormalized;
    std::vector<std::map<int32_t, float>> histogramNormalizedRemap;
    std::vector<std::vector<float>> histogramSourceValues = {std::vector<float>(256), std::vector<float>(256), std::vector<float>(256)};
    std::vector<std::vector<float>> histogramRemapValues = {std::vector<float>(256), std::vector<float>(256), std::vector<float>(256)};
    int32_t setColorType = 0;
    int32_t setMethodType = 0;
    int32_t localizeKernelX = 3;
    int32_t localizeKernelY = 3;
    float processTimeSecs = 0.0f;

    std::mutex histogramMtx;
};
