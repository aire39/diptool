#include "HistogramEqualizationOp.h"

#include <cmath>
#include <spdlog/spdlog.h>

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
    remappedValuesNormalizedRed[key] = static_cast<float>(value.size()) / static_cast<float>(outWidth * outHeight);
  }

  return remappedValuesNormalizedRed;
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
    remappedValuesNormalizedRed[key] = static_cast<float>(value.size()) / static_cast<float>(outWidth * outHeight);
  }

  return remappedValuesNormalizedRed;
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

void HistogramEqualizationOp::ProcessHistogram(MenuOp_HistogramMethod operation
                                              ,const std::vector<uint8_t> & source_image
                                              ,uint8_t bpp)
{
  remappedValuesGray.clear();
  remappedValuesRed.clear();
  remappedValuesGreen.clear();
  remappedValuesBlue.clear();
  remappedValuesNormalized.clear();

  switch (operation)
  {
    case MenuOp_HistogramMethod::GLOBAL:
      GlobalProcess(source_image, bpp);
      break;

    case MenuOp_HistogramMethod::LOCALIZE:
      LocalizeProcess(source_image, bpp);
      break;

    default:
      spdlog::warn("unrecognized method!");
      break;
  }

  histogramMethod = operation;
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
      result[0 + i*bpp] = remappedValuesGray[source_image[0 + i*bpp]];
      result[1 + i*bpp] = remappedValuesGray[source_image[0 + i*bpp]];
      result[2 + i*bpp] = remappedValuesGray[source_image[0 + i*bpp]];
      result[3 + i*bpp] = 255;
    }
  }
  else if (inputColorType == MenuOp_HistogramColor::RGBA)
  {
    for (size_t i = 0; i < (outWidth * outHeight); i++)
    {
      result[0 + i * bpp] = remappedValuesRed[source_image[0 + i * bpp]];
      result[1 + i * bpp] = remappedValuesGreen[source_image[1 + i * bpp]];
      result[2 + i * bpp] = remappedValuesBlue[source_image[2 + i * bpp]];
      result[3 + i * bpp] = 255;
    }
  }
}

void HistogramEqualizationOp::LocalizeProcess(const std::vector<uint8_t> & source_image
                                             ,uint8_t bpp)
{
  spdlog::info("Implementation not complete");
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

}
