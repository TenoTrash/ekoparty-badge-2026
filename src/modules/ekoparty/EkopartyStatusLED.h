#pragma once

#include "configuration.h"

#if defined(EKOPARTY_BADGE_V1) && defined(NEOPIXEL_STATUS_PAIRING_PIN)

#include "BluetoothStatus.h"
#include "Observer.h"
#include "PowerStatus.h"
#include "concurrency/OSThread.h"
#include <Adafruit_NeoPixel.h>

class EkopartyStatusLED : private concurrency::OSThread
{
  public:
    EkopartyStatusLED();

  protected:
    int32_t runOnce() override;

  private:
    static constexpr uint32_t pairingColor = NEOPIXEL_STATUS_PAIRING_COLOR;
    static constexpr uint32_t disconnectedColor = 0x000004;
    static constexpr uint32_t connectedColor = 0x000400;
    static constexpr uint32_t broadcastMessageColor = 0x402000;
    static constexpr uint32_t directMessageColor = 0x004000;
    static constexpr uint32_t criticalBatteryColor = 0x400000;
    static constexpr uint32_t broadcastNotificationMs = 480;
    static constexpr uint32_t directNotificationMs = 960;

    Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, NEOPIXEL_STATUS_PAIRING_PIN, NEOPIXEL_STATUS_TYPE);
    uint32_t messageReceivedMs = 0;
    bool isDirectMessage = false;

    CallbackObserver<EkopartyStatusLED, const meshtastic::Status *> bluetoothStatusObserver =
        CallbackObserver<EkopartyStatusLED, const meshtastic::Status *>(this, &EkopartyStatusLED::handleBluetoothStatus);
    CallbackObserver<EkopartyStatusLED, const meshtastic::Status *> powerStatusObserver =
        CallbackObserver<EkopartyStatusLED, const meshtastic::Status *>(this, &EkopartyStatusLED::handlePowerStatus);
    CallbackObserver<EkopartyStatusLED, const meshtastic_MeshPacket *> messageObserver =
        CallbackObserver<EkopartyStatusLED, const meshtastic_MeshPacket *>(this, &EkopartyStatusLED::handleMessage);
    CallbackObserver<EkopartyStatusLED, void *> deepSleepObserver =
        CallbackObserver<EkopartyStatusLED, void *>(this, &EkopartyStatusLED::handleDeepSleep);

    int handleBluetoothStatus(const meshtastic::Status *status);
    int handlePowerStatus(const meshtastic::Status *status);
    int handleMessage(const meshtastic_MeshPacket *packet);
    int handleDeepSleep(void *unused);
    bool isCriticalBattery() const;
    bool isMessageFlashOn(uint32_t elapsedMs) const;
    void show(uint32_t color);
};

#endif
