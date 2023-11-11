#include "RunLengthCodec.h"

#include <cmath>
#include <spdlog/spdlog.h>

namespace {
  //https://stackoverflow.com/questions/551579/get-bytes-from-a-stdvectorbool
  void writeAsBytes(const std::vector<bool> &inBits, std::vector<uint8_t> &outBytes)
  {
    int32_t bitOffset = 0;
    const int maxBitOffset = (int) inBits.size();

    int numBytes = static_cast<int32_t>(inBits.size() / 8);
    if ((inBits.size() % 8) != 0)
    {
      numBytes += 1;
    }

    for (int32_t bytei = 0; bytei < numBytes; bytei++)
    {
      // Consume next 8 bits

      uint8_t byteVal = 0;

      for (int32_t biti = 0; biti < 8; biti++ )
      {
        if (bitOffset >= maxBitOffset)
        {
          break;
        }

        bool bit = inBits[bitOffset++];

        // Flush 8 bits to backing array of bytes.

        byteVal |= (bit << (7 - biti));
      }

      outBytes.push_back(byteVal);
    }
  }
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
  bitplaneMap.clear();
  bitplaneCountMap.clear();
  bitplaneRLECountMap.clear();
  countMap.clear();

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

  for (const auto & bpm : bitplaneMap)
  {
    auto bpm_start = bpm[0];
    std::vector<uint32_t> bit_count;

    uint32_t largest_number_of_bits = 0;
    uint32_t largest_number_counter = 0;

    for (bool i : bpm)
    {
      if (bpm_start != i)
      {
        bpm_start = i;
        bit_count.push_back(largest_number_counter);

        largest_number_of_bits = (largest_number_counter > largest_number_of_bits) ? largest_number_counter : largest_number_of_bits;
        largest_number_counter = 0;
      }

      largest_number_counter++;
    }

    largest_number_of_bits = (largest_number_counter > largest_number_of_bits) ? largest_number_counter : largest_number_of_bits;
    bit_count.push_back(largest_number_counter);

    uint32_t largest_n_bits_plane = std::ceil(std::log2(largest_number_of_bits));
    uint8_t largest_n_bytes_plane = static_cast<uint8_t>(std::ceil(static_cast<float>(largest_n_bits_plane) / 8.0f)) << 4;

    auto rle_info = static_cast<uint8_t>(bpm[0]);
    rle_info |= largest_n_bytes_plane;

    bitplaneRLECountMap.emplace_back(rle_info, bit_count);
  }

  for (const auto & [start_value, bprle] : bitplaneRLECountMap)
  {
    // add the start byte of the plane and extract the largest number of bytes needed to represent
    // each bit pass (n_bytes_to_add)

    encodeBMap += static_cast<char>(start_value);
    uint32_t n_bytes_to_add = (0xF0 & start_value) >> 4;

    spdlog::info("start value index: {} ({:x})", encodeBMap.size()-1, encodeBMap.size()-1);

    // this will add the minimal amount of bytes needed to represent the number of
    // running bits for each 1 and 0 of the bit-plane. The number of bytes needed
    // is represented at the start of a plane in the upper 4 bits while the start
    // bit is in the lower 4-bits

    for (const uint32_t & v : bprle)
    {
      for(int i=0; i<n_bytes_to_add; i++)
      {
        encodeBMap += static_cast<char>(v >> (i*8) & 0xFF);
      }
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
  const uint32_t image_pixel_size = width * height;

  const uint8_t * data_position = (image_data.data() + 8);

  std::vector<uint8_t> image (image_data_size, 0);

  // iterate through each bit-plane in the encoded data.

  for (size_t m=0; m<n_bit_planes; m++)
  {
    bool start_bit = (data_position[0] & 0x0F) == 1;
    uint8_t max_bytes = (data_position[0] & 0xF0) >> 4;

    uint32_t bit_mask_shift = (sizeof(uint32_t) - max_bytes) * 8;
    uint32_t bit_mask = 0xFFFFFFFF << bit_mask_shift;

    uint32_t bit_count = 0;

    data_position++;

    uint32_t cc = 0;
    while (bit_count < image_pixel_size)
    {
      uint32_t add_n_bits = ((*reinterpret_cast<const uint32_t*>(&data_position[0])) & (bit_mask >> bit_mask_shift));

      // If there start bit is 1 then that plane bit will be added to every pixel. No need to "add" 0's since the image
      // data is already initialized with all bits set to 0. so entry of 0's are skipped but note by how much is needed
      // to be skipped as this still needs to added to the bit_count.

      if (start_bit)
      {
        for (size_t i=bit_count; i<(bit_count + add_n_bits); i++)
        {
          image[(i * 4) + 0] |= (0x80 >> m);
          image[(i * 4) + 1] |= (0x80 >> m);
          image[(i * 4) + 2] |= (0x80 >> m);
          image[(i * 4) + 3]  = 255;
        }
      }
      else
      {
        for (size_t i=bit_count; i<(bit_count + add_n_bits); i++)
        {
          image[(i * 4) + 3]  = 255;
        }
      }

      start_bit ^= true;
      data_position += max_bytes;
      bit_count += add_n_bits;
      cc++;
    }
  }

  return image;
}
