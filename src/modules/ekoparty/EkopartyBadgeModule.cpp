#include "EkopartyBadgeModule.h"

#if defined(EKOPARTY_BADGE) && defined(HAS_NEOPIXEL)

#include "EkopartyDemoscene.h"
#include "Tone.h"
#include "mesh/Throttle.h"
#include "target_specific.h"
#include <atomic>
#include <cstdio>
#include <cstring>

#if HAS_SCREEN
#include "EkopartyQrModule.h"
#include "graphics/Screen.h"
#endif

#if defined(PIN_BUZZER)
#include <NonBlockingRtttl.h>
#endif

namespace
{
std::atomic<bool> startupDemoActive{true};

const char *const aboutTextLines[] = {
    "ESPANOL",
    "Este badge fue creado",
    "en el marco de",
    "Ekoparty 2026 por el",
    "equipo de desarrollo:",
    "Lucas Leal",
    "Diseno de hardware",
    "Nicolas Restbergs",
    "Identidad visual y",
    "adaptacion de",
    "firmware",
    "Jorge Crowe",
    "Coordinacion y",
    "concepto",
    "Buenos Aires,",
    "Argentina",
    "ENGLISH",
    "This badge was",
    "created for",
    "Ekoparty 2026 by the",
    "development team:",
    "Lucas Leal",
    "Hardware design",
    "Nicolas Restbergs",
    "Visual identity and",
    "firmware adaptation",
    "Jorge Crowe",
    "Coordination and",
    "concept",
    "Buenos Aires,",
    "Argentina",
};
constexpr uint8_t aboutTextLineCount = sizeof(aboutTextLines) / sizeof(aboutTextLines[0]);
constexpr uint8_t aboutTotalLineCount = aboutTextLineCount + 2;

uint8_t triangleWave(uint32_t elapsedMs, uint32_t periodMs, uint8_t low, uint8_t high)
{
    const uint32_t halfPeriodMs = periodMs / 2U;
    const uint32_t phase = elapsedMs % periodMs;
    const uint32_t position = phase <= halfPeriodMs ? phase : periodMs - phase;
    return low + static_cast<uint8_t>((static_cast<uint32_t>(high - low) * position) / halfPeriodMs);
}

uint8_t scaleChannel(uint8_t channel, uint8_t intensity)
{
    return static_cast<uint8_t>((static_cast<uint16_t>(channel) * intensity) / 255U);
}

struct RgbColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

RgbColor paletteColor(uint8_t position)
{
    if (position < 85U) {
        return {static_cast<uint8_t>(255U - position * 3U), 0, static_cast<uint8_t>(position * 3U)};
    }
    if (position < 170U) {
        position -= 85U;
        return {0, static_cast<uint8_t>(position * 3U), static_cast<uint8_t>(255U - position * 3U)};
    }
    position -= 170U;
    return {static_cast<uint8_t>(position * 3U), static_cast<uint8_t>(255U - position * 3U), 0};
}

uint8_t smoothstep8(uint8_t progress)
{
    const uint32_t squared = static_cast<uint32_t>(progress) * progress;
    return static_cast<uint8_t>((squared * (765U - 2U * progress) + 32512U) / 65025U);
}

RgbColor blendColor(const RgbColor &from, const RgbColor &to, uint8_t progress)
{
    const uint16_t remaining = 255U - progress;
    return {
        static_cast<uint8_t>((static_cast<uint32_t>(from.red) * remaining + static_cast<uint32_t>(to.red) * progress + 127U) /
                             255U),
        static_cast<uint8_t>((static_cast<uint32_t>(from.green) * remaining + static_cast<uint32_t>(to.green) * progress + 127U) /
                             255U),
        static_cast<uint8_t>((static_cast<uint32_t>(from.blue) * remaining + static_cast<uint32_t>(to.blue) * progress + 127U) /
                             255U),
    };
}

RgbColor logoGlitchPaletteColor(uint8_t index)
{
    switch (index % 6U) {
    case 0:
        return {190, 10, 135};
    case 1:
        return {0, 150, 175};
    case 2:
        return {190, 105, 0};
    case 3:
        return {100, 15, 190};
    case 4:
        return {40, 165, 45};
    default:
        return {200, 35, 70};
    }
}

RgbColor logoGlitchTargetColor(uint16_t arrangement, uint16_t pixel)
{
    const uint16_t yellowPixel = ekoparty::signalByte(arrangement, 101) % NEOPIXEL_COUNT;
    const uint8_t paletteIndex =
        pixel == yellowPixel ? 2U : ekoparty::signalByte(arrangement + pixel * 31U, 83U + pixel * 7U) % 6U;
    return logoGlitchPaletteColor(paletteIndex);
}

#if defined(PIN_BUZZER)
struct ToneCue {
    uint16_t atMs;
    uint16_t frequency;
    uint8_t durationMs;
};

// A composed monophonic score: identity motif, unstable data ostinato,
// tritone collapse, scanner sweep, synthetic laugh and final resolution.
constexpr ToneCue introScore[] = {
    {80, 2600, 70},    {240, 3137, 12},   {320, 3951, 10},   {480, 1337, 55},   {560, 1319, 70},   {680, 1976, 55},
    {800, 2637, 100},  {1040, 1319, 55},  {1160, 1976, 45},  {1280, 2637, 90},  {1520, 3951, 10},  {1600, 1319, 45},

    {1760, 1175, 28},  {1880, 3137, 10},  {2000, 1337, 45},  {2120, 2794, 10},  {2240, 1245, 28},  {2360, 3137, 10},
    {2480, 1661, 52},  {2600, 2950, 10},  {2720, 1175, 28},  {2840, 3137, 10},  {2960, 1337, 45},  {3080, 2794, 10},
    {3200, 1245, 28},  {3320, 1976, 52},

    {3440, 1175, 70},  {3600, 1661, 58},  {3760, 1175, 48},  {3890, 1661, 42},  {4000, 2349, 25},  {4090, 2093, 28},
    {4170, 1760, 32},  {4250, 1568, 36},  {4320, 1245, 42},  {4400, 784, 70},

    {4480, 784, 120},  {4720, 3137, 10},  {4880, 1175, 65},  {5040, 3137, 10},  {5200, 1661, 48},  {5360, 659, 120},

    {5520, 784, 42},   {5680, 988, 42},   {5840, 1245, 42},  {6000, 1568, 42},  {6160, 1976, 42},  {6320, 2489, 42},
    {6480, 2794, 40},  {6640, 3951, 12},

    {6880, 1568, 38},  {6960, 1976, 65},  {7200, 1568, 34},  {7270, 2093, 58},  {7480, 1661, 30},  {7540, 2349, 55},
    {7760, 3137, 10},  {7840, 3137, 10},

    {8000, 2600, 90},  {8240, 1175, 32},  {8320, 2794, 10},  {8400, 1337, 48},  {8560, 2950, 10},  {8720, 1661, 80},

    {9280, 1319, 70},  {9520, 1976, 55},  {9760, 2637, 110}, {10160, 1319, 55}, {10320, 1976, 55}, {10480, 2637, 120},

    {10800, 2200, 18}, {10880, 2200, 18}, {11040, 1100, 35}, {11160, 1100, 35}, {11320, 1319, 55}, {11440, 1976, 80},
};

constexpr uint16_t introScoreCount = sizeof(introScore) / sizeof(introScore[0]);
constexpr uint32_t toneCueGraceMs = 35;
#endif
} // namespace

