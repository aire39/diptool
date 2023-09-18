#include "UpsampleOp.h"

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
        pixel_value_green += static_cast<int32_t>(source_image[((x + i) * bpp) + (y * w * bpp) + 1]);
        pixel_value_blue += static_cast<int32_t>(source_image[((x + i) * bpp) + (y * w * bpp) + 2]);
      }
      else
      {
        const int32_t x_fix = std::clamp(x, 0, (w - 1));
        const int32_t y_fix = std::clamp(y, 0, (h - 1));

        pixel_value_red += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp) + 0]);
        pixel_value_green += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp) + 1]);
        pixel_value_blue += static_cast<int32_t>(source_image[(x_fix * bpp) + (y_fix * w * bpp) + 2]);
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

std::vector<uint8_t> UpsampleOp::ProcessImage(MenuOp_Upsample operation
                                             ,const std::vector<uint8_t> & source_image
                                             ,uint32_t width
                                             ,uint32_t height
                                             ,uint8_t bpp
                                             ,uint16_t iterations)
{
  switch (operation)
  {
    case MenuOp_Upsample::NEAREST:
      spdlog::info("perform nearest upsample operation");
      NearestAlgorithm(source_image, width, height, bpp, iterations);
      break;

    case MenuOp_Upsample::LINEAR:
      spdlog::info("perform linear upsample operation");
      LinearAlgorithm(source_image, width, height, bpp, iterations);
      break;

    case MenuOp_Upsample::BILINEAR:
      spdlog::info("perform bilinear upsample operation");
      BilinearAlgorithm(source_image, width, height, bpp, iterations);
      break;
  }

  return result;
}

const std::vector<uint8_t> & UpsampleOp::GetImage() const
{
  return result;
}

int32_t UpsampleOp::GetWidth() const
{
  return outWidth;
}

int32_t UpsampleOp::GetHeight() const
{
  return outHeight;
}

