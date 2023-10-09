#include "HistogramOp.h"

std::vector<uint8_t> HistogramOp::ProcessImage
  (MenuOp_Downsample operation
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

  for (size_t i=0; i<(width*height); i++)
  {
    int32_t pixel_value_gray = (source_image[0 + i*bpp] + source_image[1 + i*bpp] + source_image[2 + i*bpp]) / 3;
    int32_t pixel_value_red = source_image[0 + i*bpp];
    int32_t pixel_value_green = source_image[1 + i*bpp];
    int32_t pixel_value_blue = source_image[2 + i*bpp];

    histogramPixelValuesGray[pixel_value_gray].push_back(i);
    histogramPixelValuesRed[pixel_value_red].push_back(i);
    histogramPixelValuesGreen[pixel_value_green].push_back(i);
    histogramPixelValuesBlue[pixel_value_blue].push_back(i);

    maxPixelValueGray = (maxPixelValueGray < pixel_value_gray) ? pixel_value_gray : maxPixelValueGray;
    maxPixelValueRed = (maxPixelValueRed < pixel_value_red) ? pixel_value_red : maxPixelValueRed;
    maxPixelValueGreen = (maxPixelValueGreen < pixel_value_green) ? pixel_value_green : maxPixelValueGreen;
    maxPixelValueBlue = (maxPixelValueBlue < pixel_value_blue) ? pixel_value_blue : maxPixelValueBlue;

    minPixelValueGray = (minPixelValueGray > pixel_value_gray) ? pixel_value_gray : minPixelValueGray;
    minPixelValueRed = (minPixelValueRed > pixel_value_red) ? pixel_value_red : minPixelValueRed;
    minPixelValueGreen = (minPixelValueGreen > pixel_value_green) ? pixel_value_green : minPixelValueGreen;
    minPixelValueBlue = (minPixelValueBlue > pixel_value_blue) ? pixel_value_blue : minPixelValueBlue;

  }

  volatile int nnn = 0;
  for (const auto & [pixel_value, pixel_indices] : histogramPixelValuesGray)
  {
    nnn += static_cast<int32_t>(pixel_indices.size());
    auto n_colors = static_cast<float>(pixel_indices.size());
    histogramNormalizedGray[pixel_value] = n_colors;
  }
  for (const auto & [pixel_value, pixel_indices] : histogramPixelValuesGray)
  {
    histogramNormalizedGray[pixel_value] /= static_cast<float>(nnn);
  }

  nnn = 0;
  for (const auto & [pixel_value, pixel_indices] : histogramPixelValuesRed)
  {
    nnn += static_cast<int32_t>(pixel_indices.size());
    auto n_colors = static_cast<float>(pixel_indices.size());
    histogramNormalizedRed[pixel_value] = n_colors;
  }
  for (const auto & [pixel_value, pixel_indices] : histogramPixelValuesRed)
  {
    histogramNormalizedRed[pixel_value] /= static_cast<float>(nnn);
  }

  nnn = 0;
  for (const auto & [pixel_value, pixel_indices] : histogramPixelValuesGreen)
  {
    nnn += static_cast<int32_t>(pixel_indices.size());
    histogramNormalizedGreen[pixel_value] = static_cast<float>(pixel_indices.size());
  }
  for (const auto & [pixel_value, pixel_indices] : histogramPixelValuesGreen)
  {
    histogramNormalizedGreen[pixel_value] /= static_cast<float>(nnn);
  }

  nnn = 0;
  for (const auto & [pixel_value, pixel_indices] : histogramPixelValuesBlue)
  {
    nnn += static_cast<int32_t>(pixel_indices.size());
    histogramNormalizedBlue[pixel_value] = static_cast<float>(pixel_indices.size());
  }
  for (const auto & [pixel_value, pixel_indices] : histogramPixelValuesBlue)
  {
    histogramNormalizedBlue[pixel_value] /= static_cast<float>(nnn);
  }

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

void HistogramOp::ProcessHistogram(MenuOp_Downsample operation
                                  ,const std::vector<uint8_t> & source_image
                                  ,uint8_t bpp)
{

}