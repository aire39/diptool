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
      SharpenFilter(source_image, width, height, bpp);
      break;

    case MenuOp_SpatialFilter::HIGHBOOST:
      HighBoostFilter(source_image, width, height, bpp);
      break;

    case MenuOp_SpatialFilter::ARITH_MEAN:
      ArithMeanFilter(source_image, width, height, bpp);
      break;

    case MenuOp_SpatialFilter::GEO_MEAN:
      GeoMeanFilter(source_image, width, height, bpp);
      break;

    case MenuOp_SpatialFilter::MIN:
      MinFilter(source_image, width, height, bpp);
      break;

    case MenuOp_SpatialFilter::MAX:
      MaxFilter(source_image, width, height, bpp);
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

void SpatialFilterOp::SetSharpenConstant(float sharp_const)
{
  sharpenConstant = sharp_const;
}

void SpatialFilterOp::SetSharpenUseFullKernel(bool use_full_kernel)
{
  sharpUseFullKernel = use_full_kernel;
}

void SpatialFilterOp::SetUnSharpenConstant(float unsharp_const)
{
  unsharpConstant = unsharp_const;
}

void SpatialFilterOp::ShowSharpenFilter(bool show_sharpen_filter)
{
  showSharpenFilter = show_sharpen_filter;
}

void SpatialFilterOp::ShowSharpenFilterScaling(bool show_sharpen_filter)
{
  showSharpenFilterScaling = show_sharpen_filter;
}

void SpatialFilterOp::ShowUnSharpenFilter(bool show_unsharpen_filter)
{
  showUnSharpenFilter = show_unsharpen_filter;
}

void SpatialFilterOp::ShowUnSharpenFilterScaling(bool show_unsharpen_filter)
{
  showUnSharpenFilterScaling = show_unsharpen_filter;
}

double SpatialFilterOp::ConvolutionValue(const std::vector<uint8_t> & source
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
                                       ,float kernel_div
                                       ,CONV_TYPE conv_type)
{
  double value = 0.0;

  if (conv_type == CONV_TYPE::MULT)
  {
    value = 1.0;
  }
  else if (conv_type == CONV_TYPE::MIN)
  {
    value = 255.0;
  }

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
          sum_value += static_cast<double>(source[(jj * bpp) + (ii * source_width * bpp) + offset + k]);
        }
        sum_value /= static_cast<double>(sum_count);

        if (conv_type == CONV_TYPE::SUM)
        {
          value += (sum_value * static_cast<double>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));
        }
        else if (conv_type == CONV_TYPE::MIN)
        {
          double tmp = (sum_value * static_cast<double>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));
          value = (value > tmp) ? tmp : value;
        }
        else if (conv_type == CONV_TYPE::MAX)
        {
          double tmp = (sum_value * static_cast<double>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));
          value = (value < tmp) ? tmp : value;
        }
        else // CONV_TYPE::MULT
        {
          value *= (sum_value * static_cast<double>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));
        }
      }
      else
      {
        if (conv_type == CONV_TYPE::SUM)
        {
          value += (static_cast<double>(source[(jj * bpp) + (ii * source_width * bpp) + offset]) *
                    static_cast<double>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));
        }
        else if (conv_type == CONV_TYPE::MIN)
        {
          double tmp = (static_cast<double>(source[(jj * bpp) + (ii * source_width * bpp) + offset]) *
                        static_cast<double>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));

          value = (value > tmp) ? tmp : value;
        }
        else if (conv_type == CONV_TYPE::MAX)
        {
          double tmp = (static_cast<double>(source[(jj * bpp) + (ii * source_width * bpp) + offset]) *
                        static_cast<double>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));

          value = (value < tmp) ? tmp : value;
        }
        else // CONV_TYPE::MULT
        {
          value *= (static_cast<double>(source[(jj * bpp) + (ii * source_width * bpp) + offset]) *
                    static_cast<double>(kernel[kernel_x_index + (kernel_y_index * kernel_width)]));
        }
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
      double filter_value_red = std::clamp(ConvolutionValue(source_image, j, i, width, height, 0, 0, bpp, smooth_kernel, kernelX, kernelY, smooth_kernel_div), 0.0, 255.0);
      double filter_value_green = std::clamp(ConvolutionValue(source_image, j, i, width, height, 1, 0, bpp, smooth_kernel, kernelX, kernelY, smooth_kernel_div), 0.0, 255.0);
      double filter_value_blue = std::clamp(ConvolutionValue(source_image, j, i, width, height, 2, 0, bpp, smooth_kernel, kernelX, kernelY, smooth_kernel_div), 0.0, 255.0);
      double filter_alpha_value = std::clamp(ConvolutionValue(source_image, j, i, width, height, 3, 0, bpp, smooth_kernel, kernelX, kernelY, smooth_kernel_div), 0.0, 255.0);

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