void UpsampleOp::NearestAlgorithm(const std::vector<uint8_t> & source_image
                                   ,uint32_t width
                                   ,uint32_t height
                                   ,uint8_t bpp
                                   ,uint16_t iterations)
{
  constexpr uint32_t combine_value = 1;

  auto dest_result = result = source_image;

  outWidth = static_cast<int32_t>(width << iterations);
  outHeight = static_cast<int32_t>(height << iterations);

  for (int32_t r=0; r<iterations; r++)
  {
    const auto new_size = static_cast<uint32_t>(std::floor(width << (r+1)))
                                     * static_cast<uint32_t>(std::floor(height << (r+1)))
                                     * bpp;

    result.resize(new_size, 0);

    for (int32_t i=0; i<(height << r); i++)
    {
      for (int32_t j=0; j<(width << r); j++)
      {
        // grab the pixel value of the source and set the pixel to the correct destination
        auto pixel_rgb_value = pixel_gather(j
                                                                 ,i
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        set_pixel((j * 2)
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );

        set_pixel((j * 2) + 1
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );

        set_pixel((j * 2)
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );

        set_pixel((j * 2) + 1
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );
      }
    }

    dest_result = result;
  }
}

void UpsampleOp::LinearAlgorithm(const std::vector<uint8_t> & source_image
                                  ,uint32_t width
                                  ,uint32_t height
                                  ,uint8_t bpp
                                  ,uint16_t iterations)
{
  constexpr uint32_t combine_value = 1;

  auto dest_result = result = source_image;

  outWidth = static_cast<int32_t>(width << iterations);
  outHeight = static_cast<int32_t>(height << iterations);

  for (int32_t r=0; r<iterations; r++)
  {
    const auto new_size = static_cast<uint32_t>(std::floor(width << (r+1)))
                                     * static_cast<uint32_t>(std::floor(height << (r+1)))
                                     * bpp;

    result.resize(new_size, 0);

    for (int32_t i=0; i<(height << r); i++)
    {
      for (int32_t j=0; j<(width << r); j++)
      {
        // grab the pixel value of the source and set the pixel to the correct destination
        auto pixel_rgb_value_left = pixel_gather((j - 1)
                                                                 ,i
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_right = pixel_gather((j + 1)
                                                                 ,i
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_top = pixel_gather(j
                                                                 ,(i - 1)
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_bottom = pixel_gather(j
                                                                 ,(i + 1)
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_center = pixel_gather(j
                                                                       ,i
                                                                       ,static_cast<int32_t>(width << r)
                                                                       ,static_cast<int32_t>(height << r)
                                                                       ,combine_value
                                                                       ,bpp
                                                                       ,dest_result);

        const std::array<int32_t, 3> pixel_rgb_value_sum = {(pixel_rgb_value_left[0]+pixel_rgb_value_right[0]+pixel_rgb_value_top[0]+pixel_rgb_value_bottom[0]+pixel_rgb_value_center[0]) / 5
                                                           ,(pixel_rgb_value_left[1]+pixel_rgb_value_right[1]+pixel_rgb_value_top[1]+pixel_rgb_value_bottom[1]+pixel_rgb_value_center[1]) / 5
                                                           ,(pixel_rgb_value_left[2]+pixel_rgb_value_right[2]+pixel_rgb_value_top[2]+pixel_rgb_value_bottom[2]+pixel_rgb_value_center[2]) / 5};

        std::array<uint8_t, 3> pixel_rgb_value = {static_cast<uint8_t>(pixel_rgb_value_sum[0])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_sum[1])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_sum[2])};

        set_pixel((j * 2)
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );

        set_pixel((j * 2) + 1
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );

        set_pixel((j * 2)
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );

        set_pixel((j * 2) + 1
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );
      }
    }

    dest_result = result;
  }
}

void UpsampleOp::BilinearAlgorithm(const std::vector<uint8_t> & source_image
                                  ,uint32_t width
                                  ,uint32_t height
                                  ,uint8_t bpp
                                  ,uint16_t iterations)
{
  constexpr uint32_t combine_value = 1;

  auto dest_result = result = source_image;

  outWidth = static_cast<int32_t>(width << iterations);
  outHeight = static_cast<int32_t>(height << iterations);

  for (int32_t r=0; r<iterations; r++)
  {
    const auto new_size = static_cast<uint32_t>(std::floor(width << (r+1)))
                                     * static_cast<uint32_t>(std::floor(height << (r+1)))
                                     * bpp;

    result.resize(new_size, 0);

    for (int32_t i=0; i<(height << r); i++)
    {
      for (int32_t j=0; j<(width << r); j++)
      {
        // grab the pixel value of the source and set the pixel to the correct destination
        auto pixel_rgb_value_left = pixel_gather((j - 1)
                                                                 ,i
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_right = pixel_gather((j + 1)
                                                                 ,i
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_top = pixel_gather(j
                                                                 ,(i - 1)
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_bottom = pixel_gather(j
                                                                 ,(i + 1)
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_topleft = pixel_gather((j - 1)
                                                                 ,(i - 1)
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_topright = pixel_gather((j + 1)
                                                                 ,(i - 1)
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_bottomleft = pixel_gather((j - 1)
                                                                 ,(i + 1)
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_bottomright = pixel_gather((j + 1)
                                                                 ,(i + 1)
                                                                 ,static_cast<int32_t>(width << r)
                                                                 ,static_cast<int32_t>(height << r)
                                                                 ,combine_value
                                                                 ,bpp
                                                                 ,dest_result);

        auto pixel_rgb_value_center = pixel_gather(j
                                                                       ,i
                                                                       ,static_cast<int32_t>(width << r)
                                                                       ,static_cast<int32_t>(height << r)
                                                                       ,combine_value
                                                                       ,bpp
                                                                       ,dest_result);

        const std::array<int32_t, 3> pixel_rgb_value_sum = {((pixel_rgb_value_left[0]+pixel_rgb_value_right[0]+pixel_rgb_value_top[0]+pixel_rgb_value_bottom[0]+pixel_rgb_value_center[0]) +
                                                            (pixel_rgb_value_topleft[0]+pixel_rgb_value_topright[0]+pixel_rgb_value_bottomleft[0]+pixel_rgb_value_bottomright[0])) / 9
                                                           ,((pixel_rgb_value_left[1]+pixel_rgb_value_right[1]+pixel_rgb_value_top[1]+pixel_rgb_value_bottom[1]+pixel_rgb_value_center[1]) +
                                                            (pixel_rgb_value_topleft[1]+pixel_rgb_value_topright[1]+pixel_rgb_value_bottomleft[1]+pixel_rgb_value_bottomright[1])) / 9
                                                           ,((pixel_rgb_value_left[2]+pixel_rgb_value_right[2]+pixel_rgb_value_top[2]+pixel_rgb_value_bottom[2]+pixel_rgb_value_center[2]) +
                                                            (pixel_rgb_value_topleft[2]+pixel_rgb_value_topright[2]+pixel_rgb_value_bottomleft[2]+pixel_rgb_value_bottomright[2])) / 9};

        std::array<uint8_t, 3> pixel_rgb_value = {std::clamp(static_cast<uint8_t>(pixel_rgb_value_sum[0]), uint8_t(0), uint8_t(255))
                                                 ,std::clamp(static_cast<uint8_t>(pixel_rgb_value_sum[1]), uint8_t(0), uint8_t(255))
                                                 ,std::clamp(static_cast<uint8_t>(pixel_rgb_value_sum[2]), uint8_t(0), uint8_t(255))};

        set_pixel((j * 2)
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );

        set_pixel((j * 2) + 1
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );

        set_pixel((j * 2)
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );

        set_pixel((j * 2) + 1
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value
                 );
      }
    }

    dest_result = result;
  }
}
