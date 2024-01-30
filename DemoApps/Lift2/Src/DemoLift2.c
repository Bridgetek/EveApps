/**
 * @file DemoLift2.c
 * @brief Lift with video background demo
 *
 * @author Bridgetek
 *
 * @date 2019
 * 
 * MIT License
 *
 * Copyright (c) [2019] [Bridgetek Pte Ltd (BRTChip)]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "Common.h"
#include "Platform.h"
#include "EVE_CoCmd.h"
#include "DemoLift2.h"
#if defined(EVE_SUPPORT_UNICODE)

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void DemoLift2();

// ************************************ main loop ************************************
int main(int argc, char* argv[])
{
	s_pHalContext = &s_halContext;
	Gpu_Init(s_pHalContext);

	// read and store calibration setting
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
	Esd_Calibrate(s_pHalContext);
	Calibration_Save(s_pHalContext);
#endif

	Flash_Init(s_pHalContext, TEST_DIR "/Flash/BT81X_Flash.bin", "BT81X_Flash.bin");
	EVE_Util_clearScreen(s_pHalContext);

	char* info[] =
	{ "Lift with video background demo",
		"Support QVGA, WQVGA, WVGA",
		"EVE3/4",
		"WIN32, FT900"
	};

	while (TRUE) {
		WelcomeScreen(s_pHalContext, info);
		DemoLift2();
		EVE_Util_clearScreen(s_pHalContext);
		EVE_Hal_close(s_pHalContext);
		EVE_Hal_release();

		/* Init HW Hal for next loop*/
		Gpu_Init(s_pHalContext);
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
		Calibration_Restore(s_pHalContext);
#endif
	}
	return 0;
}

// ************************************ application ************************************
#include "DemoLift2.h"
#include "Lift_Control.h"
#include "Lift_DisplayPanel.h"

#define LOAD_FROM_FLASH		1
#define LOAD_FROM_RAM		0

#define delay(x) EVE_sleep(x)

  /* Unicode integration */
#define LIFTFONTSIZE					(19*1024L)
uint8_t fontbuffer[LIFTFONTSIZE];

AppxFont_t sappxfont;

/** @name Global variables for display resolution to support various display panels
 * @note Default is WQVGA - 480x272 
 */
///@{
int16_t gDispWidth = 480;
int16_t gDispHeight = 272;
int16_t gDispHCycle = 548;
int16_t gDispHOffset = 43;
int16_t gDispHSync0 = 0;
int16_t gDispHSync1 = 41;
int16_t gDispVCycle = 292;
int16_t gDispVOffset = 12;
int16_t gDispVSync0 = 0;
int16_t gDispVSync1 = 10;
uint8_t gDispPCLK = 5;
char gDispSwizzle = 0;
char gDispPCLKPol = 1;
char gDispCSpread = 1;
char gDispDither = 1;
///@}

/* Initial boot up DL - make the back ground green color */
const uint8_t FT_DLCODE_BOOTUP[12] =
{ 0, 255, 0, 2, //GPU instruction CLEAR_COLOR_RGB
7, 0, 0, 38, //GPU instruction CLEAR
0, 0, 0, 0,  //GPU instruction DISPLAY
};

uint32_t mediafifo, mediafifolen, videoWidth, videoHeight, videoFrameStatus, filesz, videoIndex;
uint8_t firstFrame = 1, videoHasEnded = 1;

bool Lift_IsVideoEnded()
{
	return videoHasEnded;
}

bool AppLift2_IsAudioFinished()
{
	return true;
}

/* API for video file from flash */
void InitialPlayVideoFlash(EVE_HalContext *pHalContext, LiftBitmap_t *paviframe)
{
	uint32_t currchunk, bytesread;
	EVE_CoCmd_flashSource(s_pHalContext, paviframe->addr);
	videoWidth = paviframe->displaywidth;
	videoHeight = paviframe->displayheight;
	EVE_Cmd_wr32(s_pHalContext, CMD_PLAYVIDEO);
	//NOTE: Adding OPT_NODL option so that CoProcessor will not modify the current RAM_DL content,
	// then later we can write DL commands to render AVI frame on GUI
	EVE_Cmd_wr32(s_pHalContext, OPT_FLASH | OPT_SOUND | OPT_NOTEAR | 128 | OPT_NODL);
	EVE_Cmd_waitFlush(s_pHalContext);
	EVE_sleep(10);  //wait for j1 to consume the avi header and start
						/* Do a dummy command */
	EVE_CoCmd_nop(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);
	firstFrame = 1;
	videoHasEnded = 0;
}

