#include "EkopartyDemoscene.h"

#if defined(EKOPARTY_BADGE) && defined(HAS_NEOPIXEL)

#if HAS_SCREEN
#include "EkopartyLogo.h"
#include "OLEDDisplay.h"
#include "graphics/ScreenFonts.h"
#include <stdio.h>
#endif

namespace ekoparty
{
namespace
{
constexpr uint32_t signalAcquireEndMs = 560;
constexpr uint32_t logoDecodeEndMs = 1760;
constexpr uint32_t chaosLoaderEndMs = 3440;
constexpr uint32_t systemCollapseEndMs = 4480;
constexpr uint32_t skullEndMs = 5520;
constexpr uint32_t watcherEyeEndMs = 6880;
constexpr uint32_t digitalLaughEndMs = 8000;
constexpr uint32_t manifestoEndMs = 9280;
constexpr uint32_t logoRebuildEndMs = 10800;
constexpr uint32_t breakLogicEndMs = 11600;
constexpr uint32_t handoffEndMs = 12000;

static_assert(signalAcquireEndMs % visualFrameIntervalMs == 0, "Signal scene must end on a visual frame");
static_assert(logoDecodeEndMs % visualFrameIntervalMs == 0, "Logo scene must end on a visual frame");
static_assert(chaosLoaderEndMs % visualFrameIntervalMs == 0, "Loader scene must end on a visual frame");
static_assert(systemCollapseEndMs % visualFrameIntervalMs == 0, "Collapse scene must end on a visual frame");
static_assert(skullEndMs % visualFrameIntervalMs == 0, "Skull scene must end on a visual frame");
static_assert(watcherEyeEndMs % visualFrameIntervalMs == 0, "Eye scene must end on a visual frame");
static_assert(digitalLaughEndMs % visualFrameIntervalMs == 0, "Laugh scene must end on a visual frame");
static_assert(manifestoEndMs % visualFrameIntervalMs == 0, "Manifesto scene must end on a visual frame");
static_assert(logoRebuildEndMs % visualFrameIntervalMs == 0, "Rebuild scene must end on a visual frame");
static_assert(breakLogicEndMs % visualFrameIntervalMs == 0, "Claim scene must end on a visual frame");
static_assert(handoffEndMs == demoDurationMs, "Timeline and demo duration must match");
} // namespace

Timeline timelineAt(uint32_t elapsedMs)
{
    const uint32_t boundedMs = elapsedMs < demoDurationMs ? elapsedMs : demoDurationMs - 1;
    Timeline result = {DemoScene::HANDOFF, boundedMs - breakLogicEndMs,
                       static_cast<uint16_t>(boundedMs / visualFrameIntervalMs)};

    if (boundedMs < signalAcquireEndMs) {
        result.scene = DemoScene::SIGNAL_ACQUIRE;
        result.localMs = boundedMs;
    } else if (boundedMs < logoDecodeEndMs) {
        result.scene = DemoScene::LOGO_DECODE;
        result.localMs = boundedMs - signalAcquireEndMs;
    } else if (boundedMs < chaosLoaderEndMs) {
        result.scene = DemoScene::CHAOS_LOADER;
        result.localMs = boundedMs - logoDecodeEndMs;
    } else if (boundedMs < systemCollapseEndMs) {
        result.scene = DemoScene::SYSTEM_COLLAPSE;
        result.localMs = boundedMs - chaosLoaderEndMs;
    } else if (boundedMs < skullEndMs) {
        result.scene = DemoScene::SKULL;
        result.localMs = boundedMs - systemCollapseEndMs;
    } else if (boundedMs < watcherEyeEndMs) {
        result.scene = DemoScene::WATCHER_EYE;
        result.localMs = boundedMs - skullEndMs;
    } else if (boundedMs < digitalLaughEndMs) {
        result.scene = DemoScene::DIGITAL_LAUGH;
        result.localMs = boundedMs - watcherEyeEndMs;
    } else if (boundedMs < manifestoEndMs) {
        result.scene = DemoScene::MANIFESTO;
        result.localMs = boundedMs - digitalLaughEndMs;
    } else if (boundedMs < logoRebuildEndMs) {
        result.scene = DemoScene::LOGO_REBUILD;
        result.localMs = boundedMs - manifestoEndMs;
    } else if (boundedMs < breakLogicEndMs) {
        result.scene = DemoScene::BREAK_LOGIC;
        result.localMs = boundedMs - logoRebuildEndMs;
    }

    return result;
}

uint8_t signalByte(uint16_t step, uint8_t salt)
{
    const uint16_t salted = step + static_cast<uint16_t>(salt) * 37U;
    const uint16_t value = salted * ((salted >> 2) | (salted >> 5) | 3U) + (salted ^ (salted >> 3));
    return static_cast<uint8_t>((value ^ (value >> 8) ^ (static_cast<uint16_t>(salt) * 73U)) & 0xffU);
}

uint8_t loaderProgress(uint32_t localMs)
{
    if (localMs < 880) {
        return static_cast<uint8_t>((localMs * 72U) / 880U);
    }
    if (localMs < 1120) {
        return static_cast<uint8_t>(72U - ((localMs - 880U) * 18U) / 240U);
    }
    if (localMs < 1440) {
        return static_cast<uint8_t>(54U + ((localMs - 1120U) * 45U) / 320U);
    }
    return static_cast<uint8_t>(97U + ((localMs / visualFrameIntervalMs) & 0x01U));
}

#if HAS_SCREEN

namespace
{
int16_t centeredX(OLEDDisplay *display, int16_t x)
{
    return x + display->getWidth() / 2;
}

void resetDrawingState(OLEDDisplay *display)
{
    display->setColor(WHITE);
    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawGlitchOverlay(OLEDDisplay *display, int16_t x, int16_t y, uint16_t frame, uint8_t intensity)
{
    if (intensity == 0) {
        return;
    }

    const uint8_t count = 1U + intensity * 2U;
    display->setColor(INVERSE);
    for (uint8_t i = 0; i < count; i++) {
        const uint8_t a = signalByte(frame + i * 11U, 3U + i);
        const uint8_t b = signalByte(frame + i * 17U, 13U + i);
        const int16_t lineX = x + (a % 104U);
        const int16_t lineY = y + (b % 64U);
        const int16_t width = 8 + ((a ^ b) % 25U);
        display->fillRect(lineX, lineY, width, 1 + ((a >> 6) & 0x01U));
    }
    display->setColor(WHITE);
}

void drawLogoBands(OLEDDisplay *display, int16_t x, int16_t y, uint32_t localMs, uint32_t alignAtMs,
                   uint16_t frame)
{
    constexpr uint8_t bandHeight = 8;
    constexpr uint8_t bandCount = ekopartyLogoHeight / bandHeight;
    constexpr uint8_t bytesPerRow = (ekopartyLogoWidth + 7U) / 8U;
    const uint32_t remaining = localMs < alignAtMs ? alignAtMs - localMs : 0;

    for (uint8_t band = 0; band < bandCount; band++) {
        const int8_t direction = ((band + signalByte(band, 7)) & 0x01U) ? 1 : -1;
        const uint8_t amplitude = 5U + (signalByte(band, 19) % 13U);
        int16_t offset = 0;
        if (remaining > 0) {
            offset = direction * static_cast<int16_t>((amplitude * remaining) / alignAtMs);
            if (((frame + band * 3U) % 11U) == 0U) {
                offset += direction * 4;
            }
        }
        const uint8_t firstRow = band * bandHeight;
        display->drawXbm(x + offset, y + firstRow, ekopartyLogoWidth, bandHeight,
                         ekopartyLogo + firstRow * bytesPerRow);
    }

    if (remaining > 0) {
        const int16_t scanY = y + static_cast<int16_t>(((alignAtMs - remaining) * 63U) / alignAtMs);
        display->drawLine(x, scanY, x + display->getWidth() - 1, scanY);
        if ((frame % 5U) == 1U) {
            drawGlitchOverlay(display, x, y, frame, 1);
        }
    }
}

void drawSignalAcquire(OLEDDisplay *display, int16_t x, int16_t y, const Timeline &timeline)
{
    const int16_t centerX = centeredX(display, x);
    const bool locked = timeline.localMs >= 400;

    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(centerX, y + 8, locked ? "CARRIER::LOCK" : "CARRIER::SEARCH");
    display->setFont(FONT_MEDIUM);
    display->drawString(centerX, y + 27, locked ? "> SIGNAL_" : "> _");

    const uint8_t sweep = static_cast<uint8_t>((timeline.localMs * 112U) / signalAcquireEndMs);
    display->drawLine(x + 8, y + 54, x + 120, y + 54);
    display->fillRect(x + 8 + sweep, y + 51, 2, 7);
    if (!locked) {
        drawGlitchOverlay(display, x, y, timeline.visualFrame, 1);
    }
}

void drawChaosLoader(OLEDDisplay *display, int16_t x, int16_t y, const Timeline &timeline)
{
    const int16_t centerX = centeredX(display, x);
    const uint8_t progress = loaderProgress(timeline.localMs);
    const uint8_t filledSegments = static_cast<uint8_t>((progress * 16U) / 100U);
    char code[24];
    snprintf(code, sizeof(code), "%02X %02X %02X // %04X", signalByte(timeline.visualFrame, 2),
             signalByte(timeline.visualFrame, 11), signalByte(timeline.visualFrame, 23), timeline.visualFrame);

    const char *status = "LOGIC: PATCHING";
    if (timeline.localMs >= 560 && timeline.localMs < 1040) {
        status = "CONTROL: UNSTABLE";
    } else if (timeline.localMs >= 1040 && timeline.localMs < 1440) {
        status = "AUTH: NULL";
    } else if (timeline.localMs >= 1440) {
        status = "LOGIC: 0xDEAD";
    }

    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(centerX, y, "EKO/OS :: CHAOS");
    display->drawString(centerX, y + 13, status);
    display->drawString(centerX, y + 26, code);

    display->drawRect(x + 6, y + 41, 116, 9);
    for (uint8_t segment = 0; segment < filledSegments; segment++) {
        display->fillRect(x + 9 + segment * 7, y + 43, 5, 5);
    }

    display->drawString(centerX, y + 51, progress >= 97 ? "ORDER IS AN ILLUSION" : "BREAK THE LOGIC");
    if (timeline.localMs > 760 && ((timeline.visualFrame % 7U) == 2U || progress >= 97)) {
        drawGlitchOverlay(display, x, y, timeline.visualFrame, progress >= 97 ? 2 : 1);
    }
}

void drawSystemCollapse(OLEDDisplay *display, int16_t x, int16_t y, const Timeline &timeline)
{
    const int16_t centerX = centeredX(display, x);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(FONT_SMALL);
    display->drawString(centerX + ((timeline.visualFrame & 0x01U) ? 2 : -2), y, "CONTROL: LOST");
    display->setFont(FONT_MEDIUM);
    display->drawString(centerX - ((timeline.visualFrame % 3U) == 0U ? 4 : 0), y + 15, "SYSTEM");
    display->drawString(centerX + ((timeline.visualFrame % 5U) == 0U ? 5 : 0), y + 32, "COLLAPSE");
    display->setFont(FONT_SMALL);
    display->drawString(centerX, y + 51, "FAULT 0xC0DE");

    const uint8_t intensity = timeline.localMs < 320 ? 1 : (timeline.localMs < 760 ? 2 : 3);
    drawGlitchOverlay(display, x, y, timeline.visualFrame, intensity);
    if ((timeline.localMs >= 320 && timeline.localMs < 400) ||
        (timeline.localMs >= 800 && timeline.localMs < 880)) {
        display->setColor(INVERSE);
        display->fillRect(x, y, display->getWidth(), display->getHeight());
        display->setColor(WHITE);
    }
}

void drawSkull(OLEDDisplay *display, int16_t x, int16_t y, const Timeline &timeline)
{
    const int16_t centerX = centeredX(display, x);
    const int16_t skullY = y + 19;

    display->fillCircle(centerX, skullY, 18);
    display->fillRect(centerX - 17, skullY, 35, 18);
    display->fillRect(centerX - 12, skullY + 14, 25, 16);

    display->setColor(BLACK);
    display->fillCircle(centerX - 8, skullY + 1, 5);
    display->fillCircle(centerX + 8, skullY + 1, 5);
    display->fillTriangle(centerX, skullY + 5, centerX - 3, skullY + 12, centerX + 3, skullY + 12);
    display->fillRect(centerX - 10, skullY + 17, 21, 7);
    display->drawLine(centerX - 2, skullY - 18, centerX + 2, skullY - 10);
    display->drawLine(centerX + 2, skullY - 10, centerX - 1, skullY - 5);

    display->setColor(WHITE);
    display->drawLine(centerX - 10, skullY + 20, centerX + 10, skullY + 20);
    for (int8_t tooth = -8; tooth <= 8; tooth += 4) {
        display->drawLine(centerX + tooth, skullY + 17, centerX + tooth, skullY + 24);
    }

    if (timeline.localMs < 480) {
        const int16_t revealHalf = static_cast<int16_t>((timeline.localMs * 30U) / 480U);
        display->setColor(BLACK);
        display->fillRect(x, y, centerX - revealHalf - x, 51);
        display->fillRect(centerX + revealHalf, y, x + display->getWidth() - centerX - revealHalf, 51);
        display->setColor(WHITE);
    }

    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(FONT_SMALL);
    display->drawString(centerX, y + 51, "KERNEL PANIC");
    if ((timeline.visualFrame % 6U) == 3U) {
        drawGlitchOverlay(display, x, y, timeline.visualFrame, 1);
    }
}

void drawDigitalLaugh(OLEDDisplay *display, int16_t x, int16_t y, const Timeline &timeline)
{
    const int16_t centerX = centeredX(display, x);
    const uint8_t syllable = static_cast<uint8_t>((timeline.localMs / 160U) % 4U);
    const int16_t mouthBottom = y + (syllable == 1U || syllable == 3U ? 61 : 54);

    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    if (syllable != 0U) {
        display->drawString(x + 5, y, "HA");
    }
    if (syllable >= 2U) {
        display->drawString(x + 104, y + 3, "HA");
    }
    if (syllable == 3U) {
        display->drawString(x + 51, y, "HA");
    }

    display->drawLine(centerX - 43, y + 20, centerX - 17, y + 15);
    display->drawLine(centerX - 43, y + 21, centerX - 17, y + 19);
    display->drawLine(centerX + 43, y + 20, centerX + 17, y + 15);
    display->drawLine(centerX + 43, y + 21, centerX + 17, y + 19);

    display->fillTriangle(centerX - 47, y + 31, centerX + 47, y + 31, centerX, mouthBottom);
    display->setColor(BLACK);
    display->fillTriangle(centerX - 35, y + 36, centerX + 35, y + 36, centerX, mouthBottom - 6);
    display->setColor(WHITE);
    display->drawLine(centerX - 34, y + 39, centerX + 34, y + 39);
    for (int8_t tooth = -28; tooth <= 28; tooth += 8) {
        display->drawLine(centerX + tooth, y + 36, centerX + tooth + 2, y + 44);
    }

    if ((timeline.visualFrame % 4U) == 1U) {
        drawGlitchOverlay(display, x, y, timeline.visualFrame, 2);
    }
}

void drawMeshArg(OLEDDisplay *display, int16_t x, int16_t y, uint32_t elapsedMs, uint16_t frame)
{
    constexpr int16_t antennaX = 39;
    constexpr int16_t antennaY = 16;
    constexpr int16_t meshNodeX = 68;
    constexpr int16_t meshNodeY = 28;
    constexpr int16_t wheelX = 95;
    constexpr int16_t wheelY = 33;
    constexpr uint8_t rayPhaseCount = 4;
    static constexpr int8_t wheelDotX[] = {0, 8, 12, 8, 0, -8, -12, -8};
    static constexpr int8_t wheelDotY[] = {-11, -7, 0, 7, 11, 7, 0, -7};

    const uint8_t rayPhase = static_cast<uint8_t>((elapsedMs / 180U) % rayPhaseCount);
    const int16_t rayLength = 6 + rayPhase * 4;
    const uint8_t wheelPhase = static_cast<uint8_t>((elapsedMs / 120U) % 8U);

    // Radio mast and its truss.
    display->fillCircle(x + antennaX, y + antennaY, 3);
    display->drawLine(x + antennaX - 3, y + antennaY + 3, x + 24, y + 48);
    display->drawLine(x + antennaX + 3, y + antennaY + 3, x + 54, y + 48);
    display->drawLine(x + 24, y + 48, x + 54, y + 48);
    display->drawLine(x + 29, y + 38, x + 50, y + 38);
    display->drawLine(x + 33, y + 28, x + 46, y + 28);
    display->drawLine(x + 25, y + 48, x + 50, y + 38);
    display->drawLine(x + 53, y + 48, x + 29, y + 38);
    display->drawLine(x + 29, y + 38, x + 46, y + 28);
    display->drawLine(x + 50, y + 38, x + 33, y + 28);

    // A pulse leaves the antenna, then restarts close to the emitter.
    display->drawLine(x + antennaX - 4, y + antennaY - 3, x + antennaX - rayLength, y + antennaY - 3 - rayLength / 2);
    display->drawLine(x + antennaX + 4, y + antennaY - 3, x + antennaX + rayLength, y + antennaY - 3 - rayLength / 2);
    if (rayPhase >= 2U) {
        display->drawLine(x + antennaX - 7, y + antennaY + 2, x + antennaX - rayLength - 4, y + antennaY + 2);
        display->drawLine(x + antennaX + 7, y + antennaY + 2, x + antennaX + rayLength + 4, y + antennaY + 2);
    }

    // The linked ARG mesh and the m-powered wheel mark from the reference logo.
    display->drawLine(x + 47, y + 30, x + meshNodeX, y + meshNodeY);
    display->drawLine(x + 50, y + 42, x + 70, y + 46);
    display->drawLine(x + meshNodeX, y + meshNodeY, x + 91, y + 18);
    display->drawLine(x + meshNodeX, y + meshNodeY, x + 82, y + 34);
    display->drawLine(x + 70, y + 46, x + 84, y + 39);
    display->fillCircle(x + meshNodeX, y + meshNodeY, 4);
    display->fillCircle(x + 92, y + 18, 4);
    display->fillCircle(x + 70, y + 46, 4);
    display->drawCircle(x + wheelX, y + wheelY, 15);
    display->fillCircle(x + wheelX, y + wheelY, 4);
    for (uint8_t spoke = 0; spoke < 8U; spoke++) {
        display->drawLine(x + wheelX + wheelDotX[spoke] / 3, y + wheelY + wheelDotY[spoke] / 3,
                          x + wheelX + wheelDotX[spoke], y + wheelY + wheelDotY[spoke]);
    }
    display->fillCircle(x + wheelX + wheelDotX[wheelPhase], y + wheelY + wheelDotY[wheelPhase], 2);

    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(centeredX(display, x), y + 53, "MESH ARG");
    if ((frame % 11U) == 4U) {
        drawGlitchOverlay(display, x, y, frame, 1);
    }
}

void drawManifesto(OLEDDisplay *display, int16_t x, int16_t y, const Timeline &timeline)
{
    const int16_t centerX = centeredX(display, x);
    if (timeline.localMs < 320) {
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->setFont(FONT_SMALL);
        display->drawString(centerX, y + 25, "NO CARRIER // NO CONTROL");
        drawGlitchOverlay(display, x, y, timeline.visualFrame, 3);
        return;
    }

    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(FONT_MEDIUM);
    display->drawString(centerX, y + 6, "EMBRACE");
    display->drawString(centerX, y + 25, "THE CHAOS");
    display->setFont(FONT_SMALL);
    const uint8_t phrase = static_cast<uint8_t>(((timeline.localMs - 320U) / 320U) % 3U);
    display->drawString(centerX, y + 49,
                        phrase == 0U ? "ORDER: ILLUSION"
                                     : (phrase == 1U ? "PRIVACY: ILLUSION" : "CONTROL: ILLUSION"));
    if (timeline.localMs < 560 && (timeline.visualFrame & 0x01U)) {
        drawGlitchOverlay(display, x, y, timeline.visualFrame, 1);
    }
}

void drawBreakLogic(OLEDDisplay *display, int16_t x, int16_t y, const Timeline &timeline)
{
    const int16_t centerX = centeredX(display, x);
    display->drawLine(x + 4, y + 4, x + 25, y + 4);
    display->drawLine(x + 4, y + 4, x + 4, y + 17);
    display->drawLine(x + 103, y + 59, x + 124, y + 59);
    display->drawLine(x + 124, y + 46, x + 124, y + 59);

    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(FONT_MEDIUM);
    display->drawString(centerX, y + 6, "BREAK");
    display->drawString(centerX, y + 26, "THE LOGIC");
    display->setFont(FONT_SMALL);
    display->drawString(centerX, y + 49, "EKO22 // ONLINE");
    if (timeline.localMs >= 480 && timeline.localMs < 560) {
        drawGlitchOverlay(display, x, y, timeline.visualFrame, 2);
    }
}
} // namespace

void drawEye(OLEDDisplay *display, int16_t x, int16_t y, bool closed, int8_t pupilOffset, uint8_t openness,
             uint8_t pupilRadius)
{
    const int16_t centerX = centeredX(display, x);
    const int16_t centerY = y + display->getHeight() / 2;
    const int16_t left = x + 4;
    const int16_t right = x + display->getWidth() - 5;

    resetDrawingState(display);
    if (closed || openness < 12) {
        display->drawLine(left, centerY - 2, centerX - 34, centerY + 3);
        display->drawLine(centerX - 34, centerY + 3, centerX, centerY + 5);
        display->drawLine(centerX, centerY + 5, centerX + 34, centerY + 3);
        display->drawLine(centerX + 34, centerY + 3, right, centerY - 2);
        display->drawLine(left, centerY, centerX - 34, centerY + 5);
        display->drawLine(centerX - 34, centerY + 5, centerX, centerY + 7);
        display->drawLine(centerX, centerY + 7, centerX + 34, centerY + 5);
        display->drawLine(centerX + 34, centerY + 5, right, centerY);
        return;
    }

    const int16_t halfOpen = 3 + (24 * openness) / 100;
    const int16_t irisRadius = 4 + (17 * openness) / 100;
    const uint8_t boundedPupilRadius = pupilRadius < irisRadius ? pupilRadius : irisRadius - 1;
    display->fillCircle(centerX + pupilOffset, centerY, irisRadius);
    display->setColor(BLACK);
    display->fillCircle(centerX + pupilOffset, centerY, boundedPupilRadius);
    display->fillRect(x, y, display->getWidth(), centerY - halfOpen - y);
    display->fillRect(x, centerY + halfOpen + 1, display->getWidth(), display->getHeight() - halfOpen - centerY + y);
    display->setColor(WHITE);
    display->fillCircle(centerX + pupilOffset - 5, centerY - 5, openness > 55 ? 3 : 1);

    display->drawLine(left, centerY, centerX - 34, centerY - halfOpen / 2);
    display->drawLine(centerX - 34, centerY - halfOpen / 2, centerX - 17, centerY - halfOpen + 2);
    display->drawLine(centerX - 17, centerY - halfOpen + 2, centerX, centerY - halfOpen);
    display->drawLine(centerX, centerY - halfOpen, centerX + 17, centerY - halfOpen + 2);
    display->drawLine(centerX + 17, centerY - halfOpen + 2, centerX + 34, centerY - halfOpen / 2);
    display->drawLine(centerX + 34, centerY - halfOpen / 2, right, centerY);

    display->drawLine(left, centerY, centerX - 34, centerY + halfOpen / 2);
    display->drawLine(centerX - 34, centerY + halfOpen / 2, centerX - 17, centerY + halfOpen - 2);
    display->drawLine(centerX - 17, centerY + halfOpen - 2, centerX, centerY + halfOpen);
    display->drawLine(centerX, centerY + halfOpen, centerX + 17, centerY + halfOpen - 2);
    display->drawLine(centerX + 17, centerY + halfOpen - 2, centerX + 34, centerY + halfOpen / 2);
    display->drawLine(centerX + 34, centerY + halfOpen / 2, right, centerY);
}

void drawIntro(OLEDDisplay *display, int16_t x, int16_t y, uint32_t elapsedMs)
{
    const Timeline timeline = timelineAt(elapsedMs);
    resetDrawingState(display);

    switch (timeline.scene) {
    case DemoScene::SIGNAL_ACQUIRE:
        drawSignalAcquire(display, x, y, timeline);
        break;
    case DemoScene::LOGO_DECODE:
        drawLogoBands(display, x, y, timeline.localMs, 720, timeline.visualFrame);
        break;
    case DemoScene::CHAOS_LOADER:
        drawChaosLoader(display, x, y, timeline);
        break;
    case DemoScene::SYSTEM_COLLAPSE:
        drawSystemCollapse(display, x, y, timeline);
        break;
    case DemoScene::SKULL:
        drawSkull(display, x, y, timeline);
        break;
    case DemoScene::WATCHER_EYE: {
        const uint8_t openness =
            timeline.localMs < 480 ? static_cast<uint8_t>((timeline.localMs * 100U) / 480U) : 100;
        int8_t pupilOffset = 0;
        if (timeline.localMs >= 480 && timeline.localMs < 1040) {
            pupilOffset = static_cast<int8_t>(-13 + ((timeline.localMs - 480U) * 26U) / 560U);
        } else if (timeline.localMs >= 1040) {
            pupilOffset = 3;
        }
        const uint8_t pupilRadius =
            timeline.localMs < 1040 ? 9 : static_cast<uint8_t>(9 + ((timeline.localMs - 1040U) * 4U) / 320U);
        drawEye(display, x, y, false, pupilOffset, openness, pupilRadius);
        if (timeline.localMs > 960 && (timeline.visualFrame % 7U) == 3U) {
            drawGlitchOverlay(display, x, y, timeline.visualFrame, 1);
        }
        break;
    }
    case DemoScene::DIGITAL_LAUGH:
        drawDigitalLaugh(display, x, y, timeline);
        break;
    case DemoScene::MANIFESTO:
        drawManifesto(display, x, y, timeline);
        break;
    case DemoScene::LOGO_REBUILD:
        drawLogoBands(display, x, y, timeline.localMs, 1120, timeline.visualFrame);
        break;
    case DemoScene::BREAK_LOGIC:
        drawBreakLogic(display, x, y, timeline);
        break;
    case DemoScene::HANDOFF:
        if (timeline.localMs >= 160) {
            const uint8_t openness = timeline.localMs < 240
                                         ? 0
                                         : static_cast<uint8_t>(((timeline.localMs - 240U) * 100U) / 160U);
            drawEye(display, x, y, openness == 0, 0, openness, 11);
        }
        break;
    }

    resetDrawingState(display);
}

void drawIdleAnimation(OLEDDisplay *display, int16_t x, int16_t y, IdleAnimation animation, uint32_t elapsedMs)
{
    const uint16_t visualFrame = static_cast<uint16_t>(elapsedMs / visualFrameIntervalMs);
    resetDrawingState(display);

    switch (animation) {
    case IdleAnimation::WATCHER_EYE: {
        const uint32_t cycleMs = elapsedMs % 5600U;
        int8_t pupilOffset = 0;
        if (cycleMs >= 600 && cycleMs < 1200) {
            pupilOffset = static_cast<int8_t>(-((cycleMs - 600U) * 11U) / 600U);
        } else if (cycleMs < 1800 && cycleMs >= 1200) {
            pupilOffset = -11;
        } else if (cycleMs >= 1800 && cycleMs < 2800) {
            pupilOffset = static_cast<int8_t>(-11 + ((cycleMs - 1800U) * 22U) / 1000U);
        } else if (cycleMs >= 2800 && cycleMs < 3400) {
            pupilOffset = 11;
        } else if (cycleMs >= 3400 && cycleMs < 4000) {
            pupilOffset = static_cast<int8_t>(11 - ((cycleMs - 3400U) * 11U) / 600U);
        }

        uint8_t openness = 100;
        if (cycleMs >= 4400 && cycleMs < 4720) {
            const uint32_t blinkMs = cycleMs - 4400U;
            if (blinkMs < 160) {
                openness = static_cast<uint8_t>(100U - (blinkMs * 100U) / 160U);
            } else {
                openness = static_cast<uint8_t>(((blinkMs - 160U) * 100U) / 160U);
            }
        }

        const uint8_t pupilRadius = static_cast<uint8_t>(9U + ((cycleMs / 480U) % 4U));
        drawEye(display, x, y, openness < 12, pupilOffset, openness, pupilRadius);
        break;
    }
    case IdleAnimation::GLITCH_LOGO: {
        const uint32_t cycleMs = elapsedMs % 3600U;
        if (cycleMs < 720) {
            drawLogoBands(display, x, y, cycleMs, 640, visualFrame);
        } else if (cycleMs >= 2000 && cycleMs < 2480) {
            const uint32_t burstMs = cycleMs - 2000U;
            const uint32_t distortionMs = burstMs < 240U ? 480U - burstMs * 2U : (burstMs - 240U) * 2U;
            drawLogoBands(display, x, y, distortionMs, 480, visualFrame);
            drawGlitchOverlay(display, x, y, visualFrame, 2);
        } else {
            const int16_t logoX = centeredX(display, x) - ekopartyLogoWidth / 2;
            display->drawXbm(logoX, y, ekopartyLogoWidth, ekopartyLogoHeight, ekopartyLogo);
            if (cycleMs >= 1520 && cycleMs < 1600) {
                drawGlitchOverlay(display, x, y, visualFrame, 1);
            }
        }
        break;
    }
    case IdleAnimation::KERNEL_SKULL: {
        const uint32_t cycleMs = elapsedMs % 4800U;
        const Timeline timeline = {DemoScene::SKULL, cycleMs, visualFrame};
        drawSkull(display, x, y, timeline);
        break;
    }
    case IdleAnimation::CHAOS_GRIN: {
        const Timeline timeline = {DemoScene::DIGITAL_LAUGH, elapsedMs % 1120U, visualFrame};
        drawDigitalLaugh(display, x, y, timeline);
        break;
    }
    case IdleAnimation::MESH_ARG:
        drawMeshArg(display, x, y, elapsedMs, visualFrame);
        break;
    case IdleAnimation::COUNT:
        drawEye(display, x, y, false);
        break;
    }

    resetDrawingState(display);
}
#endif
} // namespace ekoparty

#endif
