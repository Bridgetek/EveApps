/**
 * @file Video.c
 * @brief Sample usage of video display
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
#include "Video.h"

#define SAMAPP_INFO_TEXT(str)  Draw_TextColor(s_pHalContext, str, (uint8_t[]) { 0x77, 0x77, 0x77 }, (uint8_t[]) { 255, 255, 255 })
#define SAMAPP_INFO_START      Display_StartColor(s_pHalContext,  (uint8_t[]) { 0x77, 0x77, 0x77 }, (uint8_t[]) { 255, 255, 255 })
#define SAMAPP_INFO_END        Display_End(s_pHalContext);
#define SAMAPP_DELAY_NEXT      EVE_sleep(2000);

#define SCRATCH_BUFF_SZ        (1024*4)

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void SAMAPP_Video();

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

    char *info[] =
    {  "EVE Sample Application",
        "This sample demonstrate the using of video", 
        "",
        ""
    }; 

    while (TRUE) {
        WelcomeScreen(s_pHalContext, info);

        SAMAPP_Video();

        EVE_Util_clearScreen(s_pHalContext);

        Gpu_Release(s_pHalContext);

        /* Init HW Hal for next loop*/
        Gpu_Init(s_pHalContext);
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
        Calibration_Restore(s_pHalContext);
#endif
    }

    return 0;
}

/**
* @brief API to demonstrate Video display from Flash
*
*/
void SAMAPP_Video_fromFlash()
{
#if defined(EVE_FLASH_AVAILABLE) && (EVE_SUPPORT_GEN >= EVE3)
    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }
    Draw_Text(s_pHalContext, "Example for: Video display from Flash");

    SAMAPP_INFO_START;
    EVE_CoCmd_text(s_pHalContext, 0, 150, 30, 0, "Video display from Flash");
    EVE_CoCmd_flashSource(s_pHalContext, 4096);
	EVE_CoCmd_playVideo(s_pHalContext, OPT_FLASH | OPT_SOUND | OPT_NOTEAR | OPT_OVERLAY);
    
    EVE_Cmd_waitFlush(s_pHalContext); //Video plays after this
    EVE_CoCmd_nop(s_pHalContext);
    EVE_Cmd_waitFlush(s_pHalContext);
    
    SAMAPP_DELAY_NEXT;
#endif // defined (BT81X_ENABLE)
}

/**
* @brief AVI video playback from file via CMD buffer
*
*/
void SAMAPP_Video_fromFile()
{
#if EVE_SUPPORT_GEN >= EVE3
    Draw_Text(s_pHalContext, "Example for: Video display from file");

    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 155);

    APP_INF("Video playback starts.\n");
	EVE_CoCmd_playVideo(s_pHalContext, OPT_SOUND | OPT_NOTEAR);
	EVE_Util_loadCmdFile(s_pHalContext, TEST_DIR "\\Big buck bunny 240p 40s  adpcm_ima_wav.avi", NULL);

    //helperStopVideoCmdFifo();
    //EVE_Cmd_restore(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext); //Video plays after this
	EVE_CoCmd_nop(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);
#endif 
}

/**
* @brief Test AVI video playback full screen from flash
*
*/
void SAMAPP_Video_fromFlashFullScreen()
{
#if defined(EVE_FLASH_AVAILABLE) && (EVE_SUPPORT_GEN >= EVE3)
    const uint32_t flashSource = 4096;

    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }
    Draw_Text(s_pHalContext, "Example for: Video display from Flash - full screen ");

    SAMAPP_INFO_START;
    EVE_CoCmd_flashSource(s_pHalContext, flashSource);
    EVE_CoCmd_playVideo(s_pHalContext, OPT_FULLSCREEN | OPT_FLASH | OPT_SOUND | OPT_NOTEAR);

    EVE_Cmd_waitFlush(s_pHalContext); //Video plays after this
    EVE_CoCmd_nop(s_pHalContext);
    EVE_Cmd_waitFlush(s_pHalContext);
    SAMAPP_DELAY_NEXT;
#endif
}

/**
* @brief Test AVI video playback via REG_CMDB_WRITE/REG_CMDB_SPACE
*
*/
void SAMAPP_Video_fromCMDB()
{
#if EVE_SUPPORT_GEN >= EVE2
    Draw_Text(s_pHalContext, "Example for: Video display via REG_CMDB_WRITE/REG_CMDB_SPACE");

    SAMAPP_INFO_START;
	EVE_CoDl_bitmapHandle(s_pHalContext, 0);
    EVE_CoDl_bitmapSource(s_pHalContext, 0);
    EVE_CoDl_bitmapLayout(s_pHalContext, RGB565, 854 * 2L, 480);
    EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, 854, 480);

    EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_bitmapHandle(s_pHalContext, 0);
    EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
	EVE_CoDl_end(s_pHalContext);
    SAMAPP_INFO_END;

	EVE_CoCmd_playVideo(s_pHalContext, OPT_NODL);
	EVE_Util_loadCmdFile(s_pHalContext, TEST_DIR "\\chickens-4.avi", NULL);

	EVE_Cmd_waitFlush(s_pHalContext); //Video plays after this