void DemoLift2_InitialVideoSetupFlash(EVE_HalContext *pHalContext, LiftBitmap_t *paviframe)
{
	if (EVE_Hal_rd16(s_pHalContext, REG_CMD_READ) != EVE_Hal_rd16(s_pHalContext, REG_CMD_WRITE)) {
		return;
	}
	/* Starting of the GRAM is utilized for the video frame, 32KIB is the audio data buffer */
	videoFrameStatus = paviframe->displaywidth * paviframe->displayheight * 2L + 32L * 1024;
	mediafifo = videoFrameStatus + 4;
	if (mediafifo > DEMO_ASSETS_RAM_OFFSET) {
		printf("Not enough device space for mediafifo.\n");
	}
	mediafifolen = DEMO_ASSETS_RAM_OFFSET - mediafifo;
	InitialPlayVideoFlash(s_pHalContext, paviframe);
}

/* Load all the assets for  */
int32_t AppLift2_LoadAssetsFromFlash(EVE_HalContext *pHalContext, LiftBitmap_t *pbitmap, uint8_t bitmaphandle)
{
	App_WrDl_Buffer(s_pHalContext, BITMAP_HANDLE(bitmaphandle)); //handle 0 is used for all the characters
	App_WrDl_Buffer(s_pHalContext, BITMAP_SOURCE2(LOAD_FROM_FLASH, pbitmap->addr));
	App_WrDl_Buffer(s_pHalContext, BITMAP_LAYOUT((pbitmap->format) & 0xff, pbitmap->layoutstride, pbitmap->layoutheight));
	App_WrDl_Buffer(s_pHalContext, BITMAP_LAYOUT_H(((pbitmap->layoutstride * 1L) >> 10), ((pbitmap->layoutheight) >> 9)));
	App_WrDl_Buffer(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, pbitmap->displaywidth + 20, pbitmap->displayheight + 20));
	App_WrDl_Buffer(s_pHalContext, BITMAP_SIZE_H(((pbitmap->displaywidth) >> 9), ((pbitmap->displayheight) >> 9)));

	if (GLFORMAT == ((pbitmap->format) & 0xff))
	{
		App_WrDl_Buffer(s_pHalContext, BITMAP_EXT_FORMAT(pbitmap->format >> 8));
	}

	return 0;
}

/* API to get the character width and address */
/* Note that the address is already been generated wrt GPU */
int32_t AppxFontChar32(AppxFont_t *pappxfont, wchar_t ele, uint32_t *paddr, uint8_t *pwidth)
{
	/* calculate the address and width of the character */
	uint32_t elesize = (pappxfont->pfontext->LayoutWidth * pappxfont->pfontext->LayoutHeight) / BITMAP_GPUBLKRESOLN, elenthblk, eleblknthele;
	uint8_t *pbuff = (uint8_t *)pappxfont->pfontext;

	elenthblk = ele / BITMAP_MAX_CELLS;//compiler shall optimize this / and % as 128 is 2^n
	eleblknthele = ele % BITMAP_MAX_CELLS;

	/* Calculate the address in flash */
	pbuff = ((uint32_t *)pappxfont->pglyphoffsets + elenthblk);
	*paddr = pappxfont->pfontext->StartOfGraphicData + (*((uint32_t*)pbuff) / BITMAP_GPUBLKRESOLN) + elesize * eleblknthele;

	/* Calculate the address in ram */
	pbuff = (char*)(pappxfont->pfontext) + (*(pappxfont->pwidthoffsets + elenthblk));
	pbuff += eleblknthele;
	*pwidth = *(pbuff);

	return 0;
}

