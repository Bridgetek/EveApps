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
#include "PatientMonitorFont.h"
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
    uint8_t silence, alarmsOff, chinese, previousTag, activeTag;
    uint32_t startMs, lastNumericMs, pressedMs;
    uint8_t pressedTag;
    int16_t hr, spo2, abpSys, abpDia, papSys, papDia, cvp, etco2;
    const char *lastActionEn, *lastActionZh;
} MonitorState;
typedef struct SoftKey {
    int16_t x, w; uint8_t tag; uint32_t color;
    const char *line1, *line2, *zh1, *zh2;
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
    {11,74,PM_TAG_SILENCE,C_WARN,"Silence","","静音",""},
    {89,74,PM_TAG_ALARMS_OFF,C_WARN,"Alarms off","","报警","关闭"},
    {167,30,PM_TAG_PREVIOUS,PM_RGB(43,50,56),"<<","","<<",""},
    {201,105,PM_TAG_NBP,PM_RGB(43,50,56),"Start/Stop","NBP","启停","无创血压"},
    {310,92,PM_TAG_STOP_ALL,PM_RGB(43,50,56),"Stop All","","全部停止",""},
    {406,96,PM_TAG_REPEAT,PM_RGB(43,50,56),"Repeat","Time","重复","时间"},
    {506,96,PM_TAG_SIZE,PM_RGB(43,50,56),"Adjust","Size","调整","大小"},
    {606,78,PM_TAG_ZERO,PM_RGB(43,50,56),"Zero","","归零",""},
    {688,96,PM_TAG_VALUES,PM_RGB(43,50,56),"Enter","Values","输入","数值"},
    {788,90,PM_TAG_RECORD,PM_RGB(43,50,56),"Record","","记录",""},
    {882,96,PM_TAG_TREND,PM_RGB(43,50,56),"Vitals","Trend","生命体征","趋势"},
    {982,30,PM_TAG_NEXT,PM_RGB(43,50,56),">>","",">>",""},
    {1016,122,PM_TAG_SETUP,C_BLUE,"Main Setup","","主设置",""},
    {1142,127,PM_TAG_MAIN,C_BLUE,"Main Screen","","主屏幕",""}
};

static void uiText(const MonitorState *s,int16_t x,int16_t y,int16_t font,
    uint16_t options,const char *english,const char *chinese)
{
    if(s->chinese && chinese && strcmp(english,chinese)!=0)
        EVE_CoCmd_text(s_pHalContext,x,y,PM_CHINESE_HANDLE,options,chinese);
    else
        pmText(s_pHalContext,x,y,font,options,english);
}

