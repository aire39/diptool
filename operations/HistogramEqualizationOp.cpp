#include "HistogramEqualizationOp.h"

#include <cmath>
#include <future>
#include <string_view>
#include <spdlog/spdlog.h>

namespace {
  constexpr uint32_t default_number_of_threads = 8;
  constexpr std::string_view default_threadpool_name = "HE_EQ";
}

HistogramEqualizationOp::HistogramEqualizationOp()
  : workPool(default_number_of_threads, default_threadpool_name.data())
{

}

const std::map<int32_t, float> & HistogramEqualizationOp::GetHistogramRemap()
{
  std::map<int32_t, std::vector<int32_t>> remapped_normalize_values;

  constexpr int32_t bpp = 4;
  for (size_t i=0; i<(outWidth * outHeight); i++)
  {
    int32_t pixel_value_gray = (result[0 + i * bpp] + result[1 + i * bpp] + result[2 + i * bpp]) / 3;
    remapped_normalize_values[pixel_value_gray].emplace_back(i);
  }

  for (const auto & [key, value] : remapped_normalize_values)
  {
    remappedValuesNormalized[key] = static_cast<float>(value.size()) / static_cast<float>(outWidth * outHeight);
  }

  return remappedValuesNormalized;
}

const std::map<int32_t, float> & HistogramEqualizationOp::GetHistogramRemapRed()
{
  std::map<int32_t, std::vector<int32_t>> remapped_normalize_values;

  constexpr int32_t bpp = 4;
  for (size_t i=0; i<(outWidth * outHeight); i++)
  {
    int32_t pixel_value = (result[0 + i * bpp]);
    remapped_normalize_values[pixel_value].emplace_back(i);
  }

  for (const auto & [key, value] : remapped_normalize_values)
  {
    remappedValuesNormalizedRed[key] = static_cast<float>(value.size()) / static_cast<float>(outWidth * outHeight);
  }

  return remappedValuesNormalizedRed;
}

const std::map<int32_t, float> & HistogramEqualizationOp::GetHistogramRemapGreen()
{
  std::map<int32_t, std::vector<int32_t>> remapped_normalize_values;

  constexpr int32_t bpp = 4;
  for (size_t i=0; i<(outWidth * outHeight); i++)
  {
    int32_t pixel_value = (result[1 + i * bpp]);
    remapped_normalize_values[pixel_value].emplace_back(i);
  }

  for (const auto & [key, value] : remapped_normalize_values)
  {
    remappedValuesNormalizedGreen[key] = static_cast<float>(value.size()) / static_cast<float>(outWidth * outHeight);
  }

  return remappedValuesNormalizedGreen;
}

const std::map<int32_t, float> & HistogramEqualizationOp::GetHistogramRemapBlue()
{
  std::map<int32_t, std::vector<int32_t>> remapped_normalize_values;

  constexpr int32_t bpp = 4;
  for (size_t i=0; i<(outWidth * outHeight); i++)
  {
    int32_t pixel_value = (result[2 + i * bpp]);
    remapped_normalize_values[pixel_value].emplace_back(i);
  }

  for (const auto & [key, value] : remapped_normalize_values)
  {
    remappedValuesNormalizedBlue[key] = static_cast<float>(value.size()) / static_cast<float>(outWidth * outHeight);
  }

  return remappedValuesNormalizedBlue;
}

const MenuOp_HistogramColor & HistogramEqualizationOp::HistogramColorType() const
{
  return inputColorType;
}

void HistogramEqualizationOp::SetHistogramColorType(MenuOp_HistogramColor color_type)
{
  inputColorType = color_type;
}

void HistogramEqualizationOp::SetLocalizeKernelSize(int32_t x, int32_t y)
{
  kernelSizeX = x;
  kernelSizeY = y;
}

void HistogramEqualizationOp::SetLocalizeKernelConstants(float k0, float k1, float k2, float k3, float c)
{
  kernelK0 = k0;
  kernelK1 = k1;
  kernelK2 = k2;
  kernelK3 = k3;
  enhanceConst = c;
}

