/**
 * @file PatientMonitor.c
 * @brief BT817/RP2040 1280x800 bedside-monitor UI demonstration.
 * All values are simulated. This is not medical-device software.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Common.h"
#include "EVE_CoCmd.h"
#include "PatientMonitor.h"
#include "PatientMonitorPlatform.h"
#include "PatientMonitorWave.h"
#include "Platform.h"

#define PM_RGB(r,g,b) ((((uint32_t)(r))<<16)|(((uint32_t)(g))<<8)|(uint32_t)(b))
#define C_BLACK PM_RGB(0,0,0)
#define C_LINE PM_RGB(42,49,56)
#define C_TEXT PM_RGB(216,221,226)
#define C_DIM PM_RGB(138,148,158)
#define C_GREEN PM_RGB(43,227,74)
#define C_CYAN PM_RGB(31,214,238)
#define C_RED PM_RGB(255,59,59)
#define C_YELLOW PM_RGB(244,226,44)
#define C_WHITE PM_RGB(242,242,242)
#define C_PINK PM_RGB(255,125,140)
#define C_MAGENTA PM_RGB(227,76,232)
#define C_WARN PM_RGB(255,210,31)
#define C_BLUE PM_RGB(27,115,217)

typedef struct MonitorState {
    uint8_t silence, alarmsOff, previousTag, activeTag;
    uint32_t startMs, lastNumericMs, pressedMs;
    uint8_t pressedTag;
    int16_t hr, spo2, abpSys, abpDia, papSys, papDia, cvp, etco2;
    const char *lastAction;
} MonitorState;
typedef struct SoftKey {
    int16_t x, w; uint8_t tag; uint32_t color; const char *line1, *line2;
} SoftKey;

static EVE_HalContext s_halContext;
static EVE_HalContext *s_pHalContext = &s_halContext;
static Gpu_Fonts_t s_fonts[16];
static const uint8_t s_fontIds[] = {16,17,18,19,20,21,26,28,31};

/* Direct ROM glyphs avoid the per-CMD_TEXT display-list state overhead.
 * VERTEX2II uses local coordinates; translation supports the full 1280px LCD. */
static void pmText(EVE_HalContext *host, int16_t x, int16_t y,
    int16_t font, uint16_t options, const char *value)
{
    const Gpu_Fonts_t *metrics;
    const unsigned char *p;
    int width=0, offset=0;
    /* Use the visually verified ASCII ROM font for small UI labels. */
    if(font==17 || font==19) font=20;
    metrics=&s_fonts[font-16];
    for (p=(const unsigned char *)value; *p; ++p) width+=metrics->FontWidth[*p & 127];
    if (options & OPT_RIGHTX) x-=width;
    else if (options & OPT_CENTERX) x-=width/2;
    if (options & OPT_CENTERY) y-=(int16_t)metrics->FontHeightInPixels/2;
    EVE_CoDl_vertexTranslateX(host, x*16);
    EVE_CoDl_vertexTranslateY(host, y*16);
    EVE_CoDl_begin(host, BITMAPS);
    for (p=(const unsigned char *)value; *p; ++p) {
        /* Rebase long strings before VERTEX2II's 9-bit coordinate wraps. */
        if (offset>480) {
            x+=(int16_t)offset; offset=0;
            EVE_CoDl_vertexTranslateX(host,x*16);
        }
        if (*p!=' ') EVE_CoDl_vertex2ii(host,offset,0,(uint8_t)font,*p & 127);
        offset+=metrics->FontWidth[*p & 127];
    }
    EVE_CoDl_end(host);
    EVE_CoDl_vertexTranslateX(host,0);
    EVE_CoDl_vertexTranslateY(host,0);
}
static const SoftKey s_keys[] = {
    {11,74,PM_TAG_SILENCE,C_WARN,"Silence",""},{89,74,PM_TAG_ALARMS_OFF,C_WARN,"Alarms off",""},
    {167,30,PM_TAG_PREVIOUS,PM_RGB(43,50,56),"<<",""},{201,105,PM_TAG_NBP,PM_RGB(43,50,56),"Start/Stop","NBP"},
    {310,92,PM_TAG_STOP_ALL,PM_RGB(43,50,56),"Stop All",""},{406,96,PM_TAG_REPEAT,PM_RGB(43,50,56),"Repeat","Time"},
    {506,96,PM_TAG_SIZE,PM_RGB(43,50,56),"Adjust","Size"},{606,78,PM_TAG_ZERO,PM_RGB(43,50,56),"Zero",""},
    {688,96,PM_TAG_VALUES,PM_RGB(43,50,56),"Enter","Values"},{788,90,PM_TAG_RECORD,PM_RGB(43,50,56),"Record",""},
    {882,96,PM_TAG_TREND,PM_RGB(43,50,56),"Vitals","Trend"},{982,30,PM_TAG_NEXT,PM_RGB(43,50,56),">>",""},
    {1016,122,PM_TAG_SETUP,C_BLUE,"Main Setup",""},{1142,127,PM_TAG_MAIN,C_BLUE,"Main Screen",""}
};

