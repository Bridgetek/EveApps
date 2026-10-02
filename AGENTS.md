# EveApps repository instructions

## Purpose

Use this repository as the authoritative source for EveApps samples, demos, shared code, and supported build targets. Help users learn from existing examples and create maintainable applications without weakening hardware compatibility.

## Repository boundaries

- Treat `SampleApp` as minimal instructional examples and `DemoApps` as larger application patterns.
- Reuse shared functionality from `common` and preserve the existing HAL boundary.
- Inspect each application's build files and `project` directory before changing it.
- Do not modify a canonical sample merely to create a user project; create a new application or use the requested output location unless the task explicitly targets that sample.
- Preserve unrelated changes in the working tree.

## Required target information

Before hardware-dependent implementation, establish the exact EVE IC or module, host platform, toolchain, host interface, LCD configuration, touch controller, resource storage, and debug method. Ask when a missing choice would change compatibility or architecture.

## Compatibility rules

- Never assume a command, register, format, memory size, touch feature, flash feature, or media feature transfers between EVE generations.
- Verify compatibility from the checked-out source, configured EveApps MCP server, or official Bridgetek documentation.
- Use exact chip models in technical decisions. Do not treat “Pre-82X” as a chip model.
- Verify complete LCD timing and touch configuration, not only resolution.
- Confirm that the target physically and logically supports every selected storage source.
- Report unsupported and unknown combinations; do not hide them behind speculative code.

## Implementation rules

- Select one primary sample or demo as the baseline and borrow narrowly scoped patterns from others.
- Keep one owner for EVE initialization, the main application loop, touch polling, coprocessor FIFO access, and each resource lifetime.
- Prevent RAM_G address, media FIFO, bitmap handle, font handle, and external-flash layout conflicts.
- Keep platform-specific transport, GPIO, filesystem, and SDK code behind existing abstractions.
- Preserve ANSI C compatibility unless the active target explicitly requires a different language mode.
- Do not silently change toolchains, SDKs, pin mappings, display timing, flash blobs, or external dependencies.

## Validation

- Use the repository's real configuration and build commands for the selected host, EVE IC, display, and target application.
- Build the exact target after configuration; configuration success is not build success.
- Run supported simulation when useful, but never present simulation as hardware validation.
- For hardware work, state the required boot, display, touch, storage, and feature smoke tests and clearly distinguish tests performed from tests still required.
- Report the first actionable error, effective target configuration, commands run, results, assumptions, and remaining limitations.

## Skill guidance

Use the `eveapps-project-builder` skill for selecting or combining examples, creating a larger application, porting a project, or investigating integration failures.
