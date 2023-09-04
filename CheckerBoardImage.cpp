#include "CheckerBoardImage.h"
#include <cmath>

CheckerBoardImage::CheckerBoardImage()
  : CheckerBoardImage(800, 600, 32)
{
}

CheckerBoardImage::CheckerBoardImage(uint32_t pixel_width, uint32_t pixel_height, uint32_t checker_tile_repeat)
{
  Generate(pixel_width, pixel_height, checker_tile_repeat);
}

void CheckerBoardImage::Generate(uint32_t pixel_width, uint32_t pixel_height, uint32_t repeat_checkers_per_line)
{
  image.resize(pixel_width * pixel_height * bytesPerPixel);

  pixelWidth = pixel_width;
  pixelHeight = pixel_height;

  uint8_t color_1[] = {0, 0, 0};
  uint8_t color_2[] = {255, 255, 255};
  uint8_t * set_color = color_1;
  uint8_t * color_reset = color_1;

  const uint32_t mod_value_x = repeat_checkers_per_line;
  const uint32_t mod_value_y = repeat_checkers_per_line;

  for (uint32_t i=0; i<pixel_height; i++)
  {
    for (uint32_t j=0; j<pixel_width; j++)
    {
      if (((j+1) % mod_value_x) == 0)
      {
        if (set_color == color_1)
        {
          set_color = color_2;
        }
        else
        {
          set_color = color_1;
        }
      }

      SetPixelColor(j, i, set_color);
    }

    if (((i+1) % mod_value_y) == 0)
    {
      if (color_reset == color_1)
      {
        color_reset = color_2;
      }
      else
      {
        color_reset = color_1;
      }
    }

    set_color = color_reset;
  }
}

const std::vector<uint8_t> & CheckerBoardImage::GetImage()
{
  return image;
}

void CheckerBoardImage::SetPixelColor(uint32_t x, uint32_t y, const uint8_t rgb[3])
{
  const uint32_t pixel_byte_location = ((x * bytesPerPixel) + (y * pixelWidth * bytesPerPixel));
  if (!image.empty() && pixel_byte_location < image.size())
  {
    const uint32_t red_index = (x * bytesPerPixel) + (y * pixelWidth * bytesPerPixel) + 0;
    const uint32_t green_index = (x * bytesPerPixel) + (y * pixelWidth * bytesPerPixel) + 1;
    const uint32_t blue_index = (x * bytesPerPixel) + (y * pixelWidth * bytesPerPixel) + 2;
    const uint32_t alpha_index = (x * bytesPerPixel) + (y * pixelWidth * bytesPerPixel) + 3;

    image[red_index] = rgb[0];
    image[green_index] = rgb[1];
    image[blue_index] = rgb[2];
    image[alpha_index] = 255;
  }
}
