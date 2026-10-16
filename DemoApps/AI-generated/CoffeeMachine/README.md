# Coffee Machine HMI Demo

This project demonstrates a bilingual coffee-machine touch interface built with
Bridgetek BT817 and Raspberry Pi RP2040. The 1280 x 800 interface includes
drink selection, recipe customization, brewing progress, language selection,
machine status, and standby screens.

The supplied demo supports English and Simplified Chinese and presents eight
drinks: Espresso, Lungo, Americano, Cappuccino, Latte, Flat White, Hot Water,
and Milk Foam.

> **Important:** This is an evaluation HMI, not a production appliance
> controller. The supplied controller simulates brewing for demonstration. A
> production system must implement and validate all heater, pump, grinder,
> cleaning, fault, safety-interlock, emergency-stop, and heartbeat behavior in
> a separate machine controller.

## Supplied configuration

The supplied firmware is configured for the following target. Other hardware
requires a reviewed and validated port; changing only the screen resolution is
not sufficient.

| Item | Supplied configuration |
| --- | --- |
| EVE device | Bridgetek BT817 (`EVE_GRAPHICS_BT817`) |
| Host | Raspberry Pi RP2040, Pico board profile |
| Host interface | Single SPI at 25 MHz |
| Display | 1280 x 800 `EVE_DISPLAY_WXGA` preset, 57 Hz |
| Touch | WXGA Goodix configuration, `REG_TOUCH_CONFIG=0x05D0` |
| EVE external flash | At least 8 MiB |
| Languages | English and Simplified Chinese |
| Debug output | UART and USB CDC |

The repository RP2040 profile uses these default SPI0 signals:

| Signal | RP2040 GPIO |
| --- | ---: |
| SCK | GP2 |
| MOSI | GP3 |
| MISO | GP4 |
| CS | GP5 |
| Interrupt | GP6 |
| Power-down | GP7 |
| IO2 | GP14 |
| IO3 | GP15 |

Confirm the final module wiring, voltage levels, flash device, display bridge,
complete LCD timing, touch-controller firmware, backlight control, and power
sequence against the actual hardware before use.

## Files required for installation

Two separately programmed files are required:

| File | Destination | Purpose |
| --- | --- | --- |
| `DemoApp_CoffeeMachine.uf2` | RP2040 program flash | Application firmware |
| `BT81X_Flash.bin` or packaged `__Flash.bin` | BT817-attached external flash, offset 0 | Fonts and ASTC 8x8 drink images |

The UF2 does **not** contain or program the BT817 external-flash assets. Copying
`BT81X_Flash.bin` to the RP2040 BOOTSEL drive will not program EVE flash.

The project flash image is `Test/Flash/BT81X_Flash.bin`:

- Size: 7,868,416 bytes
- SHA-256: `ED2CD0AFC604540EC641E8D3000A0D2ABAC06A405D1F7DD0BF3BA292F1C1CFD9`

## Installation

### 1. Program the EVE external flash

1. Connect a programmer that supports the BT817-attached flash device.
2. Program `Test/Flash/BT81X_Flash.bin` at external-flash offset 0.
3. Read back or otherwise verify the programmed data.
4. Confirm that the flash device can enter EVE FULL mode.

The application validates the font descriptors and CRC-checks all eight drink
images during startup. An incomplete or mismatched image is rejected.

### 2. Program the RP2040

1. Hold the RP2040 BOOTSEL button while connecting USB.
2. Copy `DemoApp_CoffeeMachine.uf2` to the mounted BOOTSEL drive.
3. Allow the board to reboot.

The local build places the customer artifacts under:

```text
build-coffee-machine-rp2040/deploy/DemoApp_CoffeeMachine/
```

### 3. Check first boot

On a successful boot, the display opens the Home screen and the serial log
contains messages similar to:

```text
CoffeeMachine: EVE initialization complete
CoffeeMachine: loaded ASTC8x8 flash assets into RAM_G
```

If the asset package is missing or incompatible, the display attempts to show
`FLASH ASSET MISMATCH - CHECK SERIAL LOG`. The RP2040 continues running so USB
serial remains available for diagnosis.

## Using the demonstration

1. On the Home screen, select a drink.
2. For coffee drinks, adjust the available volume, strength, temperature, and
   milk settings, then select **Start**.
3. Hot Water and Milk Foam use a shorter confirmation screen with their default
   volume.
4. The Brewing screen shows progress, remaining time, and the active step.
5. Select **Stop** to request cancellation.
6. Use the language button to switch between English and Simplified Chinese.
7. Select **Standby**, or leave the Home screen idle for 60 seconds. Touch the
   standby screen to wake the interface.

Touch buttons activate on release. A press that moves away from its original
control is cancelled, which helps prevent accidental actions.

With `COFFEE_MACHINE_DEMO=ON`, the supplied controller simulates a 30-second
brew cycle. Clean and Standby requests are reported through the debug log. They
do not operate real appliance hardware.

## Production integration

The integration hooks are declared in `Hdr/CoffeeMachineController.h`:

- `CoffeeController_ReadStatus`
- `CoffeeController_RequestBrew`
- `CoffeeController_RequestStop`
- `CoffeeController_RequestClean`
- `CoffeeController_RequestStandby`
- `CoffeeController_LoadLanguage`
- `CoffeeController_SaveLanguage`