#if EVE_SUPPORT_GEN >= EVE3
	EVE_CoCmd_nop(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);
#endif
#endif
}

void SAMAPP_Video_fromCMDBuffer()
{
#if EVE_SUPPORT_GEN >= EVE3
#define LOGO_ADDRESS (854 * 480 * 2)
#define PNG_W 236
#define PNG_H 72

	Draw_Text(s_pHalContext, "Example for: Video display via command buffer");

    EVE_Util_loadImageFile(s_pHalContext, LOGO_ADDRESS, TEST_DIR "\\images-logo.png", NULL, OPT_NODL);

	SAMAPP_INFO_START;
	EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_bitmapHandle(s_pHalContext, 0);
	EVE_CoCmd_setBitmap(s_pHalContext, 0, RGB565, 854, 480);
	EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
	EVE_CoDl_bitmapHandle(s_pHalContext, 1);
	EVE_CoCmd_setBitmap(s_pHalContext, LOGO_ADDRESS, ARGB4, PNG_W, PNG_H);
	EVE_CoDl_vertex2f(s_pHalContext, VP(100), VP(100));
	EVE_CoDl_end(s_pHalContext);
	SAMAPP_INFO_END;

	EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 155);

	APP_DBG("Video playback starts.\n");
	EVE_CoCmd_playVideo(s_pHalContext, OPT_NODL);
	EVE_Util_loadCmdFile(s_pHalContext, TEST_DIR "\\chickens-4.avi", NULL);

	EVE_Cmd_waitFlush(s_pHalContext);
	EVE_CoCmd_nop(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);
#endif
}

/**
* @brief API to demonstrate Video display frame by frame from Flash
*
*/
void SAMAPP_Video_frameByFrameFromFlash()
{
#if defined(EVE_FLASH_AVAILABLE) && (EVE_SUPPORT_GEN >= EVE3)
    const uint32_t flashSource = 4096;

    // video settings
    const uint32_t videoW = 462;
    const uint32_t videoH = 240;
    const uint32_t completionPtr = 0;

    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }
    Draw_Text(s_pHalContext, "Example for: Video display frame by frame from Flash");

    Display_Start(s_pHalContext);
    EVE_CoDl_bitmapHandle(s_pHalContext, 1);
	EVE_CoCmd_setBitmap(s_pHalContext, 8, RGB565, videoW, videoH);
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
    EVE_CoDl_end(s_pHalContext);
    Display_End(s_pHalContext);

    EVE_CoCmd_flashSource(s_pHalContext, flashSource);
    EVE_CoCmd_videoStartF(s_pHalContext);
    do
    {
        EVE_CoCmd_videoFrame(s_pHalContext, 8, completionPtr);
        EVE_Cmd_waitFlush(s_pHalContext);
        EVE_sleep(16); // wait 16ms to next frame, so maximum FPS ~= 1000 / 16 = ~60fps
    }while (EVE_Hal_rd32(s_pHalContext, completionPtr) != 0);
#endif
}

