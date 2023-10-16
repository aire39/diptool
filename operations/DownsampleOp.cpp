#include "DownsampleOp.h"

#include <array>
#include <algorithm>
#include <limits>
#include <spdlog/spdlog.h>

namespace
{
  std::array<uint8_t, 4> pixel_gather(const int32_t & x
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
    int32_t pixel_value_alpha = 0;

    for (int32_t i=0; i<(number_of_pixel_neighbors+1); i++)
    {
      const size_t pixel_byte_index_r = ((x + i) * bpp) + (y * w * bpp) + 3;
      if ((pixel_byte_index_r > 0) && (pixel_byte_index_r < w))
      {
        pixel_value_red += static_cast<int32_t>(source_image[((x + i) * bpp) + (y * w * bpp) + 0]);
        pixel_value_green += static_cast<int32_t>(source_image[((x + i) * bpp) + (y * w * bpp) + 1]);
        pixel_value_blue += static_cast<int32_t>(source_image[((x + i) * bpp) + (y * w * bpp) + 2]);
        pixel_value_alpha += static_cast<int32_t>(source_image[((x + i) * bpp) + (y * w * bpp) + 3]);
      }
      else
      {
        const int32_t x_fix = std::clamp(x, 0, (w - 1));
        const int32_t y_fix = std::clamp(y, 0, (h - 1));

        pixel_value_red += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp) + 0]);
        pixel_value_green += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp) + 1]);
        pixel_value_blue += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp) + 2]);
        pixel_value_alpha += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp) + 3]);
      }
    }

    pixel_value_red /= (number_of_pixel_neighbors + 1);
    pixel_value_red = std::clamp(pixel_value_red, 0, static_cast<int32_t>(std::numeric_limits<uint8_t>::max()));

    pixel_value_green /= (number_of_pixel_neighbors + 1);
    pixel_value_green = std::clamp(pixel_value_green, 0, static_cast<int32_t>(std::numeric_limits<uint8_t>::max()));

    pixel_value_blue /= (number_of_pixel_neighbors + 1);
    pixel_value_blue = std::clamp(pixel_value_blue, 0, static_cast<int32_t>(std::numeric_limits<uint8_t>::max()));

    pixel_value_alpha /= (number_of_pixel_neighbors + 1);
    pixel_value_alpha = std::clamp(pixel_value_alpha, 0, static_cast<int32_t>(std::numeric_limits<uint8_t>::max()));

    return {static_cast<uint8_t>(pixel_value_red)
           ,static_cast<uint8_t>(pixel_value_green)
           ,static_cast<uint8_t>(pixel_value_blue)
           ,static_cast<uint8_t>(pixel_value_alpha)};
  }

  void set_pixel(const uint32_t & x
                ,const uint32_t & y
                ,const uint32_t & width
                ,const uint32_t & bpp
                ,std::vector<uint8_t> & image
                ,const std::array<uint8_t, 4> & pixel_color_rgb)
  {
    image[(x * bpp) + (y * width * bpp) + 0] = pixel_color_rgb[0];
    image[(x * bpp) + (y * width * bpp) + 1] = pixel_color_rgb[1];
    image[(x * bpp) + (y * width * bpp) + 2] = pixel_color_rgb[2];
    image[(x * bpp) + (y * width * bpp) + 3] = pixel_color_rgb[3];
  }
}

std::vector<uint8_t> DownsampleOp::ProcessImage(MenuOp_Downsample operation
                                               ,const std::vector<uint8_t> & source_image
                                               ,uint32_t width
                                               ,uint32_t height
                                               ,uint8_t bpp
                                               ,uint16_t iterations)
{
  switch (operation)
  {
    case MenuOp_Downsample::DECIMATE:
      spdlog::info("perform decimation operation");
      DecimateAlgorithm(source_image, width, height, bpp, iterations);
      break;

    case MenuOp_Downsample::NEAREST:
      spdlog::info("perform nearest operation");
      NearestAlgorithm(source_image, width, height, bpp, iterations);
      break;
  }

  return result;
}

