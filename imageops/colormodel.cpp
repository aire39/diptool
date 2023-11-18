#include "colormodel.h"
#include <cmath>
#include <algorithm>
#include <numbers>

namespace {
  double deg2rad(double deg)
  {
    return deg * std::numbers::pi / 180.0;
  }

  double rad2deg(double rad)
  {
    return rad * 180.0 / std::numbers::pi;
  }
}

namespace colormodel
{
  std::tuple<double , double, double> rgb2hsi(const uint8_t & r, const uint8_t & g, const uint8_t & b)
  {
    double h = 0.0f;
    double s = 0.0f;
    double i = 0.0f;

    double r_norm = static_cast<double>(r) / static_cast<double>(r + g + b);
    double g_norm = static_cast<double>(g) / static_cast<double>(r + g + b);
    double b_norm = static_cast<double>(b) / static_cast<double>(r + g + b);

    i = (static_cast<double>(r) + static_cast<double>(g) + static_cast<double>(b)) / (3.0 * 255.0);

    if (g >= b)
    {
      h = std::acos((r_norm - (0.5*g_norm) - (0.5*b_norm)) / std::sqrt((r_norm*r_norm) + (g_norm*g_norm) + (b_norm*b_norm) - (r_norm*g_norm) - (r_norm*b_norm) - (g_norm*b_norm)));
      h = std::isnan(h) ? 0 : h;
    }
    else
    {
      h = deg2rad(360.0) - std::acos((r_norm - (0.5*g_norm) - (0.5*b_norm)) / std::sqrt((r_norm*r_norm) + (g_norm*g_norm) + (b_norm*b_norm) - (r_norm*g_norm) - (r_norm*b_norm) - (g_norm*b_norm)));
      h = std::isnan(h) ? 0 : h;
    }

    if (i > 0)
    {
      s = static_cast<double>(1.0 - ((std::min(std::min(r, g), b)) / 255.0) / i);
    }

    constexpr double h_bias = 0.000;
    return {rad2deg(h + h_bias), (s * 100.0), std::clamp(i * 255.0, 0.0, 255.0)};
  }

  std::tuple<uint8_t , uint8_t, uint8_t> hsi2rgb(const double & h, const double & s, const double & i)
  {
    uint8_t r = 0, g = 0, b = 0;

    double sat = static_cast<double>(s) / 100.0f;
    double hue = deg2rad(static_cast<double>(h));
    auto intensity = static_cast<double>(i) / 255.0;

    if (std::floor(h) == 0)
    {
      r = static_cast<uint8_t>(std::round((intensity + (2.0 * intensity * sat)) * 255.0));
      g = static_cast<uint8_t>(std::round((intensity - (intensity * sat)) * 255.0));
      b = static_cast<uint8_t>(std::round((intensity - (intensity * sat)) * 255.0));
    }
    else if (std::floor(h) > 0 && std::floor(h) < 120)
    {
      r = static_cast<uint8_t>(std::round((intensity + (intensity * sat) * std::cos(hue) / std::cos(deg2rad(60.0) - hue)) * 255.0));
      g = static_cast<uint8_t>(std::round((intensity + (intensity * sat) * (1.0 - std::cos(hue) / std::cos(deg2rad(60.0) - hue))) * 255.0));
      b = static_cast<uint8_t>(std::round((intensity - (intensity * sat)) * 255.0));
    }
    else if (std::floor(h) == 120)
    {
      r = static_cast<uint8_t>(std::round((intensity - (intensity * sat)) * 255.0));
      g = static_cast<uint8_t>(std::round((intensity + (2.0 * intensity * sat)) * 255.0));
      b = static_cast<uint8_t>(std::round((intensity - (intensity * sat)) * 255.0));
    }
    else if (std::floor(h) > 120 && std::floor(h) < 240)
    {
      r = static_cast<uint8_t>(std::round((intensity - (intensity * sat)) * 255.0));
      g = static_cast<uint8_t>(std::round((intensity + (intensity * sat) * std::cos(hue - deg2rad(120.0)) / std::cos(deg2rad(180.0) - hue)) * 255.0));
      b = static_cast<uint8_t>(std::round((intensity + (intensity * sat) * (1.0 - std::cos(hue - deg2rad(120.0)) / std::cos(deg2rad(180.0) - hue))) * 255.0));
    }
    else if (std::abs(h) == 240)
    {
      r = static_cast<uint8_t>(std::round((intensity - (intensity * sat)) * 255.0));
      g = static_cast<uint8_t>(std::round((intensity - (intensity * sat)) * 255.0));
      b = static_cast<uint8_t>(std::round((intensity + (2.0 * intensity * sat)) * 255.0));
    }
    else if (std::floor(h) > 240 && std::floor(h) < 360)
    {
      r = static_cast<uint8_t>(std::round((intensity + (intensity * sat) * (1.0 - std::cos(hue - deg2rad(240.0)) / std::cos(deg2rad(300.0) - hue))) * 255.0));
      g = static_cast<uint8_t>(std::round((intensity - (intensity * sat)) * 255.0));
      b = static_cast<uint8_t>(std::round((intensity + (intensity * sat) * std::cos(hue - deg2rad(240.0)) / std::cos(deg2rad(300.0) - hue)) * 255.0));
    }

    return {r, g, b};
  }
}
