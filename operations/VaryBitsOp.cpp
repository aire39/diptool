#include "VaryBitsOp.h"

#include <array>
#include <algorithm>
#include <limits>
#include <spdlog/spdlog.h>

namespace
{
  std::array<uint8_t, 3> pixel_gather(const int32_t & x
                                     ,const int32_t & y
                                     ,const int32_t & w
                                     ,const int32_t & h
                                     ,const int32_t & number_of_pixel_neighbors
                                     ,const int32_t & bpp
                                     ,const std::vector<uint8_t> & source_image)
  {
    int32_t pixel_value_red = 0;
    int32_t pixel_value_green = 0;
    int32_t pixel_value_blue = 0;

    for (int32_t i=0; i<(number_of_pixel_neighbors + 1); i++)
    {
      const size_t pixel_byte_index_r = ((x + i) * bpp) + (y * w * bpp) + 3;
      if ((pixel_byte_index_r > 0) && (pixel_byte_index_r < w))
      {
        pixel_value_red += static_cast<int32_t>(source_image[((x + i) * bpp) + (y * w * bpp) + 0]);
        pixel_value_green += static_cast<int32_t>(source_image[((x + i) * bpp) + (y * w * bpp)] + 1);
        pixel_value_blue += static_cast<int32_t>(source_image[((x + i) * bpp) + (y * w * bpp)] + 2);
      }
      else
      {
        const int32_t x_fix = std::clamp(x, 0, (w - 1));
        const int32_t y_fix = std::clamp(y, 0, (h - 1));

        pixel_value_red += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp)] + 0);
        pixel_value_green += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp)] + 1);
        pixel_value_blue += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp)] + 2);
      }
    }

    pixel_value_red /= (number_of_pixel_neighbors + 1);
    pixel_value_red = std::clamp(pixel_value_red, 0, static_cast<int32_t>(std::numeric_limits<uint8_t>::max()));

    pixel_value_green /= (number_of_pixel_neighbors + 1);
    pixel_value_green = std::clamp(pixel_value_green, 0, static_cast<int32_t>(std::numeric_limits<uint8_t>::max()));

    pixel_value_blue /= (number_of_pixel_neighbors + 1);
    pixel_value_blue = std::clamp(pixel_value_blue, 0, static_cast<int32_t>(std::numeric_limits<uint8_t>::max()));

    return {static_cast<uint8_t>(pixel_value_red)
        ,static_cast<uint8_t>(pixel_value_green)
        ,static_cast<uint8_t>(pixel_value_blue)};
  }

  void set_pixel(const uint32_t & x
                ,const uint32_t & y
                ,const uint32_t & width
                ,const uint32_t & bpp
                ,std::vector<uint8_t> & image
                ,const std::array<uint8_t, 3> & pixel_color_rgb)
  {
    image[(x * bpp) + (y * width * bpp) + 0] = pixel_color_rgb[0];
    image[(x * bpp) + (y * width * bpp) + 1] = pixel_color_rgb[1];
    image[(x * bpp) + (y * width * bpp) + 2] = pixel_color_rgb[2];
    image[(x * bpp) + (y * width * bpp) + 3] = 255;
  }
}

std::vector<uint8_t> VaryBitsOp::ProcessImage(int32_t bit_level_operation
                                             ,bool bit_contrast
                                             ,const std::vector<uint8_t> & source_image
                                             ,uint32_t width
                                             ,uint32_t height
                                             ,uint8_t bpp)
{
  spdlog::info("varying bits level: {}", bit_level_operation);
  BitLevelAlgorithm(source_image, width, height, bpp, bit_level_operation, bit_contrast);
  return result;
}

const std::vector<uint8_t> & VaryBitsOp::GetImage() const
{
  return result;
}

int32_t VaryBitsOp::GetWidth() const
{
  return outWidth;
}

int32_t VaryBitsOp::GetHeight() const
{
  return outHeight;
}

void VaryBitsOp::BitLevelAlgorithm(const std::vector<uint8_t> & source_image
                                  ,uint32_t width
                                  ,uint32_t height
                                  ,uint8_t bpp
                                  ,uint32_t bit_level
                                  ,bool bit_contrast)
{
  constexpr uint32_t combine_value = 1;
  auto dest_result = result = source_image;

  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  const uint32_t bits_to_shift = 8 - bit_level;

  for (int32_t i=0; i<height; i++)
  {
    for (int32_t j=0; j<width; j++)
    {
      // grab the pixel value of the source and set the pixel to the correct destination
      auto pixel_rgb_value = pixel_gather(j
                                                               ,i
                                                               ,static_cast<int32_t>(width)
                                                               ,static_cast<int32_t>(height)
                                                               ,combine_value
                                                               ,bpp
                                                               ,dest_result);

      if (bits_to_shift > 0)
      {
        uint32_t pixel_value_sum = (pixel_rgb_value[0] + pixel_rgb_value[1] + pixel_rgb_value[2]) / 3;
        auto gray_pixel = static_cast<uint8_t>(pixel_value_sum);

        uint8_t check = (gray_pixel & (0xFF >> bits_to_shift));
        uint32_t value = check > 0 ? gray_pixel & (0xFF >> bits_to_shift) : 0;
        value = bit_contrast ? value << bits_to_shift : value;

        pixel_rgb_value[0] = static_cast<uint8_t>(value);
        pixel_rgb_value[1] = static_cast<uint8_t>(value);
        pixel_rgb_value[2] = static_cast<uint8_t>(value);
      }

      set_pixel(j
               ,i
               ,static_cast<int32_t>(width)
               ,bpp
               ,result
               ,pixel_rgb_value
               );
    }
  }
}