bool ekoparty::isStartupDemoActive()
{
    return startupDemoActive.load(std::memory_order_relaxed);
}

EkopartyBadgeModule::EkopartyBadgeModule() : MeshModule("ekoparty"), concurrency::OSThread("EkopartyBadge")
{
    ekoparty::loadBadgeSettings();
    notifyDeepSleepObserver.observe(&notifyDeepSleep);
#if HAS_SCREEN && !MESHTASTIC_EXCLUDE_INPUTBROKER
    if (inputBroker) {
        inputObserver.observe(inputBroker);
    }
#endif

    pixels.begin();
    pixels.clear();
    applySettings(ekoparty::getBadgeSettings());
    pixels.show();

    LOG_INFO("EkopartyBadge enabled: startup demo, NeoPixel pin=%d, count=%d", NEOPIXEL_DATA, NEOPIXEL_COUNT);
}

int32_t EkopartyBadgeModule::runOnce()
{
#if HAS_SCREEN && !MESHTASTIC_EXCLUDE_INPUTBROKER
    if (!demoActive && pendingMenu != BadgeMenu::NONE) {
        showPendingMenu();
    }
    if (!demoActive && aboutScrollActive && !Throttle::isWithinTimespanMs(lastAboutScrollMs, aboutScrollIntervalMs)) {
        aboutScrollOffset = (aboutScrollOffset + 1U) % aboutTotalLineCount;
        lastAboutScrollMs = millis();
        showAboutScrollWindow();
    }
#endif

#if HAS_SCREEN
    // Screen ignores module UI events while the Meshtastic boot splash is active.
    // Retry until our frame is actually drawn, then start every output in sync.
    if (demoActive && !demoStarted) {
        if (demoFocusRequested && !Throttle::isWithinTimespanMs(demoFocusFirstRequestMs, demoFocusTimeoutMs)) {
            LOG_WARN("EkopartyBadge startup demo gave up waiting for screen focus");
            finishDemo();
            showIdlePixels();
            return frameIntervalMs;
        }
        if (!demoFocusRequested || !Throttle::isWithinTimespanMs(lastDemoFocusRequestMs, demoFocusRetryMs)) {
            requestDemoFocus();
        }
        return frameIntervalMs;
    }
#else
    if (demoActive && !demoStarted) {
        beginDemoPlayback();
    }
#endif

    if (demoActive && demoStarted && !Throttle::isWithinTimespanMs(demoStartedMs, ekoparty::demoDurationMs)) {
        finishDemo();
    }

    updateDemoTone();

    if (demoActive && demoStarted) {
        const uint32_t elapsedMs = millis() - demoStartedMs;
        const ekoparty::Timeline timeline = ekoparty::timelineAt(elapsedMs);
        if (timeline.visualFrame != lastDemoVisualFrame) {
            lastDemoVisualFrame = timeline.visualFrame;
            showDemoPixels(elapsedMs);
#if HAS_SCREEN
            requestRedraw();
#endif
        }
        return demoSchedulerIntervalMs;
    }

    showIdlePixels();
#if HAS_SCREEN
    if (screen && screen->isModuleFrameShown(this) && !screen->isOverlayBannerShowing()) {
        requestRedraw();
    }
#endif
    return frameIntervalMs;
}

