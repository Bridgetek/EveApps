/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2024  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef BT8XXEMU_H
#define BT8XXEMU_H

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26812) // Unscoped enum
#endif

#include "bt8xxemu_inttypes.h"

#ifndef EVE_TCHAR_DEFINED
#define EVE_TCHAR_DEFINED
#ifdef _WIN32
typedef wchar_t eve_tchar_t;
#else
typedef char eve_tchar_t;
#endif
#endif

#ifdef _WIN32
#define EVE_TCHAR_UTF16
#endif

/**
* API version is increased whenever BT8XXEMU_EmulatorParameters format changes or functions are modified.
* Current release DLL can also be dropped into existing applications targeting older API versions 11, 12 and 16 using the older headers.
*/
#define BT8XXEMU_VERSION_API 19

#ifdef BT8XXEMU_REMOTE
#ifndef _WIN32
#undef BT8XXEMU_REMOTE /* Not yet supported */
#else
#define BT8XXEMU_STATIC
#endif
#endif

#ifndef BT8XXEMU_STATIC
#ifdef BT8XXEMU_EXPORT_DYNAMIC
#ifdef _WIN32
#define BT8XXEMU_API __declspec(dllexport)
#else
#define BT8XXEMU_API __attribute__((visibility("default")))
#endif
#else
#ifdef _WIN32
#define BT8XXEMU_API __declspec(dllimport)
#else
#define BT8XXEMU_API
#endif
#endif
#else
#define BT8XXEMU_API
#endif

#ifdef __cplusplus

namespace BT8XXEMU {
class Emulator;
class Flash;
class SleepWake;
}

typedef BT8XXEMU::Emulator BT8XXEMU_Emulator;
typedef BT8XXEMU::Flash BT8XXEMU_Flash;
typedef BT8XXEMU::SleepWake BT8XXEMU_SleepWake;

#elif defined(BT8XXEMU_REMOTE)

typedef struct BT8XXEMUC_Remote BT8XXEMU_Emulator;
typedef struct BT8XXEMUC_Remote BT8XXEMU_Flash;
typedef struct BT8XXEMUC_Remote BT8XXEMU_SleepWake;

#else

typedef void BT8XXEMU_Emulator;
typedef void BT8XXEMU_Flash;
typedef void BT8XXEMU_SleepWake;

#endif /* #ifdef __cplusplus */

typedef enum
{
	BT8XXEMU_LogError = 0,
	BT8XXEMU_LogWarning = 1,
	BT8XXEMU_LogMessage = 2,
} BT8XXEMU_LogType;

typedef enum
{
	BT8XXEMU_EmulatorFT800 = 0x0800,
	BT8XXEMU_EmulatorFT801 = 0x0801,
	BT8XXEMU_EmulatorFT810 = 0x0810,
	BT8XXEMU_EmulatorFT811 = 0x0811,
	BT8XXEMU_EmulatorFT812 = 0x0812,
	BT8XXEMU_EmulatorFT813 = 0x0813,
	BT8XXEMU_EmulatorBT880 = 0x0880,
	BT8XXEMU_EmulatorBT881 = 0x0881,
	BT8XXEMU_EmulatorBT882 = 0x0882,
	BT8XXEMU_EmulatorBT883 = 0x0883,
	BT8XXEMU_EmulatorBT815 = 0x0815,
	BT8XXEMU_EmulatorBT816 = 0x0816,
	BT8XXEMU_EmulatorBT817 = 0x0817,
	BT8XXEMU_EmulatorBT818 = 0x0818,
	BT8XXEMU_EmulatorBT819 = 0x0819,
	BT8XXEMU_EmulatorBT820 = 0x0820,
} BT8XXEMU_EmulatorMode;

