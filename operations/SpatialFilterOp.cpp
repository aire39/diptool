#include "SpatialFilterOp.h"

#include <algorithm>
#include <spdlog/spdlog.h>

std::vector<uint8_t> SpatialFilterOp::ProcessImage(MenuOp_SpatialFilter operation
                                                  ,const std::vector<uint8_t> & source_image
                                                  ,uint32_t width
                                                  ,uint32_t height
                                                  ,uint8_t bpp
                                                  ,uint16_t iterations)
{

  result = source_image;

  switch (operation)
  {
    case MenuOp_SpatialFilter::SMOOTHING:
      SmoothingFilter(source_image, width, height, bpp);
      break;

    case MenuOp_SpatialFilter::MEDIAN:
      MedianFilter(source_image, width, height, bpp);
      break;

    case MenuOp_SpatialFilter::SHARPENING:
      spdlog::warn("sharpening not implemented");
      break;

    case MenuOp_SpatialFilter::HIGHBOOST:
      spdlog::warn("high-boost not implemented");
      break;

    default:
      spdlog::warn("not a valid filter");
      break;
  }

  return result;

}

const std::vector<uint8_t> & SpatialFilterOp::GetImage() const
{
  return result;
}

int32_t SpatialFilterOp::GetWidth() const
{
  return outWidth;
}

int32_t SpatialFilterOp::GetHeight() const
{
  return outHeight;
}

void SpatialFilterOp::SetKernelSize(int32_t kernel_x, int32_t kernel_y)
{
  kernelX = kernel_x;
  kernelY = kernel_y;
}

float SpatialFilterOp::ConvolutionValue(const std::vector<uint8_t> & source
                                       ,int32_t x
                                       ,int32_t y
                                       ,int32_t source_width
                                       ,int32_t source_height
                                       ,int32_t offset
                                       ,int32_t sum_count
                                       ,int32_t bpp
                                       ,const std::vector<float> & kernel
                                       ,int32_t kernel_width
                                       ,int32_t kernel_height
                                       ,float kernel_div)
{
  float value = 0.0f;

  int32_t k_width_centered = (kernel_width - 1) / 2;
  int32_t k_height_centered = (kernel_height - 1) / 2;

  int32_t convo_height_it_begin = y - k_height_centered;
  int32_t convo_height_it_end = (y + k_height_centered) + 1;
  int32_t convo_height_diff = (convo_height_it_end - convo_height_it_begin);
  convo_height_it_end = std::clamp(convo_height_it_end, 0, source_height);

  int32_t convo_width_it_begin = x - k_width_centered;
  int32_t convo_width_it_end = (x + k_width_centered) + 1;
  int32_t convo_width_diff = (convo_width_it_end - convo_width_it_begin);
  convo_width_it_end = std::clamp(convo_width_it_end, 0, source_width);

  for (int32_t i=convo_height_it_begin; i<convo_height_it_end; i++)
  {
    for (int32_t j=convo_width_it_begin; j<convo_width_it_end; j++)
    {
      int32_t kernel_x_index = (kernel_width - convo_width_diff) + (j - convo_width_it_begin);
      int32_t kernel_y_index = (kernel_height - convo_height_diff) + (i - convo_height_it_begin);

      int32_t jj = std::clamp(j, 0, source_width);
      int32_t ii = std::clamp(i, 0, source_height);

      if (sum_count > 0)
      {
        float sum_value = 0.0f;
        for (int32_t k=0; k<sum_count; k++)
        {
          sum_value += static_cast<float>(source[(jj * bpp) + (ii * source_width * bpp) + offset + k]);
        }
        sum_value /= static_cast<float>(sum_count);

        value += (sum_value * static_cast<float>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));
      }
      else
      {
        value += (static_cast<float>(source[(jj * bpp) + (ii * source_width * bpp) + offset]) *
                  static_cast<float>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));
      }
    }
  }

  return (value * kernel_div);
}