/**
* @brief video playback via frame by frame from Mediafifo
*
*/
void SAMAPP_Video_frameByFrameMediafifo()
{
#if EVE_SUPPORT_GEN >= EVE2
    uint16_t aviw = 854;
    uint16_t avih = 480;
	uint16_t videoFrameStatusAddr = RAM_G; //the 4 byte address for the videoframe status;
    uint32_t mediafifo;
    uint32_t mediafifolen;
	uint32_t transfered = 0;

    Draw_Text(s_pHalContext, "Example for: Video display frame by frame from Mediafifo");

    /* start video playback, load the data into media fifo */
    mediafifo = aviw * avih * 2L + 32L + 4; //the starting address of the media fifo, the begining space is for the decoded video frame,
    mediafifolen = RAM_G_SIZE - mediafifo;
	EVE_MediaFifo_set(s_pHalContext, mediafifo, mediafifolen); //address of the media fifo buffer
    printf("Mediafifo: Start address and length %d %d\n", mediafifo, mediafifolen);

    SAMAPP_INFO_START;
    EVE_CoDl_bitmapHandle(s_pHalContext, 0);
	EVE_CoCmd_setBitmap(s_pHalContext, 4, RGB565, aviw, avih);
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
	EVE_CoDl_end(s_pHalContext);
    SAMAPP_INFO_END;

    EVE_CoCmd_videoStart(s_pHalContext); //initialize AVI video decoder
	EVE_Util_loadMediaFile(s_pHalContext, TEST_DIR "\\chickens-4.avi", &transfered);
	do
	{
		EVE_CoCmd_videoFrame(s_pHalContext, 4, videoFrameStatusAddr);
		EVE_Cmd_waitFlush(s_pHalContext);
		if (s_pHalContext->LoadFileRemaining > 0)
			EVE_Util_loadMediaFile(s_pHalContext, NULL, &transfered);
	} while ((EVE_Hal_rd32(s_pHalContext, videoFrameStatusAddr) != 0) && (EVE_MediaFifo_space(s_pHalContext) != mediafifolen - 4));

    EVE_Cmd_waitFlush(s_pHalContext);
#if EVE_SUPPORT_GEN >= EVE3
	EVE_CoCmd_nop(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);
#endif
	EVE_MediaFifo_close(s_pHalContext);
#endif // FT81X
}

/**
* @brief API to demonstrate AVI video playback with ASTC overlay
*
*/
void SAMAPP_Video_ASTCOverlay()
{
#if defined(EVE_FLASH_AVAILABLE) && (EVE_SUPPORT_GEN >= EVE3)
#define ASTC_LOGO_FLASH_ADDR  (4096)

    Draw_Text(s_pHalContext, "Example for: Video display with ASTC overlay");
	EVE_Util_clearScreen(s_pHalContext);

    // Add one waiting screen while programming flash the logo image file
    SAMAPP_INFO_START;
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 80, 27, OPT_CENTER,
        "Loading video ...");
    //style 0 and scale 0.5
    EVE_CoCmd_spinner(s_pHalContext, (int16_t) (s_pHalContext->Width / 2),
        (int16_t) (s_pHalContext->Height / 2), 0, (int16_t) (float) 1.0 / 2);
	EVE_Cmd_waitFlush(s_pHalContext);
    EVE_sleep(500);

    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }
    char* files = TEST_DIR "\\Logo_480x272_480x272_COMPRESSED_RGBA_ASTC_4x4_KHR.raw";
    if (0 >= Ftf_Write_File_To_Flash_By_RAM_G(s_pHalContext, files, ASTC_LOGO_FLASH_ADDR))
    {
        return;
    }
	EVE_CoCmd_stop(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);

    SAMAPP_INFO_START;
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_bitmapHandle(s_pHalContext, 0);
	EVE_CoCmd_setBitmap(s_pHalContext, 0, RGB565, 854, 480);
    EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
	EVE_CoDl_bitmapHandle(s_pHalContext, 1);
	EVE_CoCmd_setBitmap(s_pHalContext, 0x800000 | (ASTC_LOGO_FLASH_ADDR / 32),
	    COMPRESSED_RGBA_ASTC_4x4_KHR, 480, 272);
    EVE_CoDl_vertex2f(s_pHalContext, 0, VP(100));
	EVE_CoDl_end(s_pHalContext);
    SAMAPP_INFO_END;

    EVE_CoCmd_playVideo(s_pHalContext, OPT_NODL);
	EVE_Util_loadCmdFile(s_pHalContext, TEST_DIR "\\chickens-4.avi", NULL);

	EVE_Cmd_waitFlush(s_pHalContext);
	EVE_CoCmd_nop(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);
#endif
}

/**
* @brief Video playback with audio
*
*/
void SAMAPP_Video_audioEnabled()
{
#if EVE_SUPPORT_GEN >= EVE2
    uint16_t aviw = 462;
    uint16_t avih = 240;
    uint32_t mediafifo;
    uint32_t mediafifolen;

    Draw_Text(s_pHalContext, "Example for: Video display with audio enable");

    /* start video playback, load the data into media fifo */
    mediafifo = aviw * avih * 2L + 32 * 1024L; //starting address of the media fifo
    //how much memory will be allocated for the video playback fifo
    mediafifolen = RAM_G_SIZE - mediafifo;

    //address of the media fifo buffer
	EVE_MediaFifo_set(s_pHalContext, mediafifo, mediafifolen);
    printf("Mediafifo: Start address and length %d %d\n", mediafifo, mediafifolen);

    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 155);
	EVE_CoCmd_playVideo(s_pHalContext, OPT_MEDIAFIFO | OPT_NOTEAR | OPT_SOUND | OPT_FULLSCREEN);
	EVE_Util_loadMediaFile(s_pHalContext, TEST_DIR "\\Big buck bunny 240p 40s  adpcm_ima_wav.avi", NULL);

	EVE_Cmd_waitFlush(s_pHalContext);
