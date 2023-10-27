#include "HistogramOp.h"

#include <limits>
#include <algorithm>
#include <cmath>

std::vector<uint8_t> HistogramOp::ProcessImage
  (MenuOps::HistogramMethod operation
  ,const std::vector<uint8_t> & source_image
  ,uint32_t width
  ,uint32_t height
  ,uint8_t bpp
  ,uint16_t iterations)
{
  histogramNormalizedGray.clear();
  histogramNormalizedRed.clear();
  histogramNormalizedGreen.clear();
  histogramNormalizedBlue.clear();
  histogramPixelValuesGray.clear();
  histogramPixelValuesRed.clear();
  histogramPixelValuesGreen.clear();
  histogramPixelValuesBlue.clear();

  outWidth = static_cast<int32_t>(width);
  outHeight = static_cast<int32_t>(height);

  minPixelValueGray = (source_image[0] + source_image[1] + source_image[2]) / 3;
  maxPixelValueGray = (source_image[0] + source_image[1] + source_image[2]) / 3;

  minPixelValueRed = source_image[0];
  maxPixelValueRed = source_image[0];

  minPixelValueGreen = source_image[1];
  maxPixelValueGreen = source_image[1];

  minPixelValueBlue = source_image[2];
  maxPixelValueBlue = source_image[2];

  // generate histogram map for each rgb channel and a gray channel

  std::tie(histogramPixelValuesGray, minPixelValueGray, maxPixelValueGray) = CollectPixelValues(source_image, width, height, 0, 3, bpp);
  std::tie(histogramPixelValuesRed, minPixelValueRed, maxPixelValueRed) = CollectPixelValues(source_image, width, height, 0, 0, bpp);
  std::tie(histogramPixelValuesGreen, minPixelValueGreen, maxPixelValueGreen) = CollectPixelValues(source_image, width, height, 1, 0, bpp);
  std::tie(histogramPixelValuesBlue, minPixelValueBlue, maxPixelValueBlue) = CollectPixelValues(source_image, width, height, 2, 0, bpp);

  // create a ratio that is normalized based on the number of pixels for each intensity over the total amount of pixels

  histogramNormalizedGray = NormalizeHistogramValues(histogramPixelValuesGray, width, height);
  histogramNormalizedRed = NormalizeHistogramValues(histogramPixelValuesRed, width, height);
  histogramNormalizedGreen = NormalizeHistogramValues(histogramPixelValuesGreen, width, height);
  histogramNormalizedBlue = NormalizeHistogramValues(histogramPixelValuesBlue, width, height);

  ProcessHistogram(operation, source_image, bpp);

  return result;
}

const std::vector<uint8_t> & HistogramOp::GetImage() const
{
  return result;
}

const std::map<int32_t, float> & HistogramOp::GetHistogram() const
{
  return histogramNormalizedGray;
}

const std::map<int32_t, float> & HistogramOp::GetHistogramRed() const
{
  return histogramNormalizedRed;
}

const std::map<int32_t, float> & HistogramOp::GetHistogramGreen() const
{
  return histogramNormalizedGreen;
}

const std::map<int32_t, float> & HistogramOp::GetHistogramBlue() const
{
  return histogramNormalizedBlue;
}

const std::map<int32_t, float> & HistogramOp::GetHistogramRemap()
{
  return dummy;
}

const std::map<int32_t, float> & HistogramOp::GetHistogramRemapRed()
{
  return dummy;
}

const std::map<int32_t, float> & HistogramOp::GetHistogramRemapGreen()
{
  return dummy;
}

const std::map<int32_t, float> & HistogramOp::GetHistogramRemapBlue()
{
  return dummy;
}

int32_t HistogramOp::GetWidth() const
{
  return outWidth;
}

int32_t HistogramOp::GetHeight() const
{
  return outHeight;
}

std::tuple<std::map<int32_t, std::vector<int32_t>>, int32_t, int32_t> HistogramOp::CollectPixelValues
  (const std::vector<uint8_t> & source_image
  ,uint32_t width
  ,uint32_t height
  ,int32_t offset
  ,int32_t sum_count
  ,int32_t bpp)
{
  // generate histogram map based on the input image source over the whole image. the function can specify the channel offset and
  // if an accumulation of channels needs to be done (sum_count)

  constexpr int32_t x_pos_start = 0;
  constexpr int32_t y_pos_start = 0;

  return CollectPixelValues(source_image
                           ,width
                           ,height
                           ,x_pos_start
                           ,y_pos_start
                           ,static_cast<int32_t>(width)
                           ,static_cast<int32_t>(height)
                           ,offset
                           ,sum_count
                           ,bpp);
}

