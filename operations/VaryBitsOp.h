#pragma once

#include <cstdint>
#include <vector>
#include <set>
#include <array>
#include "ImageOperation.h"

class VaryBitsOp : public ImageOperation
{
  public:
    VaryBitsOp();
    ~VaryBitsOp() = default;

    std::vector<uint8_t> ProcessImage(MenuOps::Op op
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp
                                     ,uint16_t iterations) override;

    std::vector<uint8_t> ProcessImage(int32_t bit_level_operation
                                     ,bool bit_contrast
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp);

    [[nodiscard]] std::set<uint32_t> GetUniquePixelValues() const;

    void SetUseColorChannels(bool use_color_channels);
    void SetShowBitPlanes(const std::array<bool, 8> & show_bit_planes);

  private:
    std::set<uint32_t> uniquePixelValues;
    bool useColor = false;
    std::array<bool, 8> showBitPlanes = {true};

    void BitLevelAlgorithm(const std::vector<uint8_t> & source_image
                          ,uint32_t width
                          ,uint32_t height
                          ,uint8_t bpp
                          ,uint32_t bit_level
                          ,bool bit_contrast);

    void BitLevelRemovalAlgorithm(const std::vector<uint8_t> & source_image
                                 ,uint32_t width
                                 ,uint32_t height
                                 ,uint8_t bpp
                                 ,uint32_t bit_level
                                 ,bool bit_contrast);


};