typedef enum
{
	// enables the keyboard to be used as input (default: on)
	BT8XXEMU_EmulatorEnableKeyboard = 0x01,
	// enables audio (default: on)
	BT8XXEMU_EmulatorEnableAudio = 0x02,
	// enables coprocessor (default: on)
	BT8XXEMU_EmulatorEnableCoprocessor = 0x04,
	// enables mouse as touch (default: on)
	BT8XXEMU_EmulatorEnableMouse = 0x08,
	// enable debug shortkeys (default: on)
	BT8XXEMU_EmulatorEnableDebugShortkeys = 0x10,
	// enable graphics processor multithreading (default: on)
	BT8XXEMU_EmulatorEnableGraphicsMultithread = 0x20,
	// enable dynamic graphics quality degrading by reducing resolution and dropping frames (default: on)
	BT8XXEMU_EmulatorEnableDynamicDegrade = 0x40,
	// enable usage of REG_ROTATE (default: off)
	// BT8XXEMU_EmulatorEnableRegRotate = 0x80, // Now always on
	// enable emulating REG_PWM_DUTY by fading the rendered display to black (default: off)
	BT8XXEMU_EmulatorEnableRegPwmDutyEmulation = 0x100,
	// enable usage of touch transformation matrix (default: on) (should be disabled in editor)
	BT8XXEMU_EmulatorEnableTouchTransformation = 0x200,
	// enable output to stdout from the emulator (default: off) (note: stdout is is some cases not thread safe)
	BT8XXEMU_EmulatorEnableStdOut = 0x400,
	// enable performance adjustments for running the emulator as a background process without window (default: off)
	BT8XXEMU_EmulatorEnableBackgroundPerformance = 0x800,
	// enable performance adjustments for the main MCU thread (default: on)
	BT8XXEMU_EmulatorEnableMainPerformance = 0x1000,
	// enable HSF preview (default: on for bt817 and up, off otherwise, not applicable to bt820)
	BT8XXEMU_EmulatorEnableHSFPreview = 0x2000,
	// enable SD card write protection (default: on, applicable to bt820 only)
	// can be changed at runtime when switching SD card image
	BT8XXEMU_EmulatorEnableSDCardReadOnly = 0x4000,
	// enable RAM_G to reserve the maximum 2GB address space, regardless of the actual size (default: on)
	// this is useful for guarding the program against overflows in the render engine
	// does not actually use more RAM, since it is merely a reserved virtual address space
	BT8XXEMU_EmulatorEnableRamGMaxReserve = 0x8000,
	// enable RAM_G to commit the maximum 2GB address space (default: off)
	// this will allocate the full address space, which will use more RAM
	// may be used to mitigate any overflows in the render engine
	// on Linux, this setting makes no difference, as the kernel commits on demand
	BT8XXEMU_EmulatorEnableRamGMaxCommit = 0x10000,
	// BT820: when REG_SO_SOURCE is SWAPCHAIN_0, send each frame to the Graphics callback or window immediately after rendering
	// (default: off). This reduces tool latency while preserving swapchain advancement and frame pacing at scanout.
	// If no render callback occurred since the previous scanout, scanout sends an unchanged-frame callback to maintain the
	// callback rate. Other scanout sources, including direct RAM addresses, are still presented at scanout
	BT8XXEMU_EmulatorEnableFastForwardSwapchain = 0x20000,
	// [API 19] With an all-zero RendererDeviceUuid, prefer a discrete GPU if one is available (default: on).
	// Takes priority over BT8XXEMU_EmulatorHybridIntegratedRenderer. Use BT8XXEMU_getRenderer to query the selected renderer
	BT8XXEMU_EmulatorPreferDedicatedRenderer = 0x40000,
	// [API 19] enable hybrid mode when an integrated renderer is selected (default: on)
	// with an all zero RendererDeviceUuid, also makes integrated renderers eligible for automatic selection when no discrete
	// renderer was chosen. Without this flag, automatic selection falls back to software when no discrete renderer was chosen
	// a selected integrated renderer runs alone when hybrid mode is unavailable, or when explicitly selected without this flag
	BT8XXEMU_EmulatorHybridIntegratedRenderer = 0x80000,
	// [API 19] turn the default output window back from the rotation each frame was rendered with: REG_ROTATE, or on BT820
	// the REG_RE_ROTATE of the presented frame (default: off). A portrait UI then shows upright and mirrors are undone, and the
	// mouse touches the panel pixel it shows. The Graphics callback receives the panel output unturned. Can be changed at
	// runtime, BT8XXEMU_setWindowRotate selects a fixed rotation instead
	BT8XXEMU_EmulatorEnableWindowRotate = 0x100000,

} BT8XXEMU_EmulatorFlags;

