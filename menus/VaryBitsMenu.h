#pragma once

#include <cstdint>

class VaryBitsMenu
{
  public:
    VaryBitsMenu() = default;

    void RenderMenu();
    [[nodiscard]] int32_t BitScale() const;
    bool ProcessBegin();

private:
    bool processBegin = false;
    int32_t setBit = 0;
};
