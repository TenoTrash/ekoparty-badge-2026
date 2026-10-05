# Notas de desarrollo del Ekoparty Badge 2026

Este documento ubica las partes propias del badge dentro del firmware Meshtastic y describe cómo llevarlas a una versión nueva de la base. Corresponde al PCB `ekoparty_badge_v1`; el pinmap y el procedimiento de primera carga viven en [PIN-MAP.md](../badge-hardware/PIN-MAP.md) y [BUILD.md](BUILD.md).

## Mapa del código

| Si querés cambiar… | Fuente principal | Integración que conviene revisar |
| --- | --- | --- |
| Pines, radio, OLED, batería, botón y LEDs | [variant.h](../variants/nrf52840/diy/ekoparty_badge_v1/variant.h), [variant.cpp](../variants/nrf52840/diy/ekoparty_badge_v1/variant.cpp), [PIN-MAP.md](../badge-hardware/PIN-MAP.md) | [entorno PlatformIO](../variants/nrf52840/diy/ekoparty_badge_v1/platformio.ini) y [manifiesto de placa](../boards/ekoparty_badge_v1.json) |
| Valores iniciales Meshtastic | [userPrefs.ekoparty.jsonc](../config/userPrefs.ekoparty.jsonc) | [build-ekoparty.sh](../scripts/build-ekoparty.sh), [NodeDB.cpp](../src/mesh/NodeDB.cpp) para las burbujas y [CannedMessageModule.cpp](../src/modules/CannedMessageModule.cpp) para las frases predefinidas |
| Intro, navegación del menú `BADGE`, backlights y sonidos | [EkopartyBadgeModule.cpp](../src/modules/ekoparty/EkopartyBadgeModule.cpp) | [registro de módulos](../src/modules/Modules.cpp) y [arranque](../src/main.cpp) |
| Animaciones y gráficos de la OLED | [EkopartyDemoscene.cpp](../src/modules/ekoparty/EkopartyDemoscene.cpp), [EkopartyLogo.h](../src/modules/ekoparty/EkopartyLogo.h) | [Screen.cpp](../src/graphics/Screen.cpp), [Screen.h](../src/graphics/Screen.h), [MeshModule.h](../src/mesh/MeshModule.h) y [UIRenderer.cpp](../src/graphics/draw/UIRenderer.cpp) para el foco y la barra de navegación |
| QR del manual | [EkopartyQrModule.cpp](../src/modules/ekoparty/EkopartyQrModule.cpp), [EkopartyQrCode.h](../src/modules/ekoparty/EkopartyQrCode.h) | La URL, su hash y la matriz QR deben corresponderse |
| Preferencias locales y restauración | [EkopartyBadgeSettings.cpp](../src/modules/ekoparty/EkopartyBadgeSettings.cpp), [EkopartyBadgeSettings.h](../src/modules/ekoparty/EkopartyBadgeSettings.h) | Revisar persistencia y valores iniciales tras actualizar |
| Pixel de estado | [EkopartyStatusLED.cpp](../src/modules/ekoparty/EkopartyStatusLED.cpp) | [StatusLEDModule.cpp](../src/modules/StatusLEDModule.cpp), que evita controlar dos veces el pixel |
| Emparejamiento y presentación del PIN | [main-nrf52.cpp](../src/platform/nrf52/main-nrf52.cpp), [NRF52Bluetooth.cpp](../src/platform/nrf52/NRF52Bluetooth.cpp) | [NotificationRenderer.cpp](../src/graphics/draw/NotificationRenderer.cpp) y la intro del badge |

El entorno se carga desde `platformio.ini` mediante `extra_configs`. Al cambiar un pin, actualizá la variante y el pinmap juntos; para el detalle eléctrico usá el [esquemático](../badge-hardware/SCH_Schematic1_2026-09-11.pdf). La URL del QR está en `EkopartyQrCode.h` como matriz precalculada: cambiar sólo la cadena de texto provoca un error de compilación y no genera un QR nuevo.

## Compilar y verificar esta versión

Desde la raíz del repositorio, con Git, sus submódulos, Python 3 y PlatformIO instalados:

```bash
git submodule update --init --recursive
scripts/build-ekoparty.sh ekoparty_badge_v1
```

Usá el script y el entorno explícito: el entorno predeterminado de `platformio.ini` es otro dispositivo. El script sustituye temporalmente `userPrefs.jsonc` por el perfil Ekoparty, lo restaura al salir y deja el UF2 en `dist/ekoparty_badge_v1/latest-ekoparty_badge_v1.uf2`. También crea `BUILD_INFO.txt` con commit, estado del árbol y SHA-256. `dist/` está ignorado por Git; antes de preparar una release, comprobá que `git_state=clean` y verificá el hash del archivo distribuido. [BUILD.md](BUILD.md) detalla las dependencias y la primera carga.

