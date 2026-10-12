#ifndef PATIENT_MONITOR_WAVE_H_
#define PATIENT_MONITOR_WAVE_H_
#include "Platform.h"
#define PM_WAVE_WIDTH 870U
#define PM_WAVE_STRIDE 872U
#define PM_WAVE_BYTES (6UL * 2UL * PM_WAVE_STRIDE)
#define PM_WAVE_BASE 196608UL
/* RAM_G [196608,207072), handles 0..11. No media FIFO or font cache. */
void PM_WaveInit(EVE_HalContext *host, uint32_t now);
void PM_WaveUpdate(EVE_HalContext *host, uint32_t now);
void PM_WaveDraw(EVE_HalContext *host, uint8_t lane, int16_t y, uint32_t rgb);
void PM_WaveSignal(uint8_t lane, bool valid);
#endif
