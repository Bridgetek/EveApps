

# EveApps

EveApps is a collection of applications that use Eve Series chips (FT80X, FT81X, BT815/6, BT817/8) for reference purposes. The collection includes 28 demo applications and 11 sample applications, all written in ANSI C code, and the complete source code is provided.

The demo applications are designed to simulate real-world projects and are moderately complex. The sample applications are intended for instructional purposes and are simpler to understand, focusing on individual display lists or commands.

For those new to the Eve Series chips, it is recommended to start with the SampleApp project.

All the applications rely on a set of common files located under the "common" folder. Each application has a subfolder named "project," which contains the project file necessary for building the application with the toolchain.

Users are expected to be familiar with the programming guide and data sheet for the Eve Series chips.

It should be noted that certain applications are specifically developed for particular Eve Series chips or host platforms (refer to "Support platforms").



## Folder introduction
```
📂 EveApps
    ├───common                  
    │   ├───application      | Application's common functions and fatfs library
    │   ├───eve_flash        | Blob binary for flash programming
    │   └───eve_hal          | Hardware abstraction layer to bridge different platforms
    │                        
    ├───DemoApps
    │    ├───AudioPlayback   | Audio playback demo
    │    ├───CircleView      | Image viewer demo
    │    ├───EvChargePoint   | Electric charging station demo
    │    ├───FlashBitbang    | Reading flash content in low-level mode
    │    ├───Gauges          | Gauges demo
    │    ├───Gradient        | Color gradient demo
    │    ├───Graph           | Graph demo
    │    ├───HDPictureViewer | 4K image viewer demo
    │    ├───Imageviewer     | Image viewer demo
    │    ├───Imageviewer2    | Image viewer demo
    │    ├───Instrument      | Instrument demo
    │    ├───Jackpot         | Jackpot game demo
    │    ├───Keyboard        | Keyboard demo
    │    ├───Lift            | Lift demo
    │    ├───Lift2           | Lift with video background demo
    │    ├───Mainmenu        | Main menu demo
    │    ├───MediaPlayer     | Manage and play media content on hard disk/SD card
    │    ├───Metaballs       | Balls demo
    │    ├───Meter           | Meter demo 
    │    ├───Refrigerator    | Refrigerator demo
    │    ├───RotaryDial      | Rotary dial demo
    │    ├───RunningBar      | Street light moving demo 
    │    ├───Signals         | Wave signals demo
    │    ├───Signature       | Signature demo
    │    ├───Sketch          | Sketch demo
    │    ├───Unicode         | Unicode keyboard demo
    │    ├───UnicodeRuntime  | Unicode demo with runtime determined characters
    │    ├───WashingMachine  | Washing machine UI demo
    │    
    ├───SampleApp
    │    ├───Animation       | Sample usage of animation 
    │    ├───Bitmap          | Sample usage of bitmap 
    │    ├───Flash           | Sample usage of flash 
    │    ├───Font            | Sample usage of font 
    │    ├───Power           | Sample usage of power control
    │    ├───Primitives      | Sample usage of drawing primitives
    |    ├───Sound           | Sample usage of sound functionality    
    │    ├───Touch           | Sample usage of touch functionality
    │    ├───Utility         | Sample usage of helper utilities
    │    ├───Video           | Sample usage of video playback
    │    ├───Widget          | Sample for default widgets
    │            
    ├───Tools/EveApps_Configure  | An .NET GUI tool to help user select hardware quicly
```

Note:
- For Eve registers and commands/instructions definition, user can find it at the file common/eve_hal/EVE_GpuDef.h and common/eve_hal/Eve_CoCmd.h.
- For Eve Emulator, it is for windows platform only and located at common/eve_hal/Bin/Simulation and common/eve_hal/Hdr
- For flash blob file used to access the eve connected flash chip, user can find it at common/eve_flash.  

## Build instructions

### Raspberry Pi Pico

Requires the Pico toolchain https://github.com/ndabas/pico-setup-windows to be installed.

Pico-SDK version 1.3.0 is required


The following steps will build for Raspberry Pi Pico.

 1. Install cmake 3.19.x, python, Visual Studio 2019 community (must select C++), GNU Arm Embedded Toolchain for window.
 2. Launch the *Developer Command Prompt for VS*