void EkopartyBadgeModule::requestDemoFocus()
{
    const uint32_t now = millis();
    if (!demoFocusRequested) {
        demoFocusFirstRequestMs = now;
    }
    demoFocusRequested = true;
    lastDemoFocusRequestMs = now;
#if HAS_SCREEN
    requestFocus();
    UIFrameEvent event;
    event.action = UIFrameEvent::Action::REGENERATE_FRAMESET;
    notifyObservers(&event);
#endif
}

void EkopartyBadgeModule::beginDemoPlayback()
{
    if (demoStarted) {
        return;
    }

    demoStarted = true;
    demoStartedMs = millis();
    nextToneCue = 0;
    lastDemoVisualFrame = UINT16_MAX;
    LOG_INFO("EkopartyBadge %s demo started", startupDemoActive.load(std::memory_order_relaxed) ? "startup" : "replay");
}

void EkopartyBadgeModule::finishDemo()
{
    const bool wasStartupDemo = startupDemoActive.load(std::memory_order_relaxed);
    demoActive = false;
    startupDemoActive.store(false, std::memory_order_relaxed);
    stopDemoTone();
    lastDemoVisualFrame = UINT16_MAX;
    idleAnimationStartedMs = millis();
    if (wasStartupDemo) {
        setBluetoothEnable(true);
    }
#if HAS_SCREEN
    if (wasStartupDemo && qrModule && qrModule->isVisible()) {
        qrModule->requestFrameFocus();
    } else {
        requestRedraw();
    }
#endif
    LOG_INFO("EkopartyBadge %s demo complete", wasStartupDemo ? "startup" : "replay");
}

void EkopartyBadgeModule::replayDemo()
{
    stopDemoTone();
    demoActive = true;
    demoStarted = false;
    demoFocusRequested = false;
    demoToneActive = false;
    nextToneCue = 0;
    lastDemoVisualFrame = UINT16_MAX;
    demoStartedMs = 0;
    demoFocusFirstRequestMs = 0;
    lastDemoFocusRequestMs = 0;
    startupDemoActive.store(false, std::memory_order_relaxed);
    requestDemoFocus();
    LOG_INFO("EkopartyBadge demo replay requested");
}

