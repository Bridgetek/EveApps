# EVE Hardware UI Guidelines

Use these checks for interactive EVE pages, especially when touch tags, localized fonts, external-flash assets, or scanout bandwidth affect behavior. Verify every generation-specific command, register, cache, format, and memory limit against the exact EVE IC.

## Touch behavior

- Draw the complete visual hit area of every interactive control while a nonzero tag is active. Disable `TAG_MASK` immediately after that control or group.
- Match page actions to the events emitted by the input layer. Review press, debounce, hold or long-press, release, and page/dialog lock timing as one flow.
- Validate calibration and raw coordinates/tags before debugging the page state machine.
- Verify touch-controller type, rotation and coordinate transformation on the physical panel. Emulator mouse input is not hardware touch validation.
- Keep simulated navigation behind an explicit build option and separate it from production device or backend events.

## Scanout and external-flash bandwidth

- Budget flash-backed glyph and bitmap demand spatially, including demand on the same horizontal scanlines, rather than considering only bytes per frame.
- On ICs that provide an underrun diagnostic such as `REG_UNDERRUN`, record counter deltas with the active page, modal state, and elapsed interval. Confirm register availability for the exact IC.
- A fault-free coprocessor command stream or emulator frame does not prove that hardware scanout bandwidth is sufficient.
- Prefer RAM_G for assets that dominate affected screens when capacity permits. Use generation-specific caches only after verifying their number, size, commands, and lifetime.
- Reduce competing flash reads by lowering asset size, separating dense rows, consolidating onto a cached font, or moving frequently scanned glyphs and bitmaps into RAM_G.
- Do not draw expensive page content behind a fully opaque modal.

## Resource and display-list integrity

- Maintain one global map of half-open RAM_G intervals. Include font metadata, glyph copies, dynamic bitmaps, cached display lists, media FIFO buffers, decompression or snapshot scratch space, font caches, alignment padding, and HAL/library reservations.
- Verify every interval mathematically and ensure its final address is within the exact device's RAM_G capacity.
- Track bitmap and font handles separately from memory addresses and prevent lifetime collisions.
- When relocating extended-font glyph data, preserve the descriptor's base-address semantics and validate every relative or sparse glyph pointer used by the supported character range.
- Rebind required bitmap, font, palette, scissor, blend, and color state in each independently generated display list; do not rely on state from a previous frame.
- Keep firmware resource definitions, converted assets, flash layout/map, and the programmed flash image versioned as one compatible set.

## Layout, localization, and data

- Align icons and labels by their visual centers while accounting for bitmap top-left coordinates and centered-text anchors.
- Define repeated cards or columns from shared centers or widths. Test the longest translation, largest plausible formatted number, units, signs, and the rightmost field.
- Exercise every conditional layout, including connected/disconnected states, empty/error values, dialogs, and language variants.
- Trace each displayed value to its producer, unit, valid range, and update interval. Keep demonstration data explicit and preserve production semantics.

## Validation

1. Statically review commands, handles, resource intervals, font coverage, tagged hit areas, and state transitions.
2. Build the exact emulator and hardware targets with their effective compiler definitions and matching assets.
3. Render every page, modal, conditional state, and supported language. Capture representative frames when layout or resources change.
4. Inspect mixed-script font samples at every generated size, including spaces, digits with units, currency, punctuation, and fallback characters.
5. On hardware, verify boot, calibration, touch event timing, page transitions, layout, localization, storage, and scanout diagnostics.
6. Report emulator and hardware evidence separately, including any unperformed hardware checks.