typedef enum
{
	// frame render has changes since last render
	BT8XXEMU_FrameBufferChanged = 0x01,
	// frame is completely rendered (without degrade)
	BT8XXEMU_FrameBufferComplete = 0x02,
	// frame has changes since last render
	BT8XXEMU_FrameChanged = 0x04,
	// frame rendered right after display list swap
	BT8XXEMU_FrameSwap = 0x08,
	// [API 19] the current render state still matches the state used for rendering the frame. On BT820, rendering
	// an intermediate target clears this flag; rendering a display target sets it again. Always set on EVE1 to EVE4
	BT8XXEMU_FrameCurrentState = 0x10,
	// [API 19] the renderer device of BT8XXEMU_getRenderer rendered the frame, alone or together with the CPU in hybrid
	// mode. Clear when the software renderer did, e.g. while a renderer device is still starting up or failed to render.
	// On BT820 the flag follows the pixels: a frame copied from one rendered on the device keeps it
	BT8XXEMU_FrameAccelerated = 0x20,

	// NOTE: Difference between FrameChanged and FrameBufferChanged is that
	// FrameChanged will only be true if the content of the frame changed,
	// whereas FrameBufferChanged will be true if the rendered buffer changed.
	// For example, when the emulator renders a frame incompletely due to
	// CPU overload, it will then finish the frame in the next callback,
	// and when this is the same frame, FrameChanged will be false,
	// but FrameBufferChanged will be true as the buffer has changed.

	// NOTE: Frames can change even though no frame was swapped, due to
	// several parameters such as REG_MACRO or REG_ROTATE.

	// NOTE: If you only want completely rendered frames, turn OFF
	// the EmulatorEnableDynamicDegrade feature.

	// NOTE: To get the accurate frame after a frame swap, wait for FrameSwap
	// to be set, and get the first frame which has FrameBufferComplete set.

	// NOTE: To get the accurate frame after any frame change, wait for
	// FrameChanged, and get the first frame which has FrameBufferComplete set.

} BT8XXEMU_FrameFlags;

// [API 19] Flags of a BT8XXEMU_RenderTarget
typedef enum
{
	// the buffer is used for scanout, as configured by REG_SO_SOURCE. Clear for an intermediate render target
	BT8XXEMU_RenderTargetScanout = 0x01,
	// the display list swap changed the contents of the render target buffer. Clear otherwise, or when the target is invalid
	BT8XXEMU_RenderTargetChanged = 0x02,
	// the renderer device of BT8XXEMU_getRenderer rendered the pixels of the target, alone or together with the CPU in
	// hybrid mode. Clear when the software renderer did, e.g. while a renderer device is still starting up or failed to render.
	// A target copied from, or left unchanged as, an earlier render keeps the flag of that render
	BT8XXEMU_RenderTargetAccelerated = 0x04,

} BT8XXEMU_RenderTargetFlags;

// [API 19] Target rendered by EVE5 after a display list swap, see BT8XXEMU_EmulatorParameters::RenderTarget
typedef struct
{
	// The address in RAM_G where the render engine wrote the frame. REG_RE_DEST, or the PTR of a SWAPCHAIN_0 entry
	uint32_t Address;
	// REG_RE_FORMAT
	uint32_t Format;
	// The size as written by the render engine. REG_RE_W rounded down to a multiple of 16 (1 to 15 counts as 2048), and REG_RE_H
	uint32_t Width;
	uint32_t Height;
	// REG_RE_ROTATE and REG_RE_DITHER
	uint32_t Rotate;
	uint32_t Dither;
	// See BT8XXEMU_RenderTargetFlags
	BT8XXEMU_RenderTargetFlags Flags;
} BT8XXEMU_RenderTarget;

