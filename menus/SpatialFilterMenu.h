#pragma once

#include "operations/MenuOps.h"

class SpatialFilterMenu
{
  public:
    SpatialFilterMenu() = default;
    ~SpatialFilterMenu() = default;

    void RenderMenu();
    [[nodiscard]] MenuOps::SpatialFilter CurrentOperation();
    [[nodiscard]] bool ProcessBegin();

    [[nodiscard]] int32_t GetKernelX() const;
    [[nodiscard]] int32_t GetKernelY() const;
    [[nodiscard]] float GetSharpenConstant() const;
    [[nodiscard]] float GetUnsharpConstant() const;
    [[nodiscard]] float GetContraHarminocConstant() const;
    [[nodiscard]] int32_t GetAlphaTrimConstant() const;
    [[nodiscard]] bool IsSharpenFullUse() const;
    [[nodiscard]] bool ShowSharpenFilter() const;
    [[nodiscard]] bool ShowSharpenFilterScaling() const;
    [[nodiscard]] bool InvertSharpenFilterScaling() const;
    [[nodiscard]] bool ShowUnSharpenFilter() const;
    [[nodiscard]] bool ShowUnSharpenFilterScaling() const;


  private:
    bool processBegin = false;
    MenuOps::SpatialFilter operation = MenuOps::SpatialFilter::SMOOTHING;
    int32_t currentItem = 0;
    int32_t kernelX = 3;
    int32_t kernelY = 3;
    float sharpenConstant = -1.0f;
    float unsharpConstant = 1.0f;
    float contraHarminocConstant = 1.0f;
    int32_t alphaTrimConstant = 1;
    bool isKernelUniform = true;
    bool isSharpUsingFullKernel = false;
    bool showSharpenFilter = false;
    bool showSharpenFilterScaling = false;
    bool invertSharpenFilterScaling = true;
    bool showUnSharpenFilter = false;
    bool showUnSharpenFilterScaling = false;
};

