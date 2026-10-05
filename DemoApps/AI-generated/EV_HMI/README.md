# EV Charging HMI

EV Charging HMI is an 800 × 480 evaluation interface for the Bridgetek
IDM2040-7A module. It demonstrates idle, authorization, charging, summary,
fault, and service-settings screens with English and Chinese text.

> **Evaluation software only.** The supplied application uses simulated or
> inactive charger data, does not authorize payments, and must not control
> charging equipment. Complete the integration and hardware validation work
> described below before considering product use.

## Supported configuration

| Component | Supported configuration |
|---|---|
| Module | Bridgetek IDM2040-7A |
| EVE device | BT817 |
| Host | RP2040 on the IDM2040-7A |
| Display | Physical 800 × 480 LCD; module WVGA preset with 860-pixel render width and 800-pixel HSF output |
| Touch | Focal capacitive touch, single-touch application behavior |
| EVE flash | W25Q128 containing the supplied BT81x flash image |
| Embedded toolchain | Raspberry Pi Pico SDK 1.5.1, ARM GNU Toolchain 13.2.Rel1, Ninja |
| Emulator toolchain | Windows, Visual Studio 2022 C++ tools, Windows SDK, CMake, PowerShell |

Other modules, displays, touch controllers, flash devices, SDK versions, and
pin mappings are not qualified by this project. Verify their complete timing,
touch, storage, and host-interface configuration before use.

## Package contents

- Application source: `DemoApps/AI-generated/EV_HMI`
- CMake target: `DemoApp_EV_HMI`
- External-flash image: `Test/Flash/BT81X_Flash.bin`
- Design material: [`Design_Doc`](Design_Doc/)
- Integration interface: [`Hdr/EVCharger.h`](Hdr/EVCharger.h)

Run all build commands from the EveApps repository root.

## 1. Windows emulator demonstration

Build into a directory reserved for the emulator demo:

```powershell
./DemoApps/AI-generated/EV_HMI/build.ps1 `
    -Platform Emulator `
    -BuildDirectory ./build-ev-hmi-emulator-demo `
    -Demo
```

Run the deployed executable from its output directory so it can find the
emulator libraries and flash image:

```powershell
Set-Location ./build-ev-hmi-emulator-demo/deploy/DemoApp_EV_HMI
./DemoApp_EV_HMI.exe
```

Mouse input represents touch. Emulator results demonstrate functional
rendering and input flow only; they do not validate physical LCD timing,
touch accuracy, external-flash bandwidth, or scanout performance.

## 2. IDM2040-7A hardware demonstration

Use this build to exercise the simulated charger flow on hardware:

```powershell
./DemoApps/AI-generated/EV_HMI/build.ps1 `
    -Platform RP2040 `
    -BuildDirectory ./build-ev-hmi-rp2040-demo `
    -Demo
```

The demo build defines `EV_CHARGER_DEMO=1`. Tapping the connector starts the
simulated session; it is not evidence of physical connector detection.

### Deploy the demo firmware

