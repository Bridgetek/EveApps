# Porting EveApps to another MCU

Use this guide when the target MCU does not already have an EveApps host implementation. Keep the port behind the existing EVE HAL boundary; application and sample code must not call the MCU SDK directly.

Use `common/eve_hal/EVE_HalImpl_RP2040.c` and `common/eve_hal/EVE_Platform_RP2040.h` as the primary embedded examples. Refer to `common/eve_hal/EVE_HalImpl.h` for the required interface. Do not assume RP2040 pin choices, SDK calls, timing behavior, or single-channel SPI restrictions apply to another MCU.

## 1. Establish the hardware profile

Record these items before writing code:

- Exact MCU, board, SDK, toolchain, and build system
- Exact EVE IC or module and its supported SPI modes
- SPI peripheral instance and maximum safe clock rate
- SCK, MOSI, MISO, CS_N, and PD_N pins; add INT_N and IO2/IO3 only when used
- MCU alternate-function or pin-mux selection for every SPI signal
- GPIO voltage levels, drive strength, pull configuration, and board-level level shifting
- Timer source, resolution, wrap behavior, and sleep facility
- Debug output and programmer

Check the MCU datasheet and the board schematic, not just GPIO numbers. Confirm that SCK, MOSI, and MISO belong to the same SPI instance and valid alternate-function set. CS_N and PD_N may be ordinary GPIO outputs. Confirm that the EVE module exposes every signal required by the selected single, dual, or quad mode.

Start with conservative single-channel SPI. Increase the clock or enable dual/quad transfer only after the MCU, EVE generation, board routing, HAL, and read dummy-byte handling have all been verified.

## 2. Add the platform integration

Use an uppercase platform token such as `XXX_PLATFORM` and an external target selection such as `EVE_PLATFORM_XXX`, following the existing naming pattern.

1. Add `common/eve_hal/EVE_Platform_XXX.h` for MCU SDK includes, portable C includes, pin defaults, and platform-specific definitions.
2. Include that header from `common/eve_hal/EVE_Platform.h` under `XXX_PLATFORM`.
3. Map `EVE_PLATFORM_XXX` to `XXX_PLATFORM`, `EMBEDDED_PLATFORM`, and `EVE_HOST_EMBEDDED` in `common/eve_hal/EVE_Config.h`. Extend its platform availability and exactly-one-platform checks.
4. Add only the fields the new port needs to `EVE_HalParameters` and `EVE_HalContext` in `common/eve_hal/EVE_HalDefs.h`. Keep SDK handle types out of generic code when a `void *` or a small platform-guarded field is sufficient.
5. Register the source, SDK libraries, include directories, linker inputs, and target definitions in the selected application's real build files and `project` directory.
6. Review filesystem, debug, display-GPIO, and resource-loading conditionals that enumerate supported embedded platforms. Do not enable FatFS, external flash, dual/quad SPI, or a display-specific helper merely because RP2040 enables it.

Do not disguise a new MCU as `RP2040_PLATFORM`. A distinct platform token makes capabilities and build failures reviewable.

## 3. Generate the HAL implementation

Create `common/eve_hal/EVE_HalImpl_XXX.c`. Include `EVE_HalImpl.h` and `EVE_Platform.h`, and guard the implementation with `#if defined(XXX_PLATFORM)`.

Implement the complete interface used by the selected EveApps configuration. The RP2040 file provides a practical function inventory:

- Platform lifecycle and discovery: `EVE_HalImpl_initialize`, `EVE_HalImpl_release`, `EVE_Hal_list`, `EVE_Hal_info`, `EVE_Hal_isDevice`, and `EVE_HalImpl_defaults`
- Context lifecycle: `EVE_HalImpl_open`, `EVE_HalImpl_close`, and `EVE_HalImpl_idle`
- SPI framing and transfer: `EVE_Hal_startTransfer`, `EVE_Hal_endTransfer`, `EVE_Hal_flush`, `EVE_Hal_transfer8`, `EVE_Hal_transfer16`, `EVE_Hal_transfer32`, `EVE_Hal_transferMem`, `EVE_Hal_transferProgMem`, and `EVE_Hal_transferString`
- Host commands and bus mode: `EVE_Hal_hostCommand`, `EVE_Hal_hostCommandExt3`, `EVE_Hal_setSPI`, `EVE_Hal_restoreSPI`, and `EVE_Hal_currentFrequency` when required by the selected configuration
- Reset: `EVE_Hal_powerCycle`
- MCU and time services: `EVE_Mcu_initialize`, `EVE_Mcu_release`, `EVE_Millis_initialize`, `EVE_Millis_release`, `EVE_millis`, `EVE_millis64`, and `EVE_sleep`
- Board-specific display GPIO hook: `EVE_UtilImpl_bootupDisplayGpio`

