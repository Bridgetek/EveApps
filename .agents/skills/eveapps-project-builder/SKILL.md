---
name: eveapps-project-builder
description: Build, extend, combine, port, or troubleshoot Bridgetek EveApps projects from existing SampleApp and DemoApps code. Use when a user wants to turn one or more EveApps samples into a larger application; select a baseline example; add EVE graphics, touch, audio, video, font, bitmap, SD-card, or external-flash features; target a different EVE IC, host platform, LCD, or toolchain; or avoid known integration mistakes. Verify every hardware- and generation-specific claim from the repository, the configured EveApps MCP server, or Bridgetek documentation rather than assuming compatibility.
---

# EveApps Project Builder

Construct a maintainable application from EveApps examples while preserving the repository's HAL, platform, and generation boundaries.

## Start with evidence

1. Locate the EveApps repository root and read its nearest `AGENTS.md` files.
2. Inspect the current branch and working tree. Preserve unrelated user changes.
3. Prefer the configured EveApps MCP server for semantic sample, API, command, and compatibility discovery. If it is unavailable, search the checked-out repository with `rg` and state that limitation.
4. Treat the checked-out source and its build files as the authority for names, paths, targets, and supported combinations. Use current Bridgetek documentation when the repository does not establish a hardware fact.
5. Never infer that a feature supported by one EVE generation is supported by another.

Read the bundled references as needed:

- Before creating a new `SampleApp` or `DemoApps` application, read `docs/development/adding-an-app.md` and follow its directory, naming, build-registration, documentation, and validation requirements.
- Before porting EveApps to an MCU that has no existing host implementation, read `docs/development/porting-other-mcus.md`. Use `common/eve_hal/EVE_HalImpl_RP2040.c` as the embedded reference, but verify the new MCU's SPI pin multiplexing, SDK, timer, and GPIO behavior from its hardware documentation.
- Before porting EveApps to an MCU that has no existing host implementation, read `docs/development/porting-other-mcus.md`. Use `common/eve_hal/EVE_HalImpl_RP2040.c` as the embedded reference, but verify the new MCU's SPI pin multiplexing, SDK, timer, and GPIO behavior from its hardware documentation.
- Read `docs/best-practices/hardware-ui-guidelines.md` when implementing or diagnosing touch controls, localized layouts, external-flash graphics, RAM_G caches, or hardware-only display corruption.
- Read `references/sample-selection.md` before selecting or combining examples.
- Read `references/compatibility-checks.md` whenever the task changes or depends on an EVE IC, host, SPI mode, display, touch controller, storage medium, or toolchain.
- Read `references/known-errors.md` before implementation review and when diagnosing a failure.

## Collect the project profile

Resolve these fields before making hardware-dependent changes:

- Desired application behavior and required features
- EVE IC or module
- Host MCU/MPU or PC bridge
- Toolchain, SDK, and build system
- Host interface and SPI mode, frequency, and signal mapping
- LCD resolution, timing configuration, pixel clock, rotation, and color format
- Touch type, controller, and any custom firmware requirement
- Resource types and storage locations: RAM_G, EVE flash, SD card, host filesystem, or mixed
- Debug channel, programmer, and available hardware
- Output location and acceptance criteria

Ask one focused question for any missing choice that can change architecture or compatibility. Do not block on preferences that can be safely defaulted and reported.

## Select the baseline

1. Inspect candidate applications and their dependencies under `common`.
2. Select one primary baseline. Treat other examples as pattern sources, not folders to merge wholesale.
3. Record a short reuse map containing:
   - Required feature
   - Source example and relevant files or functions
   - Dependencies
   - Required adaptation
   - Compatibility evidence

Do not modify the canonical example unless the user explicitly asks to update it.

## Plan before editing

Describe the proposed structure briefly:

- Initialization ownership
- Screen or application state flow
- Input and touch event flow
- Resource loading and lifetime
- Media FIFO, flash, SD, or host-file ownership when applicable
- Error handling and debug output
- Build target and hardware configuration

Identify conflicts between examples before copying code. In particular, avoid duplicate EVE initialization, competing main loops, duplicate resource addresses, conflicting bitmap handles, repeated touch setup, and multiple owners of the coprocessor FIFO.

## Resource guidance

When the task uses fonts or bitmap resources:

1. Read `docs/best-practices/font-guidelines.md` when it exists.
2. Otherwise, query the EveApps MCP server for the applicable guidance.
3. Confirm that the selected EVE IC supports the proposed resource format.
4. Compare visual quality, resource size, and storage usage.
5. Validate representative resources on the target LCD.

When the task builds an interactive page, also verify the complete tagged hit area, the input layer's press/release/long-press semantics, conditional page states, and the longest localized content. Treat emulator rendering as functional evidence only; use target-hardware counters and observation for scanout bandwidth and touch behavior.

## Implement minimally

1. Reuse the repository's existing HAL and helper APIs.
2. Copy only the feature-specific code and data required by the reuse map.
3. Centralize EVE, display, touch, storage, and resource initialization.
4. Keep platform-specific code behind existing abstractions and compile-time configuration.
5. Preserve ANSI C compatibility unless the selected target explicitly requires another language mode.
6. Keep resource addresses, sizes, alignment, bitmap handles, and storage locations explicit and reviewable.
7. Add comments for non-obvious hardware constraints, not for obvious control flow.
8. Do not silently change SDKs, toolchains, pin assignments, display timing, flash blobs, or third-party dependencies.

When the requested feature is unsupported on the target, stop that implementation path. Report the evidence and offer a supported alternative.

## Validate in layers

Perform every applicable layer:

1. **Static review**: check includes, macros, target definitions, resource addresses, alignment, handles, command availability, and initialization order.
2. **Configure**: run the repository's actual configuration command for the selected platform, EVE IC, and display.
3. **Build**: build the exact application target. Do not claim success from configuration alone.
4. **Simulation**: run the supported emulator or simulator when it represents the selected features.
5. **Hardware**: provide or execute a concise smoke test covering boot, display, touch, resources, and the new feature. Clearly distinguish performed tests from instructions for the user.
6. **Regression**: ensure the reused baseline behavior and shared libraries were not unintentionally changed.

Never claim hardware validation unless it was actually performed on the named hardware.

## Report the result

Summarize:

- Selected baseline and borrowed patterns
- Target EVE, host, display, touch, interface, and storage configuration
- Files changed or created
- Configure/build/test commands and outcomes
- Compatibility evidence and remaining assumptions
- Hardware checks still required
- Known limitations or follow-up work

If blocked, report the exact missing decision, unavailable dependency, unsupported combination, or first actionable error. Do not conceal it with speculative code.
