#pragma once

#include "configuration.h"

#if defined(EKOPARTY_BADGE) && defined(HAS_NEOPIXEL)

#include "modules/ekoparty/EkopartyDemoscene.h"
#include <stdint.h>

namespace ekoparty
{
enum class StatusLedMode : uint8_t {
    FULL,
    BLUETOOTH_ONLY,
    MESSAGES_ONLY,
    OFF,
    COUNT,
};

enum class BacklightMode : uint8_t {
    OFF,
    DIM,
    BRIGHT,
    COUNT,
};

struct BadgeSettings {
    IdleAnimation idleAnimation = IdleAnimation::WATCHER_EYE;
    BacklightMode backlightMode = BacklightMode::BRIGHT;
    StatusLedMode statusLedMode = StatusLedMode::FULL;
    bool manualVisible = true;
};

void loadBadgeSettings();
const BadgeSettings &getBadgeSettings();
bool saveBadgeSettings(const BadgeSettings &settings);
bool resetBadgeSettings();
} // namespace ekoparty

#endif