static void color(uint32_t c) { EVE_CoDl_colorRgb(s_pHalContext,(uint8_t)(c>>16),(uint8_t)(c>>8),(uint8_t)c); }
static void rect(int16_t x,int16_t y,int16_t w,int16_t h,uint32_t c) {
    color(c); EVE_CoDl_begin(s_pHalContext,RECTS); EVE_CoDl_vertex2f(s_pHalContext,x,y);
    EVE_CoDl_vertex2f(s_pHalContext,x+w,y+h); EVE_CoDl_end(s_pHalContext);
}
static void line(int16_t x0,int16_t y0,int16_t x1,int16_t y1,uint32_t c) {
    color(c); EVE_CoDl_lineWidth(s_pHalContext,16); EVE_CoDl_begin(s_pHalContext,LINES);
    EVE_CoDl_vertex2f(s_pHalContext,x0,y0); EVE_CoDl_vertex2f(s_pHalContext,x1,y1); EVE_CoDl_end(s_pHalContext);
}
static void drawWave(uint8_t lane,int16_t y,uint32_t c,const char *label,const char *note,uint32_t elapsed) {
    (void)elapsed;
    line(0,y+87,870,y+87,PM_RGB(18,23,27));
    PM_WaveDraw(s_pHalContext, lane, y, c);
    color(c);
    pmText(s_pHalContext,7,y+5,26,0,label);
    if(note[0]){color(C_DIM);pmText(s_pHalContext,860,y+5,20,OPT_RIGHTX,note);}
}
static void numeric(int16_t x,int16_t y,int16_t w,int16_t h,uint32_t c,const char *label,const char *units,const char *value,uint8_t font) {
    rect(x,y,w,h,C_BLACK);line(x,y+h-1,x+w,y+h-1,PM_RGB(18,23,27));color(c);
    pmText(s_pHalContext,x+7,y+7,21,0,label);color(C_DIM);
    pmText(s_pHalContext,x+7,y+24,18,0,units);color(c);
    pmText(s_pHalContext,x+w-8,y+h/2,font,OPT_RIGHTX|OPT_CENTERY,value);
}
static void status(void) {
    rect(0,0,1280,30,PM_RGB(233,237,240));color(PM_RGB(17,17,17));
    pmText(s_pHalContext,10,15,21,OPT_CENTERY,"bed-04");
    pmText(s_pHalContext,330,15,21,OPT_CENTER,"No Patient - DEMO");
    pmText(s_pHalContext,650,15,21,OPT_CENTER,"Adult");
    pmText(s_pHalContext,900,15,21,OPT_CENTER,"6 May 2018 09:07");
    pmText(s_pHalContext,1120,15,21,OPT_CENTER,"Profiles");
    pmText(s_pHalContext,1270,15,21,OPT_RIGHTX|OPT_CENTERY,"SSC Sepsis");
}
static void stGrid(void) {
    static const char *const left[]={"I 0.2","II 0.1","III -0.1","aVR -0.2","aVL 0.3","aVF 0.0"};
    static const char *const right[]={"V1 -0.4","V2 -0.3","V3 -0.7","V4 0.4","V5 0.5","V6 0.2"};uint8_t i;
    rect(1075,30,205,88,C_BLACK);color(C_GREEN);
    for(i=0;i<6;i++) {
        pmText(s_pHalContext,1082,32+i*14,16,0,left[i]);
        pmText(s_pHalContext,1182,32+i*14,16,0,right[i]);
    }
}
static void bottom(uint32_t elapsed,const MonitorState *s) {
    char t[12];uint32_t m=(18U*60U+51U+elapsed/60000U)%(24U*60U);
    snprintf(t,sizeof(t),"%02lu:%02lu",(unsigned long)(m/60U),(unsigned long)(m%60U));
    line(8,686,1272,686,C_TEXT);line(8,746,1272,746,C_TEXT);line(8,686,8,746,C_TEXT);line(1272,686,1272,746,C_TEXT);
    line(398,686,398,746,C_LINE);line(518,686,518,746,C_LINE);color(C_TEXT);
    pmText(s_pHalContext,16,690,20,0,"Sepsis Management Bundle");color(C_DIM);
    pmText(s_pHalContext,16,708,16,0,"Demo profile - no clinical guidance");
    pmText(s_pHalContext,16,726,17,0,s->lastAction);color(C_WHITE);pmText(s_pHalContext,458,716,31,OPT_CENTER,t);
    color(C_DIM);pmText(s_pHalContext,530,692,17,0,"NBPm");pmText(s_pHalContext,850,692,17,0,"CVPm");
    pmText(s_pHalContext,760,738,17,0,"-4 hrs");pmText(s_pHalContext,1080,738,17,0,"-2 hrs");
    line(540,716,1255,716,C_LINE);rect(700,706,38,12,C_PINK);rect(820,721,300,13,PM_RGB(14,106,115));rect(1120,712,45,22,C_CYAN);
}
static void softKeys(const MonitorState *s) {
    uint8_t i;rect(8,752,1264,44,PM_RGB(21,25,29));EVE_CoDl_tagMask(s_pHalContext,1);
    for(i=0;i<(uint8_t)(sizeof(s_keys)/sizeof(s_keys[0]));i++){const SoftKey *k=&s_keys[i];
        uint8_t sel=(uint8_t)(s->pressedTag==k->tag||(k->tag==PM_TAG_SILENCE&&s->silence)||(k->tag==PM_TAG_ALARMS_OFF&&s->alarmsOff));
        EVE_CoDl_tag(s_pHalContext,k->tag);rect(k->x,752,k->w,44,sel?C_WHITE:k->color);color((k->color==C_WARN||sel)?C_BLACK:C_TEXT);
        if(k->line2[0]){pmText(s_pHalContext,k->x+k->w/2,766,19,OPT_CENTER,k->line1);pmText(s_pHalContext,k->x+k->w/2,783,19,OPT_CENTER,k->line2);}
        else pmText(s_pHalContext,k->x+k->w/2,774,19,OPT_CENTER,k->line1);}
    EVE_CoDl_tagMask(s_pHalContext,0);EVE_CoDl_tag(s_pHalContext,0);
}
static bool frame(const MonitorState *s,uint32_t elapsed) {
    char b[32]; uint32_t dlBytes; uint8_t i;
    EVE_CoCmd_dlStart(s_pHalContext);EVE_CoDl_clearColorRgb(s_pHalContext,0,0,0);EVE_CoDl_clear(s_pHalContext,1,1,1);EVE_CoDl_vertexFormat(s_pHalContext,0);
    for(i=0;i<sizeof(s_fontIds);++i) EVE_CoCmd_romFont(s_pHalContext,s_fontIds[i],s_fontIds[i]);
    status();drawWave(0,30,C_GREEN,"II","SV Rhythm",elapsed);drawWave(1,118,C_CYAN,"Pleth","",elapsed);
    drawWave(2,206,C_RED,"ABP","150 / 50",elapsed);drawWave(3,294,C_YELLOW,"PAP","30 / 0",elapsed);
    drawWave(4,382,C_CYAN,"CVP","30 / 0",elapsed);drawWave(5,470,C_WHITE,"CO2","60",elapsed);
    snprintf(b,sizeof(b),"%d",s->hr);numeric(870,30,205,88,C_GREEN,"HR","bpm",b,31);stGrid();
    snprintf(b,sizeof(b),"%d",s->spo2);numeric(870,118,205,88,C_CYAN,"SpO2","%",b,31);
    snprintf(b,sizeof(b),"%d",s->hr);numeric(1075,118,205,88,C_CYAN,"Pulse","bpm",b,31);
    snprintf(b,sizeof(b),"%d/%d  (%d)",s->abpSys,s->abpDia,(s->abpSys+2*s->abpDia)/3);numeric(870,206,410,88,C_RED,"ABP","mmHg",b,31);
    snprintf(b,sizeof(b),"%d/%d  (%d)",s->papSys,s->papDia,(s->papSys+2*s->papDia)/3);numeric(870,294,410,88,C_YELLOW,"PAP","mmHg",b,31);
    snprintf(b,sizeof(b),"(%d)",s->cvp);numeric(870,382,205,88,C_CYAN,"CVP","mmHg",b,31);
    numeric(1075,382,205,88,C_RED,"UAP","mmHg","120/70 (91)",28);snprintf(b,sizeof(b),"%d",s->etco2);
    numeric(870,470,205,88,C_WHITE,"etCO2","mmHg",b,31);numeric(1075,470,205,88,C_WHITE,"awRR","rpm","30",31);
    numeric(0,558,435,92,C_PINK,"NBP Auto","mmHg   09:01 Pulse 68","120/80 (90)",31);numeric(435,558,435,92,C_GREEN,"STIndx","","1.7",31);
    numeric(870,558,205,92,C_MAGENTA,"ICP","mmHg","(9)",31);numeric(1075,558,205,92,C_WHITE,"imCO2","mmHg","2",31);
    bottom(elapsed,s);softKeys(s);
    EVE_CoDl_display(s_pHalContext);
    if (!EVE_Cmd_waitFlush(s_pHalContext)) return false;
    dlBytes = EVE_Hal_rd32(s_pHalContext, REG_CMD_DL);
    printf("PatientMonitor: DL %lu bytes (%lu words)\n", (unsigned long)dlBytes, (unsigned long)(dlBytes/4));
    fflush(stdout);
    if (dlBytes > 7200U) {
        fprintf(stderr, "PatientMonitor: display list exceeds 1800-word budget\n");
        return false;
    }
    EVE_CoCmd_swap(s_pHalContext);
    return EVE_Cmd_waitFlush(s_pHalContext);
}
static void update(MonitorState *s,uint32_t now) {
    uint32_t n;if((uint32_t)(now-s->lastNumericMs)<1000U)return;s->lastNumericMs=now;n=(now/2000U)%5U;
    s->hr=(int16_t)(60+(int16_t)(n%3U)-1);s->spo2=(int16_t)(95+(int16_t)(n&1U));
    s->abpSys=(int16_t)(120+(int16_t)(n%4U)-2);s->abpDia=(int16_t)(70+(int16_t)(n%3U)-1);
    s->papSys=(int16_t)(28+(int16_t)(n&1U));s->papDia=(int16_t)(15-(int16_t)(n&1U));
    s->cvp=(int16_t)(9+(int16_t)(n%3U)-1);s->etco2=(int16_t)(30+(int16_t)(n%3U)-1);
}
static void release(MonitorState *s,uint8_t tag) {
    printf("PatientMonitor: key release tag=%u\n",(unsigned)tag);
    switch(tag){case PM_TAG_SILENCE:s->silence=(uint8_t)!s->silence;s->lastAction=s->silence?"DEMO: audio alarm silence selected":"DEMO: audio alarm silence cleared";break;
    case PM_TAG_ALARMS_OFF:s->alarmsOff=(uint8_t)!s->alarmsOff;s->lastAction=s->alarmsOff?"DEMO: alarms-off visual state selected":"DEMO: alarms-off visual state cleared";break;
    case PM_TAG_PREVIOUS:s->lastAction="DEMO: Previous selected";break;case PM_TAG_NEXT:s->lastAction="DEMO: Next selected";break;
    case PM_TAG_NBP:s->lastAction="DEMO: Start/Stop NBP selected";break;case PM_TAG_STOP_ALL:s->lastAction="DEMO: Stop All selected";break;
    case PM_TAG_REPEAT:s->lastAction="DEMO: Repeat Time selected";break;case PM_TAG_SIZE:s->lastAction="DEMO: Adjust Size selected";break;
    case PM_TAG_ZERO:s->lastAction="DEMO: Zero selected";break;case PM_TAG_VALUES:s->lastAction="DEMO: Enter Values selected";break;
    case PM_TAG_RECORD:s->lastAction="DEMO: Record selected";break;case PM_TAG_TREND:s->lastAction="DEMO: Vitals Trend selected";break;
    case PM_TAG_SETUP:s->lastAction="DEMO: Main Setup selected";break;case PM_TAG_MAIN:s->lastAction="DEMO: Main Screen selected";break;default:break;}
}
#if defined(_WIN32)
int main(int argc,char *argv[])
#else
int main(void)
#endif
{
    MonitorState s;
    uint32_t now, lastDraw = 0, diagnosticMs, lastFrames = 0, lastUnderrun = 0;
    bool dirty = true, ok = true;
#if defined(_WIN32)
    bool smoke = argc > 1 && strcmp(argv[1], "--smoke") == 0;
#else
    /* Pico crt0 does not supply argc/argv. */
    bool smoke = false;
#endif
#if defined(BT8XXEMU_PLATFORM) && defined(_WIN32)
    bool selfTest = argc > 1 && strcmp(argv[1], "--self-test") == 0;
    uint32_t testMs=0;
    uint8_t testStep=0, testPhase=0;
#endif
    memset(&s,0,sizeof(s));
    setvbuf(stdout, NULL, _IONBF, 0);
    s.hr=60;s.spo2=95;s.abpSys=120;s.abpDia=70;s.papSys=28;s.papDia=15;s.cvp=9;s.etco2=30;
    s.lastAction="Simulated data";
    if (!PatientMonitor_PlatformInit(s_pHalContext)) return 1;
    if(s_pHalContext->Width!=PM_SCREEN_WIDTH||s_pHalContext->Height!=PM_SCREEN_HEIGHT) {
        fprintf(stderr,"PatientMonitor requires 1280x800\n");
        PatientMonitor_PlatformRelease(s_pHalContext);
        return 1;
    }
    s.startMs=EVE_millis();s.lastNumericMs=s.startMs;diagnosticMs=s.startMs;
    PM_WaveInit(s_pHalContext,s.startMs);
    {
        uint8_t i;
        uint32_t table=EVE_Hal_rd32(s_pHalContext,ROMFONT_TABLEADDRESS);
        for(i=0;i<sizeof(s_fontIds);++i)
            EVE_Hal_rdMem(s_pHalContext,(uint8_t *)&s_fonts[s_fontIds[i]-16],
                table+(s_fontIds[i]-16)*GPU_FONT_TABLE_SIZE,GPU_FONT_TABLE_SIZE);
    }
    printf("PatientMonitor: RAM_G wave arrays initialized (%lu bytes)\n", (unsigned long)PM_WAVE_BYTES);
    while(PatientMonitor_PlatformPump()) {
        now=EVE_millis();
        PM_WaveUpdate(s_pHalContext,now);
        s.activeTag=EVE_Hal_rd8(s_pHalContext,REG_TOUCH_TAG);
#if defined(BT8XXEMU_PLATFORM) && defined(_WIN32)
        if (selfTest && (uint32_t)(now-s.startMs)>=1000U &&
            (uint32_t)(now-testMs)>=300U) {
            const SoftKey *key=&s_keys[testStep%14];
            if(testPhase==0) {
                PatientMonitor_PlatformTouch(
                    testStep<14 ? key->x+2 : key->x+key->w-3,
                    testStep<14 ? 754 : 793,1);
                testPhase=1;
            } else if(testPhase==1) {
                if(s.activeTag!=key->tag) {
                    fprintf(stderr,"SELF-TEST: expected tag %u, got %u at edge %u\n",key->tag,s.activeTag,testStep/14);
                    ok=false;break;
                }
                PatientMonitor_PlatformTouch(0,0,false);
                testPhase=2;
            } else {
                if(s.activeTag) {fprintf(stderr,"SELF-TEST: touch did not release\n");ok=false;break;}
                if(++testStep==28) {
                    printf("SELF-TEST PASS: all 14 touch tags at both edges, release and sustained rendering\n");
                    break;
                }
                testPhase=0;
            }
            testMs=now;
        }
#endif
        if (s.activeTag && s.activeTag != s.previousTag) {
            s.pressedTag=s.activeTag; s.pressedMs=now; dirty=true;
        }
        if(s.activeTag==PM_TAG_NONE&&s.previousTag!=PM_TAG_NONE) {
            release(&s,s.previousTag); dirty=true;
        }
        if (!s.activeTag && s.pressedTag && (uint32_t)(now-s.pressedMs)>=100U) {
            s.pressedTag=0;dirty=true;
        }
        if ((uint32_t)(now-s.lastNumericMs)>=1000U) {update(&s,now);dirty=true;}
        if (dirty) {
            ok=frame(&s,(uint32_t)(now-s.startMs));
            if (!ok) {
                char report[129];
                EVE_Hal_rdMem(s_pHalContext, (uint8_t *)report, RAM_ERR_REPORT, 128);
                report[128]=0;
                fprintf(stderr,"PatientMonitor: frame submission failed: %s (DL=%lu)\n",
                    report, (unsigned long)EVE_Hal_rd32(s_pHalContext,REG_CMD_DL));
                break;
            }
            dirty=false; lastDraw=now;
        }
        s.previousTag=s.activeTag;
        if ((uint32_t)(now-diagnosticMs)>=1000U) {
            uint32_t frames=EVE_Hal_rd32(s_pHalContext,REG_FRAMES);
            uint32_t underrun=EVE_Hal_rd32(s_pHalContext,REG_UNDERRUN);
            printf("PatientMonitor: frames=%lu delta=%lu underrun=%lu delta=%lu adaptive=%u\n",
                (unsigned long)frames,(unsigned long)(frames-lastFrames),
                (unsigned long)underrun,(unsigned long)(underrun-lastUnderrun),
                (unsigned)EVE_Hal_rd8(s_pHalContext,REG_ADAPTIVE_FRAMERATE));
            if(lastFrames && frames==lastFrames) fprintf(stderr,"PatientMonitor: frame counter stalled\n");
            lastFrames=frames;lastUnderrun=underrun;diagnosticMs=now;
        }
        if (smoke && (uint32_t)(now-s.startMs)>=4000U) break;
        EVE_sleep(5);
    }
    (void)lastDraw;
    PatientMonitor_PlatformRelease(s_pHalContext);
    return ok ? 0 : 1;
}