MenuOp_HistogramMethod HistogramEqualizationOp::GetCurrentSetOperation() const
{
  return histogramMethod;
}

void HistogramEqualizationOp::ProcessHistogram(MenuOp_HistogramMethod operation
                                              ,const std::vector<uint8_t> & source_image
                                              ,uint8_t bpp)
{
  remappedValuesGray.clear();
  remappedValuesRed.clear();
  remappedValuesGreen.clear();
  remappedValuesBlue.clear();
  remappedValuesNormalized.clear();
  remappedValuesNormalizedRed.clear();
  remappedValuesNormalizedGreen.clear();
  remappedValuesNormalizedBlue.clear();

  histogramMethod = operation;

  switch (operation)
  {
    case MenuOp_HistogramMethod::GLOBAL:
      GlobalProcess(source_image, bpp);
      break;

    case MenuOp_HistogramMethod::LOCALIZE:
      LocalizeProcess(source_image, bpp);
      break;

    case MenuOp_HistogramMethod::LOCALIZE_ENCHANCEMENT:
      LocalizeEnhancementProcess(source_image, bpp);
      break;

    default:
      spdlog::warn("unrecognized method!");
      break;
  }
}

void HistogramEqualizationOp::GlobalProcess(const std::vector<uint8_t> & source_image
                                           ,uint8_t bpp)
{
  result = source_image;

  std::vector<int> sorted_gray_values;
  std::vector<int> sorted_red_values;
  std::vector<int> sorted_green_values;
  std::vector<int> sorted_blue_values;

  // sort pixel values by channel. iterating through the map should have the values as if sorted

  for(const auto & [opv, pvf] : histogramNormalizedGray)
  {
    sorted_gray_values.emplace_back(opv);
  }

  for(const auto & [opv, pvf] : histogramNormalizedRed)
  {
    sorted_red_values.emplace_back(opv);
  }

  for(const auto & [opv, pvf] : histogramNormalizedGreen)
  {
    sorted_green_values.emplace_back(opv);
  }

  for(const auto & [opv, pvf] : histogramNormalizedBlue)
  {
    sorted_blue_values.emplace_back(opv);
  }

  // generate new mapped values for each channel

  for (size_t i=0; i<sorted_gray_values.size(); i++)
  {
    float new_mapped_value_gray = 0.0f;

    for (size_t j=0; j<(i+1); j++)
    {
      new_mapped_value_gray += histogramNormalizedGray[sorted_gray_values[j]];
    }

    remappedValuesGray[sorted_gray_values[i]] = static_cast<int>(std::round(static_cast<float>(HistogramOp::maxBppValue * new_mapped_value_gray)));
  }

  for (size_t i=0; i<sorted_red_values.size(); i++)
  {
    float new_mapped_value_red = 0.0f;

    for (size_t j=0; j<(i+1); j++)
    {
      new_mapped_value_red += histogramNormalizedRed[sorted_red_values[j]];
    }

    remappedValuesRed[sorted_red_values[i]] = static_cast<int>(std::round(static_cast<float>(HistogramOp::maxBppValue * new_mapped_value_red)));
  }

  for (size_t i=0; i<sorted_green_values.size(); i++)
  {
    float new_mapped_value_green = 0.0f;

    for (size_t j=0; j<(i+1); j++)
    {
      new_mapped_value_green += histogramNormalizedGreen[sorted_green_values[j]];
    }

    remappedValuesGreen[sorted_green_values[i]] = static_cast<int>(std::round(static_cast<float>(HistogramOp::maxBppValue * new_mapped_value_green)));
  }

  for (size_t i=0; i<sorted_blue_values.size(); i++)
  {
    float new_mapped_value_blue = 0.0f;

    for (size_t j=0; j<(i+1); j++)
    {
      new_mapped_value_blue += histogramNormalizedBlue[sorted_blue_values[j]];
    }

    remappedValuesBlue[sorted_blue_values[i]] = static_cast<int>(std::round(static_cast<float>(HistogramOp::maxBppValue * new_mapped_value_blue)));
  }

  // remap the values for Histogram Equalization into the result buffer

  if (inputColorType == MenuOp_HistogramColor::GRAY)
  {
    for (size_t i=0; i<(outWidth*outHeight); i++)
    {
      int32_t gray_value = (source_image[0 + i * bpp] + source_image[1 + i * bpp] + source_image[2 + i * bpp]) / 3;
      result[0 + i * bpp] = remappedValuesGray[gray_value];
      result[1 + i * bpp] = remappedValuesGray[gray_value];
      result[2 + i * bpp] = remappedValuesGray[gray_value];
      result[3 + i * bpp] = source_image[3 + i * bpp];
    }
  }
  else if (inputColorType == MenuOp_HistogramColor::RGBA)
  {
    for (size_t i = 0; i < (outWidth * outHeight); i++)
    {
      result[0 + i * bpp] = remappedValuesRed[source_image[0 + i * bpp]];
      result[1 + i * bpp] = remappedValuesGreen[source_image[1 + i * bpp]];
      result[2 + i * bpp] = remappedValuesBlue[source_image[2 + i * bpp]];
      result[3 + i * bpp] = source_image[3 + i * bpp];
    }
  }
}

