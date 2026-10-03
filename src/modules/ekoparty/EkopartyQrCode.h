#pragma once

#include <stdint.h>

namespace ekoparty
{
constexpr char projectUrl[] = "https://fabricamarciana.com/eko";
constexpr uint8_t qrSize = 25;
constexpr uint8_t qrScale = 2;
constexpr uint8_t qrCanvasSize = 64;
constexpr uint8_t qrOffset = (qrCanvasSize - qrSize * qrScale) / 2;

constexpr uint32_t projectUrlHash()
{
    uint32_t hash = 2166136261U;
    for (const char value : projectUrl) {
        if (value == '\0') {
            break;
        }
        hash = (hash ^ static_cast<uint8_t>(value)) * 16777619U;
    }
    return hash;
}

static_assert(projectUrlHash() == 0x04D40715U, "Regenerate the QR matrix after changing the project URL");

constexpr uint32_t qrRows[qrSize] = {
    0x1FC487FU, 0x1048941U, 0x174C25DU, 0x175995DU, 0x175215DU, 0x104C941U, 0x1FD557FU, 0x0006500U, 0x18EBC18U,
    0x15BFE3EU, 0x134435BU, 0x1C284E9U, 0x0679361U, 0x11BA322U, 0x125437BU, 0x13A632DU, 0x1653FF4U, 0x0012910U,
    0x1FDE151U, 0x105B312U, 0x1741FF6U, 0x174A7C3U, 0x174538DU, 0x1052571U, 0x1FD5589U,
};
} // namespace ekoparty