static void color(uint32_t c) { EVE_CoDl_colorRgb(s_pHalContext,(uint8_t)(c>>16),(uint8_t)(c>>8),(uint8_t)c); }
static void rect(int16_t x,int16_t y,int16_t w,int16_t h,uint32_t c) {
    color(c); EVE_CoDl_begin(s_pHalContext,RECTS); EVE_CoDl_vertex2f(s_pHalContext,x,y);
    EVE_CoDl_vertex2f(s_pHalContext,x+w,y+h); EVE_CoDl_end(s_pHalContext);
}
static void line(int16_t x0,int16_t y0,int16_t x1,int16_t y1,uint32_t c) {
    color(c); EVE_CoDl_lineWidth(s_pHalContext,16); EVE_CoDl_begin(s_pHalContext,LINES);
    EVE_CoDl_vertex2f(s_pHalContext,x0,y0); EVE_CoDl_vertex2f(s_pHalContext,x1,y1); EVE_CoDl_end(s_pHalContext);
}
static void drawWave(const MonitorState *s,uint8_t lane,int16_t y,uint32_t c,
    const char *label,const char *labelZh,const char *note,const char *noteZh,uint32_t elapsed) {
    (void)elapsed;
    line(0,y+87,870,y+87,PM_RGB(18,23,27));
    PM_WaveDraw(s_pHalContext, lane, y, c);
    color(c);
    uiText(s,7,y+5,26,0,label,labelZh);
    if(note[0]){color(C_DIM);uiText(s,860,y+5,20,OPT_RIGHTX,note,noteZh);}
}
static void numeric(const MonitorState *s,int16_t x,int16_t y,int16_t w,int16_t h,
    uint32_t c,const char *label,const char *labelZh,const char *units,const char *value,uint8_t font) {
    rect(x,y,w,h,C_BLACK);line(x,y+h-1,x+w,y+h-1,PM_RGB(18,23,27));color(c);
    uiText(s,x+7,y+5,21,0,label,labelZh);color(C_DIM);
    pmText(s_pHalContext,x+7,y+(s->chinese?27:24),18,0,units);color(c);
    pmText(s_pHalContext,x+w-8,y+h/2,font,OPT_RIGHTX|OPT_CENTERY,value);
}
static void status(const MonitorState *s) {
    rect(0,0,1280,30,PM_RGB(233,237,240));color(PM_RGB(17,17,17));
    uiText(s,10,15,21,OPT_CENTERY,"bed-04","病床-04");
    uiText(s,330,15,21,OPT_CENTER,"No Patient - DEMO","无患者 - 演示");
    uiText(s,650,15,21,OPT_CENTER,"Adult","成人");
    uiText(s,900,15,21,OPT_CENTER,"6 May 2018 09:07","2018年5月6日 09:07");
    EVE_CoDl_tagMask(s_pHalContext,1);EVE_CoDl_tag(s_pHalContext,PM_TAG_LANGUAGE);
    rect(1040,2,100,26,PM_RGB(220,226,232));color(PM_RGB(17,17,17));
    if(s->chinese) pmText(s_pHalContext,1090,15,20,OPT_CENTER,"English");
    else EVE_CoCmd_text(s_pHalContext,1090,15,PM_CHINESE_HANDLE,OPT_CENTER,"中文");
    EVE_CoDl_tagMask(s_pHalContext,0);EVE_CoDl_tag(s_pHalContext,0);
    uiText(s,1270,15,21,OPT_RIGHTX|OPT_CENTERY,"SSC Sepsis","脓毒症");
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
    uiText(s,16,689,20,0,"Sepsis Management Bundle","脓毒症管理方案");color(C_DIM);
    uiText(s,16,707,16,0,"Demo profile - no clinical guidance","演示模式 - 非临床指导");
    uiText(s,16,725,17,0,s->lastActionEn,s->lastActionZh);
    color(C_WHITE);pmText(s_pHalContext,458,716,31,OPT_CENTER,t);
    color(C_DIM);pmText(s_pHalContext,530,692,17,0,"NBPm");pmText(s_pHalContext,850,692,17,0,"CVPm");
    uiText(s,760,737,17,OPT_CENTERY,"-4 hrs","-4 小时");
    uiText(s,1180,737,17,OPT_CENTERY,"-2 hrs","-2 小时");
    line(540,716,1255,716,C_LINE);rect(700,706,38,12,C_PINK);rect(820,721,300,13,PM_RGB(14,106,115));rect(1120,712,45,22,C_CYAN);
}
static void softKeys(const MonitorState *s) {
    uint8_t i;rect(8,752,1264,44,PM_RGB(21,25,29));EVE_CoDl_tagMask(s_pHalContext,1);
    for(i=0;i<(uint8_t)(sizeof(s_keys)/sizeof(s_keys[0]));i++){const SoftKey *k=&s_keys[i];
        uint8_t sel=(uint8_t)(s->pressedTag==k->tag||(k->tag==PM_TAG_SILENCE&&s->silence)||(k->tag==PM_TAG_ALARMS_OFF&&s->alarmsOff));
        EVE_CoDl_tag(s_pHalContext,k->tag);rect(k->x,752,k->w,44,sel?C_WHITE:k->color);color((k->color==C_WARN||sel)?C_BLACK:C_TEXT);
        if(s->chinese?k->zh2[0]:k->line2[0]){
            uiText(s,k->x+k->w/2,765,19,OPT_CENTER,k->line1,k->zh1);
            uiText(s,k->x+k->w/2,783,19,OPT_CENTER,k->line2,k->zh2);
        } else uiText(s,k->x+k->w/2,774,19,OPT_CENTER,k->line1,k->zh1);}
    EVE_CoDl_tagMask(s_pHalContext,0);EVE_CoDl_tag(s_pHalContext,0);
}
static bool frame(const MonitorState *s,uint32_t elapsed) {
    char b[32]; uint32_t dlBytes; uint8_t i;
    EVE_CoCmd_dlStart(s_pHalContext);EVE_CoDl_clearColorRgb(s_pHalContext,0,0,0);EVE_CoDl_clear(s_pHalContext,1,1,1);EVE_CoDl_vertexFormat(s_pHalContext,0);
    EVE_CoCmd_setFont2(s_pHalContext,PM_CHINESE_HANDLE,PM_FONT_XFONT_ADDR,0);
    for(i=0;i<sizeof(s_fontIds);++i) EVE_CoCmd_romFont(s_pHalContext,s_fontIds[i],s_fontIds[i]);
    status(s);drawWave(s,0,30,C_GREEN,"II","II","SV Rhythm","窦性心律",elapsed);
    drawWave(s,1,118,C_CYAN,"Pleth","脉搏波","","",elapsed);
    drawWave(s,2,206,C_RED,"ABP","ABP","150 / 50","150 / 50",elapsed);
    drawWave(s,3,294,C_YELLOW,"PAP","PAP","30 / 0","30 / 0",elapsed);
    drawWave(s,4,382,C_CYAN,"CVP","CVP","30 / 0","30 / 0",elapsed);
    drawWave(s,5,470,C_WHITE,"CO2","CO2","60","60",elapsed);
    snprintf(b,sizeof(b),"%d",s->hr);numeric(s,870,30,205,88,C_GREEN,"HR","HR","bpm",b,31);stGrid();
    snprintf(b,sizeof(b),"%d",s->spo2);numeric(s,870,118,205,88,C_CYAN,"SpO2","SpO2","%",b,31);
    snprintf(b,sizeof(b),"%d",s->hr);numeric(s,1075,118,205,88,C_CYAN,"Pulse","脉搏","bpm",b,31);
    snprintf(b,sizeof(b),"%d/%d  (%d)",s->abpSys,s->abpDia,(s->abpSys+2*s->abpDia)/3);
    numeric(s,870,206,410,88,C_RED,"ABP","ABP","mmHg",b,31);
    snprintf(b,sizeof(b),"%d/%d  (%d)",s->papSys,s->papDia,(s->papSys+2*s->papDia)/3);
    numeric(s,870,294,410,88,C_YELLOW,"PAP","PAP","mmHg",b,31);
    snprintf(b,sizeof(b),"(%d)",s->cvp);numeric(s,870,382,205,88,C_CYAN,"CVP","CVP","mmHg",b,31);
    numeric(s,1075,382,205,88,C_RED,"UAP","UAP","mmHg","120/70 (91)",28);
    snprintf(b,sizeof(b),"%d",s->etco2);
    numeric(s,870,470,205,88,C_WHITE,"etCO2","etCO2","mmHg",b,31);
    numeric(s,1075,470,205,88,C_WHITE,"awRR","awRR","rpm","30",31);
    numeric(s,0,558,435,92,C_PINK,"NBP Auto","无创血压 自动","mmHg   09:01 HR 68","120/80 (90)",31);
    numeric(s,435,558,435,92,C_GREEN,"STIndx","ST指数","","1.7",31);
    numeric(s,870,558,205,92,C_MAGENTA,"ICP","ICP","mmHg","(9)",31);
    numeric(s,1075,558,205,92,C_WHITE,"imCO2","imCO2","mmHg","2",31);
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
static void setAction(MonitorState *s,const char *english,const char *chinese) {
    s->lastActionEn=english;s->lastActionZh=chinese;
}
static void release(MonitorState *s,uint8_t tag) {
    printf("PatientMonitor: key release tag=%u\n",(unsigned)tag);
    switch(tag){
    case PM_TAG_SILENCE:
        s->silence=(uint8_t)!s->silence;
        setAction(s,s->silence?"DEMO: audio alarm silence selected":"DEMO: audio alarm silence cleared",
            s->silence?"演示：已选择静音":"演示：已清除静音");break;
    case PM_TAG_ALARMS_OFF:
        s->alarmsOff=(uint8_t)!s->alarmsOff;
        setAction(s,s->alarmsOff?"DEMO: alarms-off visual state selected":"DEMO: alarms-off visual state cleared",
            s->alarmsOff?"演示：已关闭报警":"演示：已开启报警");break;
    case PM_TAG_PREVIOUS:setAction(s,"DEMO: Previous selected","演示：已选择上一项");break;
    case PM_TAG_NEXT:setAction(s,"DEMO: Next selected","演示：已选择下一项");break;
    case PM_TAG_NBP:setAction(s,"DEMO: Start/Stop NBP selected","演示：已选择无创血压启停");break;
    case PM_TAG_STOP_ALL:setAction(s,"DEMO: Stop All selected","演示：已选择全部停止");break;
    case PM_TAG_REPEAT:setAction(s,"DEMO: Repeat Time selected","演示：已选择重复时间");break;
    case PM_TAG_SIZE:setAction(s,"DEMO: Adjust Size selected","演示：已选择调整大小");break;
    case PM_TAG_ZERO:setAction(s,"DEMO: Zero selected","演示：已选择归零");break;
    case PM_TAG_VALUES:setAction(s,"DEMO: Enter Values selected","演示：已选择输入数值");break;
    case PM_TAG_RECORD:setAction(s,"DEMO: Record selected","演示：已选择记录");break;
    case PM_TAG_TREND:setAction(s,"DEMO: Vitals Trend selected","演示：已选择生命体征趋势");break;
    case PM_TAG_SETUP:setAction(s,"DEMO: Main Setup selected","演示：已选择主设置");break;
    case PM_TAG_MAIN:setAction(s,"DEMO: Main Screen selected","演示：已选择主屏幕");break;
    case PM_TAG_LANGUAGE:s->chinese=(uint8_t)!s->chinese;break;
    default:break;}
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
    bool smoke = argc > 1 && (strcmp(argv[1], "--smoke") == 0 ||
        strcmp(argv[1], "--smoke-zh") == 0);
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
    setAction(&s,"Simulated data","模拟数据");
#if defined(_WIN32)
    s.chinese=(uint8_t)(argc>1 && strcmp(argv[1],"--smoke-zh")==0);
#endif
    if (!PatientMonitor_PlatformInit(s_pHalContext)) return 1;
    if(s_pHalContext->Width!=PM_SCREEN_WIDTH||s_pHalContext->Height!=PM_SCREEN_HEIGHT) {
        fprintf(stderr,"PatientMonitor requires 1280x800\n");
        PatientMonitor_PlatformRelease(s_pHalContext);
        return 1;
    }
    s.startMs=EVE_millis();s.lastNumericMs=s.startMs;diagnosticMs=s.startMs;
    PM_WaveInit(s_pHalContext,s.startMs);
    if (!PM_FontLoad(s_pHalContext)) {
        PatientMonitor_PlatformRelease(s_pHalContext);
        return 1;
    }
    {
        uint8_t i;
        uint32_t table=EVE_Hal_rd32(s_pHalContext,ROMFONT_TABLEADDRESS);
        for(i=0;i<sizeof(s_fontIds);++i)
            EVE_Hal_rdMem(s_pHalContext,(uint8_t *)&s_fonts[s_fontIds[i]-16],
                table+(s_fontIds[i]-16)*GPU_FONT_TABLE_SIZE,GPU_FONT_TABLE_SIZE);
    }
    printf("PatientMonitor: RAM_G wave arrays initialized (%lu bytes)\n", (unsigned long)PM_WAVE_BYTES);
    /* Dirty list rebuild with live RAM_G waves; see docs/best-practices/display-list-update-strategies.md. */
    while(PatientMonitor_PlatformPump()) {
        now=EVE_millis();
        PM_WaveUpdate(s_pHalContext,now);
        s.activeTag=EVE_Hal_rd8(s_pHalContext,REG_TOUCH_TAG);
#if defined(BT8XXEMU_PLATFORM) && defined(_WIN32)
        if (selfTest && (uint32_t)(now-s.startMs)>=1000U &&
            (uint32_t)(now-testMs)>=300U) {
            const SoftKey *key=testStep<28?&s_keys[testStep%14]:NULL;
            if(testPhase==0) {
                PatientMonitor_PlatformTouch(
                    key?(testStep<14 ? key->x+2 : key->x+key->w-3):1090,
                    key?(testStep<14 ? 754 : 793):15,1);
                testPhase=1;
            } else if(testPhase==1) {
                uint8_t expected=key?key->tag:PM_TAG_LANGUAGE;
                if(s.activeTag!=expected) {
                    fprintf(stderr,"SELF-TEST: expected tag %u, got %u at step %u\n",
                        expected,s.activeTag,testStep);
                    ok=false;break;
                }
                PatientMonitor_PlatformTouch(0,0,false);
                testPhase=2;
            } else if(testPhase==2) {
                if(s.activeTag) {fprintf(stderr,"SELF-TEST: touch did not release\n");ok=false;break;}
                if(key){++testStep;testPhase=0;}
                else testPhase=3;
            } else {
                if(s.chinese!=(uint8_t)(testStep==28)) {
                    fprintf(stderr,"SELF-TEST: language toggle failed at step %u\n",testStep);
                    ok=false;break;
                }
                if(++testStep==30) {
                    printf("SELF-TEST PASS: 14 key tags at both edges, release, language toggle and sustained rendering\n");
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
