#include <stdint.h>
#include <stdio.h>
#include "Common.h"
#include "EVE_CoCmd.h"
#include "PatientMonitorFont.h"

extern const uint8_t PM_ChineseXFont[];
extern const uint32_t PM_ChineseXFont_size;
extern const uint8_t PM_ChineseGlyph[];
extern const uint32_t PM_ChineseGlyph_size;

static void writeFontData(EVE_HalContext *host,uint32_t address,
    const uint8_t *bytes,uint32_t length)
{
    uint32_t offset;
    for(offset=0;offset<length;offset+=4096U) {
        uint32_t count=length-offset;
        if(count>4096U) count=4096U;
        EVE_Hal_wrMem(host,address+offset,bytes+offset,count);
    }
}

bool PM_FontLoad(EVE_HalContext *host)
{
    if (PM_FONT_XFONT_ADDR + PM_ChineseXFont_size > PM_FONT_GLYPH_ADDR ||
        PM_FONT_GLYPH_ADDR + PM_ChineseGlyph_size > PM_WAVE_BASE ||
        PM_WAVE_BASE + PM_WAVE_BYTES > RAM_G_SIZE)
    {
        fprintf(stderr, "PatientMonitor: Chinese font overlaps RAM_G resources\n");
        return false;
    }
    printf("PatientMonitor: loading Chinese font XFONT=%lu glyph=%lu bytes\n",
        (unsigned long)PM_ChineseXFont_size, (unsigned long)PM_ChineseGlyph_size);
    writeFontData(host, PM_FONT_XFONT_ADDR, PM_ChineseXFont, PM_ChineseXFont_size);
    writeFontData(host, PM_FONT_GLYPH_ADDR, PM_ChineseGlyph, PM_ChineseGlyph_size);
    EVE_CoCmd_setFont2(host, PM_CHINESE_HANDLE, PM_FONT_XFONT_ADDR, 0);
    if (!EVE_Cmd_waitFlush(host)) {
        char report[129];
        EVE_Hal_rdMem(host, (uint8_t *)report, RAM_ERR_REPORT, 128);
        report[128] = 0;
        fprintf(stderr, "PatientMonitor: Chinese font setup failed: %s\n", report);
        return false;
    }
    printf("PatientMonitor: Chinese font RAM_G [%lu,%lu) and [%lu,%lu)\n",
        (unsigned long)PM_FONT_XFONT_ADDR,
        (unsigned long)(PM_FONT_XFONT_ADDR + PM_ChineseXFont_size),
        (unsigned long)PM_FONT_GLYPH_ADDR,
        (unsigned long)(PM_FONT_GLYPH_ADDR + PM_ChineseGlyph_size));
    return true;
}