void SpatialFilterOp::SharpenFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp)
{
  spdlog::info("begin spatial filter: sharpening");
  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  std::vector<uint8_t> laplacian_image ((width * height * bpp), 0.0f);
  std::vector<float> laplacian_kernel ((kernelX * kernelY), 0.0f);
  int32_t kernel_x_center = (kernelX - 1) / 2;
  int32_t kernel_y_center = (kernelY - 1) / 2;

  if (!sharpUseFullKernel)
  {
    for (int32_t i = (-kernel_x_center); i < (kernel_x_center + 1); i++)
    {
      laplacian_kernel[(kernel_x_center + i) + (kernel_y_center * kernelX)] = 1;
      laplacian_kernel[(kernel_x_center - i) + (kernel_y_center * kernelX)] = 1;

      laplacian_kernel[kernel_x_center + ((kernel_y_center + i) * kernelX)] = 1;
      laplacian_kernel[kernel_x_center + ((kernel_y_center - i) * kernelX)] = 1;
    }

    laplacian_kernel[kernel_x_center + (kernel_y_center * kernelX)] = -(static_cast<float>(kernelX - 1) +
                                                                        static_cast<float>(kernelY - 1));
  }
  else
  {
    std::fill(laplacian_kernel.begin(), laplacian_kernel.end(), 1.0f);
    laplacian_kernel[kernel_x_center + (kernel_y_center * kernelX)] = -(static_cast<float>(kernelX * kernelY) - 1.0f);
  }

  constexpr float kernel_div = 1.0f;

  float sharpen_filter_scaling = showSharpenFilterScaling ? 128.0f : 0.0f;

  for (size_t i=0; i<height; i++)
  {
    for (size_t j=0; j<width; j++)
    {
      float filter_value_red = ConvolutionValue(source_image, j, i, width, height, 0, 0, bpp, laplacian_kernel, kernelX, kernelY, kernel_div) * sharpenConstant;
      float filter_value_green = ConvolutionValue(source_image, j, i, width, height, 1, 0, bpp, laplacian_kernel, kernelX, kernelY, kernel_div) * sharpenConstant;
      float filter_value_blue = ConvolutionValue(source_image, j, i, width, height, 2, 0, bpp, laplacian_kernel, kernelX, kernelY, kernel_div) * sharpenConstant;
      float filter_value_alpha = ConvolutionValue(source_image, j, i, width, height, 3, 0, bpp, laplacian_kernel, kernelX, kernelY, kernel_div * sharpenConstant);


      if (showSharpenFilter)
      {
        result[(j*bpp) + (i*width*bpp) + 0] = static_cast<uint8_t>(std::clamp(filter_value_red + sharpen_filter_scaling, 0.0f, 255.0f));
        result[(j*bpp) + (i*width*bpp) + 1] = static_cast<uint8_t>(std::clamp(filter_value_green + sharpen_filter_scaling, 0.0f, 255.0f));
        result[(j*bpp) + (i*width*bpp) + 2] = static_cast<uint8_t>(std::clamp(filter_value_blue + sharpen_filter_scaling, 0.0f, 255.0f));
        result[(j*bpp) + (i*width*bpp) + 3] = static_cast<uint8_t>(std::clamp(filter_value_alpha + 255.0f, 0.0f, 255.0f));
      }
      else
      {
        result[(j*bpp) + (i*width*bpp) + 0] = static_cast<uint8_t>(std::clamp(static_cast<float>(source_image[(j*bpp) + (i*width*bpp) + 0]) + filter_value_red, 0.0f, 255.0f));
        result[(j*bpp) + (i*width*bpp) + 1] = static_cast<uint8_t>(std::clamp(static_cast<float>(source_image[(j*bpp) + (i*width*bpp) + 1]) + filter_value_green, 0.0f, 255.0f));
        result[(j*bpp) + (i*width*bpp) + 2] = static_cast<uint8_t>(std::clamp(static_cast<float>(source_image[(j*bpp) + (i*width*bpp) + 2]) + filter_value_blue, 0.0f, 255.0f));
        result[(j*bpp) + (i*width*bpp) + 3] = static_cast<uint8_t>(std::clamp(static_cast<float>(source_image[(j*bpp) + (i*width*bpp) + 3]) + filter_value_alpha, 0.0f, 255.0f));
      }
    }
  }
}