void HistogramEqualizationOp::LocalizeProcess(const std::vector<uint8_t> & source_image
                                             ,uint8_t bpp)
{
  result = source_image;

  if (inputColorType == MenuOp_HistogramColor::GRAY)
  {
    std::vector<std::map<int32_t, int32_t>> kernel_histogram_remap(outWidth * outHeight);

    auto process_local_pixel = [this] (const std::vector<uint8_t> & source_image, std::vector<std::map<int32_t, int32_t>> & khr, uint8_t & bpp, int32_t i, int32_t j) {
      auto [kernel_he_collection, min_value, max_value] = CollectPixelValues(source_image, outWidth, outHeight, j-(kernelSizeX / 2), i-(kernelSizeY / 2), j+(kernelSizeX / 2) + 1, i+(kernelSizeY / 2)+1, 0, 3, bpp);
      auto kernel_he_normalized = NormalizeHistogramValues(kernel_he_collection, kernelSizeX, kernelSizeY);

      float new_mapped_value_gray = 0.0f;
      for (const auto & [k, v] : kernel_he_normalized)
      {
        new_mapped_value_gray += kernel_he_normalized[k];
        khr[j + (i * outWidth)][k] = static_cast<int32_t>(std::round(static_cast<float>(HistogramOp::maxBppValue * new_mapped_value_gray)));
      }
    };

    for (int32_t i=0; i<outHeight; i++)
    {
      for (int32_t j=0; j<outWidth; j++)
      {
        workPool.addjob([&, ii=i, jj=j](){
          process_local_pixel(source_image, kernel_histogram_remap, bpp, ii, jj);
        });
      }
    }

    auto work_done = std::async([&](){
      float work_completed = 0.0f;
      while(work_completed < 1.0f)
      {
        work_completed = (static_cast<float>(outWidth * outHeight) - static_cast<float>(workPool.numberofjobs())) / static_cast<float>(outWidth * outHeight);
        spdlog::info("histogram work completed: {:.2f}%", work_completed * 100.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    });

    workPool.waitforthread();
    work_done.wait_for(std::chrono::milliseconds(1000));

    for (size_t i=0; i<(outWidth * outHeight); i++)
    {
      int32_t gray_value = (source_image[0 + i * bpp] + source_image[1 + i * bpp] + source_image[2 + i * bpp]) / 3;
      result[0 + i * bpp] = kernel_histogram_remap[i][gray_value];
      result[1 + i * bpp] = kernel_histogram_remap[i][gray_value];
      result[2 + i * bpp] = kernel_histogram_remap[i][gray_value];
      result[3 + i * bpp] = source_image[3 + i * bpp];
    }

  }
  else // inputColorType == MenuOp_HistogramColor::RGBA
  {
    std::vector<std::map<int32_t, int32_t>> kernel_histogram_remap_red(outWidth * outHeight);
    std::vector<std::map<int32_t, int32_t>> kernel_histogram_remap_green(outWidth * outHeight);
    std::vector<std::map<int32_t, int32_t>> kernel_histogram_remap_blue(outWidth * outHeight);

    auto process_local_pixel = [this] (const std::vector<uint8_t> & source_image, std::vector<std::map<int32_t, int32_t>> & khr, uint8_t & bpp, int32_t i, int32_t j, int32_t offset) {
      auto [kernel_he_collection, min_value, max_value] = CollectPixelValues(source_image, outWidth, outHeight, j-(kernelSizeX / 2), i-(kernelSizeY / 2), j+(kernelSizeX / 2) + 1, i+(kernelSizeY / 2)+1, offset, 1, bpp);
      auto kernel_he_normalized = NormalizeHistogramValues(kernel_he_collection, kernelSizeX, kernelSizeY);

      float new_mapped_value_gray = 0.0f;
      for (const auto & [k, v] : kernel_he_normalized)
      {
        new_mapped_value_gray += kernel_he_normalized[k];
        khr[j + (i * outWidth)][k] = static_cast<int32_t>(std::round(static_cast<float>(HistogramOp::maxBppValue * new_mapped_value_gray)));
      }
    };

    for (int32_t i=0; i<outHeight; i++)
    {
      for (int32_t j=0; j<outWidth; j++)
      {
        workPool.addjob([&, ii=i, jj=j](){
          process_local_pixel(source_image, kernel_histogram_remap_red, bpp, ii, jj, 0);
          process_local_pixel(source_image, kernel_histogram_remap_green, bpp, ii, jj, 1);
          process_local_pixel(source_image, kernel_histogram_remap_blue, bpp, ii, jj, 2);
        });
      }
    }

    auto work_done = std::async([&](){
      float work_completed = 0.0f;
      while(work_completed < 1.0f)
      {
        work_completed = (static_cast<float>(outWidth * outHeight) - static_cast<float>(workPool.numberofjobs())) / static_cast<float>(outWidth * outHeight);
        spdlog::info("histogram work completed: {:.2f}%", work_completed * 100.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    });

    workPool.waitforthread();
    work_done.wait_for(std::chrono::milliseconds(1000));

    for (size_t i = 0; i < (outWidth * outHeight); i++)
    {
      result[0 + i * bpp] = kernel_histogram_remap_red[i][source_image[0 + i * bpp]];
      result[1 + i * bpp] = kernel_histogram_remap_green[i][source_image[1 + i * bpp]];
      result[2 + i * bpp] = kernel_histogram_remap_blue[i][source_image[2 + i * bpp]];
      result[3 + i * bpp] = source_image[3 + i * bpp];
    }
  }
}

void HistogramEqualizationOp::LocalizeEnhancementProcess(const std::vector<uint8_t> & source_image
                                                        ,uint8_t bpp)
{
  result = source_image;

  auto process_local_pixel = [this] (const std::vector<uint8_t> & source_image, std::vector<uint8_t> & r, float g_mean, float g_sd, float k0, float k1, float k2, float k3, float enhance_const, int32_t offset, int32_t count, uint8_t & bpp, int32_t i, int32_t j) {
    auto [kernel_he_collection, min_value, max_value] = CollectPixelValues(source_image, outWidth, outHeight, j-(kernelSizeX / 2), i-(kernelSizeY / 2), j+(kernelSizeX / 2) + 1, i+(kernelSizeY / 2)+1, 0, 3, bpp);
    auto kernel_he_normalized = NormalizeHistogramValues(kernel_he_collection, kernelSizeX, kernelSizeY);
    auto kernel_mean = HistogramMean(kernel_he_normalized);
    auto kernel_standard_deviation = HistogramStandardDeviation(kernel_he_normalized, kernel_mean);

    if (((k0 * g_mean) <= kernel_mean && kernel_mean <= (k1 * g_mean)) && ((k2 * g_sd <= kernel_standard_deviation) && (kernel_standard_deviation <= k3 * g_sd)))
    {
      if (count > 0)
      {
        float gray_value = 0.0f;
        for (int32_t channel_counter = 0; channel_counter < count; channel_counter++)
        {
          gray_value += static_cast<float>(r[(j * bpp) + (i * outWidth * bpp) + offset + channel_counter]);
        }

        gray_value /= static_cast<float>(count);

        for (int32_t channel_counter = 0; channel_counter < count; channel_counter++)
        {
          r[(j * bpp) + (i * outWidth * bpp) + offset + channel_counter] = static_cast<uint8_t>(std::clamp((gray_value * enhance_const), 0.0f, 255.0f));
        }

      }
      else
      {
        r[(j * bpp) + (i * outWidth * bpp) + offset] = static_cast<uint8_t>(std::clamp(static_cast<float>(r[(j * bpp) + (i * outWidth * bpp) + offset]) * enhance_const, 0.0f,255.0f));
      }
    }
  };

  if (inputColorType == MenuOp_HistogramColor::GRAY)
  {
    auto global_mean = std::round(HistogramMean(histogramNormalizedGray));
    auto global_standard_deviation = std::round(HistogramStandardDeviation(histogramNormalizedGray, global_mean));

    spdlog::info("global mean: {}", global_mean);
    spdlog::info("global standard deviation: {}", global_standard_deviation);

    for (int32_t i=0; i<outHeight; i++)
    {
      for (int32_t j=0; j<outWidth; j++)
      {
        workPool.addjob([&, ii=i, jj=j](){
          process_local_pixel(source_image, result, global_mean, global_standard_deviation, kernelK0, kernelK1, kernelK2, kernelK3, enhanceConst, 0, 3, bpp, ii, jj);
        });
      }
    }
  }
  else // MenuOp_HistogramColor::RGBA
  {
    auto global_mean_red = std::round(HistogramMean(histogramNormalizedRed));
    auto global_standard_deviation_red = std::round(HistogramStandardDeviation(histogramNormalizedRed, global_mean_red));

    auto global_mean_green = std::round(HistogramMean(histogramNormalizedGreen));
    auto global_standard_deviation_green = std::round(HistogramStandardDeviation(histogramNormalizedGreen, global_mean_green));

    auto global_mean_blue = std::round(HistogramMean(histogramNormalizedBlue));
    auto global_standard_deviation_blue = std::round(HistogramStandardDeviation(histogramNormalizedBlue, global_mean_blue));

    spdlog::info("global mean (red): {}", global_mean_red);
    spdlog::info("global standard deviation (red): {}", global_standard_deviation_red);

    spdlog::info("global mean (green): {}", global_mean_green);
    spdlog::info("global standard deviation (green): {}", global_standard_deviation_green);

    spdlog::info("global mean (blue): {}", global_mean_blue);
    spdlog::info("global standard deviation (blue): {}", global_standard_deviation_blue);

    for (int32_t i=0; i<outHeight; i++)
    {
      for (int32_t j=0; j<outWidth; j++)
      {
        workPool.addjob([&, ii=i, jj=j](){
          process_local_pixel(source_image, result, global_mean_red, global_standard_deviation_red, kernelK0, kernelK1, kernelK2, kernelK3, enhanceConst, 0, 0, bpp, ii, jj);
          process_local_pixel(source_image, result, global_mean_green, global_standard_deviation_green, kernelK0, kernelK1, kernelK2, kernelK3, enhanceConst, 1, 0, bpp, ii, jj);
          process_local_pixel(source_image, result, global_mean_blue, global_standard_deviation_blue, kernelK0, kernelK1, kernelK2, kernelK3, enhanceConst, 2, 0, bpp, ii, jj);
        });
      }
    }

    auto work_done = std::async([&](){
      float work_completed = 0.0f;
      while(work_completed < 1.0f)
      {
        work_completed = (static_cast<float>(outWidth * outHeight) - static_cast<float>(workPool.numberofjobs())) / static_cast<float>(outWidth * outHeight);
        spdlog::info("histogram work completed: {:.2f}%", work_completed * 100.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    });

    workPool.waitforthread();
    work_done.wait();
  }
}
