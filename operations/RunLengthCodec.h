#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <vector>

class RunLengthCodec
{
  public:
    RunLengthCodec() = default;

    std::string AsciiEncode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp);
    std::string Encode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp);
    static std::vector<uint8_t> Decode(const std::vector<uint8_t> & image_data);

  private:
    std::map<uint8_t, std::vector<size_t>> grayImageValues;
    std::string asciiEncodeMap;
    std::string encodeMap;
    std::vector<std::vector<uint8_t>> countMap;
};
