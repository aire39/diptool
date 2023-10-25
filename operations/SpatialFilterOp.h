#pragma once

#include <vector>
#include "MenuOps.h"

class SpatialFilterOp
{
  public:
    SpatialFilterOp() = default;
    ~SpatialFilterOp() = default;

    std::vector<uint8_t> ProcessImage(MenuOp_SpatialFilter operation
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations);

    [[nodiscard]] const std::vector<uint8_t> & GetImage() const;

    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;

    void SetKernelSize(int32_t kernel_x, int32_t kernel_y);
    void SetSharpenConstant(float sharp_const);
    void SetSharpenUseFullKernel(bool use_full_kernel);
    void SetUnSharpenConstant(float unsharp_const);
    void ShowSharpenFilter(bool show_sharpen_filter);
    void ShowSharpenFilterScaling(bool show_sharpen_filter);
    void SetContraHarmonicConstant(float q_constant);
    void ShowUnSharpenFilter(bool show_sharpen_filter);
    void ShowUnSharpenFilterScaling(bool show_sharpen_filter);

  private:

    enum class CONV_TYPE : uint16_t {SUM=0, MULT, MIN, MAX, FRAC, POW};

    static double ConvolutionValue(const std::vector<uint8_t> & source
                                 ,int32_t x
                                 ,int32_t y
                                 ,int32_t source_width
                                 ,int32_t source_height
                                 ,int32_t offset
                                 ,int32_t sum_count
                                 ,int32_t bpp
                                 ,const std::vector<float> & kernel
                                 ,int32_t kernel_width
                                 ,int32_t kernel_height
                                 ,float kernel_div
                                 ,CONV_TYPE conv_type = CONV_TYPE::SUM);

    static float MedianValue(const std::vector<uint8_t> & source
                            ,int32_t x
                            ,int32_t y
                            ,int32_t source_width
                            ,int32_t source_height
                            ,int32_t offset
                            ,int32_t sum_count
                            ,int32_t bpp
                            ,int32_t kernel_width
                            ,int32_t kernel_height
                            ,float median_scale_factor);

    void SmoothingFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void MedianFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void SharpenFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void HighBoostFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void ArithMeanFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void GeoMeanFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void MinFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void MaxFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void MidPointFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void HarmonicFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);
    void ContraHarmonicFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp);

    std::vector<uint8_t> result;
    int32_t outWidth = 0;
    int32_t outHeight = 0;
    int32_t kernelX = 3;
    int32_t kernelY = 3;
    float sharpenConstant = -1.0f;
    float unsharpConstant = 1.0f;
    float contraHarmonicConstant = 1.0f;
    bool sharpUseFullKernel = false;
    bool showSharpenFilter = false;
    bool showSharpenFilterScaling = true;
    bool showUnSharpenFilter = false;
    bool showUnSharpenFilterScaling = true;
};