void SpatialFilterOp::HighBoostFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp)
{
  spdlog::info("begin spatial filter: high-boost");

  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  std::vector<float> unsharp_mask (width * height * bpp);

  SmoothingFilter(source_image, width, height, bpp);
  std::vector<uint8_t> blur_image = result;

  float unsharp_filter_scaling = showUnSharpenFilterScaling ? 128.0f : 0.0f;

  for (size_t i=0; i<height; i++)
  {
    for (size_t j = 0; j < width; j++)
    {
      unsharp_mask[((j * bpp) + (i * width * bpp)) + 0] = unsharpConstant * (static_cast<float>(source_image[((j * bpp) + (i * width * bpp)) + 0]) - static_cast<float>(blur_image[((j * bpp) + (i * width * bpp)) + 0]));
      unsharp_mask[((j * bpp) + (i * width * bpp)) + 1] = unsharpConstant * (static_cast<float>(source_image[((j * bpp) + (i * width * bpp)) + 1]) - static_cast<float>(blur_image[((j * bpp) + (i * width * bpp)) + 1]));
      unsharp_mask[((j * bpp) + (i * width * bpp)) + 2] = unsharpConstant * (static_cast<float>(source_image[((j * bpp) + (i * width * bpp)) + 2]) - static_cast<float>(blur_image[((j * bpp) + (i * width * bpp)) + 2]));
      unsharp_mask[((j * bpp) + (i * width * bpp)) + 3] = unsharpConstant * (static_cast<float>(source_image[((j * bpp) + (i * width * bpp)) + 3]) - static_cast<float>(blur_image[((j * bpp) + (i * width * bpp)) + 3]));
    }
  }

  for (size_t i=0; i<height; i++)
  {
    for (size_t j = 0; j < width; j++)
    {
      if (!showUnSharpenFilter)
      {
        result[((j * bpp) + (i * width * bpp)) + 0] = static_cast<uint8_t>(std::clamp(static_cast<float>(source_image[((j * bpp) + (i * width * bpp)) + 0]) + static_cast<float>(unsharp_mask[((j * bpp) + (i * width * bpp)) + 0]), 0.0f, 255.0f));
        result[((j * bpp) + (i * width * bpp)) + 1] = static_cast<uint8_t>(std::clamp(static_cast<float>(source_image[((j * bpp) + (i * width * bpp)) + 1]) + static_cast<float>(unsharp_mask[((j * bpp) + (i * width * bpp)) + 1]), 0.0f, 255.0f));
        result[((j * bpp) + (i * width * bpp)) + 2] = static_cast<uint8_t>(std::clamp(static_cast<float>(source_image[((j * bpp) + (i * width * bpp)) + 2]) + static_cast<float>(unsharp_mask[((j * bpp) + (i * width * bpp)) + 2]), 0.0f, 255.0f));
        result[((j * bpp) + (i * width * bpp)) + 3] = static_cast<uint8_t>(std::clamp(static_cast<float>(source_image[((j * bpp) + (i * width * bpp)) + 3]) + static_cast<float>(unsharp_mask[((j * bpp) + (i * width * bpp)) + 3]), 0.0f, 255.0f));
      }
      else
      {
        result[((j * bpp) + (i * width * bpp)) + 0] = static_cast<uint8_t>(std::clamp(unsharp_mask[((j * bpp) + (i * width * bpp)) + 0] + unsharp_filter_scaling, 0.0f, 255.0f));
        result[((j * bpp) + (i * width * bpp)) + 1] = static_cast<uint8_t>(std::clamp(unsharp_mask[((j * bpp) + (i * width * bpp)) + 1] + unsharp_filter_scaling, 0.0f, 255.0f));
        result[((j * bpp) + (i * width * bpp)) + 2] = static_cast<uint8_t>(std::clamp(unsharp_mask[((j * bpp) + (i * width * bpp)) + 2] + unsharp_filter_scaling, 0.0f, 255.0f));
        result[((j * bpp) + (i * width * bpp)) + 3] = 255;
      }
    }
  }
}