void EkopartyBadgeModule::showDemoPixels(uint32_t elapsedMs)
{
    const ekoparty::Timeline timeline = ekoparty::timelineAt(elapsedMs);
    pixels.clear();
    if (ekoparty::getBadgeSettings().backlightMode == ekoparty::BacklightMode::OFF) {
        pixels.show();
        return;
    }
    const uint8_t sample = ekoparty::signalByte(timeline.visualFrame, static_cast<uint8_t>(timeline.scene));

    switch (timeline.scene) {
    case ekoparty::DemoScene::SIGNAL_ACQUIRE: {
        const uint16_t sweep = (timeline.visualFrame / 2U) % NEOPIXEL_COUNT;
        pixels.setPixelColor(sweep, pixels.Color(0, 55, 70));
        if (timeline.localMs >= 400) {
            pixels.setPixelColor((sweep + NEOPIXEL_COUNT - 1U) % NEOPIXEL_COUNT, pixels.Color(45, 0, 35));
        }
        break;
    }
    case ekoparty::DemoScene::LOGO_DECODE:
    case ekoparty::DemoScene::LOGO_REBUILD:
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            const bool cyan = ((i + timeline.visualFrame / 3U) & 0x01U) == 0U;
            pixels.setPixelColor(i, cyan ? pixels.Color(0, 52, 66) : pixels.Color(70, 0, 48));
        }
        break;
    case ekoparty::DemoScene::CHAOS_LOADER: {
        const uint8_t progress = ekoparty::loaderProgress(timeline.localMs);
        const uint16_t filled = (progress * NEOPIXEL_COUNT + 99U) / 100U;
        for (uint16_t i = 0; i < filled && i < NEOPIXEL_COUNT; i++) {
            pixels.setPixelColor(i, pixels.Color(0, 48, 64));
        }
        if (progress >= 97) {
            pixels.setPixelColor(sample % NEOPIXEL_COUNT, pixels.Color(85, 0, 45));
        }
        break;
    }
    case ekoparty::DemoScene::SYSTEM_COLLAPSE:
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            const bool fault = ((sample >> (i & 0x07U)) & 0x01U) != 0U;
            pixels.setPixelColor(i, fault ? pixels.Color(90, 0, 8) : pixels.Color(25, 0, 34));
        }
        break;
    case ekoparty::DemoScene::SKULL:
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            const uint16_t mirror = NEOPIXEL_COUNT - 1U - i;
            const bool pulse = ((timeline.visualFrame / 2U + (i < mirror ? i : mirror)) & 0x01U) == 0U;
            pixels.setPixelColor(i, pulse ? pixels.Color(68, 0, 12) : pixels.Color(22, 0, 55));
        }
        break;
    case ekoparty::DemoScene::WATCHER_EYE: {
        int16_t point = 0;
        if (timeline.localMs >= 480 && timeline.localMs < 1040) {
            point = static_cast<int16_t>(((timeline.localMs - 480U) * (NEOPIXEL_COUNT - 1U)) / 560U);
        } else if (timeline.localMs >= 1040) {
            point = NEOPIXEL_COUNT / 2U;
        }
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            pixels.setPixelColor(i, pixels.Color(10, 0, 28));
        }
        pixels.setPixelColor(point, pixels.Color(0, 75, 75));
        break;
    }
    case ekoparty::DemoScene::DIGITAL_LAUGH: {
        const bool alternate = ((timeline.localMs / 160U) & 0x01U) != 0U;
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            const bool bright = ((i & 0x01U) != 0U) == alternate;
            pixels.setPixelColor(i, bright ? pixels.Color(78, 0, 58) : pixels.Color(18, 0, 45));
        }
        break;
    }
    case ekoparty::DemoScene::MANIFESTO:
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            if (timeline.localMs < 320) {
                pixels.setPixelColor(i, (sample & (1U << (i & 0x07U))) ? pixels.Color(80, 0, 30) : pixels.Color(0, 38, 52));
            } else {
                pixels.setPixelColor(i, (i & 0x01U) ? pixels.Color(52, 0, 44) : pixels.Color(0, 45, 58));
            }
        }
        break;
    case ekoparty::DemoScene::BREAK_LOGIC: {
        const uint8_t wave = static_cast<uint8_t>((timeline.localMs / 80U) % 4U);
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            const uint16_t centerDistance = i < NEOPIXEL_COUNT / 2U ? NEOPIXEL_COUNT / 2U - 1U - i : i - NEOPIXEL_COUNT / 2U;
            if (centerDistance <= wave) {
                pixels.setPixelColor(i, pixels.Color(0, 58, 68));
            }
        }
        break;
    }
    case ekoparty::DemoScene::HANDOFF:
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            pixels.setPixelColor(i, pixels.Color(0, 18, 6));
        }
        break;
    }

    pixels.show();
}

