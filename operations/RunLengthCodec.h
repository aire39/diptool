#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include <utility>

class RunLengthCodec
{
  public:
    RunLengthCodec() = default;

    std::string Encode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp);
    std::string BEncode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp);
    static std::vector<uint8_t> Decode(const std::vector<uint8_t> & image_data);
    static std::vector<uint8_t> BDecode(const std::vector<uint8_t> & image_data);

  private:
    std::map<uint8_t, std::vector<size_t>> grayImageValues;
    std::string encodeMap;
    std::string encodeBMap;
    std::vector<std::vector<uint8_t>> countMap;
    std::vector<std::vector<bool>> bitplaneMap;
    std::vector<std::vector<uint8_t>> bitplaneCountMap;
    std::vector<std::pair<uint8_t, std::vector<uint32_t>>> bitplaneRLECountMap;
};
