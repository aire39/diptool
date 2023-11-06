#include "RunLengthCodec.h"

#include <cmath>
#include <iostream>

namespace {
  //https://stackoverflow.com/questions/551579/get-bytes-from-a-stdvectorbool
  void writeAsBytes(const std::vector<bool> & inBits, std::vector<uint8_t> & outBytes) {
    int32_t bitOffset = 0;
    const int maxBitOffset = (int) inBits.size();

    const bool emitMSB = true;

    int numBytes = static_cast<int32_t>(inBits.size() / 8);
    if ((inBits.size() % 8) != 0) {
      numBytes += 1;
    }

    for (int32_t bytei = 0; bytei < numBytes; bytei++) {
      // Consume next 8 bits

      uint8_t byteVal = 0;

      for (int32_t biti = 0; biti < 8; biti++ ) {
        if (bitOffset >= maxBitOffset) {
          break;
        }

        bool bit = inBits[bitOffset++];

        // Flush 8 bits to backing array of bytes.
        // Note that bits can be written as either
        // LSB first (reversed) or MSB first (not reversed).

        if (emitMSB) {
          byteVal |= (bit << (7 - biti));
        } else {
          byteVal |= (bit << biti);
        }
      }

      outBytes.push_back(byteVal);
    }
  }
}

std::string RunLengthCodec::AsciiEncode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp)
{
  uint16_t current_value = 0;
  for (size_t i=0; i<(bpp-1); i++)
  {
    current_value += image_data[i];
  }

  current_value /= bpp;

  uint32_t total_pixel_count = 0;
  uint16_t value_count = 0;
  for (size_t i=0; i<image_data.size(); i+=bpp)
  {
    total_pixel_count++;
    uint16_t value = 0;
    for (size_t k=0; k<(bpp-1); k++)
    {
      value += image_data[i + k];
    }

    value /= (bpp-1);

    if (value != current_value)
    {
      value_count = value_count == 0 ? 1 : value_count;

      std::string value_encode;
      value_encode += std::to_string(value);
      value_encode += "!";
      value_encode += std::to_string(value_count);
      value_encode += " ";

      asciiEncodeMap += value_encode;
      current_value = value;

      value_count = 0;
    }
    else
    {
      value_count++;
    }
  }

  return asciiEncodeMap;
}

std::string RunLengthCodec::Encode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp)
{
  countMap.clear();

  encodeMap =  static_cast<char>(width & 0xFF);
  encodeMap += static_cast<char>(width >> 8 & 0xFF);
  encodeMap += static_cast<char>(width >> 16 & 0xFF);
  encodeMap += static_cast<char>(width >> 24 & 0xFF);

  encodeMap += static_cast<char>(height & 0xFF);
  encodeMap += static_cast<char>(height >> 8 & 0xFF);
  encodeMap += static_cast<char>(height >> 16 & 0xFF);
  encodeMap += static_cast<char>(height >> 24 & 0xFF);

  uint16_t current_value = 0;
  for (size_t i=0; i<(bpp-1); i++)
  {
    current_value += image_data[i];
  }

  current_value /= (bpp-1);

  std::vector<uint8_t> cc;

  for (size_t i=0; i<image_data.size(); i+=bpp)
  {
    uint32_t value = 0;
    for (size_t k=0; k<(bpp-1); k++)
    {
      value += image_data[i + k];
    }

    value /= (bpp-1);

    if (value != current_value)
    {
      current_value = value;
      countMap.push_back(cc);
      cc = std::vector<uint8_t>();
    }

    cc.push_back(value);
  }

  countMap.push_back(cc);

  for (const auto & cm : countMap)
  {
    uint8_t pixel_value = cm[0]; // grab the 1st element which has the pixel value
    uint32_t n_pixels = cm.size(); // number of pixels for the pixel value

    std::string value_encode;
    value_encode += static_cast<char>(pixel_value);
    value_encode += static_cast<char>(n_pixels & 0xFF);
    value_encode += static_cast<char>((n_pixels >> 8) & 0xFF);
    value_encode += static_cast<char>((n_pixels >> 16) & 0xFF);
    value_encode += static_cast<char>((n_pixels >> 24) & 0xFF);

    encodeMap += value_encode;
  }

  return encodeMap;
}