void EkopartyBadgeModule::showIdlePixels()
{
    pixels.clear();
    if (ekoparty::getBadgeSettings().backlightMode == ekoparty::BacklightMode::OFF) {
        pixels.show();
        return;
    }
    const uint32_t elapsedMs = millis() - idleAnimationStartedMs;

    switch (idleAnimation) {
    case ekoparty::IdleAnimation::WATCHER_EYE: {
        const uint8_t intensity = triangleWave(elapsedMs, 4200U, 70, 175);
        const RgbColor baseColor = paletteColor(static_cast<uint8_t>(elapsedMs / 96U));
        const uint32_t color = pixels.Color(scaleChannel(baseColor.red, intensity), scaleChannel(baseColor.green, intensity),
                                            scaleChannel(baseColor.blue, intensity));
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            pixels.setPixelColor(i, color);
        }
        break;
    }
    case ekoparty::IdleAnimation::GLITCH_LOGO:
    case ekoparty::IdleAnimation::MESH_ARG: {
        constexpr uint32_t transitionDurationMs = 1600U;
        const uint16_t arrangement = elapsedMs / transitionDurationMs;
        const uint8_t linearProgress = static_cast<uint8_t>(((elapsedMs % transitionDurationMs) * 255U) / transitionDurationMs);
        const uint8_t transitionProgress = smoothstep8(linearProgress);
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            const uint8_t intensity = triangleWave(elapsedMs + i * 240U, 3600U, 145, 230);
            const RgbColor color =
                blendColor(logoGlitchTargetColor(arrangement, i), logoGlitchTargetColor(arrangement + 1U, i), transitionProgress);
            pixels.setPixelColor(i, pixels.Color(scaleChannel(color.red, intensity), scaleChannel(color.green, intensity),
                                                 scaleChannel(color.blue, intensity)));
        }
        break;
    }
    case ekoparty::IdleAnimation::KERNEL_SKULL: {
        const uint8_t breath = triangleWave(elapsedMs, 5200U, 18, 70);
        const uint32_t alertPhase = elapsedMs % 7600U;
        const bool alertWindow = alertPhase >= 6800U;
        const uint32_t alertLocalMs = alertWindow ? alertPhase - 6800U : 0;
        const bool alertOn = alertWindow && alertLocalMs % 180U < 75U;
        const uint16_t frame = elapsedMs / frameIntervalMs;
        const uint8_t sample = ekoparty::signalByte(frame, 73);
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            if (alertOn) {
                const bool yellow = (sample & (1U << (i & 0x07U))) != 0U;
                pixels.setPixelColor(i, yellow ? pixels.Color(210, 105, 0) : pixels.Color(190, 35, 0));
            } else {
                pixels.setPixelColor(i, pixels.Color(breath + 18U, 0, 2));
            }
        }
        break;
    }
    case ekoparty::IdleAnimation::CHAOS_GRIN: {
        const uint32_t phase = elapsedMs % 4400U;
        const uint16_t frame = elapsedMs / frameIntervalMs;
        const uint8_t sample = ekoparty::signalByte(frame, 41);
        const uint8_t warm = triangleWave(elapsedMs, 3200U, 32, 78);
        const bool glitch = phase >= 3100U && phase < 3720U;
        const uint32_t amber = pixels.Color(warm + 30U, warm / 2U, 2);
        for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) {
            if (glitch) {
                if (((sample + i) & 0x07U) == 0U) {
                    pixels.setPixelColor(i, 0);
                    continue;
                }
                switch ((sample >> ((i & 0x03U) * 2U)) & 0x03U) {
                case 0:
                    pixels.setPixelColor(i, pixels.Color(150, 80, 0));
                    break;
                case 1:
                    pixels.setPixelColor(i, pixels.Color(190, 8, 95));
                    break;
                case 2:
                    pixels.setPixelColor(i, pixels.Color(180, 0, 12));
                    break;
                default:
                    pixels.setPixelColor(i, pixels.Color(105, 0, 165));
                    break;
                }
            } else {
                pixels.setPixelColor(i, amber);
            }
        }
        break;
    }
    case ekoparty::IdleAnimation::COUNT:
        break;
    }

    pixels.show();
}

void EkopartyBadgeModule::applySettings(const ekoparty::BadgeSettings &settings)
{
    idleAnimation = settings.idleAnimation;
    pixels.setBrightness(settings.backlightMode == ekoparty::BacklightMode::DIM ? dimPixelBrightness : brightPixelBrightness);
    idleAnimationStartedMs = millis();
#if HAS_SCREEN
    if (qrModule) {
        qrModule->setVisible(settings.manualVisible);
    }
#endif
}

bool EkopartyBadgeModule::persistSettings(const ekoparty::BadgeSettings &settings)
{
    const bool saved = ekoparty::saveBadgeSettings(settings);
    applySettings(ekoparty::getBadgeSettings());
    requestRedraw();
    return saved;
}

bool EkopartyBadgeModule::systemSoundsEnabled() const
{
#if defined(PIN_BUZZER)
    return config.device.buzzer_mode == meshtastic_Config_DeviceConfig_BuzzerMode_ALL_ENABLED ||
           config.device.buzzer_mode == meshtastic_Config_DeviceConfig_BuzzerMode_SYSTEM_ONLY;
#else
    return false;
#endif
}