```sh
set PICO_SDK_PATH=[path to pico-sdk]
set PICO_TOOLCHAIN_PATH=[path to GNU Arm Embedded Toolchain\\10 2020-q4-major\\bin]
cd EveApps
mkdir build
cd build
cmake -G "NMake Makefiles" -DEVE_APPS_PLATFORM=EVE_PLATFORM_RP2040 -DEVE_APPS_GRAPHICS=[EVE graphics] ..
nmake [Project name]

# [EVE graphics] can be EVE or module name, such as BT817, BT815, VM800B35A_BK ...
# [Project name] is the folder name of a subfolder inside DemoApps or SampleApp
```

Example: 
```
$ cmake.exe -G "NMake Makefiles" -DEVE_APPS_PLATFORM=EVE_PLATFORM_RP2040 -DEVE_APPS_GRAPHICS=EVE_GRAPHICS_BT817 -DEVE_APPS_DISPLAY=EVE_DISPLAY_WXGA ..
$ nmake 
```

Display resolution is set via `EVE_APPS_DISPLAY`, example: cmake -G "NMake Makefiles"  -DEVE_APPS_DISPLAY=EVE_DISPLAY_WXGA
By default, WVGA is set

EVE graphics is set via `EVE_APPS_GRAPHICS`, example: cmake -G "NMake Makefiles"  -DEVE_APPS_GRAPHICS=EVE_GRAPHICS_BT815
By default, MULTI is set

#### Connections

| RP2040 | EVE | UART | SD |
| --- | --- | --- | --- |
| GP0 (TX) | | RX (debug) | |
| GP1 (RX) | | TX (optional) | |
| GP2 (SPI0 SCK) | SCK | | |
| GP3 (SPI0 MOSI) | MOSI | | |
| GP4 (SPI0 MISO) | MISO | | |
| GP5 (GPIO) | CS | | |
| GP6 (GPIO) | INT | | |
| GP7 (GPIO) | PWD | | |
| 5V | 5V | | |
| GND | GND | | |

### Emulator, FT4222 and MPSSE
Way 1: Open EveApps_Emulator.sln or EveApps_MSVC.sln, press F5 to build and run

Way 2: Use Cmake: (Need Cmake 3.19)
```sh
#Launch the Developer Command Prompt for VS
cd X:\source\EveApps
mkdir build
cd build
cmake -G "NMake Makefiles" -DEVE_APPS_PLATFORM=EVE_PLATFORM_FT4222 -DEVE_APPS_GRAPHICS=[EVE graphics] ..
nmake [Project name]

# EVE_APPS_PLATFORM can be BT8XXEMU_PLATFORM, FT4222 of MPSSE
# [EVE graphics] can be EVE or module name, such as BT817, BT815, VM800B35A_BK ...
# [Project name] is the folder name of a subfolder inside DemoApps or SampleApp
```

example: 
```
$ cmake.exe -G "NMake Makefiles" -DEVE_APPS_PLATFORM=EVE_PLATFORM_FT4222 -DEVE_APPS_GRAPHICS=EVE_GRAPHICS_BT817 -DEVE_APPS_DISPLAY=EVE_DISPLAY_WXGA ..
$ nmake 
```

### FT9XX (FT93X and FT90X)
Way 1: Import FT9XX's project inside foler "FT9XX" of any project into FT9XX toolchain. Build and run using GUI.

Way 2: Use Cmake:

```sh
cd X:\source\EveApps
mkdir build
cd build
cmake -G "Eclipse CDT4 - Unix Makefiles" -DEVE_APPS_PLATFORM=[FT9XX module name] -DEVE_APPS_GRAPHICS=[EVE graphics] "-DFT9XX_TOOLCHAIN=C:\Program Files (x86)\Bridgetek\FT9xx Toolchain" ..
cmake --build ./ --target [Project name]

# [EVE graphics] can be EVE or module name, such as BT817, BT815, VM800B35A_BK ...
# [FT9XX module name] can be MM900EV3A, MM930LITE ... etc
# [Project name] is the folder name of a subfolder inside DemoApps or SampleApp
```

example:
```
$ cmake -G "Eclipse CDT4 - Unix Makefiles" -DEVE_APPS_PLATFORM=MM900EV3A -DEVE_APPS_GRAPHICS=EVE_GRAPHICS_BT817 "-DFT9XX_TOOLCHAIN=C:\Program Files (x86)\Bridgetek\FT9xx Toolchain" ..
$ cmake --build ./ 
```

<details>

<summary> Information for application supported host platforms </summary>

