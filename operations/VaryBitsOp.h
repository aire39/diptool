#pragma once

#include <cstdint>
#include <vector>
#include <set>

class VaryBitsOp
{
  public:
    VaryBitsOp() = default;
    ~VaryBitsOp() = default;

    std::vector<uint8_t> ProcessImage(int32_t bit_level_operation
                                     ,bool bit_contrast
                                     ,const std::vector<uint8_t> & source_image
                                     ,uint32_t width
                                     ,uint32_t height
                                     ,uint8_t bpp);

    [[nodiscard]] const std::vector<uint8_t> & GetImage() const;

    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;

    [[nodiscard]] std::set<uint32_t> GetUniquePixelValues() const;

    void SetUseColorChannels(bool use_color_channels);

  private:
    int32_t outWidth = 0;
    int32_t outHeight = 0;
    std::set<uint32_t> uniquePixelValues;
    std::vector<uint8_t> result;
    bool useColor = false;

    void BitLevelAlgorithm(const std::vector<uint8_t> & source_image
                          ,uint32_t width
                          ,uint32_t height
                          ,uint8_t bpp
                          ,uint32_t bit_level
                          ,bool bit_contrast);


};
