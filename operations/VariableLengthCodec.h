#pragma once

#include <string>
#include <cstdint>
#include <vector>

class VariableLengthCodec
{
  public:
    VariableLengthCodec() = default;

    std::string Encode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp);
    static std::vector<uint8_t> Decode(const std::vector<uint8_t> & image_data);

  private:
    std::string encodeMap;
};