std::tuple<std::map<int32_t, std::vector<int32_t>>, int32_t, int32_t> HistogramOp::CollectPixelValues
  (const std::vector<uint8_t> & source_image
  ,uint32_t width
  ,uint32_t height
  ,int32_t x_pos_start
  ,int32_t y_pos_start
  ,int32_t x_pos_end
  ,int32_t y_pos_end
  ,int32_t offset
  ,int32_t sum_count
  ,int32_t bpp
  )
{
  // generate histogram map based on the input image source. this can be done over a region of the source image.
  // the function can specify the channel offset and if an accumulation of channels needs to be done (sum_count).

  std::map<int32_t, std::vector<int32_t>> pixel_collection;
  int32_t min_pixel_value = 0;
  int32_t max_pixel_value = 0;

  offset = std::clamp(offset, 0, (bpp-1));
  sum_count = std::clamp(sum_count, 0, bpp);
  sum_count = std::max(0, sum_count - offset);

  y_pos_end = std::clamp(y_pos_end, 0, static_cast<int32_t>(height));
  x_pos_end = std::clamp(x_pos_end, 0, static_cast<int32_t>(width));

  for (int32_t i=y_pos_start; i<y_pos_end; i++)
  {
    for(int32_t j=x_pos_start; j<x_pos_end; j++)
    {
      int32_t ii = std::clamp(i, static_cast<int32_t>(0), static_cast<int32_t>(width)-1);
      int32_t jj = std::clamp(j, static_cast<int32_t>(0), static_cast<int32_t>(height)-1);
      int32_t pixel_value = 0;
      if (sum_count > 0)
      {
        for (size_t k = 0; k < sum_count; k++)
        {
          pixel_value += source_image[(offset + k + (jj * bpp)) + (ii * width * bpp)];
        }
        pixel_value /= sum_count;
      }
      else
      {
        pixel_value = source_image[(offset + (jj * bpp)) + (ii * width * bpp)];
      }

      pixel_collection[pixel_value].emplace_back(((jj * bpp) + (ii * width * bpp)));


      min_pixel_value = (min_pixel_value > pixel_value) ? pixel_value : min_pixel_value;
      max_pixel_value = (max_pixel_value < pixel_value) ? pixel_value : max_pixel_value;
    }
  }

  return {pixel_collection, min_pixel_value, max_pixel_value};
}

float HistogramOp::HistogramMean(const std::map<int32_t, float> & normalized_pixel_probability_map)
{
  float mean = 0.0f;

  for (const auto & [pixel_value, probability] : normalized_pixel_probability_map)
  {
    mean += (static_cast<float>(pixel_value) * probability);
  }

  return mean;

}

float HistogramOp::HistogramVariance(const std::map<int32_t, float> & normalized_pixel_probability_map
                                    ,float mean)
{
  return HistogramNthMoment(normalized_pixel_probability_map, mean, 2);
}

float HistogramOp::HistogramStandardDeviation(const std::map<int32_t, float> & normalized_pixel_probability_map
                                             ,float mean)
{
  return std::sqrt(HistogramVariance(normalized_pixel_probability_map, mean));
}

float HistogramOp::HistogramNthMoment(const std::map<int32_t, float> & normalized_pixel_probability_map
                                     ,float mean
                                     ,int32_t root)
{
  float moment = 0.0f;

  for (const auto & [pixel_value, probability] : normalized_pixel_probability_map)
  {
    moment += ((static_cast<float>(std::pow(static_cast<float>(pixel_value) - mean, root)) * probability));
  }

  return moment;
}

std::map<int32_t, float> HistogramOp::NormalizeHistogramValues(const std::map<int32_t, std::vector<int32_t>> & histogram
                                                              ,uint32_t width
                                                              ,uint32_t height)
{
  // create a ratio that is normalized based on the number of pixels for each intensity over the total amount of pixels

  std::map<int32_t, float> normalized_histogram;

  for (const auto & [pixel_value, pixel_indices] : histogram)
  {
    normalized_histogram[pixel_value] = static_cast<float>(pixel_indices.size()) / static_cast<float>(width*height);
  }

  return normalized_histogram;
}

void HistogramOp::ProcessHistogram(MenuOps::HistogramMethod operation
                                  ,const std::vector<uint8_t> & source_image
                                  ,uint8_t bpp)
{
}