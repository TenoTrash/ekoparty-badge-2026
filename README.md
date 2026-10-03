# Ekoparty Badge 2026

Firmware del badge oficial del evento Ekoparty 2026, basado en [Meshtastic Firmware](https://github.com/meshtastic/firmware). Esta adaptación es independiente de Meshtastic y está destinada al PCB `ekoparty_badge_v1` con nRF52840, radio SX1262 y pantalla OLED.

El perfil distribuido usa la región LoRa **ANZ** y el USB VID/PID **`CAFE:E001`** por decisión del evento. Se ofrece para este hardware; el identificador USB no se presenta como un VID asignado al proyecto para otros productos.

## Usar el badge

- [Manual de usuario](docs/MANUAL.md): controles, Bluetooth, mensajes, menú local y actualización UF2.
- [Última versión](https://github.com/marsfactory/ekoparty-badge-2026/releases): firmware para el PCB final y sumas SHA-256.
- [Pinmap y hardware](badge-hardware/PIN-MAP.md): señales del PCB y [esquemático](badge-hardware/SCH_Schematic1_2026-09-11.pdf).

El QR de la OLED abre `https://fabricamarciana.com/eko`, que redirige al manual de este repositorio.

## Compilar

Clonar con submódulos y usar PlatformIO:

```bash
git clone --recurse-submodules https://github.com/marsfactory/ekoparty-badge-2026.git
cd ekoparty-badge-2026
scripts/build-ekoparty.sh ekoparty_badge_v1
```

El UF2 queda en `dist/ekoparty_badge_v1/latest-ekoparty_badge_v1.uf2`. La [guía de compilación y carga](docs/BUILD.md) explica las dependencias, el primer flash SWD con [EKOBOOT](https://github.com/marsfactory/eko-bootloader) y la verificación de imágenes.

## Procedencia y licencia

El código conserva la licencia [GPL-3.0](LICENSE) y las atribuciones de Meshtastic. El árbol público comienza con un commit raíz propio; los archivos de Meshtastic se mantienen como base para que la adaptación sea compilable. EKOBOOT deriva de Adafruit nRF52 Bootloader y se publica por separado.