void EkopartyBadgeModule::updateDemoTone()
{
#if defined(PIN_BUZZER)
    if (!demoActive || !demoStarted) {
        return;
    }

    const uint32_t elapsedMs = millis() - demoStartedMs;
    while (nextToneCue < introScoreCount && introScore[nextToneCue].atMs <= elapsedMs) {
        const ToneCue &cue = introScore[nextToneCue++];
        if (elapsedMs - cue.atMs > toneCueGraceMs || !systemSoundsEnabled() || rtttl::isPlaying()) {
            continue;
        }

        tone(PIN_BUZZER, cue.frequency, cue.durationMs);
        demoToneActive = true;
        break;
    }
#endif
}

void EkopartyBadgeModule::stopDemoTone()
{
#if defined(PIN_BUZZER)
    if (demoToneActive && !rtttl::isPlaying()) {
        noTone(PIN_BUZZER);
    }
#endif
    demoToneActive = false;
}

#if HAS_SCREEN
void EkopartyBadgeModule::drawFrame(OLEDDisplay *display, OLEDDisplayUiState *screenState, int16_t x, int16_t y)
{
    (void)screenState;

#if !MESHTASTIC_EXCLUDE_INPUTBROKER
    if (badgeMenuActive) {
        display->clear();
        display->setColor(WHITE);
        return;
    }
#endif

    if (demoActive) {
        beginDemoPlayback();
        ekoparty::drawIntro(display, x, y, millis() - demoStartedMs);
    } else {
        ekoparty::drawIdleAnimation(display, x, y, idleAnimation, millis() - idleAnimationStartedMs);
    }
}

#if !MESHTASTIC_EXCLUDE_INPUTBROKER
int EkopartyBadgeModule::handleInputEvent(const InputEvent *event)
{
    if (!event || event->inputEvent != INPUT_BROKER_SELECT || demoActive || !screen || screen->isOverlayBannerShowing() ||
        !screen->isModuleFrameShown(this)) {
        return 0;
    }

    badgeMenuActive = true;
    queueMenu(BadgeMenu::ROOT);
    // Stop this opening SELECT before Screen can also treat it as a menu
    // confirmation.
    return 1;
}

void EkopartyBadgeModule::queueMenu(BadgeMenu menu)
{
    pendingMenu = menu;
    setIntervalFromNow(0);
}

void EkopartyBadgeModule::closeBadgeMenu()
{
    badgeMenuActive = false;
    aboutScrollActive = false;
    requestRedraw();
}

void EkopartyBadgeModule::showPendingMenu()
{
    const BadgeMenu menu = pendingMenu;
    pendingMenu = BadgeMenu::NONE;
    switch (menu) {
    case BadgeMenu::ROOT:
        showBadgeMenu();
        break;
    case BadgeMenu::IDLE_ANIMATION:
        showIdleAnimationMenu();
        break;
    case BadgeMenu::BACKLIGHTS:
        showBacklightsMenu();
        break;
    case BadgeMenu::MANUAL:
        showManualMenu();
        break;
    case BadgeMenu::STATUS_LED:
#if defined(EKOPARTY_BADGE_V1) && defined(NEOPIXEL_STATUS_PAIRING_PIN)
        showStatusLedMenu();
#else
        showBadgeMenu();
#endif
        break;
    case BadgeMenu::ABOUT:
        showAboutMenu();
        break;
    case BadgeMenu::RESET_CONFIRM:
        showResetConfirmation();
        break;
    case BadgeMenu::RESET_DONE:
        closeBadgeMenu();
        screen->showSimpleBanner("Preferencias del\nbadge restauradas", 2500);
        break;
    case BadgeMenu::SAVE_ERROR:
        closeBadgeMenu();
        screen->showSimpleBanner("No se pudo guardar\nla configuracion", 3000);
        break;
    case BadgeMenu::NONE:
        break;
    }
}

void EkopartyBadgeModule::showBadgeMenu()
{
    badgeMenuActive = true;
    aboutScrollActive = false;
#if defined(EKOPARTY_BADGE_V1) && defined(NEOPIXEL_STATUS_PAIRING_PIN)
    static const char *options[] = {"Volver",     "Animacion idle", "Backlights",       "Manual",
                                    "Status LED", "Acerca de",      "Reproducir intro", "Restaurar"};
    enum Options { BACK, IDLE, BACKLIGHTS, MANUAL, STATUS_LED, ABOUT, REPLAY_INTRO, RESET };
#else
    static const char *options[] = {"Volver",    "Animacion idle",   "Backlights", "Manual",
                                    "Acerca de", "Reproducir intro", "Restaurar"};
    enum Options { BACK, IDLE, BACKLIGHTS, MANUAL, ABOUT, REPLAY_INTRO, RESET };
#endif
    graphics::BannerOverlayOptions menu;
    menu.message = "BADGE";
    menu.durationMs = 0;
    menu.optionsArrayPtr = options;
    menu.optionsCount = sizeof(options) / sizeof(options[0]);
    menu.bannerCallback = [this](int selected) {
        switch (selected) {
        case IDLE:
            queueMenu(BadgeMenu::IDLE_ANIMATION);
            break;
        case BACKLIGHTS:
            queueMenu(BadgeMenu::BACKLIGHTS);
            break;
        case MANUAL:
            queueMenu(BadgeMenu::MANUAL);
            break;
#if defined(EKOPARTY_BADGE_V1) && defined(NEOPIXEL_STATUS_PAIRING_PIN)
        case STATUS_LED:
            queueMenu(BadgeMenu::STATUS_LED);
            break;
#endif
        case ABOUT:
            queueMenu(BadgeMenu::ABOUT);
            break;
        case REPLAY_INTRO:
            closeBadgeMenu();
            replayDemo();
            break;
        case RESET:
            queueMenu(BadgeMenu::RESET_CONFIRM);
            break;
        case BACK:
            closeBadgeMenu();
            break;
        default:
            break;
        }
    };
    screen->showOverlayBanner(menu);
}