void SpatialFilterOp::ArithMeanFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp)
{
  spdlog::info("begin spatial filter: arithmetic mean");

  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  std::vector<float> kernel (kernelX * kernelY, 1.0f);
  float kernel_div = 1.0f;

  for (size_t i=0; i<height; i++)
  {
    for (size_t j=0; j<width; j++)
    {
      float filter_value_red = ConvolutionValue(source_image, j, i, width, height, 0, 0, bpp, kernel, kernelX, kernelY, kernel_div) / static_cast<float>(kernelX * kernelY);
      float filter_value_green = ConvolutionValue(source_image, j, i, width, height, 1, 0, bpp, kernel, kernelX, kernelY, kernel_div) / static_cast<float>(kernelX * kernelY);
      float filter_value_blue = ConvolutionValue(source_image, j, i, width, height, 2, 0, bpp, kernel, kernelX, kernelY, kernel_div) / static_cast<float>(kernelX * kernelY);
      float filter_value_alpha = ConvolutionValue(source_image, j, i, width, height, 3, 0, bpp, kernel, kernelX, kernelY, kernel_div) / static_cast<float>(kernelX * kernelY);

      result[(j*bpp) + (i*width*bpp) + 0] = static_cast<uint8_t>(std::clamp(filter_value_red, 0.0f, 255.0f));
      result[(j*bpp) + (i*width*bpp) + 1] = static_cast<uint8_t>(std::clamp(filter_value_green, 0.0f, 255.0f));
      result[(j*bpp) + (i*width*bpp) + 2] = static_cast<uint8_t>(std::clamp(filter_value_blue, 0.0f, 255.0f));
      result[(j*bpp) + (i*width*bpp) + 3] = static_cast<uint8_t>(std::clamp(filter_value_alpha, 0.0f, 255.0f));
    }
  }
}

void SpatialFilterOp::GeoMeanFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp)
{
  spdlog::info("begin spatial filter: geometric mean");

  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  std::vector<float> kernel (kernelX * kernelY, 1.0f);
  float kernel_div = 1.0f;

  for (size_t i=0; i<height; i++)
  {
    for (size_t j=0; j<width; j++)
    {
      double filter_value_red = ConvolutionValue(source_image, j, i, width, height, 0, 0, bpp, kernel, kernelX, kernelY, kernel_div, CONV_TYPE::MULT);
      double filter_value_green = ConvolutionValue(source_image, j, i, width, height, 1, 0, bpp, kernel, kernelX, kernelY, kernel_div, CONV_TYPE::MULT);
      double filter_value_blue = ConvolutionValue(source_image, j, i, width, height, 2, 0, bpp, kernel, kernelX, kernelY, kernel_div, CONV_TYPE::MULT);
      double filter_value_alpha = ConvolutionValue(source_image, j, i, width, height, 3, 0, bpp, kernel, kernelX, kernelY, kernel_div , CONV_TYPE::MULT);

      filter_value_red = std::pow(filter_value_red, 1.0 / (static_cast<double>(kernelX * kernelY)));
      filter_value_green = std::pow(filter_value_green, 1.0 / (static_cast<double>(kernelX * kernelY)));
      filter_value_blue = std::pow(filter_value_blue, 1.0 / (static_cast<double>(kernelX * kernelY)));
      filter_value_alpha = std::pow(filter_value_alpha, 1.0 / (static_cast<double>(kernelX * kernelY)));

      result[(j*bpp) + (i*width*bpp) + 0] = static_cast<uint8_t>(std::clamp(filter_value_red, 0.0, 255.0));
      result[(j*bpp) + (i*width*bpp) + 1] = static_cast<uint8_t>(std::clamp(filter_value_green, 0.0, 255.0));
      result[(j*bpp) + (i*width*bpp) + 2] = static_cast<uint8_t>(std::clamp(filter_value_blue, 0.0, 255.0));
      result[(j*bpp) + (i*width*bpp) + 3] = static_cast<uint8_t>(std::clamp(filter_value_alpha, 0.0, 255.0));
    }
  }
}

