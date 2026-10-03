#include "EkopartyBadgeSettings.h"

#if defined(EKOPARTY_BADGE) && defined(HAS_NEOPIXEL)

#include "DebugConfiguration.h"
#include "FSCommon.h"
#include "SPILock.h"
#include "SafeFile.h"
#include "concurrency/LockGuard.h"
#include <ErriezCRC32.h>
#include <cstddef>
#include <cstring>

namespace ekoparty
{
namespace
{
constexpr const char *settingsPath = "/prefs/ekoparty.dat";
constexpr uint32_t settingsMagic = 0x454B4F31;
constexpr uint8_t legacyBooleanBacklightVersion = 3;
constexpr uint8_t previousSettingsVersion = 4;
constexpr uint8_t settingsVersion = 5;

struct LegacyPersistedSettings {
    uint32_t magic;
    uint8_t version;
    uint8_t idleAnimation;
    uint8_t backlightMode;
    uint8_t statusLedMode;
    uint32_t crc;
} __attribute__((packed));

struct PersistedSettings {
    uint32_t magic;
    uint8_t version;
    uint8_t idleAnimation;
    uint8_t backlightMode;
    uint8_t statusLedMode;
    uint8_t manualVisible;
    uint32_t crc;
} __attribute__((packed));

BadgeSettings currentSettings;
bool settingsLoaded = false;

bool hasValidModes(uint8_t idleAnimation, uint8_t backlightMode, uint8_t statusLedMode)
{
    return idleAnimation < static_cast<uint8_t>(IdleAnimation::COUNT) &&
           backlightMode < static_cast<uint8_t>(BacklightMode::COUNT) &&
           statusLedMode < static_cast<uint8_t>(StatusLedMode::COUNT);
}

bool isValid(const PersistedSettings &settings)
{
    return settings.magic == settingsMagic && settings.version == settingsVersion && settings.manualVisible <= 1 &&
           hasValidModes(settings.idleAnimation, settings.backlightMode, settings.statusLedMode) &&
           crc32Buffer(&settings, offsetof(PersistedSettings, crc)) == settings.crc;
}

bool isValid(const LegacyPersistedSettings &settings)
{
    const bool validVersion = settings.version == legacyBooleanBacklightVersion || settings.version == previousSettingsVersion;
    const bool validBacklightMode = settings.version == legacyBooleanBacklightVersion
                                        ? settings.backlightMode <= 1
                                        : settings.backlightMode < static_cast<uint8_t>(BacklightMode::COUNT);
    return settings.magic == settingsMagic && validVersion &&
           settings.idleAnimation < static_cast<uint8_t>(IdleAnimation::COUNT) && validBacklightMode &&
           settings.statusLedMode < static_cast<uint8_t>(StatusLedMode::COUNT) &&
           crc32Buffer(&settings, offsetof(LegacyPersistedSettings, crc)) == settings.crc;
}

PersistedSettings encode(const BadgeSettings &settings)
{
    PersistedSettings persisted{};
    persisted.magic = settingsMagic;
    persisted.version = settingsVersion;
    persisted.idleAnimation = static_cast<uint8_t>(settings.idleAnimation);
    persisted.backlightMode = static_cast<uint8_t>(settings.backlightMode);
    persisted.statusLedMode = static_cast<uint8_t>(settings.statusLedMode);
    persisted.manualVisible = settings.manualVisible ? 1 : 0;
    persisted.crc = crc32Buffer(&persisted, offsetof(PersistedSettings, crc));
    return persisted;
}
} // namespace

void loadBadgeSettings()
{
    if (settingsLoaded) {
        return;
    }
    settingsLoaded = true;

#ifdef FSCom
    concurrency::LockGuard guard(spiLock);
    auto file = FSCom.open(settingsPath, FILE_O_READ);
    if (!file) {
        LOG_INFO("EkopartyBadge settings not found, using defaults");
        return;
    }

    uint8_t buffer[sizeof(PersistedSettings)]{};
    const size_t bytesRead = file.read(buffer, sizeof(buffer));
    file.close();

    if (bytesRead == sizeof(PersistedSettings)) {
        PersistedSettings persisted{};
        memcpy(&persisted, buffer, sizeof(persisted));
        if (!isValid(persisted)) {
            LOG_WARN("EkopartyBadge settings invalid, using defaults");
            return;
        }

        currentSettings.idleAnimation = static_cast<IdleAnimation>(persisted.idleAnimation);
        currentSettings.backlightMode = static_cast<BacklightMode>(persisted.backlightMode);
        currentSettings.statusLedMode = static_cast<StatusLedMode>(persisted.statusLedMode);
        currentSettings.manualVisible = persisted.manualVisible != 0;
        LOG_INFO("EkopartyBadge settings loaded");
        return;
    }

    if (bytesRead != sizeof(LegacyPersistedSettings)) {
        LOG_WARN("EkopartyBadge settings invalid, using defaults");
        return;
    }

    LegacyPersistedSettings legacy{};
    memcpy(&legacy, buffer, sizeof(legacy));
    if (!isValid(legacy)) {
        LOG_WARN("EkopartyBadge settings invalid, using defaults");
        return;
    }

    currentSettings.idleAnimation = static_cast<IdleAnimation>(legacy.idleAnimation);
    currentSettings.backlightMode = legacy.version == legacyBooleanBacklightVersion
                                        ? (legacy.backlightMode != 0 ? BacklightMode::BRIGHT : BacklightMode::OFF)
                                        : static_cast<BacklightMode>(legacy.backlightMode);
    currentSettings.statusLedMode = static_cast<StatusLedMode>(legacy.statusLedMode);
    currentSettings.manualVisible = true;
    LOG_INFO("EkopartyBadge legacy settings loaded");
#endif
}

const BadgeSettings &getBadgeSettings()
{
    loadBadgeSettings();
    return currentSettings;
}

bool saveBadgeSettings(const BadgeSettings &settings)
{
    loadBadgeSettings();

#ifdef FSCom
    {
        concurrency::LockGuard guard(spiLock);
        FSCom.mkdir("/prefs");
    }
    const PersistedSettings persisted = encode(settings);
    auto file = SafeFile(settingsPath, true);
    const size_t written = file.write(reinterpret_cast<const uint8_t *>(&persisted), sizeof(persisted));
    if (file.close() && written == sizeof(persisted)) {
        currentSettings = settings;
        LOG_INFO("EkopartyBadge settings saved");
        return true;
    }
    LOG_WARN("EkopartyBadge settings save failed");
#else
    LOG_WARN("EkopartyBadge settings filesystem unavailable");
#endif
    return false;
}

bool resetBadgeSettings()
{
    return saveBadgeSettings(BadgeSettings{});
}
} // namespace ekoparty

#endif