void EkopartyBadgeModule::showIdleAnimationMenu()
{
    static const char *options[] = {"Volver", "Ojo", "Logo glitch", "Calavera", "Risa", "Mesh ARG"};
    constexpr uint8_t optionCount = sizeof(options) / sizeof(options[0]);
    static_assert(optionCount == static_cast<uint8_t>(ekoparty::IdleAnimation::COUNT) + 1,
                  "Idle animation menu and enum must stay in sync");
    graphics::BannerOverlayOptions menu;
    menu.message = "Animacion idle";
    menu.durationMs = 0;
    menu.optionsArrayPtr = options;
    menu.optionsCount = optionCount;
    menu.InitialSelected = static_cast<int8_t>(idleAnimation) + 1;
    menu.bannerCallback = [this](int selected) {
        if (selected == 0) {
            queueMenu(BadgeMenu::ROOT);
            return;
        }
        if (selected < 0 || selected > static_cast<int>(ekoparty::IdleAnimation::COUNT)) {
            return;
        }

        ekoparty::BadgeSettings settings = ekoparty::getBadgeSettings();
        settings.idleAnimation = static_cast<ekoparty::IdleAnimation>(selected - 1);
        if (persistSettings(settings)) {
            closeBadgeMenu();
        } else {
            queueMenu(BadgeMenu::SAVE_ERROR);
        }
        LOG_INFO("EkopartyBadge idle animation selected: %d", selected - 1);
    };
    screen->showOverlayBanner(menu);
}

void EkopartyBadgeModule::showBacklightsMenu()
{
    static const char *options[] = {"Volver", "Apagados", "Tenues", "Brillantes"};
    constexpr uint8_t optionCount = sizeof(options) / sizeof(options[0]);
    static_assert(optionCount == static_cast<uint8_t>(ekoparty::BacklightMode::COUNT) + 1,
                  "Backlight menu and enum must stay in sync");
    const ekoparty::BadgeSettings &settings = ekoparty::getBadgeSettings();

    graphics::BannerOverlayOptions menu;
    menu.message = "BACKLIGHTS";
    menu.durationMs = 0;
    menu.optionsArrayPtr = options;
    menu.optionsCount = optionCount;
    menu.InitialSelected = static_cast<int8_t>(settings.backlightMode) + 1;
    menu.bannerCallback = [this](int selected) {
        if (selected == 0) {
            queueMenu(BadgeMenu::ROOT);
            return;
        }
        if (selected < 0 || selected > static_cast<int>(ekoparty::BacklightMode::COUNT)) {
            return;
        }

        ekoparty::BadgeSettings updated = ekoparty::getBadgeSettings();
        updated.backlightMode = static_cast<ekoparty::BacklightMode>(selected - 1);
        if (persistSettings(updated)) {
            closeBadgeMenu();
        } else {
            queueMenu(BadgeMenu::SAVE_ERROR);
        }
    };
    screen->showOverlayBanner(menu);
}

void EkopartyBadgeModule::showManualMenu()
{
    static const char *options[] = {"Volver", "Ocultar", "Mostrar"};
    const ekoparty::BadgeSettings &settings = ekoparty::getBadgeSettings();
    graphics::BannerOverlayOptions menu;
    menu.message = "MANUAL";
    menu.durationMs = 0;
    menu.optionsArrayPtr = options;
    menu.optionsCount = sizeof(options) / sizeof(options[0]);
    menu.InitialSelected = settings.manualVisible ? 2 : 1;
    menu.bannerCallback = [this](int selected) {
        if (selected == 0) {
            queueMenu(BadgeMenu::ROOT);
            return;
        }
        if (selected < 1 || selected > 2) {
            return;
        }

        ekoparty::BadgeSettings updated = ekoparty::getBadgeSettings();
        updated.manualVisible = selected == 2;
        if (persistSettings(updated)) {
            closeBadgeMenu();
        } else {
            queueMenu(BadgeMenu::SAVE_ERROR);
        }
    };
    screen->showOverlayBanner(menu);
}