Replace the demonstration hooks with the product's validated UART, CAN, or
other controller protocol. The machine controller must own the authoritative
state, acknowledge start/stop requests, report faults, and enforce every safety
condition. When the controller heartbeat is unavailable, the HMI disables the
Start action and displays an alert.

The Windows emulator stores the selected language in
`CoffeeMachineLanguage.cfg`. The default RP2040 hook does not write its program
flash, so the language returns to English after reset. Override the language
load/save hooks if persistent selection is required.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| `FLASH ASSET MISMATCH` | Program the current `BT81X_Flash.bin` at offset 0; verify its SHA-256 and EVE FULL mode. |
| Blank or unstable display | Verify BT817 power-up, PD_N, the complete WXGA timing, bridge configuration, backlight, and wiring. |
| Incorrect or missing images | Verify the external-flash image, flash-device compatibility, and SPI signal integrity. |
| Touch position is wrong | Verify the Goodix controller/firmware, display rotation, calibration, and coordinate mapping. |
| Start is disabled | Check the controller heartbeat and serial fault output. |
| Language resets after power cycle | This is expected with the default RP2040 storage hook; add validated nonvolatile storage. |
| UF2 runs but assets are missing | Program the EVE external flash separately; BOOTSEL programs only the RP2040. |

The serial log reports the active page, frame count, `REG_UNDERRUN` changes,
and selected font cache. Repeated underrun increments on physical hardware
indicate that display timing, flash compatibility, signal integrity, or scanout
bandwidth needs investigation. Emulator results cannot validate those physical
conditions.

## Customer acceptance check

After programming both artifacts, verify the following on the intended
hardware:

1. Cold boot, backlight, color, orientation, and a stable 1280 x 800 image.
2. Touch alignment and every button, including drag-away cancellation.
3. All eight drinks in English and Simplified Chinese.
4. Customization, quick confirmation, Start, Stop, completion, and return Home.
5. Manual standby, 60-second automatic standby, and touch-to-wake.
6. Correct water, bean, grounds, temperature, alert, and controller-heartbeat
   status from the real controller.
7. No continuing `REG_UNDERRUN` increase during a ten-minute test of the most
   demanding page.
8. Verified production versions and checksums for both the UF2 and external
   flash image.

## Build from source

### RP2040 release build

Requirements:

- Pico SDK 1.5.1
- ARM GNU toolchain 13.2.Rel1
- Ninja 1.12.1

From the EveApps repository root:

```powershell
$picoSdk = "$env:USERPROFILE/.pico-sdk/sdk/1.5.1"
$armToolchain = "$env:USERPROFILE/.pico-sdk/toolchain/13_2_Rel1"
$ninjaPath = "$env:USERPROFILE/.pico-sdk/ninja/v1.12.1/ninja.exe"
cmake -S . -B build-coffee-machine-rp2040 -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninjaPath" "-DPICO_SDK_PATH=$picoSdk" "-DPICO_TOOLCHAIN_PATH=$armToolchain" -DPICO_BOARD=pico -DEVE_APPS_PLATFORM=EVE_PLATFORM_RP2040 -DEVE_APPS_GRAPHICS=EVE_GRAPHICS_BT817 -DEVE_APPS_DISPLAY=EVE_DISPLAY_WXGA -DCOFFEE_MACHINE_DEMO=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-coffee-machine-rp2040 --target DemoApp_CoffeeMachine
```

The deployment directory contains the UF2 and a copy of the matching external
flash image.

### Windows emulator

```powershell
cmake -S . -B build-coffee-machine-vs -G "Visual Studio 17 2022" -A x64 -DEVE_APPS_PLATFORM=EVE_PLATFORM_BT8XXEMU -DEVE_APPS_GRAPHICS=EVE_GRAPHICS_BT817 -DEVE_APPS_DISPLAY=EVE_DISPLAY_WXGA -DCOFFEE_MACHINE_DEMO=ON
cmake --build build-coffee-machine-vs --config Release --target DemoApp_CoffeeMachine
Set-Location build-coffee-machine-vs/deploy/DemoApp_CoffeeMachine
./DemoApp_CoffeeMachine.exe --self-test
./DemoApp_CoffeeMachine.exe --smoke
./DemoApp_CoffeeMachine.exe --smoke-zh
```

The emulator is suitable for functional UI checks. It does not validate the
physical touch controller, LCD timing, EVE external-flash bandwidth, electrical
interfaces, or appliance safety behavior.

## Implementation summary

`CoffeeMachine.c` owns the page state machine, touch polling, display-list
construction, and coprocessor FIFO access. Display lists are rebuilt only when
UI state, touch feedback, or controller status changes. At startup, the eight
600 x 456 ASTC 8x8 drink images are CRC-verified and copied from external flash
into RAM_G; glyph data remains in external flash and uses a 448 KiB font cache.

The RAM_G allocation is:

| Resource | Half-open interval |
| --- | --- |
| Five XFONT descriptors | `[0, 20480)` |
| BT817 font cache | `[20480, 479232)` |
| Eight drink bitmap slots | `[479232, 1026560)` |
| Free RAM_G | `[1026560, 1048576)` |

Generated asset metadata and previews are under `Assets`. Flash offsets are
recorded in `Test/Flash/BT81X_Flash.map` and
`Hdr/CoffeeMachineAssets.h`.