const std::vector<uint8_t> & DownsampleOp::GetImage() const
{
  return result;
}

int32_t DownsampleOp::GetWidth() const
{
  return outWidth;
}

int32_t DownsampleOp::GetHeight() const
{
  return outHeight;
}

void DownsampleOp::DecimateAlgorithm(const std::vector<uint8_t> & source_image
                                    ,uint32_t width
                                    ,uint32_t height
                                    ,uint8_t bpp
                                    ,uint16_t iterations)
{
  constexpr uint32_t combine_value = 0;

  auto dest_result = result = source_image;

  outWidth = static_cast<int32_t>(width >> iterations);
  outHeight = static_cast<int32_t>(height >> iterations);

  for (int32_t r=0; r<iterations; r++)
  {
    const auto new_size = static_cast<uint32_t>(std::floor(width >> (r+1)))
                                    * static_cast<uint32_t>(std::floor(height >> (r+1)))
                                    * bpp;

    result.resize(new_size, 0);

    for (int32_t i=0; i<height; i+=2)
    {
      for (int32_t j=0; j<width; j+=2)
      {
        // average the 4 neighboring pixels and set the new pixel value in the output image buffer
        auto pixel_rgb_value_0 = pixel_gather(j
                                             ,i
                                             ,static_cast<int32_t>(width >> r)
                                             ,static_cast<int32_t>(height >> r)
                                             ,combine_value
                                             ,bpp
                                             ,dest_result);

        std::array<uint8_t, 4> pixel_rgb_value = {static_cast<uint8_t>(pixel_rgb_value_0[0])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_0[1])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_0[2])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_0[3])};

        set_pixel((j / 2)
                 ,(i / 2)
                 ,static_cast<int32_t>(width >> (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);
      }
    }

    dest_result = result;
  }
}

void DownsampleOp::NearestAlgorithm(const std::vector<uint8_t> & source_image
                                   ,uint32_t width
                                   ,uint32_t height
                                   ,uint8_t bpp
                                   ,uint16_t iterations)
{
  constexpr uint32_t combine_value = 1;

  auto dest_result = result = source_image;

  outWidth = static_cast<int32_t>(width >> iterations);
  outHeight = static_cast<int32_t>(height >> iterations);

  for (int32_t r=0; r<iterations; r++)
  {
    const auto new_size = static_cast<uint32_t>(std::floor(width >> (r+1)))
                                    * static_cast<uint32_t>(std::floor(height >> (r+1)))
                                    * bpp;

    result.resize(new_size, 0);

    for (int32_t i=0; i<height; i+=2)
    {
      for (int32_t j=0; j<width; j+=2)
      {
        // average the 4 neighboring pixels and set the new pixel value in the output image buffer
        auto pixel_rgb_value_0 = pixel_gather(j
                                             ,i
                                             ,static_cast<int32_t>(width >> r)
                                             ,static_cast<int32_t>(height >> r)
                                             ,combine_value
                                             ,bpp
                                             ,dest_result);

        auto pixel_rgb_value_1 = pixel_gather(j
                                             ,i+1
                                             ,static_cast<int32_t>(width >> r)
                                             ,static_cast<int32_t>(height >> r)
                                             ,combine_value
                                             ,bpp
                                             ,dest_result);

        std::array<int32_t, 4> pixel_rgb_value_sum = {(pixel_rgb_value_0[0]+pixel_rgb_value_1[0]) / 2
                                                     ,(pixel_rgb_value_0[1]+pixel_rgb_value_1[1]) / 2
                                                     ,(pixel_rgb_value_0[2]+pixel_rgb_value_1[2]) / 2
                                                     ,(pixel_rgb_value_0[3]+pixel_rgb_value_1[3]) / 2};

        std::array<uint8_t, 4> pixel_rgb_value = {static_cast<uint8_t>(pixel_rgb_value_sum[0])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_sum[1])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_sum[2])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_sum[3])};

        set_pixel((j / 2)
                 ,(i / 2)
                 ,static_cast<int32_t>(width >> (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);
      }
    }

    dest_result = result;
  }
}
