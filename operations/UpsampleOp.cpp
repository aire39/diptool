#include "UpsampleOp.h"

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

    for (int32_t i=0; i<(number_of_pixel_neighbors + 1); i++)
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

std::vector<uint8_t> UpsampleOp::ProcessImage(MenuOps::Upsample operation
                                             ,const std::vector<uint8_t> & source_image
                                             ,uint32_t width
                                             ,uint32_t height
                                             ,uint8_t bpp
                                             ,uint16_t iterations)
{
  switch (operation)
  {
    case MenuOps::Upsample::NEAREST:
      spdlog::info("perform nearest upsample operation");
      NearestAlgorithm(source_image, width, height, bpp, iterations);
      break;

    case MenuOps::Upsample::LINEAR:
      spdlog::info("perform linear upsample operation");
      LinearAlgorithm(source_image, width, height, bpp, iterations);
      break;

    case MenuOps::Upsample::BILINEAR:
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
    const auto new_size = static_cast<uint32_t>( std::floor(width << (r+1)))
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
                 ,pixel_rgb_value);

        set_pixel((j * 2) + 1
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);

        set_pixel((j * 2)
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);

        set_pixel((j * 2) + 1
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);
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
    const auto new_size = static_cast<uint32_t>( std::floor(width << (r+1)))
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

        // get the 2 (x,y) coordinates (left and right neighboring pixels) that need to be used to produce a linear factor to be multiplied
        // against the intensity values at those corners. This needs to be done for each channel. Assuming
        // an RGB pixel (ignoring the Alpha channel)

        std::array<int32_t, 4> pixel_rgb_value_sum = {0, 0, 0};

        auto idx_tr = static_cast<float>(j+1);
        auto idx_tl = static_cast<float>(j-1);
        auto idx_px  = (idx_tr + idx_tl) / 2.0f;

        for (size_t k=0; k<4; k++)
        {
          float pix_1 = ((idx_tr - idx_px) / (idx_tr - idx_tl)) * static_cast<float>(pixel_rgb_value_left[k]);
          float pix_2 = ((idx_px - idx_tl) / (idx_tr - idx_tl)) * static_cast<float>(pixel_rgb_value_right[k]);
          float pix_p = (pix_1 + pix_2);

          pixel_rgb_value_sum[k] = static_cast<int32_t>(pix_p);
        }

        std::array<uint8_t, 4> pixel_rgb_value = {static_cast<uint8_t>(pixel_rgb_value_sum[0])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_sum[1])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_sum[2])
                                                 ,static_cast<uint8_t>(pixel_rgb_value_sum[3])};

        set_pixel((j * 2)
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);

        set_pixel((j * 2) + 1
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);

        set_pixel((j * 2)
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);

        set_pixel((j * 2) + 1
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);
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
    const auto new_size = static_cast<uint32_t>( std::floor(width << (r+1)))
                                               * static_cast<uint32_t>(std::floor(height << (r+1)))
                                               * bpp;

    result.resize(new_size, 0);

    for (int32_t i=0; i<(height << r); i++)
    {
      for (int32_t j=0; j<(width << r); j++)
      {
        // grab the pixel value of the source and set the pixel to the correct destination

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

        // get the 4 (x,y) coordinates that need to be used to produce a linear factor to be multiplied
        // against the intensity values at those corners. This needs to be done for each channel. Assuming
        // an RGB pixel (ignoring the Alpha channel)

        std::array<int32_t, 4> pixel_rgb_value_sum = {0, 0, 0};

        auto idx_tr = static_cast<float>(j+1);
        auto idx_tl = static_cast<float>(j-1);
        auto idx_px  = (idx_tr + idx_tl) / 2.0f;
        auto idx_t = static_cast<float>(i-1);
        auto idx_b  = static_cast<float>(i+1);
        auto idx_py = (idx_t + idx_b) / 2.0f;

        for (size_t k=0; k<4; k++)
        {
          float pix_top_1 =  ((idx_tr - idx_px) / (idx_tr - idx_tl)) * static_cast<float>(pixel_rgb_value_topleft[k]);
          float pix_top_2 =  ((idx_px - idx_tl) / (idx_tr - idx_tl)) * static_cast<float>(pixel_rgb_value_topright[k]);
          float pix_top = (pix_top_1 + pix_top_2);

          float pix_bot_1 =  ((idx_tr - idx_px) / (idx_tr - idx_tl)) * static_cast<float>(pixel_rgb_value_bottomleft[k]);
          float pix_bot_2 =  ((idx_tr - idx_px) / (idx_tr - idx_tl)) * static_cast<float>(pixel_rgb_value_bottomright[k]);
          float pix_bot = (pix_bot_1 + pix_bot_2);

          float pix_p_1 =  ((idx_b - idx_py) / (idx_b - idx_t)) * pix_bot;
          float pix_p_2 =  ((idx_py - idx_t) / (idx_b - idx_t)) * pix_top;
          float pix_p = (pix_p_1 + pix_p_2);

          pixel_rgb_value_sum[k] = static_cast<int32_t>(pix_p);
        }

        std::array<uint8_t, 4> pixel_rgb_value = {std::clamp(static_cast<uint8_t>(pixel_rgb_value_sum[0]), uint8_t(0), uint8_t(255))
                                                 ,std::clamp(static_cast<uint8_t>(pixel_rgb_value_sum[1]), uint8_t(0), uint8_t(255))
                                                 ,std::clamp(static_cast<uint8_t>(pixel_rgb_value_sum[2]), uint8_t(0), uint8_t(255))
                                                 ,std::clamp(static_cast<uint8_t>(pixel_rgb_value_sum[3]), uint8_t(0), uint8_t(255))};

        set_pixel((j * 2)
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);

        set_pixel((j * 2) + 1
                 ,(i * 2)
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);

        set_pixel((j * 2)
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);

        set_pixel((j * 2) + 1
                 ,(i * 2) + 1
                 ,static_cast<int32_t>(width << (r+1))
                 ,bpp
                 ,result
                 ,pixel_rgb_value);
      }
    }

    dest_result = result;
  }
}