1. Program the external-flash image as described in
   [External-flash image](#external-flash-image).
2. Put the IDM2040-7A RP2040 into BOOTSEL mode.
3. Copy
   `build-ev-hmi-rp2040-demo/deploy/DemoApp_EV_HMI/DemoApp_EV_HMI.uf2`
   to the RP2040 mass-storage drive.
4. Restart the module.
5. Verify boot, display, touch, both languages, every page and dialog, and the
   UART/USB diagnostic output.

## 3. Charger integration build

Omit `-Demo` and use a different build directory:

```powershell
./DemoApps/AI-generated/EV_HMI/build.ps1 `
    -Platform RP2040 `
    -BuildDirectory ./build-ev-hmi-rp2040-integration
```

This is an integration starting point, not production-qualified firmware. The
default weak hooks return inactive charger inputs and do not communicate with
a charger, payment service, or backend. Replace them with application-specific
implementations before testing the non-demo flow.

Do not reuse one build directory for demo and integration configurations. The
output file has the same name in both modes and can otherwise be overwritten
or deployed with the wrong configuration.

## External-flash image

Chinese fonts require the firmware-matched BT817 flash image:

```text
File:   DemoApps/AI-generated/EV_HMI/Test/Flash/BT81X_Flash.bin
Size:   3,559,424 bytes
SHA256: 8B52FD35A0E355049DAD6404E43F51D6874084388E49D05DF6984B02833554AF
Offset: 0
```

Use EVE Asset Builder with a programming connection supported by the
IDM2040-7A setup. Erase the target flash as required, program the complete file
from offset 0, and verify the programmed contents. The build copies the same
image to the deployment directory as `__Flash.bin` for emulator use.

The UF2 contains RP2040 firmware only; it does not program EVE external flash.
Always deploy firmware and flash assets from the same project revision. A
missing or mismatched image can prevent Chinese text from rendering correctly.

## Demo controls

- Tap the connector on the idle page to start simulated authorization.
- Authorization completes after approximately five seconds.
- Charging completes approximately 40 seconds after the session starts.
- Tap Stop to open the confirmation dialog, or hold Stop for 1.5 seconds to
  request an immediate stop.
- Hold station ID `A-07` for five seconds to open service settings.
- Enter demonstration PIN `1234`; settings closes after 120 seconds.
- Tap `EN` or `ZH` to change language.

The QR code and all displayed charger readings are demonstration data. The
service PIN is not a security mechanism and must not be retained in a product.

## Integration interface

Override the weak functions declared in `Hdr/EVCharger.h`:

| Function | Purpose |
|---|---|
| `EVCharger_ReadInputs()` | Supplies the current connector, authorization, charging, fault, meter, clock, ambient-light, and touch-acceptance state once per UI loop |
| `EVCharger_RequestStop()` | Receives a user stop request |
| `EVCharger_RequestCancelAuthorization()` | Receives an authorization-cancel request |
| `EVCharger_RequestRetry()` | Receives a retry request from the fault page |
| `EVCharger_Beep()` | Requests audible feedback for the specified duration in milliseconds |
| `EVCharger_GetQrRow()` | Supplies one row of a 41 × 41 QR symbol; bit 0 is the leftmost module |

Important input units and ranges:

| Field | Meaning |
|---|---|
| `power_kw` | Whole kilowatts |
| `soc_percent` | State of charge, 0–100 |
| `energy_deci_kwh` | Energy in tenths of a kWh |
| `cost_cents` | Cost in the smallest currency unit used by the integration |
| `elapsed_seconds` | Session duration in seconds |
| `ambient_lux` | Ambient illumination in lux |
| `hour`, `minute` | Display clock using 24-hour values |
| `touch_allowed` | Set false when the integration rejects touch because of rain, noise, or another external policy |

These hooks run synchronously in the UI loop and should return promptly. The
current interface is a demonstration contract; define production requirements
for stale data, communication loss, value limits, currency, authorization,
fault latching, and stop acknowledgement in the charger integration layer.

## Current code limitations

The following behaviors in the current checkout must be resolved before a
customer product release:

- The connector hit area is enabled in both demo and non-demo builds. Tapping
  it can enter the authorization page without `plug_connected`. Do not treat
  the current UI state as a connector safety interlock.
- The current tag-only debounce logic converts `touch_allowed == false` or a
  drag onto an untagged area into tag zero. If a control was already stable,
  this can be interpreted as a release. Do not use the current touch path for
  safety-critical actions until cancellation and physical-release handling are
  separated and validated on IDM2040-7A hardware.
- The QR payload, charger values, callbacks, and PIN are demonstration
  implementations.
- The application has not been qualified for payment processing or charger
  safety functions.

## Validation status

The following configurations were successfully configured and compiled on
2026-10-05:

| Configuration | Build result | Hardware qualification |
|---|---|---|
| BT817 Windows emulator, WVGA, demo enabled | Passed | Not applicable |
| RP2040/IDM2040-7A, module display preset, demo enabled | Passed | Not performed by this build check |
| RP2040/IDM2040-7A, module display preset, demo disabled | Passed | Not performed by this build check |

The RP2040 target enables UART and USB standard output. At startup, confirm the
reported display registers. During hardware testing, also inspect
`REG_UNDERRUN` diagnostics and record results for every page, language, and
dialog state.

Minimum hardware acceptance checks:

1. Reliable cold boot and reset.
2. Stable full-screen output with correct timing and no clipping.
3. Touch calibration, press, release, drag-away, rejected-touch, and long-press
   behavior.
4. Connector insertion/removal and every page transition using real inputs.
5. English and Chinese rendering from the programmed flash image.
6. Stop, cancel, retry, fault, timeout, and communication-loss behavior.
7. No unexpected `REG_UNDERRUN` increase during representative operation.
8. Confirmation that the deployed UF2 and flash image match the recorded
   release revision and checksum.

## Licensing and redistribution

Review the repository [license](../../../LICENSE.md) before use or
redistribution. The flash image contains Noto Sans CJK-derived font assets.
Confirm and include all required third-party font notices and license files in
the customer package; a standalone font notice is not currently included in
this EV_HMI directory.
