# Adding a New EveApps Application

## 1. Choose the application type

- Use `SampleApp` for a small instructional example focused on one feature.
- Use `DemoApps` for a complete or multi-feature application.
- Do not duplicate an existing application without explaining the difference.

## 2. Select a baseline

- Select an existing application supporting the closest EVE IC and host platform.
- Reuse the existing EveApps HAL.
- Do not copy multiple complete applications together.
- Keep one EVE initialization path and one main application loop.

## 3. Create the directory

Example:

DemoApps/
└── MyApplication/
    ├── MyApplication.c
    ├── MyApplication.h
    ├── CMakeLists.txt
    ├── project/
    └── assets/

Document the required naming rules here.

## 4. Register the build target

Explain:

- Which `CMakeLists.txt` files must be updated
- Target naming convention
- Required source files
- Required include directories
- Compile definitions
- Platform-specific project files

## 5. Configure supported hardware

Document:

- Supported EVE ICs
- Supported host platforms
- Supported displays
- Touch-controller requirements
- SPI or QSPI requirements
- External Flash or SD-card requirements

## 6. Add resources

Document:

- Bitmap/font/audio/video conversion
- RAM_G allocation
- External Flash address allocation
- Alignment requirements
- Bitmap and font handle allocation
- Generated resource files
- Resource licensing requirements

For interactive hardware pages, also follow `docs/best-practices/hardware-ui-guidelines.md` and choose a host redraw rule using `docs/best-practices/display-list-update-strategies.md`. Document the complete RAM_G interval map, including metadata, temporary buffers, cached display lists, copied glyphs, media FIFO space, font caches, and library-reserved scratch areas.

## 7. Update documentation

Update:

- Application list
- Supported host-platform table
- Supported EVE table
- Supported display table
- Build instructions when necessary

## 8. Validate

- For a new RP2040 application, establish the intended hardware configuration first. Keep emulator test hooks and platform-specific initialization behind the existing application/HAL boundary.
- When the emulator represents the application behavior, configure and build its exact application target. Run a bounded smoke test for startup and sustained frame progress, then deterministic self-tests for applicable touch, state, and resource behavior. Fix failures before adding RP2040-specific implementation.
- Record emulator configuration, build and test commands, exit status, and any generated frame or diagnostics. PatientMonitor provides an example in `DemoApps/AI-generated/PatientMonitor/README.md` (`--smoke` and `--self-test`). Document features the emulator cannot represent and define their hardware tests.
- Configure and build the exact intended hardware target after the emulator gate passes, then test on that hardware.
- Verify display, touch and resources
- Exercise every page, modal, conditional layout, and supported language
- On hardware, check touch calibration and any available underrun or scanout diagnostics
- Confirm existing applications still build

## 9. Submission checklist

- [ ] Application name follows repository naming rules
- [ ] No duplicated EVE initialization
- [ ] No RAM_G address conflict
- [ ] No bitmap/font handle conflict
- [ ] Every control's complete hit area has the intended nonzero touch tag
- [ ] Font subsets include runtime text, whitespace, units, and fallback characters
- [ ] Emulator build, smoke test, and applicable self-tests passed before RP2040-specific work, or the emulator gap is documented
- [ ] Emulator results and hardware touch/scanout results are distinguished
- [ ] Hardware dependencies are documented
- [ ] Application is registered in CMake
- [ ] Support tables are updated
- [ ] Build result is recorded
- [ ] Hardware validation status is stated