typedef struct
{
	// Microcontroller main function. This will be run on a new thread managed by the emulator.
	// When not provided the calling thread is assumed to be the MCU thread
	void (*Main)(BT8XXEMU_Emulator *sender, void *context);
	// See EmulatorFlags.
	int Flags;
	// Emulator mode
	BT8XXEMU_EmulatorMode Mode;

	// The default mouse pressure, default 0 (maximum).
	// See REG_TOUCH_RZTRESH, etc.
	uint32_t MousePressure;
	// External frequency in Hz. See CLK, etc. Default 0 uses internal clock.
	// On BT820 this refers to the input to SYSPLL (PLL1). Default 0 uses the
	// default oscillator value of 38.4 MHz. The SYSPLL output value is passed through
	// the SYSCLK divider to obtain the system clock, and through TXCLK for PCLK.
	// The default SYSPLL multiplier 15 gives a 576 MHz SYSPLL output.
	// For SYSCLK, the default divider 8 results in a 72 MHz system clock,
	// configured from (SYSCLKDIV+1). The pixel clock, or scan clock,
	// is configured from (TXCLKDIV+1), also with a default divider of 8,
	// resulting in a 72 MHz pixel clock. Using the pixel clock, the frame
	// rate can be calculated as (72 MHz / (HCYCLE * VCYCLE)).
	uint32_t ExternalFrequency;

	// Reduce graphics processor threads by specified number, default 0
	// Necessary when doing very heavy work on the MCU or Coprocessor
	uint32_t ReduceGraphicsThreads;

	// [API 19] UUID of the GPU to use, from BT8XXEMU_rendererDevices.
	// An all-zero UUID (the default) selects a renderer using BT8XXEMU_EmulatorPreferDedicatedRenderer and
	// BT8XXEMU_EmulatorHybridIntegratedRenderer. With neither flag set, it selects the CPU renderer.
	uint8_t RendererDeviceUuid[16];

	// Sleep function for MCU thread usage throttle, defaults to BT8XXEMU_SleepWake implementation.
	// This must implement a high resolution sleep function.
	// A corresponding wake function must be provided.
	// For reference, see:
	// - https://randomascii.wordpress.com/2020/10/04/windows-timer-resolution-the-great-rule-change/
	// - https://github.com/python/cpython/issues/89592
	void (*MCUSleep)(BT8XXEMU_Emulator *sender, void *context, int ms);

	// Wake function for MCU thread usage throttle, defaults to BT8XXEMU_SleepWake implementation.
	// This may be called whenever registers that the MCU is waiting for are written to.
	void (*MCUWake)(BT8XXEMU_Emulator *sender, void *context);

	// Replaces the default builtin ROM with a custom ROM from a file.
	eve_tchar_t RomFilePath[260];
	// Replaces the default builtin OTP with a custom OTP from a file.
	eve_tchar_t OtpFilePath[260];
	// Replaces the builtin coprocessor ROM.
	eve_tchar_t CoprocessorRomFilePath[260];

	// Graphics driverless mode
	// Setting this callback means no window will be created, and all
	// rendered graphics will be automatically sent to this function.
	// For enabling touch functionality, the functions
	// Memory.setTouchScreenXY and Memory.resetTouchScreenXY must be
	// called manually from the host application.
	// Builtin keyboard functionality is not supported and must be
	// implemented manually when using this mode.
	// The output parameter is false (0) when the display is turned off.
	// The contents of the buffer pointer are undefined after this
	// function returns. Create a copy to use it on another thread.
	// On BT820 the device state during the Graphics callback corresponds to the state at scanout
	// (except when fast-forward is enabled), not to the render engine state of the buffer.
	// Return false (0) when the application must exit, otherwise return true (1).
	int (*Graphics)(BT8XXEMU_Emulator *sender, void *context, int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, BT8XXEMU_FrameFlags flags);

	// [API 19] BT820 render-engine callback. Called on the render thread after processing each display list swap,
	// for both intermediate and display targets, including copied or unchanged targets.
	// The rendered buffer is guaranteed to exist in RAM_G at the reported address until this call returns,
	// unless the address is written to outside of the render process.
	void (*RenderTarget)(BT8XXEMU_Emulator *sender, void *context, const BT8XXEMU_RenderTarget *target);

	// [API 19] Interrupt callback, called when INT_N asserts: REG_INT_EN is set, and a flag of REG_INT_FLAGS is set in REG_INT_MASK.
	// May run on an emulator thread or on a caller's thread, for example during an SPI write to REG_INT_EN.
	// Return immediately without blocking or calling back into the emulator. Notify the interrupt-service thread,
	// which should read REG_INT_FLAGS to identify and acknowledge the sources, as a hardware host would.
	// Concurrent interrupt sources may cause extra callbacks; use the flags to determine what needs servicing.
	// Reading REG_INT_FLAGS clears the flags except for sources held active. In particular, INT_TAG stays set after
	// a render changes REG_TOUCH_TAG until the next render. If enabled in REG_INT_MASK, it keeps INT_N asserted.
	// See BT8XXEMU_hasInterrupt for the current level
	void (*Interrupt)(BT8XXEMU_Emulator *sender, void *context);

	// Log callback
	void (*Log)(BT8XXEMU_Emulator *sender, void *context, BT8XXEMU_LogType type, const char *message);

	// Safe exit. Called when the emulator window is closed.
	// When using sleep functions in the main thread, it may be necessary to wake the thread when this is called.
	// The application should set one or more flags to exit the main loop once all emulator windows are closed.
	void (*Close)(BT8XXEMU_Emulator *sender, void *context);

	// User context that will be passed along to callbacks
	void *UserContext;

	// Flash device to connect with, default NULL
	BT8XXEMU_Flash *Flash;

	// RAM_G size in bytes, applicable to 5th gen EVE (BT820 series) only, default 1GiB
	ptrdiff_t RamGSizeBytes;

	// SD card image file to mount (BT820 series), the file must already exist, defaults to an empty string
	eve_tchar_t SDCardFilePath[260];

} BT8XXEMU_EmulatorParameters;

