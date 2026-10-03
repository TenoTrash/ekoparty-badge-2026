#ifndef _VARIANT_EKOPARTY_BADGE_V1_
#define _VARIANT_EKOPARTY_BADGE_V1_

#define VARIANT_MCK (64000000ul)
#define USE_LFXO

#include "WVariant.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PINS_COUNT (48)
#define NUM_DIGITAL_PINS (48)
#define NUM_ANALOG_INPUTS (1)
#define NUM_ANALOG_OUTPUTS (0)

#define NRF_APM

#define DFU_LED_PIN (32 + 6)
#define LED_STATE_ON 1
#define LED_STATE_OFF 0

#define BUTTON_PIN (32 + 4)
#define BUTTON_ACTIVE_LOW true
#define BUTTON_ACTIVE_PULLUP true
#define BUTTON_SENSE_TYPE INPUT_PULLUP_SENSE

// Schematic label nRF_RESET: a GPIO button, not the MCU's P0.18/nRESET input.
#define NRF_RESET_BUTTON_PIN (0 + 17)

#define PIN_SERIAL1_RX (-1)
#define PIN_SERIAL1_TX (-1)
#define PIN_SERIAL2_RX (-1)
#define PIN_SERIAL2_TX (-1)

#define WIRE_INTERFACES_COUNT 1
#define PIN_WIRE_SCL (0 + 20)
#define PIN_WIRE_SDA (0 + 22)
#define USE_SSD1306

#define SPI_INTERFACES_COUNT 1
#define PIN_SPI_MOSI (0 + 8)
#define PIN_SPI_MISO (0 + 12)
#define PIN_SPI_SCK (0 + 6)

#define USE_SX1262
#define SX126X_CS (32 + 9)
#define SX126X_DIO1 (0 + 4)
#define SX126X_BUSY (0 + 15)
#define SX126X_RESET (0 + 26)
#define SX126X_DIO2_AS_RF_SWITCH

static const uint8_t SS = SX126X_CS;
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK = PIN_SPI_SCK;
static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

#define BATTERY_PIN (0 + 2)
#define ADC_RESOLUTION 14
#define BATTERY_SENSE_RESOLUTION_BITS 12
#define BATTERY_SENSE_RESOLUTION 4096.0
#undef AREF_VOLTAGE
#define AREF_VOLTAGE 3.0
#define VBAT_AR_INTERNAL AR_INTERNAL_3_0
#define ADC_MULTIPLIER (2.0F)

#define PIN_BUZZER (0 + 24)

#define NEOPIXEL_STATUS_PAIRING_PIN (32 + 2)
#define NEOPIXEL_STATUS_PAIRING_COLOR 0x000040
#define NEOPIXEL_STATUS_TYPE (NEO_GRB + NEO_KHZ800)

#ifdef __cplusplus
}
#endif

#endif