Compare this list with `EVE_HalImpl.h` and the actual link errors for the selected configuration. Some transfer helpers are declared elsewhere in the HAL and are still supplied by each implementation.

### MCU and SPI initialization

Initialize clocks and the MCU SDK in `EVE_Mcu_initialize` only when EveApps owns that initialization. Preserve the `EVE_Hal_NoInit` behavior when the application or bootloader may initialize the MCU first.

In `EVE_HalImpl_defaults`, populate the SPI instance, verified pin set, CS_N, PD_N, and conservative clock defaults. In `EVE_HalImpl_open`:

1. Copy the selected parameters into the HAL context.
2. Configure the SPI peripheral and pin mux.
3. Configure CS_N as inactive high and PD_N as an output.
4. Select the EVE-required SPI mode and bit order.
5. Set `SpiChannels` and `SpiDummyBytes` to the mode actually implemented.
6. Update `phost->Status`, `g_HalPlatform.OpenedDevices`, and any per-instance ownership counters only after initialization succeeds.

Make `EVE_HalImpl_close` undo owned resources without deinitializing an SPI peripheral still used by another EVE context or application component.

### EVE SPI transactions

Keep CS_N asserted for the whole address-plus-payload transaction:

- Read: transmit the three-byte address with the write bit clear, transmit the configured dummy byte or bytes, then receive the payload.
- Write: transmit the three-byte address with the write bit set in the first byte, then transmit the payload.
- Host command: assert CS_N, transmit the three command bytes in the order required by EVE, and deassert CS_N.

Match the existing HAL's little-endian return and write behavior for 16- and 32-bit values. Handle zero-length buffers. Preserve `EVE_STATUS_OPENED`, `EVE_STATUS_READING`, and `EVE_STATUS_WRITING` transitions so generic HAL assertions remain meaningful. Use blocking transfers first; add DMA only after buffer lifetime, cache coherency, completion, and CS_N ownership are defined.

### Power cycle

Implement `EVE_Hal_powerCycle` with the board's PD_N GPIO and the delays required by the selected EVE hardware. Restore the host SPI controller to single-channel mode before boot communication. If a software core-reset command is retained as a fallback, verify that it is valid for the exact EVE IC and safe while PD_N is low; do not copy this detail without checking.

### Millisecond time and sleep

Back `EVE_millis` and `EVE_millis64` with a monotonic hardware or SDK time source. Preserve elapsed time across the underlying counter's wrap, and ensure the 32-bit function intentionally returns the low 32 bits. Initialize all extension state in `EVE_Millis_initialize`. Implement `EVE_sleep(ms)` with a delay that is valid after MCU initialization and does not truncate long waits unexpectedly.

The RP2040 implementation accumulates microseconds into a 64-bit millisecond total; another MCU may use a tick counter or RTOS clock instead. Document its resolution and concurrency assumptions. Protect shared timer state if calls can occur from multiple tasks or interrupt contexts.

## 4. Validate the port in layers

1. Build the exact application, MCU, EVE IC, display, and toolchain target with warnings enabled.
2. Check CS_N and PD_N idle levels before opening the HAL.
3. Use a logic analyzer to verify SPI mode, frequency, three-byte address framing, dummy cycles, byte order, and CS_N boundaries.
4. Power-cycle EVE and read a stable identification register at a conservative SPI rate.
5. Write and read back a safe RAM_G test region.
6. Run a minimal display list and verify a stable image.
7. Test repeated transfers, large buffers, timer wrap handling where practical, and open/close or reset cycles.
8. Test touch and resource storage separately after the base display path works.
9. Raise the SPI rate or enable wider channels one change at a time.

Report the effective pin map, SPI instance/mode/rate, EVE IC, display, commands run, first actionable failure, and which checks were performed on physical hardware. A successful build or simulator run is not hardware validation.
