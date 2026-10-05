# AI-generated applications

Source and bundled assets for the applications below. Each application deploys
to `build-*/deploy/<target-name>`.

| Application | Target | Existing configuration |
| --- | --- | --- |
| EV_HMI | `DemoApp_EV_HMI` | BT817 emulator / IDM2040 RP2040, WVGA |
| PatientMonitor | `DemoApp_PatientMonitor` | BT817 emulator / RP2040, WXGA |

Configure from the repository root and use separate build directories for the
different display/module profiles. Existing build directories regenerate their
CMake files on the next build. Do not open stale generated projects under their
old `build-*/SampleApp` locations.

See each application's README for dependencies, assets, commands and
hardware-validation limitations. Design documentation is available in
`EV_HMI/Design_Doc` and `PatientMonitor/Design_Doc`.
