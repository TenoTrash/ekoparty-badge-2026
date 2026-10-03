#include "EkopartyStatusLED.h"

#if defined(EKOPARTY_BADGE_V1) && defined(NEOPIXEL_STATUS_PAIRING_PIN)

#include "configuration.h"
#include "mesh/MeshTypes.h"
#include "mesh/Throttle.h"
#include "modules/ekoparty/EkopartyBadgeSettings.h"
#include "modules/TextMessageModule.h"
#include "sleep.h"

EkopartyStatusLED::EkopartyStatusLED() : concurrency::OSThread("EkopartyStatusLED")
{
    bluetoothStatusObserver.observe(&bluetoothStatus->onNewStatus);
    powerStatusObserver.observe(&powerStatus->onNewStatus);
    if (textMessageModule) {
        messageObserver.observe(textMessageModule);
    }
    deepSleepObserver.observe(&notifyDeepSleep);

    pixel.begin();
    pixel.clear();
    pixel.show();
}

int EkopartyStatusLED::handleBluetoothStatus(const meshtastic::Status *)
{
    setIntervalFromNow(0);
    return 0;
}

int EkopartyStatusLED::handlePowerStatus(const meshtastic::Status *)
{
    setIntervalFromNow(0);
    return 0;
}

int EkopartyStatusLED::handleMessage(const meshtastic_MeshPacket *packet)
{
    if (!packet || packet->from == 0) {
        return 0;
    }

    isDirectMessage = !isBroadcast(packet->to);
    messageReceivedMs = millis();
    setIntervalFromNow(0);
    return 0;
}

int EkopartyStatusLED::handleDeepSleep(void *)
{
    show(0);
    return 0;
}

bool EkopartyStatusLED::isCriticalBattery() const
{
    return powerStatus->getHasBattery() && !powerStatus->getHasUSB() && !powerStatus->getIsCharging() &&
           powerStatus->getBatteryChargePercent() <= 5;
}

bool EkopartyStatusLED::isMessageFlashOn(uint32_t elapsedMs) const
{
    constexpr uint32_t flashPeriodMs = 240;
    constexpr uint32_t flashOnMs = 120;
    return elapsedMs % flashPeriodMs < flashOnMs;
}

void EkopartyStatusLED::show(uint32_t color)
{
    pixel.setPixelColor(0, color);
    pixel.show();
}

int32_t EkopartyStatusLED::runOnce()
{
    if (isCriticalBattery()) {
        // Two short red flashes every three seconds are visible without keeping the pixel lit.
        const uint32_t phase = millis() % 3000;
        show((phase < 160 || (phase >= 320 && phase < 480)) ? criticalBatteryColor : 0);
        return 80;
    }

    const ekoparty::StatusLedMode statusMode = ekoparty::getBadgeSettings().statusLedMode;
    const bool messagesEnabled =
        statusMode == ekoparty::StatusLedMode::FULL || statusMode == ekoparty::StatusLedMode::MESSAGES_ONLY;
    const bool bluetoothEnabled =
        statusMode == ekoparty::StatusLedMode::FULL || statusMode == ekoparty::StatusLedMode::BLUETOOTH_ONLY;
    const uint32_t notificationMs = isDirectMessage ? directNotificationMs : broadcastNotificationMs;
    if (messagesEnabled && messageReceivedMs != 0 && Throttle::isWithinTimespanMs(messageReceivedMs, notificationMs)) {
        const uint32_t messageColor = isDirectMessage ? directMessageColor : broadcastMessageColor;
        show(isMessageFlashOn(millis() - messageReceivedMs) ? messageColor : 0);
        return 60;
    }

    if (!bluetoothEnabled) {
        show(0);
        return 1000;
    }

    if (!config.bluetooth.enabled) {
        show(0);
        return 1000;
    }

    switch (bluetoothStatus->getConnectionState()) {
    case meshtastic::BluetoothStatus::ConnectionState::PAIRING:
        show((millis() % 500U) < 250U ? pairingColor : 0);
        return 100;
    case meshtastic::BluetoothStatus::ConnectionState::CONNECTED:
        show(connectedColor);
        return 1000;
    case meshtastic::BluetoothStatus::ConnectionState::DISCONNECTED:
        show((millis() % 2000U) < 250U ? disconnectedColor : 0);
        return 100;
    }

    return 1000;
}

#endif