En un badge de prueba, comprobá la versión desde **BADGE → Acerca de** y ejercitá al menos: arranque e intro, QR, navegación y persistencia del menú, emparejamiento Bluetooth después de la intro, mensajes LoRa de canal y directos, frases predefinidas, pixel de estado, buzzer, batería y reconexión USB. Probá la actualización UF2 y un reinicio adicional antes de distribuirla.

## Portar el badge a una versión nueva de Meshtastic

Actualizar la base de Meshtastic es una tarea de desarrollo. Requiere adaptar y recompilar el target `ekoparty_badge_v1`; copiar al badge un UF2 oficial de otra placa no incorpora sus pines, menú ni intro.

El historial del repositorio de desarrollo identifica como punto de partida el commit Meshtastic `dc67d3b7e848a5ac76034ae3ebb5682056f37353`, padre del primer commit Ekoparty (`164429443`). La comparación con `cff9ea5e5` (badge v1.0.4) permite separar los archivos agregados de los archivos existentes de Meshtastic que se modificaron. Los puntos de integración indicados abajo coinciden con el código de este repositorio público. Este último se publicó como un árbol nuevo y no comparte esa historia para hacer un merge directo con upstream.

**Archivos agregados para el badge.** Tomá como punto de partida [el manifiesto de placa](../boards/ekoparty_badge_v1.json), [la variante completa](../variants/nrf52840/diy/ekoparty_badge_v1/), [los módulos Ekoparty](../src/modules/ekoparty/), [el perfil inicial](../config/userPrefs.ekoparty.jsonc) y [el script de compilación](../scripts/build-ekoparty.sh). Revisá sus APIs y opciones de compilación antes de incorporarlos a la versión nueva.

**Archivos existentes de Meshtastic con cambios para el badge.** Buscá la función equivalente en la versión elegida y adaptá el comportamiento; no reemplaces archivos completos por copias de esta versión.

| Archivo | Cambio que hay que revisar al portar |
| --- | --- |
| [src/configuration.h](../src/configuration.h) | Define `MESHTASTIC_ENABLE_POSITION_MODULE` por separado del hardware GPS para dispositivos sin receptor físico. |
| [src/modules/Modules.cpp](../src/modules/Modules.cpp) | Registra el módulo del badge, el QR, el controlador del pixel de estado y PositionModule cuando está habilitado. Copiar los módulos sin este registro no los activa. |
| [src/modules/PositionModule.cpp](../src/modules/PositionModule.cpp) | Compila PositionModule con `MESHTASTIC_ENABLE_POSITION_MODULE`, aun si el target excluye el GPS físico. |
| [src/mesh/MeshService.cpp](../src/mesh/MeshService.cpp) | Permite responder con la posición conocida cuando PositionModule está habilitado, sin depender de `HAS_GPS`. |
| [src/modules/AdminModule.cpp](../src/modules/AdminModule.cpp) | Envía inmediatamente una posición fija configurada desde la app aunque no exista hardware GPS. |
| [src/mesh/MeshModule.h](../src/mesh/MeshModule.h) | Añade `suppressNavigationBar()`, que usan las pantallas propias. |
| [src/graphics/Screen.h](../src/graphics/Screen.h) | Declara la consulta del módulo visible y el control de la barra de navegación. |
| [src/graphics/Screen.cpp](../src/graphics/Screen.cpp) | Implementa esas consultas y permite despertar la OLED con mensajes aun cuando estén activas las notificaciones externas. |
| [src/graphics/draw/UIRenderer.cpp](../src/graphics/draw/UIRenderer.cpp) | Oculta la barra de navegación en las pantallas propias del badge. |
| [src/graphics/draw/NotificationRenderer.cpp](../src/graphics/draw/NotificationRenderer.cpp) | Limpia el fondo antes de dibujar el PIN de emparejamiento sobre las animaciones. |
| [src/main.cpp](../src/main.cpp) | Omite la melodía genérica de inicio para dejar el audio a cargo de la intro. |
| [src/platform/nrf52/main-nrf52.cpp](../src/platform/nrf52/main-nrf52.cpp) | Pospone la publicidad Bluetooth durante la intro. |
| [src/platform/nrf52/NRF52Bluetooth.cpp](../src/platform/nrf52/NRF52Bluetooth.cpp) | Rechaza un emparejamiento que llegue mientras la intro sigue activa. |
| [src/modules/StatusLEDModule.cpp](../src/modules/StatusLEDModule.cpp) | Evita que el módulo general escriba en el pixel de emparejamiento controlado por `EkopartyStatusLED`. |
| [src/mesh/NodeDB.cpp](../src/mesh/NodeDB.cpp) | Aplica `USERPREFS_CONFIG_DISPLAY_ENABLE_MESSAGE_BUBBLES` al instalar la configuración inicial. |
| [src/modules/CannedMessageModule.cpp](../src/modules/CannedMessageModule.cpp) | Usa `USERPREFS_CANNED_MESSAGES` para las frases predefinidas iniciales. |

