#pragma once

#include <cstdint>

class VaryBitsMenu
{
  public:
    VaryBitsMenu() = default;

    void RenderMenu();
    [[nodiscard]] int32_t BitScale() const;
    bool ShiftBitsForContrast() const;
    bool ProcessBegin();

private:
    bool processBegin = false;
    bool shiftBitsContrast = true;
    int32_t setBit = 8;
};