typedef enum
{
	BT8XXEMU_RendererOther = 0,
	BT8XXEMU_RendererIntegrated = 1,
	BT8XXEMU_RendererDiscrete = 2,
	BT8XXEMU_RendererVirtual = 3,
	BT8XXEMU_RendererCpu = 4, // A Vulkan implementation on the CPU
	BT8XXEMU_RendererSoftware = 5, // The emulator's own CPU renderer, all zero Uuid, from BT8XXEMU_getRenderer
} BT8XXEMU_RendererType;

// [API 19] Flags returned by BT8XXEMU_getRenderer
typedef enum
{
	// the renderer device renders together with the CPU (BT8XXEMU_EmulatorHybridIntegratedRenderer)
	BT8XXEMU_RendererStateHybrid = 0x01,
	// the renderer device is still starting up, and the CPU renders the frames until it is ready, without BT8XXEMU_FrameAccelerated
	BT8XXEMU_RendererStateStarting = 0x02,

} BT8XXEMU_RendererState;

typedef struct
{
	// Device name, UTF-8
	char Name[256];
	// Identifies the device across processes and runs, for BT8XXEMU_EmulatorParameters RendererDeviceUuid
	uint8_t Uuid[16];
	BT8XXEMU_RendererType Type;
	// PCI vendor and device ID where applicable
	uint32_t VendorId;
	uint32_t DeviceId;

} BT8XXEMU_RendererDevice;

