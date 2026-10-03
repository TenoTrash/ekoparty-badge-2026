#pragma once

#include "configuration.h"
#include <stdint.h>

class OLEDDisplay;

namespace ekoparty
{
constexpr uint32_t demoDurationMs = 12000;
constexpr uint32_t visualFrameIntervalMs = 80;

enum class DemoScene : uint8_t {
    SIGNAL_ACQUIRE,
    LOGO_DECODE,
    CHAOS_LOADER,
    SYSTEM_COLLAPSE,
    SKULL,
    WATCHER_EYE,
    DIGITAL_LAUGH,
    MANIFESTO,
    LOGO_REBUILD,
    BREAK_LOGIC,
    HANDOFF,
};

enum class IdleAnimation : uint8_t {
    WATCHER_EYE,
    GLITCH_LOGO,
    KERNEL_SKULL,
    CHAOS_GRIN,
    MESH_ARG,
    COUNT,
};

struct Timeline {
    DemoScene scene;
    uint32_t localMs;
    uint16_t visualFrame;
};

Timeline timelineAt(uint32_t elapsedMs);
uint8_t signalByte(uint16_t step, uint8_t salt = 0);
uint8_t loaderProgress(uint32_t localMs);

#if HAS_SCREEN
void drawIntro(OLEDDisplay *display, int16_t x, int16_t y, uint32_t elapsedMs);
void drawIdleAnimation(OLEDDisplay *display, int16_t x, int16_t y, IdleAnimation animation, uint32_t elapsedMs);
void drawEye(OLEDDisplay *display, int16_t x, int16_t y, bool closed, int8_t pupilOffset = 0,
             uint8_t openness = 100, uint8_t pupilRadius = 11);
#endif
} // namespace ekoparty
