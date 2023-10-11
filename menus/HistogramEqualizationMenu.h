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
    [[nodiscard]] MenuOp_HistogramMethod CurrentOperation() const;
    bool ProcessBegin();

    void SetHistogramData(std::vector<std::map<int32_t, float>> & histogram_data);
    void SetHistogramRemapData(std::vector<std::map<int32_t, float>> & histogram_data);

    void SetProcessTime(float process_time);

    bool IsHistogramColorTypeGray() const;
    bool IsHistogramColorTypeRGBA() const;
    bool IsGlobalMethodType() const;
    bool IsLocalizeMethodType() const;
    bool IsLocalizeEnchancementMethodType() const;

    int32_t GetKernelX() const;
    int32_t GetKernelY() const;
    float GetKernelK0() const;
    float GetKernelK1() const;

    void ClearData();

  private:
    bool processBegin = false;
    MenuOp_HistogramMethod operation = MenuOp_HistogramMethod::GLOBAL;
    std::vector<std::map<int32_t, float>> histogramNormalized;
    std::vector<std::map<int32_t, float>> histogramNormalizedRemap;
    std::vector<std::vector<float>> histogramSourceValues = {std::vector<float>(256), std::vector<float>(256), std::vector<float>(256)};
    std::vector<std::vector<float>> histogramRemapValues = {std::vector<float>(256), std::vector<float>(256), std::vector<float>(256)};
    int32_t setColorType = 0;
    int32_t setMethodType = 0;
    int32_t localizeKernelX = 3;
    int32_t localizeKernelY = 3;
    float localizeKernelK0 = 0.25f;
    float localizeKernelK1 = 0.75f;
    float processTimeSecs = 0.0f;

    std::mutex histogramMtx;
};
