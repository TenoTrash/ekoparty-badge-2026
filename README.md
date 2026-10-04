# Ekoparty Badge 2026

Firmware del badge oficial del evento Ekoparty 2026, basado en [Meshtastic Firmware](https://github.com/meshtastic/firmware). Esta adaptación es independiente de Meshtastic y está destinada al PCB `ekoparty_badge_v1` con nRF52840, radio SX1262 y pantalla OLED.

![Ilustración del Ekoparty Badge](./docs/web/assets/PORTADA.png)

> Consultá la [guía web](https://marsfactory.github.io/ekoparty-badge-2026/) para conocer rápidamente las funciones del badge.

## Documentación

| Necesitás… | Ir a… |
| --- | --- |
| Entender y usar el badge | [Manual de uso](./docs/MANUAL.md) · [Guía web con imágenes](https://marsfactory.github.io/ekoparty-badge-2026/) |
| Preparar el entorno, compilar, cargar por UF2 o hacer la primera carga por SWD | [Compilar y cargar el firmware](./docs/BUILD.md) |
| Modificar código o incorporar una versión nueva de Meshtastic | [Notas de desarrollo](./docs/DEV_NOTES.md) |
| Consultar pines y señales del PCB v1 | [Pinmap](./badge-hardware/PIN-MAP.md) |
| Consultar el circuito | [Esquemático PDF](./badge-hardware/SCH_Schematic1_2026-09-11.pdf) · [imagen](./badge-hardware/SCH_Schematic1_1-P1_2026-09-11.png) |

## Compilar

Clonar con submódulos y usar PlatformIO:

```bash
git clone --recurse-submodules https://github.com/marsfactory/ekoparty-badge-2026.git
cd ekoparty-badge-2026
scripts/build-ekoparty.sh ekoparty_badge_v1
```

El UF2 queda en `dist/ekoparty_badge_v1/latest-ekoparty_badge_v1.uf2`. La [guía de compilación y carga](docs/BUILD.md) explica las dependencias, el primer flash SWD con [EKOBOOT](https://github.com/marsfactory/eko-bootloader) y la verificación de imágenes.

Para ubicar los módulos propios y portar el badge a una versión nueva de Meshtastic, consultá las [notas de desarrollo](docs/DEV_NOTES.md).

## Procedencia y licencia

El código conserva la licencia [GPL-3.0](LICENSE) y las atribuciones de Meshtastic. El árbol público comienza con un commit raíz propio; los archivos de Meshtastic se mantienen como base para que la adaptación sea compilable. EKOBOOT deriva de Adafruit nRF52 Bootloader y se publica por separado.


>El perfil distribuido usa la región LoRa **ANZ** y el USB VID/PID **`CAFE:E001`**. Se ofrece para este hardware; el identificador USB no se presenta como un VID asignado al proyecto para otros productos.
