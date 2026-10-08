# Bedside Patient Monitor UI

A 1280 × 800 patient-monitor interface demonstration for Bridgetek BT817 and
Raspberry Pi RP2040. It includes six simulated waveform lanes, numeric
measurements, a status area, and 14 touch-enabled soft keys.

> **Evaluation software only.** This application is not medical-device software
> and must not be used for patient monitoring or clinical decisions. All values
> and waveforms are simulated.

## Requirements

- EveApps repository, including shared HAL and emulator libraries
- CMake and PowerShell
- Windows emulator: Visual Studio 2022 with C++ tools and a Windows SDK
- RP2040 build: Raspberry Pi Pico SDK 1.5.1, ARM GNU Toolchain 13.2.Rel1,
  and Ninja
- Hardware: an RP2040 board using the Pico board profile, BT817, and a
  compatible 1280 × 800 LCD/touch assembly

- Project directory: `DemoApps/AI-generated/PatientMonitor`
- Build target: `DemoApp_PatientMonitor`
- Design documents: `DemoApps/AI-generated/PatientMonitor/Design_Doc`

The UI rebuilds its display list on touch or numeric changes while waveform
bitmap data changes in RAM_G. See
[Display-list update strategies](../../../docs/best-practices/display-list-update-strategies.md)
when using this application as a starting point.

## Hardware configuration

The supplied configuration uses:

- Display: `EVE_DISPLAY_WXGA`, 1280 × 800
- Interface: single SPI, 25 MHz in the RP2040 HAL
- Touch: repository WXGA configuration for a Goodix controller
- Resources: ROM fonts and RAM_G waveform data; no external graphics assets
- Console: UART and USB CDC

Default RP2040 SPI0 GPIO assignments:

```text
SCK GP2
MOSI GP3
MISO GP4
CS GP5
INT GP6
PD_N GP7
```

Confirm the board, flash device, signal levels, wiring, full LCD timing and
touch controller before deployment. Resolution alone does not establish panel
compatibility. Hardware operation requires validation on the selected assembly.

## Build the Windows emulator

Run these PowerShell commands from the EveApps repository root:

```powershell
cmake -S . -B build-patient-monitor-vs -G "Visual Studio 17 2022" -A x64 -DEVE_APPS_PLATFORM=EVE_PLATFORM_BT8XXEMU -DEVE_APPS_GRAPHICS=EVE_GRAPHICS_BT817 -DEVE_APPS_DISPLAY=EVE_DISPLAY_WXGA
cmake --build build-patient-monitor-vs --config Release --target DemoApp_PatientMonitor
```

Run from the deploy directory:

```powershell
Set-Location build-patient-monitor-vs/deploy/DemoApp_PatientMonitor
./DemoApp_PatientMonitor.exe
```

Keep the executable and supplied DLLs together. The window supports mouse input
as emulated touch. This executable is a BT8XXEMU application, not an FT4222 or
MPSSE hardware-bridge application.

## Build for RP2040

Run from the EveApps repository root. These paths use the standard per-user Pico
SDK installation; adjust them to match your installation:

```powershell
$picoSdk = "$env:USERPROFILE/.pico-sdk/sdk/1.5.1"
$armToolchain = "$env:USERPROFILE/.pico-sdk/toolchain/13_2_Rel1"
$ninjaPath = "$env:USERPROFILE/.pico-sdk/ninja/v1.12.1/ninja.exe"

cmake -S . -B build-patient-monitor-rp2040 -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninjaPath" "-DPICO_SDK_PATH=$picoSdk" "-DPICO_TOOLCHAIN_PATH=$armToolchain" -DPICO_BOARD=pico -DEVE_APPS_PLATFORM=EVE_PLATFORM_RP2040 -DEVE_APPS_GRAPHICS=EVE_GRAPHICS_BT817 -DEVE_APPS_DISPLAY=EVE_DISPLAY_WXGA -DCMAKE_BUILD_TYPE=Release
cmake --build build-patient-monitor-rp2040 --target DemoApp_PatientMonitor
```

Firmware output:

```text
build-patient-monitor-rp2040/deploy/DemoApp_PatientMonitor/DemoApp_PatientMonitor.uf2
```

Use separate build directories for different board or display configurations.

## Deploy and operate

1. Confirm that the hardware matches the selected build configuration.
2. Enter the RP2040 board's BOOTSEL mode.
3. Copy `DemoApp_PatientMonitor.uf2` to the board's boot drive.
4. Allow the board to restart, then check the display and touch input.
5. Open its USB COM port in a serial terminal at 115200 baud, 8-N-1, with no
   flow control.

A serial console window does not open automatically. No external font or bitmap
image needs to be programmed for this application.

Six waveform lanes sweep at 150 pixels per second with a 22-pixel erase gap.
Numerics update once per second. Soft keys act on release; Silence and Alarms
off toggle demonstration states, while other keys report their selection. These
controls do not operate clinical alarms or measurement equipment.

## Emulator checks

From the emulator deploy directory:

```powershell
./DemoApp_PatientMonitor.exe --smoke
./DemoApp_PatientMonitor.exe --self-test
```

The smoke test runs for approximately four seconds. The emulator writes
`PatientMonitor_Frame.bmp` at frame 120. The self-test checks all 14 touch tags
at two interior edges per key and verifies release behavior.

Diagnostics report display-list size, frame progress and underrun counters. The
application enforces a 7,200-byte display-list budget. Emulator results do not
establish physical display bandwidth or touch calibration.

## Troubleshooting

### No display

Confirm the hardware profile, power, SPI wiring, PD_N connection, LCD timing and
backlight configuration. Check serial diagnostics for startup or coprocessor
errors.

### No USB COM port

Confirm successful BOOTSEL programming, a USB data cable, and the board's USB
device connector. Check Windows Device Manager after restart.

## Scope and limitations

- Measurements, waveforms, timestamps, and trend graphics are demonstration data.
- Real acquisition, clinical alarm logic/audio, trend storage, production
  settings, and calibration persistence are not implemented.
- The interface uses ROM fonts. Custom external-flash fonts and pre-rendered
  graphics are not included.
- The host interface uses single SPI, not QSPI.
- Validate boot, display timing, touch accuracy, sustained rendering, console
  output, and power-cycle behavior on the final hardware before integration.
