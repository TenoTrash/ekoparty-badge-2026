#pragma once

#include "configuration.h"

#if defined(EKOPARTY_BADGE) && defined(HAS_NEOPIXEL)

#include "Observer.h"
#include "concurrency/OSThread.h"
#include "mesh/MeshModule.h"
#include "modules/ekoparty/EkopartyBadgeSettings.h"
#include "modules/ekoparty/EkopartyDemoscene.h"
#include "sleep.h"
#include <Adafruit_NeoPixel.h>
#include <stdint.h>

#if HAS_SCREEN && !MESHTASTIC_EXCLUDE_INPUTBROKER
#include "input/InputBroker.h"
#endif

namespace ekoparty
{
bool isStartupDemoActive();
}

#if HAS_SCREEN
class EkopartyQrModule;
#endif

class EkopartyBadgeModule : public MeshModule, public Observable<const UIFrameEvent *>, private concurrency::OSThread
{
  public:
    EkopartyBadgeModule();
#if HAS_SCREEN
    void setQrModule(EkopartyQrModule *module) { qrModule = module; }
#endif

  protected:
    virtual int32_t runOnce() override;
    virtual bool wantPacket(const meshtastic_MeshPacket *) override { return false; }
#if HAS_SCREEN
    virtual void drawFrame(OLEDDisplay *display, OLEDDisplayUiState *screenState, int16_t x, int16_t y) override;
    virtual bool wantUIFrame() override { return true; }
    virtual Observable<const UIFrameEvent *> *getUIFrameObservable() override { return this; }
    virtual bool suppressNavigationBar() override { return true; }
    virtual bool interceptingKeyboardInput() override { return demoActive && demoStarted; }
#endif

  private:
    static constexpr uint8_t dimPixelBrightness = 64;
    static constexpr uint8_t brightPixelBrightness = 255;
    static constexpr uint32_t frameIntervalMs = 80;
    static constexpr uint32_t demoSchedulerIntervalMs = 20;
    static constexpr uint32_t demoFocusRetryMs = 500;
    static constexpr uint32_t demoFocusTimeoutMs = 12000;
    static constexpr uint32_t aboutScrollIntervalMs = 1100;

    Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_DATA, NEOPIXEL_TYPE);
    bool demoActive = true;
    bool demoStarted = false;
    bool demoFocusRequested = false;
    bool demoToneActive = false;
    uint16_t nextToneCue = 0;
    uint16_t lastDemoVisualFrame = UINT16_MAX;
    ekoparty::IdleAnimation idleAnimation = ekoparty::IdleAnimation::WATCHER_EYE;
    uint32_t demoStartedMs = 0;
    uint32_t idleAnimationStartedMs = 0;
    uint32_t demoFocusFirstRequestMs = 0;
    uint32_t lastDemoFocusRequestMs = 0;
#if HAS_SCREEN
    EkopartyQrModule *qrModule = nullptr;
#endif

    CallbackObserver<EkopartyBadgeModule, void *> notifyDeepSleepObserver =
        CallbackObserver<EkopartyBadgeModule, void *>(this, &EkopartyBadgeModule::handleDeepSleep);

#if HAS_SCREEN && !MESHTASTIC_EXCLUDE_INPUTBROKER
    CallbackObserver<EkopartyBadgeModule, const InputEvent *> inputObserver =
        CallbackObserver<EkopartyBadgeModule, const InputEvent *>(this, &EkopartyBadgeModule::handleInputEvent);
#endif

    void requestDemoFocus();
    void beginDemoPlayback();
    void finishDemo();
    void replayDemo();
    void showDemoPixels(uint32_t elapsedMs);
    void showIdlePixels();
    void applySettings(const ekoparty::BadgeSettings &settings);
    bool persistSettings(const ekoparty::BadgeSettings &settings);
    bool systemSoundsEnabled() const;
    void updateDemoTone();
    void stopDemoTone();
#if HAS_SCREEN
#if !MESHTASTIC_EXCLUDE_INPUTBROKER
    enum class BadgeMenu : uint8_t {
        NONE,
        ROOT,
        IDLE_ANIMATION,
        BACKLIGHTS,
        MANUAL,
        STATUS_LED,
        ABOUT,
        RESET_CONFIRM,
        RESET_DONE,
        SAVE_ERROR,
    };

    BadgeMenu pendingMenu = BadgeMenu::NONE;
    bool badgeMenuActive = false;
    bool aboutScrollActive = false;
    uint8_t aboutScrollOffset = 0;
    uint32_t lastAboutScrollMs = 0;
    char aboutFirmwareLine[32] = {};
    char aboutCommitLine[24] = {};

    int handleInputEvent(const InputEvent *event);
    void queueMenu(BadgeMenu menu);
    void closeBadgeMenu();
    void showPendingMenu();
    void showBadgeMenu();
    void showIdleAnimationMenu();
    void showBacklightsMenu();
    void showManualMenu();
#if defined(EKOPARTY_BADGE_V1) && defined(NEOPIXEL_STATUS_PAIRING_PIN)
    void showStatusLedMenu();
#endif
    void showAboutMenu();
    void showAboutScrollWindow();
    void showResetConfirmation();
#endif
    void requestRedraw();
#endif
    int handleDeepSleep(void *unused);
};

#endif
