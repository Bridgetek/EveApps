/* BARGRAPH layout follows SAMAPP_Primitives_barGraph: stride bytes, height 1.
 * Samples are heights above the bottom of an 88-pixel lane. */
#include <string.h>
#include "PatientMonitorWave.h"
#include "EVE_CoCmd.h"

static uint8_t s_columns[6][2][PM_WAVE_STRIDE];
static uint8_t s_previous[6];
static bool s_valid[6];
static uint16_t s_cursor;
static uint32_t s_lastMs, s_fraction, s_sample;

static int16_t triangle(uint16_t p, uint16_t center, uint16_t width, int16_t amp)
{
    int16_t distance = (int16_t)p - (int16_t)center;
    if (distance < 0) distance = -distance;
    return distance >= width ? 0 : (int16_t)((int32_t)amp * (width - distance) / width);
}

static uint8_t sampleHeight(uint8_t lane, uint32_t sample)
{
    uint16_t p = (uint16_t)((sample % 150U) * 1000U / 150U);
    int16_t v = 20;
    switch (lane)
    {
    case 0: v += triangle(p,120,55,7)-triangle(p,260,18,10)+triangle(p,300,20,45)
                  -triangle(p,338,24,13)+triangle(p,560,90,12); break;
    case 1: v += triangle(p,250,180,38)+triangle(p,520,150,13); break;
    case 2: v += triangle(p,230,160,43)+triangle(p,520,220,16)-triangle(p,420,24,5); break;
    case 3: v += triangle(p,250,210,25)+triangle(p,570,250,14); break;
    case 4: v += triangle(p,150,75,12)+triangle(p,360,65,7)+triangle(p,650,110,14); break;
    default:
        p = (uint16_t)((sample % 300U) * 1000U / 300U);
        v = p < 100 ? p * 45 / 100 : p < 560 ? 45 + (p-100)/90 :
            p < 650 ? (650-p)*50/90 : 2;
        break;
    }
    if (!s_valid[lane]) v = 2;
    if (v < 1) v = 1;
    if (v > 75) v = 75;
    return (uint8_t)v;
}

static void setColumn(uint16_t x, uint32_t sample)
{
    uint8_t lane, v, previous;
    for (lane = 0; lane < 6; ++lane)
    {
        v = sampleHeight(lane, sample);
        previous = x ? s_previous[lane] : v;
        /* BARGRAPH covers pixels below its sample Y, not above it. */
        s_columns[lane][0][x] = 86 - (v > previous ? v : previous);
        s_columns[lane][1][x] = 88 - (v < previous ? v : previous);
        s_previous[lane] = v;
    }
}

void PM_WaveInit(EVE_HalContext *host, uint32_t now)
{
    uint16_t x;
    uint8_t lane;
    memset(s_columns, 255, sizeof(s_columns));
    for (lane = 0; lane < 6; ++lane) s_valid[lane] = true;
    for (x = 0; x < PM_WAVE_WIDTH; ++x) setColumn(x, x);
    for (lane = 0; lane < 6; ++lane)
    {
        memset(s_columns[lane][0], 255, 22);
        memset(s_columns[lane][1], 255, 22);
    }
    EVE_Hal_wrMem(host, RAM_G, (const uint8_t *)s_columns, sizeof(s_columns));
    s_cursor = 0;
    s_sample = PM_WAVE_WIDTH;
    s_lastMs = now;
    s_fraction = 0;
}

void PM_WaveUpdate(EVE_HalContext *host, uint32_t now)
{
    uint32_t elapsed = now - s_lastMs, steps;
    uint16_t start, count, first, x;
    uint8_t lane, pass;
    if (!elapsed) return;
    s_lastMs = now;
    /* Drop stale time after a debugger pause; never flood the bus on resume. */
    if (elapsed > 100U) elapsed = 100U;
    s_fraction += elapsed * 150U;
    steps = s_fraction / 1000U;
    s_fraction %= 1000U;
    if (!steps) return;
    start = s_cursor;
    count = (uint16_t)(steps + 22U);
    while (steps--)
    {
        setColumn(s_cursor, s_sample++);
        s_cursor = (uint16_t)((s_cursor + 1U) % PM_WAVE_WIDTH);
    }
    for (x = 0; x < 22; ++x)
        for (lane = 0; lane < 6; ++lane)
            s_columns[lane][0][(s_cursor+x)%PM_WAVE_WIDTH] =
                s_columns[lane][1][(s_cursor+x)%PM_WAVE_WIDTH] = 255;
    first = PM_WAVE_WIDTH - start;
    if (first > count) first = count;
    /* Each plane is one contiguous burst, or two at the circular wrap. */
    for (lane = 0; lane < 6; ++lane)
        for (pass = 0; pass < 2; ++pass)
        {
            uint32_t base = (lane*2UL+pass)*PM_WAVE_STRIDE;
            EVE_Hal_wrMem(host, base+start, &s_columns[lane][pass][start], first);
            if (count > first)
                EVE_Hal_wrMem(host, base, s_columns[lane][pass], count-first);
        }
}

void PM_WaveDraw(EVE_HalContext *host, uint8_t lane, int16_t y, uint32_t rgb)
{
    uint8_t pass;
    EVE_CoDl_saveContext(host);
    EVE_CoDl_scissorXY(host, 0, y);
    EVE_CoDl_scissorSize(host, PM_WAVE_WIDTH, 88);
    EVE_CoDl_cell(host, 0);
    for (pass = 0; pass < (lane == 5 ? 1 : 2); ++pass)
    {
        EVE_CoDl_bitmapHandle(host, lane*2+pass);
        EVE_CoDl_bitmapSource(host, (lane*2UL+pass)*PM_WAVE_STRIDE);
        EVE_CoDl_bitmapLayout(host, BARGRAPH, PM_WAVE_STRIDE, 1);
        EVE_CoDl_bitmapSize(host, NEAREST, BORDER, BORDER, PM_WAVE_WIDTH, 88);
        EVE_CoDl_colorRgb_ex(host, pass ? 0 : rgb);
        EVE_CoDl_begin(host, BITMAPS);
        EVE_CoDl_vertex2f(host, 0, y);
        EVE_CoDl_end(host);
    }
    EVE_CoDl_restoreContext(host);
}

void PM_WaveSignal(uint8_t lane, bool valid)
{
    if (lane < 6) s_valid[lane] = valid;
}
