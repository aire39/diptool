#include "RunLengthCodec.h"

#include <iostream>

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

      std::cout << value_encode << std::endl;

      value_count = 0;
    }
    else
    {
      value_count++;
    }
  }

  std::cout << "total pixel count: " << total_pixel_count << std::endl;

  return asciiEncodeMap;
}

std::string RunLengthCodec::Encode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp)
{
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