std::string RunLengthCodec::BEncode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp)
{
  bitplaneCountMap.clear();

  encodeBMap =  static_cast<char>(width & 0xFF);
  encodeBMap += static_cast<char>(width >> 8 & 0xFF);
  encodeBMap += static_cast<char>(width >> 16 & 0xFF);
  encodeBMap += static_cast<char>(width >> 24 & 0xFF);

  encodeBMap += static_cast<char>(height & 0xFF);
  encodeBMap += static_cast<char>(height >> 8 & 0xFF);
  encodeBMap += static_cast<char>(height >> 16 & 0xFF);
  encodeBMap += static_cast<char>(height >> 24 & 0xFF);

  const uint8_t n_bit_planes = std::ceil(std::log2(std::pow(2, 8 * sizeof(uint8_t))));
  const size_t image_data_size = width * height * bpp;

  for (size_t m=0; m<n_bit_planes; m++)
  {
    std::vector<bool> bit_plane;

    for (size_t i=0; i<image_data_size; i+=4)
    {
      uint16_t gray = (image_data[i + 0] + image_data[i + 1] + image_data[i + 2]) / 3;
      uint8_t value = (gray & (0x80 >> m)) >> (7 - m);
      bit_plane.push_back(value == 1);
    }

    bitplaneMap.push_back(bit_plane);

    std::vector<uint8_t> bit_pack_to_bytes;
    writeAsBytes(bit_plane, bit_pack_to_bytes);
    bitplaneCountMap.push_back(bit_pack_to_bytes);
  }

  for (const auto & bp : bitplaneCountMap)
  {
    for (const auto & bp_data : bp)
    {
      encodeBMap += static_cast<char>(bp_data);
    }
  }

  return encodeBMap;
}

std::vector<uint8_t> RunLengthCodec::Decode(const std::vector<uint8_t> & image_data)
{
  const uint32_t width = *reinterpret_cast<const uint32_t*>(&image_data[0]);
  const uint32_t height = *reinterpret_cast<const uint32_t*>(&image_data[4]);

  std::vector<uint8_t> image (width * height * 4);

  uint32_t pixel_index = 0;
  for (uint32_t i=8; i<image_data.size(); i+=5)
  {
    auto & value = image_data[i];
    const auto & value_count = *reinterpret_cast<const uint32_t*>(&image_data[i+1]);

    for (size_t k=0; k<=value_count; k++)
    {
      uint32_t red_index   = ((pixel_index + k) * 4) + 0;
      uint32_t green_index = ((pixel_index + k) * 4) + 1;
      uint32_t blue_index  = ((pixel_index + k) * 4) + 2;
      uint32_t alpha_index = ((pixel_index + k) * 4) + 3;

      image[red_index]   = value;
      image[green_index] = value;
      image[blue_index]  = value;
      image[alpha_index] = 255;
    }

    pixel_index += value_count;
  }

  return image;
}

std::vector<uint8_t> RunLengthCodec::BDecode(const std::vector<uint8_t> & image_data)
{
  // width and height values occupy the first 8 bytes of the encoded file

  const auto width = *reinterpret_cast<const uint32_t*>(&image_data[0]);
  const auto height = *reinterpret_cast<const uint32_t*>(&image_data[4]);

  const uint8_t n_bit_planes = std::ceil(std::log2(std::pow(2, 8 * sizeof(uint8_t))));
  const size_t image_data_size = width * height * 4;
  const size_t image_pixel_size = width * height;

  const uint8_t * data_position = (image_data.data() + 8);

  std::vector<uint8_t> image (image_data_size, 0);

  // iterate through each bit-plane in the encoded data.

  for (size_t m=0; m<n_bit_planes; m++)
  {
    for (size_t i=0; i<image_pixel_size; i++)
    {
      // iterate through the encoded bits per plane ,so we set where the data is for each bit plane starts.
      // make sure the index that we want matches up with the bits in the byte as each bit in a byte represents
      // a bit in a pixel. The ANDing of the data value and bit_index should let us know if a bit exist for the pixel
      // for the current iterated bit-plane.
      // If a bit exists then we should OR against the pixel value to add the bit otherwise it will just remain 0 since
      // the image data is initialized to 0 anyway.

      auto data_pos_index_start = static_cast<size_t>(m * (size_t)std::ceil((double)image_pixel_size / 8.0));
      auto data_pos_index = (i / 8);
      uint8_t bit_index = 0x80 >> (i % 8);
      uint8_t data_value = data_position[data_pos_index_start + data_pos_index];

      bool add_bit = (data_value & bit_index) > 0;

      if (add_bit)
      {
        image[(i * 4) + 0] |= (0x80 >> m);
        image[(i * 4) + 1] |= (0x80 >> m);
        image[(i * 4) + 2] |= (0x80 >> m);
      }

      image[(i * 4) + 3]  = 255;
    }
  }

  return image;
}