## Support host platforms

| Application        | MPSSE<br>PC |  FT4222<br>PC|   MM900|   MM930|  Emulator<br>PC | MM2040EV| 
|--------------------|-------------|--------------| -------| -------|-----------------|---------|
| DemoAudioPlayback  | .           | .            | .      | .      |.                |.        |
| DemoCircleView     | .           | .            | .      | .      |.                |.        |
| DemoEvChargePoint  | .           | .            | .      | x      |.                |.        |
| DemoFlashBitbang   | .           | .            | .      | .      |.                |.        |
| DemoGauges         | .           | .            | .      | .      |.                |.        |
| DemoGradient       | .           | .            | .      | .      |.                |.        |
| DemoGraph          | .           | .            | .      | .      |.                |.        |
| DemoHDPictureViewer| .           | .            | .      | .      |.                |.        |
| DemoImageViewer    | .           | .            | .      | .      |.                |.        |
| DemoImageViewer2   | .           | .            | .      | x      |.                |.        |
| DemoInstrument     | .           | .            | x      | x      |.                |.        |
| DemoJackpot        | .           | .            | .      | .      |.                |.        |
| DemoKeyboard       | .           | .            | .      | .      |.                |.        |
| DemoLift           | .           | .            | .      | x      |.                |.        |
| DemoLift2          | .           | .            | x      | x      |.                |x        |
| DemoMainmenu       | .           | .            | .      | .      |.                |.        |
| DemoMediaPlayer    | .           | .            | x      | x      |.                |x        |
| DemoMetaballs      | .           | .            | .      | .      |.                |.        |
| DemoMeter          | .           | .            | .      | x      |.                |.        |
| DemoRefrigerator   | .           | .            | .      | .      |.                |x        |
| DemoRotaryDial     | .           | .            | .      | .      |.                |.        |
| DemoRunningBar     | .           | .            | .      | .      |.                |.        |
| DemoSignals        | .           | .            | .      | .      |.                |.        |
| DemoSignature      | .           | .            | .      | .      |.                |.        |
| DemoSketch         | .           | .            | .      | .      |.                |.        |
| DemoUnicode        | .           | .            | .      | .      |.                |.        |
| DemoUnicodeRuntime | .           | .            | x      | x      |.                |.        |
| DemoWashingMachine | .           | .            | .      | .      |.                |.        |
| SampleApp          | .           | .            | .      | .      |.                |.        |
x = unsupport
. = support

</details>

<details>

<summary> Information for application supported graphics </summary>

## Support graphics

| Application        |  FT80X | BT81X | FT81X / BT88X|
|--------------------|--------|-------|--------------|
| DemoAudioPlayback  |  x     | .     | x            | 
| DemoCircleView     |  x     | .     | x            |
| DemoEvChargePoint  |  x     | .     | x            |
| DemoFlashBitbang   |  x     | .     | x            |
| DemoGauges         |  .     | .     | .            |
| DemoGradient       |  .     | .     | .            |
| DemoGraph          |  .     | .     | .            |
| DemoHDPictureViewer|  x     | .     | x            | 
| DemoImageViewer    |  .     | .     | .            | 
| DemoImageViewer2   |  x     | .     | x            | 
| DemoInstrument     |  x     | .     | .            | 
| DemoJackpot        |  .     | .     | .            |
| DemoKeyboard       |  .     | .     | .            |
| DemoLift           |  .     | .     | .            | 
| DemoLift2          |  x     | .     | x            |
| DemoMainmenu       |  .     | .     | .            |
| DemoMediaPlayer    |  x     | .     | x            |
| DemoMetaballs      |  .     | .     | .            |
| DemoMeter          |  .     | .     | .            |
| DemoRefrigerator   |  .     | .     | .            |
| DemoRotaryDial     |  .     | .     | .            |
| DemoRunningBar     |  x     | .     | .            |
| DemoSignals        |  .     | .     | .            |
| DemoSignature      |  .     | .     | .            |
| DemoSketch         |  .     | .     | .            |
| DemoUnicode        |  x     | .     | x            |
| DemoUnicodeRuntime |  x     | .     | x            |
| DemoWashingMachine |  .     | .     | .            |
| SampleApp          |  .     | .     | .            | 
x = unsupport
. = support

</details>

<details>

<summary> Information for application supported displays </summary>

## Support display