float SpatialFilterOp::MedianValue(const std::vector<uint8_t> & source
                                  ,int32_t x
                                  ,int32_t y
                                  ,int32_t source_width
                                  ,int32_t source_height
                                  ,int32_t offset
                                  ,int32_t sum_count
                                  ,int32_t bpp
                                  ,int32_t kernel_width
                                  ,int32_t kernel_height
                                  ,float median_scale_factor)
{
  int32_t k_width_centered = (kernel_width - 1) / 2;
  int32_t k_height_centered = (kernel_height - 1) / 2;

  int32_t convo_height_it_begin = y - k_height_centered;
  int32_t convo_height_it_end = (y + k_height_centered) + 1;
  int32_t convo_height_diff = (convo_height_it_end - convo_height_it_begin);
  convo_height_it_end = std::clamp(convo_height_it_end, 0, source_height);

  int32_t convo_width_it_begin = x - k_width_centered;
  int32_t convo_width_it_end = (x + k_width_centered) + 1;
  int32_t convo_width_diff = (convo_width_it_end - convo_width_it_begin);
  convo_width_it_end = std::clamp(convo_width_it_end, 0, source_width);

  std::vector<float> kernel (kernel_width * kernel_height);

  for (int32_t i=convo_height_it_begin; i<convo_height_it_end; i++)
  {
    for (int32_t j=convo_width_it_begin; j<convo_width_it_end; j++)
    {
      float value;
      int32_t jj = std::clamp(j, 0, source_width);
      int32_t ii = std::clamp(i, 0, source_height);

      if (sum_count > 0)
      {
        float sum_value = 0.0f;
        for (int32_t k=0; k<sum_count; k++)
        {
          sum_value += static_cast<float>(source[(jj * bpp) + (ii * source_width * bpp) + offset + k]);
        }
        sum_value /= static_cast<float>(sum_count);

        value = std::clamp(median_scale_factor * sum_value, 0.0f, 255.0f);
      }
      else
      {
        value = median_scale_factor * static_cast<float>(source[(jj * bpp) + (ii * source_width * bpp) + offset]);
      }

      int32_t kernel_x_index = (kernel_width - convo_width_diff) + (j - convo_width_it_begin);
      int32_t kernel_y_index = (kernel_height - convo_height_diff) + (i - convo_height_it_begin);
      kernel[kernel_x_index + (kernel_width * kernel_y_index)] = value;
    }
  }

  // sort values and return the median from the kernel matrix

  std::sort(kernel.begin(), kernel.end());
  float median_value = kernel[((kernel_width * kernel_height) - 1) / 2];

  return median_value;
}

void SpatialFilterOp::SmoothingFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp)
{
  spdlog::info("begin spatial filter: smoothing");
  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  std::vector<float> smooth_kernel ((kernelX * kernelY), 1.0f);

  float smooth_kernel_div = 0.0f;
  for (const auto & v : smooth_kernel)
  {
    smooth_kernel_div += v;
  }
  smooth_kernel_div = 1.0f / smooth_kernel_div;

  for (size_t i=0; i<height; i++)
  {
    for (size_t j=0; j<width; j++)
    {
      float filter_value_red = std::clamp(ConvolutionValue(source_image, j, i, width, height, 0, 0, bpp, smooth_kernel, kernelX, kernelY, smooth_kernel_div), 0.0f, 255.0f);
      float filter_value_green = std::clamp(ConvolutionValue(source_image, j, i, width, height, 1, 0, bpp, smooth_kernel, kernelX, kernelY, smooth_kernel_div), 0.0f, 255.0f);
      float filter_value_blue = std::clamp(ConvolutionValue(source_image, j, i, width, height, 2, 0, bpp, smooth_kernel, kernelX, kernelY, smooth_kernel_div), 0.0f, 255.0f);
      float filter_alpha_value = std::clamp(ConvolutionValue(source_image, j, i, width, height, 3, 0, bpp, smooth_kernel, kernelX, kernelY, smooth_kernel_div), 0.0f, 255.0f);

      result[(j*bpp) + (i*width*bpp) + 0] = static_cast<uint8_t>(filter_value_red);
      result[(j*bpp) + (i*width*bpp) + 1] = static_cast<uint8_t>(filter_value_green);
      result[(j*bpp) + (i*width*bpp) + 2] = static_cast<uint8_t>(filter_value_blue);
      result[(j*bpp) + (i*width*bpp) + 3] = static_cast<uint8_t>(filter_alpha_value);
    }
  }
}

void SpatialFilterOp::MedianFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp)
{
  spdlog::info("begin spatial filter: median");
  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  const float median_scale_factor = 1.0f;

  for (size_t i=0; i<height; i++)
  {
    for (size_t j=0; j<width; j++)
    {
      float filter_value_red = std::clamp(MedianValue(source_image, j, i, width, height, 0, 0, bpp, kernelX, kernelY, median_scale_factor), 0.0f, 255.0f);
      float filter_value_green = std::clamp(MedianValue(source_image, j, i, width, height, 1, 0, bpp, kernelX, kernelY, median_scale_factor), 0.0f, 255.0f);
      float filter_value_blue = std::clamp(MedianValue(source_image, j, i, width, height, 2, 0, bpp, kernelX, kernelY, median_scale_factor), 0.0f, 255.0f);
      float filter_alpha_value = std::clamp(MedianValue(source_image, j, i, width, height, 3, 0, bpp, kernelX, kernelY, median_scale_factor), 0.0f, 255.0f);

      result[(j*bpp) + (i*width*bpp) + 0] = static_cast<uint8_t>(filter_value_red);
      result[(j*bpp) + (i*width*bpp) + 1] = static_cast<uint8_t>(filter_value_green);
      result[(j*bpp) + (i*width*bpp) + 2] = static_cast<uint8_t>(filter_value_blue);
      result[(j*bpp) + (i*width*bpp) + 3] = static_cast<uint8_t>(filter_alpha_value);
    }
  }
}