void SpatialFilterOp::MinFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp)
{
  spdlog::info("begin spatial filter: min");

  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  std::vector<float> kernel (kernelX * kernelY, 1.0f);
  float kernel_div = 1.0f;

  for (size_t i=0; i<height; i++)
  {
    for (size_t j=0; j<width; j++)
    {
      double filter_value_red = ConvolutionValue(source_image, j, i, width, height, 0, 0, bpp, kernel, kernelX, kernelY, kernel_div, CONV_TYPE::MIN);
      double filter_value_green = ConvolutionValue(source_image, j, i, width, height, 1, 0, bpp, kernel, kernelX, kernelY, kernel_div, CONV_TYPE::MIN);
      double filter_value_blue = ConvolutionValue(source_image, j, i, width, height, 2, 0, bpp, kernel, kernelX, kernelY, kernel_div, CONV_TYPE::MIN);
      double filter_value_alpha = ConvolutionValue(source_image, j, i, width, height, 3, 0, bpp, kernel, kernelX, kernelY, kernel_div , CONV_TYPE::MIN);

      result[(j*bpp) + (i*width*bpp) + 0] = static_cast<uint8_t>(std::clamp(filter_value_red, 0.0, 255.0));
      result[(j*bpp) + (i*width*bpp) + 1] = static_cast<uint8_t>(std::clamp(filter_value_green, 0.0, 255.0));
      result[(j*bpp) + (i*width*bpp) + 2] = static_cast<uint8_t>(std::clamp(filter_value_blue, 0.0, 255.0));
      result[(j*bpp) + (i*width*bpp) + 3] = static_cast<uint8_t>(std::clamp(filter_value_alpha, 0.0, 255.0));
    }
  }
}

void SpatialFilterOp::MaxFilter(const std::vector<uint8_t> & source_image, uint32_t width, uint32_t height, int32_t bpp)
{
  spdlog::info("begin spatial filter: max");

  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  std::vector<float> kernel (kernelX * kernelY, 1.0f);
  float kernel_div = 1.0f;

  for (size_t i=0; i<height; i++)
  {
    for (size_t j=0; j<width; j++)
    {
      double filter_value_red = ConvolutionValue(source_image, j, i, width, height, 0, 0, bpp, kernel, kernelX, kernelY, kernel_div, CONV_TYPE::MAX);
      double filter_value_green = ConvolutionValue(source_image, j, i, width, height, 1, 0, bpp, kernel, kernelX, kernelY, kernel_div, CONV_TYPE::MAX);
      double filter_value_blue = ConvolutionValue(source_image, j, i, width, height, 2, 0, bpp, kernel, kernelX, kernelY, kernel_div, CONV_TYPE::MAX);
      double filter_value_alpha = ConvolutionValue(source_image, j, i, width, height, 3, 0, bpp, kernel, kernelX, kernelY, kernel_div , CONV_TYPE::MAX);

      result[(j*bpp) + (i*width*bpp) + 0] = static_cast<uint8_t>(std::clamp(filter_value_red, 0.0, 255.0));
      result[(j*bpp) + (i*width*bpp) + 1] = static_cast<uint8_t>(std::clamp(filter_value_green, 0.0, 255.0));
      result[(j*bpp) + (i*width*bpp) + 2] = static_cast<uint8_t>(std::clamp(filter_value_blue, 0.0, 255.0));
      result[(j*bpp) + (i*width*bpp) + 3] = static_cast<uint8_t>(std::clamp(filter_value_alpha, 0.0, 255.0));
    }
  }
}