#if defined(EKOPARTY_BADGE_V1) && defined(NEOPIXEL_STATUS_PAIRING_PIN)
void EkopartyBadgeModule::showStatusLedMenu()
{
    static const char *options[] = {"Volver", "Completo", "Solo Bluetooth", "Solo mensajes", "Apagado"};
    const ekoparty::BadgeSettings &settings = ekoparty::getBadgeSettings();
    graphics::BannerOverlayOptions menu;
    menu.message = "STATUS LED";
    menu.durationMs = 0;
    menu.optionsArrayPtr = options;
    menu.optionsCount = sizeof(options) / sizeof(options[0]);
    menu.InitialSelected = static_cast<int8_t>(settings.statusLedMode) + 1;
    menu.bannerCallback = [this](int selected) {
        if (selected == 0) {
            queueMenu(BadgeMenu::ROOT);
            return;
        }
        if (selected < 0 || selected > static_cast<int>(ekoparty::StatusLedMode::COUNT)) {
            return;
        }

        ekoparty::BadgeSettings updated = ekoparty::getBadgeSettings();
        updated.statusLedMode = static_cast<ekoparty::StatusLedMode>(selected - 1);
        if (persistSettings(updated)) {
            closeBadgeMenu();
        } else {
            queueMenu(BadgeMenu::SAVE_ERROR);
        }
    };
    screen->showOverlayBanner(menu);
}
#endif

void EkopartyBadgeModule::showAboutMenu()
{
    const char *fullVersion = optstr(APP_VERSION);
    const char *commit = strrchr(fullVersion, '.');
    commit = commit ? commit + 1 : fullVersion;
    snprintf(aboutFirmwareLine, sizeof(aboutFirmwareLine), "Firmware %s", optstr(APP_VERSION_SHORT));
    snprintf(aboutCommitLine, sizeof(aboutCommitLine), "Commit %s", commit);

    aboutScrollActive = true;
    aboutScrollOffset = 0;
    lastAboutScrollMs = millis();
    showAboutScrollWindow();
}

void EkopartyBadgeModule::showAboutScrollWindow()
{
    static const char *options[] = {"Volver"};
    auto lineAt = [this](uint8_t index) {
        if (index < aboutTextLineCount) {
            return aboutTextLines[index];
        }
        return index == aboutTextLineCount ? static_cast<const char *>(aboutFirmwareLine)
                                           : static_cast<const char *>(aboutCommitLine);
    };

    char message[192];
    snprintf(message, sizeof(message), "ACERCA DE\n%s\n%s\n%s", lineAt(aboutScrollOffset),
             lineAt((aboutScrollOffset + 1U) % aboutTotalLineCount), lineAt((aboutScrollOffset + 2U) % aboutTotalLineCount));

    graphics::BannerOverlayOptions menu;
    menu.message = message;
    menu.durationMs = 0;
    menu.optionsArrayPtr = options;
    menu.optionsCount = sizeof(options) / sizeof(options[0]);
    menu.bannerCallback = [this](int) {
        aboutScrollActive = false;
        queueMenu(BadgeMenu::ROOT);
    };
    screen->showOverlayBanner(menu);
}

void EkopartyBadgeModule::showResetConfirmation()
{
    static const char *options[] = {"Volver", "Restaurar"};
    graphics::BannerOverlayOptions menu;
    menu.message = "Restaurar opciones";
    menu.durationMs = 0;
    menu.optionsArrayPtr = options;
    menu.optionsCount = sizeof(options) / sizeof(options[0]);
    menu.bannerCallback = [this](int selected) {
        if (selected == 0) {
            queueMenu(BadgeMenu::ROOT);
            return;
        }

        const bool saved = ekoparty::resetBadgeSettings();
        applySettings(ekoparty::getBadgeSettings());
        requestRedraw();
        queueMenu(saved ? BadgeMenu::RESET_DONE : BadgeMenu::SAVE_ERROR);
    };
    screen->showOverlayBanner(menu);
}
#endif

void EkopartyBadgeModule::requestRedraw()
{
    UIFrameEvent event;
    event.action = UIFrameEvent::Action::REDRAW_ONLY;
    notifyObservers(&event);
}
#endif

int EkopartyBadgeModule::handleDeepSleep(void *)
{
    stopDemoTone();
    pixels.clear();
    pixels.show();
    return 0;
}

#endif