La cabecera `StatusLEDModule.h` ya tenía soporte para el pixel en la base identificada; no contiene un cambio Ekoparty que haya que reaplicar. La comparación histórica también muestra una corrección general en `FSCommon.cpp`, ajena a las funciones del badge: evaluá por separado si la versión nueva la necesita. Desde el checkout de desarrollo, `git diff dc67d3b7e848a5ac76034ae3ebb5682056f37353 ekoparty-badge-v1.0.4 -- src/` permite auditar los cambios originales; el diff completo también incluye archivos propios agregados.

Para preparar una copia separada de upstream desde la raíz de este repositorio, reemplazá `TAG_O_COMMIT` por la referencia elegida:

```bash
git clone --recurse-submodules https://github.com/meshtastic/firmware.git ../meshtastic-upstream
git -C ../meshtastic-upstream fetch --tags
git -C ../meshtastic-upstream switch --detach TAG_O_COMMIT
git -C ../meshtastic-upstream submodule update --init --recursive
```

1. **Elegí una referencia fija de upstream.** Usá un tag o commit concreto del [repositorio oficial](https://github.com/meshtastic/firmware), leé sus cambios y seguí su [guía de compilación](https://meshtastic.org/docs/development/firmware/build/). Registrá la referencia y los commits de los submódulos. Compará sus APIs y archivos con la base histórica y los puntos de integración de esta sección.
2. **Portá el soporte de hardware.** Llevá el manifiesto de `boards/`, la variante completa y su entorno PlatformIO al árbol nuevo. Compará las opciones heredadas de `nrf52840_base`, las dependencias, el linker script, el tamaño de flash, la definición `PRIVATE_HW`, los pines y el USB `CAFE:E001` con [PIN-MAP.md](../badge-hardware/PIN-MAP.md) y el esquemático.
3. **Portá el comportamiento Ekoparty.** Llevá `src/modules/ekoparty/` y revisá los doce archivos compartidos de la tabla. Verificá el registro de módulos, el foco y la navegación de pantalla, el audio de arranque, Bluetooth, notificaciones y el control exclusivo del pixel. Conservá el QR y su URL coherentes.
4. **Revisá los valores y dependencias.** Adaptá `config/userPrefs.ekoparty.jsonc` a las claves admitidas por la nueva base y comprobá que `NodeDB.cpp` y `CannedMessageModule.cpp` sigan aplicando las preferencias que este perfil añade. Verificá región `ANZ`, preset `MEDIUM_FAST`, rol `CLIENT_MUTE`, saltos, mensajes predefinidos, GPS físico ausente, PositionModule activo, MQTT y tono. Actualizá los submódulos según la referencia de upstream y revisá las APIs de protobuf antes de regenerar o copiar código generado. Ajustá `build-ekoparty.sh` si cambian los nombres o rutas de salida.
5. **Comprobá la compatibilidad de arranque antes del primer flash.** El UF2 nuevo debe respetar la disposición de memoria y el contrato de EKOBOOT: S140 7.3.0 y aplicación desde `0x27000` en esta revisión. Si upstream modifica SoftDevice, linker o formato de imagen, resolvé primero la compatibilidad con el [bootloader separado](https://github.com/marsfactory/eko-bootloader) y [BUILD.md](BUILD.md). Una actualización normal por UF2 no sustituye el bootloader ni el SoftDevice.
6. **Compilá y probá en una sola unidad.** Ejecutá `scripts/build-ekoparty.sh ekoparty_badge_v1`, revisá `BUILD_INFO.txt`, respaldá la configuración del badge de prueba y conservá un UF2 conocido para volver atrás. Para entrar en EKOBOOT, apagá el badge, conectá USB-C con `nRF_RESET` presionado y encendelo sin soltar el botón hasta que aparezca la unidad. Copiá allí **el UF2 de este target** y realizá las pruebas de la sección anterior; verificá también la conservación de preferencias, canales y vínculo Bluetooth.
7. **Integrá y registrá el resultado.** Incorporá el árbol portado y probado en una rama de este repositorio, conservando los documentos y scripts propios. Anotá la referencia upstream, commit del portado, SHA de submódulos, versión de EKOBOOT, hash del UF2, pruebas realizadas y cambios de configuración o migración. Actualizá [MANUAL.md](MANUAL.md), [BUILD.md](BUILD.md) y este mapa cuando cambie el comportamiento. Publicá una release sólo con un artefacto verificado.

Para distinguir la actualización de **la aplicación Meshtastic del badge** de una **primera carga por SWD** o una actualización del bootloader, consultá [BUILD.md](BUILD.md). El botón rotulado `nRF_RESET` es el GPIO `P0.17` usado por EKOBOOT; no está conectado al `P0.18/nRESET` del microcontrolador.