#if EVE_SUPPORT_GEN >= EVE3
	EVE_CoCmd_nop(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);
#endif
	EVE_MediaFifo_close(s_pHalContext);
#endif
}

/**
* @brief Video playback frame by frame with pause/resume button
*
*/
void SAMAPP_Video_pauseResumeFBF()
{
#if defined(EVE_FLASH_AVAILABLE) && (EVE_SUPPORT_GEN >= EVE3)
    const uint32_t flashSource = 4096;

    // video 1st settings
    const uint32_t videoW = 462;
    const uint32_t videoH = 240;
    const uint32_t videoX = s_pHalContext->Width / 2 - videoW / 2;
    const uint32_t videoY = s_pHalContext->Height / 2 - videoH / 2;
    const uint32_t videoSource = 0;
    const uint32_t completionPtr = 0;
    static uint8_t isPause = 0;
    const uint8_t btnPauseTag = 1;
    uint8_t txtPause[][20] = { "RESUME", "PAUSE" };
    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }
    Draw_Text(s_pHalContext, "Example for: Video playback frame by frame with pause/resume button");
    for (int i = 0; i < 2; i++) {
        Display_Start(s_pHalContext);
        // video 
        EVE_CoDl_bitmapHandle(s_pHalContext, 1);
		EVE_CoCmd_setBitmap(s_pHalContext, videoSource, RGB565, videoW, videoH);
        EVE_CoDl_begin(s_pHalContext, BITMAPS);
        EVE_CoDl_vertex2f(s_pHalContext, VP(videoX), VP(videoY));
        EVE_CoDl_end(s_pHalContext);

        /*** Show a button ***/
        uint32_t btnW = 200;
        uint32_t btnH = 50;
        uint32_t btnX = s_pHalContext->Width/2 - btnW/2;
        uint32_t btnY = s_pHalContext->Height - btnH - 10;
        EVE_CoDl_tag(s_pHalContext, btnPauseTag);
        EVE_CoCmd_button(s_pHalContext, btnX, btnY, btnW, btnH, 30, 0, txtPause[i]);
        /*** Done button ***/
        Display_End(s_pHalContext);
    }
    
    EVE_CoCmd_flashSource(s_pHalContext, flashSource);
    EVE_CoCmd_videoStartF(s_pHalContext);
    do
    {
        Gesture_Renew(s_pHalContext);
        if (Gesture_Get()->tagReleased == btnPauseTag) {
            isPause = !isPause;
            EVE_CoCmd_swap(s_pHalContext);
            EVE_Cmd_waitFlush(s_pHalContext);
        }

        if (!isPause) {
            EVE_CoCmd_videoFrame(s_pHalContext, videoSource, completionPtr);
            EVE_Cmd_waitFlush(s_pHalContext);
        }
    }while (EVE_Hal_rd32(s_pHalContext, completionPtr) != 0);
#endif
}


/**
* @brief Video playback with pause/resume button
*
*/
void SAMAPP_Video_pauseResumeWithAudio()
{
#if defined(EVE_FLASH_AVAILABLE) && (EVE_SUPPORT_GEN >= EVE3)
    const uint32_t flashSource = 4096;

    const uint32_t videoW = 462;
    const uint32_t videoH = 240;
    const uint32_t videoX = s_pHalContext->Width / 2 - videoW / 2;
    const uint32_t videoY = s_pHalContext->Height / 2 - videoH / 2;
    const uint32_t videoSource = 0;
    const uint32_t txtOffset = RAM_G_SIZE - 1024;
    static uint8_t isPause = 0;
    const uint8_t btnPauseTag = 1;
    uint8_t txtPause[][20] = { "RESUME\n", "PAUSE\n" };

    Draw_Text(s_pHalContext, "Example for: Video+audio playback with pause/resume button");

    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }

    SAMAPP_INFO_START;
    // video
    EVE_CoDl_bitmapHandle(s_pHalContext, 1);
	EVE_CoCmd_setBitmap(s_pHalContext, videoSource, RGB565, videoW, videoH);
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoDl_vertex2f(s_pHalContext, VP(videoX), VP(videoY));
    EVE_CoDl_end(s_pHalContext);

    /*** Show a button ***/
    uint32_t btnW = 300;
    uint32_t btnH = 50;
    uint32_t btnX = s_pHalContext->Width / 2 - btnW / 2;
    uint32_t btnY = s_pHalContext->Height - btnH - 10;
    EVE_CoDl_tag(s_pHalContext, btnPauseTag);
    EVE_CoCmd_button(s_pHalContext, btnX, btnY, btnW, btnH, 30, 0, "Pause/Resume");
    /*** Done button ***/
    SAMAPP_INFO_END;
   
    EVE_CoCmd_flashSource(s_pHalContext, flashSource);
	EVE_CoCmd_playVideo(s_pHalContext, OPT_NODL | OPT_FLASH | OPT_SOUND | OPT_NOTEAR | OPT_OVERLAY);
	while (EVE_Cmd_space(s_pHalContext) != (EVE_CMD_FIFO_SIZE - 4))
    {
        Gesture_Renew(s_pHalContext);
        if (Gesture_Get()->tagReleased == btnPauseTag) {
            isPause = !isPause;
            EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, isPause?0:1); // stop the video
        }
    }
	EVE_Cmd_waitFlush(s_pHalContext);
	EVE_Hal_wr32(s_pHalContext, REG_PLAY_CONTROL, 1); // restore default value