typedef struct
{
	// [API 19] Flash library, default "btflashemu", which contains every
	// bundled flash chip. A custom flash library is named by its file name
	// without extension. Blank selects "btflashemu"
	eve_tchar_t LibraryName[26];

	// Exact part name or family name from the device list of the library
	// (BT8XXEMU_Flash_list), default blank. Blank selects the part that
	// works with the default flash driver blob at the requested size.
	// The former library names "mx25l" and "mx25lemu" are treated as blank
	eve_tchar_t DeviceType[26];

	// Size of the flash memory in bytes, default 8MB. With an exact part
	// name, 0 selects the size of that part, and another size selects the
	// family of the part at that size
	ptrdiff_t SizeBytes;

	// Data file to load onto the flash, default NULL
	eve_tchar_t DataFilePath[260];

	// Internal flash status file, device specific, default NULL
	eve_tchar_t StatusFilePath[260];

	// Write actions to the flash are persisted to the used file.
	// This is accomplished by memory mapping the file, instead of
	// the file being copied to memory. Default false
	int Persistent;

	// Print log to standard output. Default false
	int StdOut;

	// Data buffer that is written to the flash initially,
	// overriding any existing contents that may have been
	// loaded from a flash file already, default NULL and 0
	void *Data;
	ptrdiff_t DataSizeBytes;

	// Log callback
	void (*Log)(BT8XXEMU_Flash *sender, void *context, BT8XXEMU_LogType type, const char *message);

	// User context that will be passed along to callbacks
	void *UserContext;

} BT8XXEMU_FlashParameters;

// [API 19] A flash chip of a flash library
typedef struct
{
	// Exact part name, for example "W25Q64JV", usable as DeviceType
	eve_tchar_t PartName[26];

	// Family name, for example "W25Q-JV", usable as DeviceType with any SizeBytes
	eve_tchar_t FamilyName[26];

	// Manufacturer, for example "Winbond"
	eve_tchar_t ManufacturerName[26];

	// Size of the part in bytes
	ptrdiff_t SizeBytes;

	// Readable description for a device list in a user interface, for example "Winbond W25Q64JV (8 MiB)"
	eve_tchar_t Description[64];

} BT8XXEMU_FlashDevice;

// [API 19] The flash chip emulated by a flash emulator instance
typedef struct
{
	// Flash library of the instance
	eve_tchar_t LibraryName[26];

	// Part and size emulated. PartName is blank when the size is not that of a listed part
	BT8XXEMU_FlashDevice Device;

} BT8XXEMU_FlashInfo;