| Application        | QVGA (320x240)  |WQVGA (480x272)  |WVGA (800x480)   |WSVGA (1024x600)  |WXGA (1280x800)| 
|--------------------|-----------------|-----------------|-----------------|------------------|---------------|
| DemoAudioPlayback  | .               |.                |.                |x                 |x              | 
| DemoCircleView     | .               |.                |.                |.                 |x              | 
| DemoEvChargePoint  | x               |x                |x                |x                 |.              | 
| DemoFlashBitbang   | .               |.                |.                |.                 |.              | 
| DemoGauges         | .               |.                |.                |x                 |x              | 
| DemoGradient       | .               |.                |.                |x                 |x              | 
| DemoGraph          | .               |.                |.                |x                 |x              | 
| DemoHDPictureViewer| .               |.                |.                |x                 |x              | 
| DemoImageViewer    | .               |.                |.                |x                 |x              | 
| DemoImageViewer2   | .               |.                |.                |x                 |x              | 
| DemoInstrument     | .               |.                |.                |x                 |x              | 
| DemoJackpot        | .               |.                |.                |x                 |x              | 
| DemoKeyboard       | .               |.                |.                |.                 |.              | 
| DemoLift           | .               |.                |.                |x                 |x              | 
| DemoLift2          | .               |.                |.                |x                 |x              | 
| DemoMainmenu       | .               |.                |.                |x                 |x              | 
| DemoMediaPlayer    | .               |.                |.                |.                 |.              | 
| DemoMetaballs      | .               |.                |.                |.                 |.              | 
| DemoMeter          | .               |.                |.                |x                 |x              | 
| DemoRefrigerator   | x               |.                |.                |.                 |.              | 
| DemoRotaryDial     | .               |.                |.                |x                 |x              | 
| DemoRunningBar     | .               |.                |.                |x                 |x              | 
| DemoSignals        | .               |.                |.                |x                 |x              | 
| DemoSignature      | .               |.                |.                |x                 |x              | 
| DemoSketch         | .               |.                |.                |x                 |x              | 
| DemoUnicode        | .               |.                |.                |x                 |x              | 
| DemoUnicodeRuntime | .               |.                |.                |x                 |x              | 
| DemoWashingMachine | x               |.                |.                |x                 |x              | 
| SampleApp          | .               |.                |.                |x                 |x              | 
x = unsupport
. = support

</details>

## FatFs library

For SD card access, EveApps using FatFs library version R0.14a from http://elm-chan.org/fsw/ff/00index_e.html.

License: BSD (http://elm-chan.org/fsw/ff/doc/appnote.html#license)

FatFs library is locate at folder common/application/fatfs.

## Documentation

EveApps has created an online documentation (https://Bridgetek.github.io/EveApps/Doxygen/html/) for easy and quick access to the functions it offers.

To customize it according to your setup, kindly refer to Docs/Doxygen folder.

## Version
This version is v1.7.0-rc1
        
## Release log
```
v1.7.0-rc1
  - Updated EveAppsConfig tool
  - Resolve the Flash blob inconsistency issue and update the README with a reminder
  - Address the audio playback freeze problem
  - Fix the underrun issue affecting certain demos, including HDPictureViewer, Lift2, and AudioPlayback
  - Correct the errors in the inflate function, formatted text display, and bulk transfer function in utility sample application
v1.6.0-rc1
  - Add online document support by Doxygen
  - Add DXT1, DXT1L2, DXT1PALETTED and DXT1L2PALETTED support in SampleApp / Bitmap
  - Fix minor issues
v1.5.1-rc1
  - Fix Cmake compile error
  - Fix issue: No sound when start Sound project
v1.5.0-rc1
  - Update FatFS library in common/application/fatfs to R0.14b
  - Add DemoApp.snl and SampleApp.snl to build all the demo projects and sampleApp project in one batch
  - Update4d Cmakelist.txt to build every project in one shot for FT90X/Win32/RP2040 platform 
  - Break the SampleApp into 11 smaller projects
v1.4.0-rc3:
    - Fix minor issue: DemoGauges's UI broken on LCD WXGA
v1.4.0-rc2
    - Fix QSPI connection issue of MM930LITE and MM817EV
    - Code clean up and minor bug fix     
v1.4.0-rc1
    - Add 28 demo application and 1 sample application
    - Add Pico RP2040 platform support
    - Trigger the system reset (at bootup) for some platform which has no Power Down pin toggled (such as GD3X)
```

