#pragma once

#include <cstdint>

class VaryBitsMenu
{
  public:
    VaryBitsMenu() = default;

    void RenderMenu();
    [[nodiscard]] int32_t BitScale() const;
    [[nodiscard]] bool ShiftBitsForContrast() const;
    bool ProcessBegin();

    bool UseColorChannels() const;

private:
    bool processBegin = false;
    bool shiftBitsContrast = true;
    int32_t setBit = 8;
    bool useColor = false;
};
