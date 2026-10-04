# Índice técnico del Ekoparty Badge 2026

Punto de entrada para encontrar documentación y código del PCB `ekoparty_badge_v1`. El firmware parte de Meshtastic y mantiene una adaptación propia para este hardware. La explicación de cada componente y el procedimiento para portar el badge a otra versión están en [DEV_NOTES.md](DEV_NOTES.md).

## Documentación

| Necesitás… | Ir a… |
| --- | --- |
| Entender y usar el badge | [Manual de uso](MANUAL.md) · [guía web con imágenes](https://marsfactory.github.io/ekoparty-badge-2026/) |
| Preparar el entorno, compilar, cargar por UF2 o hacer la primera carga por SWD | [Compilar y cargar el firmware](BUILD.md) |
| Modificar código o incorporar una versión nueva de Meshtastic | [Notas de desarrollo](DEV_NOTES.md) |
| Consultar pines y señales del PCB v1 | [Pinmap](../badge-hardware/PIN-MAP.md) |
| Consultar el circuito | [Esquemático PDF](../badge-hardware/SCH_Schematic1_2026-09-11.pdf) · [imagen](../badge-hardware/SCH_Schematic1_1-P1_2026-09-11.png) |
| Ver el proyecto y sus instrucciones iniciales | [README](../README.md) |

El [bootloader EKOBOOT](https://github.com/marsfactory/eko-bootloader) se mantiene en otro repositorio. Este árbol contiene la aplicación y los scripts para reproducir y verificar su imagen inicial; [BUILD.md](BUILD.md) explica cuándo corresponde usar cada procedimiento.

## Mapa rápido del código

| Área | Archivos de entrada |
| --- | --- |
| Placa, pines y entorno PlatformIO | [platformio.ini del target](../variants/nrf52840/diy/ekoparty_badge_v1/platformio.ini) · [variant.h](../variants/nrf52840/diy/ekoparty_badge_v1/variant.h) · [variant.cpp](../variants/nrf52840/diy/ekoparty_badge_v1/variant.cpp) · [manifiesto](../boards/ekoparty_badge_v1.json) |
| Perfil inicial Meshtastic | [userPrefs.ekoparty.jsonc](../config/userPrefs.ekoparty.jsonc) |
| Intro, menú, audio, luces y preferencias | [EkopartyBadgeModule.cpp](../src/modules/ekoparty/EkopartyBadgeModule.cpp) · [EkopartyBadgeSettings.cpp](../src/modules/ekoparty/EkopartyBadgeSettings.cpp) |
| Animaciones, QR y pixel de estado | [EkopartyDemoscene.cpp](../src/modules/ekoparty/EkopartyDemoscene.cpp) · [EkopartyQrModule.cpp](../src/modules/ekoparty/EkopartyQrModule.cpp) · [EkopartyStatusLED.cpp](../src/modules/ekoparty/EkopartyStatusLED.cpp) |
| Registro e integración con Meshtastic | [Modules.cpp](../src/modules/Modules.cpp) · [main-nrf52.cpp](../src/platform/nrf52/main-nrf52.cpp) · [NRF52Bluetooth.cpp](../src/platform/nrf52/NRF52Bluetooth.cpp) |
| Compilación y primera imagen | [build-ekoparty.sh](../scripts/build-ekoparty.sh) · [build-ekoparty-bootloader.sh](../scripts/build-ekoparty-bootloader.sh) · [verificador HEX](../scripts/verify-ekoparty-first-flash.py) |

En este repositorio sólo existe el target Ekoparty `ekoparty_badge_v1`. El target XIAO y las guías o scripts de aprovisionamiento por lotes mencionados por versiones anteriores de este índice no forman parte de este árbol. Los artefactos locales se generan en `dist/`, que Git ignora; las versiones para descarga están en [Releases](https://github.com/marsfactory/ekoparty-badge-2026/releases).

## Al cambiar el proyecto

- Si cambia el hardware, mantené sincronizados la variante, el [pinmap](../badge-hardware/PIN-MAP.md) y el esquema correspondiente.
- Si cambia el comportamiento, actualizá el [manual](MANUAL.md) y la [guía web](https://marsfactory.github.io/ekoparty-badge-2026/) según corresponda.
- Si se porta una versión nueva de Meshtastic, registrá la referencia upstream, los cambios de integración y las pruebas en [DEV_NOTES.md](DEV_NOTES.md). La fuente general es el [repositorio oficial Meshtastic Firmware](https://github.com/meshtastic/firmware).