/*  API to generate the DL for unicode construction of the font */
/* 0 for success and -ve for failure */
/* Assumeing that bitmap configurations are done at the top level */
int32_t AppLift2_FontString(EVE_HalContext *pHalContext, wchar_t *pstring, AppxFont_t *pappxfont, uint8_t fonthandle, int32_t x, int32_t y)
{
	int32_t i, j, hoffset, voffset, stringlen;

	stringlen = wcslen(pstring);
	hoffset = x * 16;
	voffset = y * 16;
	/* xfont can be in either GRAM or in flash */
	int cell = 0;
	for (int32_t i = 0; i<stringlen; i++)
	{
		uint8_t charwidth = 0;
		uint32_t charaddr = 0;
		AppxFontChar32(pappxfont, *pstring++, &charaddr, &charwidth);
		App_WrDl_Buffer(s_pHalContext, BITMAP_SOURCE2(LOAD_FROM_FLASH, charaddr));
		App_WrDl_Buffer(s_pHalContext, VERTEX2F(hoffset, voffset));
		hoffset += charwidth * 16;
	}
	/* To check if the conversion from utf8 to char32 should be done or repying on the compiler */
	return 0;
}

/* Api to construct unicode font - note that whole font table will be in RAM, so for FT900 need to check the RAMsize */
/* As of now the font table will be in MCU RAM, yet to implement font table in GRAM */
int32_t AppxFontInstall(const char *pfilename, char *pbuffer, AppxFont_t *pappxfont)
{
	int status = 0, filelen = 0;
	uint32_t freadlen = 0;

	/* xfont can be in either GRAM or mcu ram */
	FILE *pFile = fopen(pfilename, "rb");
	if (pFile == NULL)
	{
		/* Error in opening the file */
		printf("Error in opening the file %s\n", pfilename);
		return -1;
	}
	fseek(pFile, 0, SEEK_END);
	filelen = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	/* Read the data into the buffer */
	freadlen = fread(pbuffer, 1, filelen, pFile);
	pappxfont->pfontext = (Gpu_FontsExt_t *)pbuffer;
	pappxfont->pglyphoffsets = (int32_t *)((char *)pbuffer + XFONTGLYPHDATAOFFSET);
	pappxfont->pwidthoffsets = pappxfont->pglyphoffsets + ((pappxfont->pfontext->OffsetGlyphData + BITMAP_MAX_CELLS - 1) / BITMAP_MAX_CELLS);
	printf("Install success of font %d %d %d\n", status, pappxfont->pfontext, pappxfont->pwidthoffsets);
	status = fclose(pFile);
	return 0;
}

void Lift_Bootup(Gpu_Hal_Context_t *pHalContext)
{
	/*It is optional to clear the screen here*/
	EVE_Hal_wrMem(s_pHalContext, RAM_DL, (uint8_t *)FT_DLCODE_BOOTUP, sizeof(FT_DLCODE_BOOTUP));
	EVE_Hal_wr8(s_pHalContext, REG_DLSWAP, DLSWAP_FRAME);

	EVE_CoCmd_dlStart(s_pHalContext);
	EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(0, 0, 0));
	EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
	EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 255, 255));
	EVE_Cmd_wr32(s_pHalContext, DISPLAY());
	EVE_CoCmd_swap(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);

	EVE_CoCmd_flashDetach(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);

	EVE_CoCmd_flashAttach(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);

	EVE_CoCmd_flashFast(s_pHalContext, 0);
	EVE_Cmd_waitFlush(s_pHalContext);
}

void DemoLift2() {
	Lift_Bootup(s_pHalContext);
	LiftControl_Init();
	Lift_DispPanel_Init(s_pHalContext);

	//Install custom font
	AppxFontInstall(TEST_DIR "\\housekeeper_auto.xfont", fontbuffer, &sappxfont);

	App_Set_DlBuffer_Index(0);

	EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 255);

	// Add one waiting screen
	Display_Start(s_pHalContext);
	EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
	EVE_CoCmd_text(s_pHalContext, (s_pHalContext->Width / 2), 80, 27, OPT_CENTER, "Loading ...");
	EVE_CoCmd_spinner(s_pHalContext, (s_pHalContext->Width / 2), (s_pHalContext->Height / 2), 0, (float)1.0 / 2);//style 0 and scale 0.5
	Display_End(s_pHalContext);
	EVE_sleep(500);
	
	while (1)
	{
		Lift_DisplayPanel();
		LiftControl_FloorTransition();
		LiftControl_RunStateMachine();
	}
}
#else
#warning Platform is not supported
int main(int argc, char* argv[]) {}
#endif // EVE_SUPPORT_UNICODE