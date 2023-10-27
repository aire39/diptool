#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include <mutex>

#include "operations/MenuOps.h"

class HistogramEqualizationMenu
{
  public:
    HistogramEqualizationMenu() = default;
    ~HistogramEqualizationMenu() = default;

    void RenderMenu();
    [[nodiscard]] MenuOps::HistogramMethod CurrentOperation() const;
    bool ProcessBegin();

    void SetHistogramData(std::vector<std::map<int32_t, float>> & histogram_data);
    void SetHistogramRemapData(std::vector<std::map<int32_t, float>> & histogram_data);

    void SetProcessTime(float process_time);
    void SetSizeOfImage(int32_t pixel_width, int32_t pixel_height);

    bool IsHistogramColorTypeGray() const;
    bool IsHistogramColorTypeRGBA() const;
    bool IsGlobalMethodType() const;
    bool IsLocalizeMethodType() const;
    bool IsLocalizeEnchancementMethodType() const;

    int32_t GetKernelX() const;
    int32_t GetKernelY() const;
    float GetKernelK0() const;
    float GetKernelK1() const;
    float GetKernelK2() const;
    float GetKernelK3() const;
    float GetKernelEnhanceConst() const;

    void ClearData();

  private:
    bool processBegin = false;
    MenuOps::HistogramMethod operation = MenuOps::HistogramMethod::GLOBAL;
    std::vector<std::map<int32_t, float>> histogramNormalized;
    std::vector<std::map<int32_t, float>> histogramNormalizedRemap;
    std::vector<std::vector<float>> histogramSourceValues = {std::vector<float>(256), std::vector<float>(256), std::vector<float>(256)};
    std::vector<std::vector<float>> histogramRemapValues = {std::vector<float>(256), std::vector<float>(256), std::vector<float>(256)};
    int32_t setColorType = 0;
    int32_t setMethodType = 0;
    int32_t localizeKernelX = 3;
    int32_t localizeKernelY = 3;
    float localizeKernelK0 = 0.0f;
    float localizeKernelK1 = 0.25f;
    float localizeKernelK2 = 0.0f;
    float localizeKernelK3 = 0.1f;
    float localizeKernelEnhanceConst = 22.8f;
    float processTimeSecs = 0.0f;
    int32_t imagePixelWidth = 64;
    int32_t imagePixelHeight = 64;
    std::mutex histogramMtx;
};
