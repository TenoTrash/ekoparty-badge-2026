# Pinmap del Ekoparty Badge v1

Este pinmap corresponde al PCB `ekoparty_badge_v1` con nRF52840 discreto. El [esquemático](SCH_Schematic1_2026-09-11.pdf) contiene el detalle eléctrico.

## LoRa / SX1262

| Señal | Pin nRF52840 |
| --- | --- |
| `SX_CS / CS` | **P1.09** |
| `MOSI` | **P0.08** |
| `MISO` | **P0.12** |
| `SCK` | **P0.06** |
| `RESET` | **P0.26** |
| `DIO1` | **P0.04** |
| `BUSY` | **P0.15** |

## Pantalla OLED / I2C

| Señal | Pin nRF52840 |
| --- | --- |
| `SCL` | **P0.20** |
| `SDA` | **P0.22** |

## LEDs / NeoPixel

| Señal | Pin nRF52840 |
| --- | --- |
| `DATA_PIN` | **P0.13** |
| `STATUS_PIN` | **P1.02** |
| `DFU_LED` | **P1.06** |

## Controles y periféricos

| Señal | Pin nRF52840 |
| --- | --- |
| `PIEZO` | **P0.24** |
| `BUTTON` | **P1.04** |
| `ADC` | **P0.02** |
| `nRF_RESET` | **P0.17** |

## Programación / debug

| Señal | Pin |
| --- | --- |
| `SWDIO` | **SWDIO** |
| `SWDCLK` | **SWDCLK** |

## Componentes asociados

- Seis píxeles SK6812mini-012 encadenados en `DATA_PIX`.
- Un SK6812mini-012 independiente en `STATUS_PIX`, usado para batería, Bluetooth y mensajes.
- Un piezo pasivo.
- Un LED convencional para DFU.
- Una OLED HS96L03W2C03.

## Aclaración sobre reset

La señal rotulada `nRF_RESET` está conectada a `P0.17` y corresponde al botón DFU de EKOBOOT. No está conectada al pin dedicado `P0.18/nRESET` del nRF52840, que queda sin conexión en este PCB. Mantener el botón pulsado al conectar alimentación abre la unidad UF2 `EKOBOOT`.