#ifdef __cplusplus
extern "C" {
#endif

//////////
// INIT //
//////////

// Return version information
BT8XXEMU_API extern const char *BT8XXEMU_version();

// Initialize the default emulator parameters
BT8XXEMU_API extern void BT8XXEMU_defaults(uint32_t versionApi, BT8XXEMU_EmulatorParameters *params, BT8XXEMU_EmulatorMode mode);

// [API 19] List host GPUs that support Vulkan 1.1 and have a compute queue. Does not require an emulator instance.
// Writes up to maxDevices entries into devices, which may be NULL, and returns the number of devices. Returns 0 when Vulkan is not available.
BT8XXEMU_API extern int BT8XXEMU_rendererDevices(uint32_t versionApi, BT8XXEMU_RendererDevice *devices, int maxDevices);

// [API 19] Write the selected renderer to device. CPU rendering, including fallback after a GPU failure, is reported as
// BT8XXEMU_RendererSoftware with an all-zero UUID. Returns BT8XXEMU_RendererState flags: hybrid CPU/GPU rendering
// (BT8XXEMU_EmulatorHybridIntegratedRenderer), and starting while the device is prepared in the background and the CPU
// renders the frames. Returns 0 otherwise
BT8XXEMU_API extern int BT8XXEMU_getRenderer(BT8XXEMU_Emulator *emulator, BT8XXEMU_RendererDevice *device);

// Run the emulator on the current thread. Returns when the emulator is fully stopped when a Main function is supplied, returns when the emulator is fully started otherwise. Parameter versionApi must be set to BT8XXEMU_VERSION_API
BT8XXEMU_API extern void BT8XXEMU_run(uint32_t versionApi, BT8XXEMU_Emulator **emulator, const BT8XXEMU_EmulatorParameters *params);

// Stop the emulator. Can be called from any thread. Returns when the emulator has fully stopped. Safe to call multiple times.
BT8XXEMU_API extern void BT8XXEMU_stop(BT8XXEMU_Emulator *emulator);

// Destroy the emulator. Calls BT8XXEMU_stop implicitly. Emulator must be destroyed before process exits.
BT8XXEMU_API extern void BT8XXEMU_destroy(BT8XXEMU_Emulator *emulator);

// Poll if the emulator is still running. Returns 0 when the output window has been closed, or when the emulator has been stopped.
BT8XXEMU_API extern int BT8XXEMU_isRunning(BT8XXEMU_Emulator *emulator);

/////////////
// RUNTIME //
/////////////

// Transfer data over the imaginary SPI bus. Call from the MCU thread (from the setup/loop callbacks). See FT8XX documentation for SPI transfer protocol
BT8XXEMU_API extern uint8_t BT8XXEMU_transfer(BT8XXEMU_Emulator *emulator, uint8_t data);

// Set chip select. Must be set to 1 to start data transfer, 0 to end. See FT8XX documentation for CS_N
BT8XXEMU_API extern void BT8XXEMU_chipSelect(BT8XXEMU_Emulator *, int cs);

// Returns 1 while INT_N is asserted: REG_INT_EN is set, and a flag of REG_INT_FLAGS is set in REG_INT_MASK. See FT8XX documentation for INT_N
BT8XXEMU_API extern int BT8XXEMU_hasInterrupt(BT8XXEMU_Emulator *emulator);

//////////////
// ADVANCED //
//////////////

// Set touch XY. Param idx 0..4. Call on every frame during mouse down or touch when using custom graphics output
BT8XXEMU_API extern void BT8XXEMU_touchSetXY(BT8XXEMU_Emulator *emulator, int idx, int x, int y, int pressure);

// Reset touch XY. Call once no longer touching when using custom graphics output
BT8XXEMU_API extern void BT8XXEMU_touchResetXY(BT8XXEMU_Emulator *emulator, int idx);

// Reset the touch transform matrix to default identity, applying current display rotation and mirroring (ROTATE, HSIZE, VSIZE). Call after CMD_CALIBRATE to restore to pixel perfect settings
BT8XXEMU_API extern void BT8XXEMU_touchTransformReset(BT8XXEMU_Emulator *emulator);

// Set a single emulation flag on or off. Only PWM, HSF and window rotate options can be changed at runtime. Returns the value of the flag after the operation
BT8XXEMU_API extern int BT8XXEMU_setFlag(BT8XXEMU_Emulator *emulator, BT8XXEMU_EmulatorFlags flag, int value);

// [API 19] Turn the default output window back from a fixed rotation, a CMD_SETROTATE value from 0 to 7 (0 shows the panel
// output as it is), or with -1 from the rotation of each frame, as BT8XXEMU_EmulatorEnableWindowRotate does. A fixed turn
// suits an application whose mirror is meant to be seen, such as a right to left layout drawn with a mirrored rotation.
// Does not affect the Graphics callback
BT8XXEMU_API extern void BT8XXEMU_setWindowRotate(BT8XXEMU_Emulator *emulator, int rotate);

/////////////
// SD HOST //
/////////////

// Use an filesystem image as virtual SD card. BT820 and up only. Returns 1 on success
BT8XXEMU_API extern int BT8XXEMU_insertSDCardImage(BT8XXEMU_Emulator *emulator, const eve_tchar_t *filePath, int readOnly);

// Mount a folder as a virtual SD card. Writes are stored in-memory and discarded on exit. BT820 and up only. Returns image size on success
BT8XXEMU_API extern ptrdiff_t BT8XXEMU_insertSDCardFolder(BT8XXEMU_Emulator *emulator, const eve_tchar_t *folderPath, ptrdiff_t minimumSize, int readOnly);

// Ejects the current virtual SD card
BT8XXEMU_API extern void BT8XXEMU_ejectSDCard(BT8XXEMU_Emulator *emulator);

////////////
// LVDSRX //
////////////

// Set behaviour of LVDSRX emulation. State 0: manual, 1: automatic test pattern (TODO)
// This function also doubles as a disconnection flag, LVDSRX status will read as connected once a writing pointer is requested
BT8XXEMU_API extern void BT8XXEMU_setLvdsRXState(BT8XXEMU_Emulator *emulator, int state, int width, int height, double freq);

// Get raw pointer to the LVDSRX frame buffer RAM area for writing. Discards any queued frame if the swapchain is full
// Returns NULL in case the memory area is invalid, or in case the LVDSRX is not enabled or not capturing
// User is responsible for writing in the correct pixel format (RGB8 / RGB565) and supplying frames at the expected rate
// This function is thread safe, but the user must ensure that the emulator is not destroyed while the pointer is in use
BT8XXEMU_API extern void *BT8XXEMU_lvdsRXWp(BT8XXEMU_Emulator *emulator, ptrdiff_t size);

// Commit a frame written to the LVDSRX frame buffer RAM area
BT8XXEMU_API extern void BT8XXEMU_advanceLvdsRXWp(BT8XXEMU_Emulator *emulator);

///////////
// FLASH //
///////////

#ifndef BT8XXEMU_FLASH_LIBRARY

// Initialize the default flash emulator parameters: the "btflashemu" library, no device type, 8MB
BT8XXEMU_API extern void BT8XXEMU_Flash_defaults(uint32_t versionApi, BT8XXEMU_FlashParameters *params);

// [API 19] Initialize the flash emulator parameters with defaults that works with the default
// flash firmware blob or ROM firmware of the specified emulator mode at the given size.
// Returns 1 when the mode and size have a known working part, 0 when the parameters are set to the nearest valid configuration
// Size 0 selects the flash chip fitted on the development boards of the emulated chip generation
BT8XXEMU_API extern int BT8XXEMU_Flash_defaultsEx(uint32_t versionApi, BT8XXEMU_FlashParameters *params, BT8XXEMU_EmulatorMode mode, ptrdiff_t sizeBytes);

// [API 19] List the flash chips of a flash library (NULL or blank for "btflashemu"). Fills up to
// count entries of devices and returns the total number of chips of the library, or a negative
// value when the library cannot be loaded
BT8XXEMU_API extern int BT8XXEMU_Flash_list(uint32_t versionApi, const eve_tchar_t *libraryName, BT8XXEMU_FlashDevice *devices, int count);

// Create flash emulator instance
BT8XXEMU_API extern BT8XXEMU_Flash *BT8XXEMU_Flash_create(uint32_t versionApi, const BT8XXEMU_FlashParameters *params);

// Destroy flash emulator instance
BT8XXEMU_API extern void BT8XXEMU_Flash_destroy(BT8XXEMU_Flash *flash);

// [API 19] Get the library and the chip emulated by a flash emulator instance. Returns 1 on success
BT8XXEMU_API extern int BT8XXEMU_Flash_info(BT8XXEMU_Flash *flash, BT8XXEMU_FlashInfo *info);

// Transfer data using SPI or Quad SPI protocol. Bit 0:3 are data, bit 4 is cable select (0 active), SCK is clock. In single mode bit 0 is MOSI and bit 1 is MISO
BT8XXEMU_API extern uint8_t BT8XXEMU_Flash_transferSpi4(BT8XXEMU_Flash *flash, uint8_t signal);

#endif

///////////////
// SLEEPWAKE //
///////////////

// Create wakeable sleep object. This implements high resolution sleep, one object is usable from a single thread. Use instead of standard sleep for sleep < 14ms
BT8XXEMU_API extern BT8XXEMU_SleepWake *BT8XXEMU_SleepWake_create();

// Destroy wakeable sleep object
BT8XXEMU_API extern void BT8XXEMU_SleepWake_destroy(BT8XXEMU_SleepWake *sleepWake);

// Wake up from sleep. Can be called from any thread. If the thread is not sleeping, this will wake up the thread immediately next time it sleeps
BT8XXEMU_API extern void BT8XXEMU_SleepWake_wake(BT8XXEMU_SleepWake *sleepWake);

// Sleep for a number of milliseconds. Can only be used from the thread that owns the sleep object
BT8XXEMU_API extern void BT8XXEMU_SleepWake_sleep(BT8XXEMU_SleepWake *sleepWake, int ms);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* #ifndef BT8XXEMU_H */

/* end of file */


