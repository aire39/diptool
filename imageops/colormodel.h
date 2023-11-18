#pragma once

#include <cstdint>
#include <tuple>

namespace colormodel
{
  std::tuple<double , double, double> rgb2hsi(const uint8_t & r, const uint8_t & g, const uint8_t & b);
  std::tuple<uint8_t , uint8_t, uint8_t> hsi2rgb(const double & h, const double & s, const double & i);
}