#endif
}

/**
* @brief Video playback with pause/resume button
*
*/
void SAMAPP_Video_pauseResumeWithAudio_buttonchange()
{
#if defined(EVE_FLASH_AVAILABLE) && (EVE_SUPPORT_GEN >= EVE3)
    const uint32_t flashSource = 4096;

    const uint32_t videoW = 462;
    const uint32_t videoH = 240;
    const uint32_t videoX = s_pHalContext->Width / 2 - videoW / 2;
    const uint32_t videoY = s_pHalContext->Height / 2 - videoH / 2;
    const uint32_t videoSource = 0;
    const uint32_t txtOffset = RAM_G_SIZE - 1024;
    static uint8_t isPause = 0;
    const uint8_t btnPauseTag = 1;
    uint8_t txtPause[][20] = { "RESUME", "PAUSE" };

    Draw_Text(s_pHalContext, "Example for: Video+audio playback with pause/resume button(button change)");

    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }

	for (int i = 0; i < 2; i++)
	{
		Display_Start(s_pHalContext);
		// video
		EVE_CoDl_bitmapHandle(s_pHalContext, 1);
		EVE_CoCmd_setBitmap(s_pHalContext, videoSource, RGB565, videoW, videoH);
		EVE_CoDl_begin(s_pHalContext, BITMAPS);
		EVE_CoDl_vertex2f(s_pHalContext, VP(videoX), VP(videoY));
		EVE_CoDl_end(s_pHalContext);

		/*** Show a button ***/
		uint32_t btnW = 200;
		uint32_t btnH = 50;
		uint32_t btnX = s_pHalContext->Width / 2 - btnW / 2;
		uint32_t btnY = s_pHalContext->Height - btnH - 10;
		EVE_Cmd_wr32(s_pHalContext, TAG(btnPauseTag));
		EVE_CoCmd_button(s_pHalContext, btnX, btnY, btnW, btnH, 30, 0, txtPause[i]);
		/*** Done button ***/
		Display_End(s_pHalContext);
	}

    EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, 1); // restore default value
    EVE_CoCmd_flashSource(s_pHalContext, flashSource);
	EVE_CoCmd_playVideo(s_pHalContext, OPT_NODL | OPT_FLASH | OPT_SOUND | OPT_NOTEAR | OPT_OVERLAY);

    while (EVE_Cmd_space(s_pHalContext) != (EVE_CMD_FIFO_SIZE - 4))
    {
        Gesture_Renew(s_pHalContext);
        if (Gesture_Get()->tagReleased == btnPauseTag) {
            isPause = !isPause;
            EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, isPause?0:1); // stop the video
			GPU_DLSwap(s_pHalContext, DLSWAP_FRAME);
        }
    }
	EVE_Cmd_waitFlush(s_pHalContext);
	EVE_Hal_wr32(s_pHalContext, REG_PLAY_CONTROL, 1); // restore default value
#endif
}

void SAMAPP_Video() {
    SAMAPP_Video_pauseResumeFBF();
    SAMAPP_Video_pauseResumeWithAudio();
    SAMAPP_Video_pauseResumeWithAudio_buttonchange();
    SAMAPP_Video_fromFlash();
    SAMAPP_Video_fromFile();
    SAMAPP_Video_fromFlashFullScreen();
    SAMAPP_Video_fromCMDB();
    SAMAPP_Video_fromCMDBuffer();
    SAMAPP_Video_frameByFrameFromFlash();
    SAMAPP_Video_frameByFrameMediafifo();
    SAMAPP_Video_audioEnabled();
    SAMAPP_Video_ASTCOverlay();
}


