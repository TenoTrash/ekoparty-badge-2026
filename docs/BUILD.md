# Compilar y cargar el firmware

Este repositorio distribuye únicamente el target `ekoparty_badge_v1`. El PCB usa un nRF52840 discreto, SoftDevice S140 7.3.0 y [EKOBOOT](https://github.com/marsfactory/eko-bootloader). La aplicación se enlaza desde `0x27000`.

## Requisitos

- Git con submódulos, Python 3 y [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html).
- Para el primer flash: programador SWD compatible con nRF52840 y el bootloader EKOBOOT. Una actualización normal posterior sólo requiere USB.
- Para reproducir EKOBOOT: toolchain ARM GCC, entorno Python con `intelhex` y `adafruit-nrfutil`, y sus submódulos.

## Aplicación

```bash
git clone --recurse-submodules https://github.com/marsfactory/ekoparty-badge-2026.git
cd ekoparty-badge-2026
scripts/build-ekoparty.sh ekoparty_badge_v1
```

El script aplica `config/userPrefs.ekoparty.jsonc` durante la compilación y restaura `userPrefs.jsonc` al terminar. El perfil fija `ANZ`, `MEDIUM_FAST` y `CLIENT_MUTE`. Los artefactos y `BUILD_INFO.txt` quedan en `dist/ekoparty_badge_v1/`; usar el UF2 `latest-ekoparty_badge_v1.uf2` para USB. Una release oficial contiene archivos con el commit incluido en el nombre y un `SHA256SUMS`.

## Imagen inicial por SWD

Clonar EKOBOOT como directorio hermano e inicializar sus submódulos:

```bash
cd ..
git clone https://github.com/marsfactory/eko-bootloader.git
cd eko-bootloader
git submodule update --init
python3 -m venv .venv
.venv/bin/pip install intelhex adafruit-nrfutil
cd ../ekoparty-badge-2026
scripts/build-ekoparty-bootloader.sh
```

El script crea `dist/ekoparty_badge_v1/EKO_FIRST_FLASH.hex` y su suma SHA-256. Incluye MBR, S140, EKOBOOT y UICR; **no** incluye la aplicación. Antes de programar por SWD, identificar el nRF52840, respaldar flash y UICR si el chip tiene datos, borrar cuando corresponda y verificar la escritura por lectura. Para comprobar una imagen local:

```bash
scripts/verify-ekoparty-first-flash.py \
  dist/ekoparty_badge_v1/EKO_FIRST_FLASH.hex \
  --softdevice bin/s140_nrf52_7.3.0_softdevice.hex
```

`P0.17` es el botón DFU, aunque el PCB lo rotule `nRF_RESET`; `P0.18/nRESET` no está conectado. Con el botón pulsado al alimentar el badge aparece la unidad USB `EKOBOOT`. Copiar el UF2 de `ekoparty_badge_v1` a esa unidad y esperar el reinicio. El toque serie a 1200 baudios entra en un modo DFU temporal sólo CDC y no expone la unidad UF2.

## Verificar una release

Descargar todos sus archivos en un mismo directorio y ejecutar `sha256sum -c SHA256SUMS` en Linux o `shasum -a 256 -c SHA256SUMS` en macOS. El `BUILD_INFO.txt` indica el commit de origen y debe declarar `git_state=clean`. Después de cargar el UF2, consultar `BADGE → Acerca de` para comprobar la versión.
