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

/**
* @brief Helper function to stop video playback from command buffer
*
*/
static void helperStopVideoCmdFifo()
{
    int availfreesz = 0;
    int cmdRead = 0;
    int cmdWrite = 0;
#define EMPTY_BUFFER_SIZE (4 * 50)

#if defined(BT81X_ENABLE)
    eve_printf_debug("End of video\n");
    EVE_sleep(1000);
    eve_printf_debug("Stop video\n");
    EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, 0xFF); // stop the video

    EVE_sleep(1000);
    eve_printf_debug("Restore play control video\n");
    EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, 1); // restore default value
#endif                               

    // fill remain padding bytes to command fifo
    EVE_sleep(5000);
    eve_printf_debug("Pad FIFO\n");
    int emptyBuffer[EMPTY_BUFFER_SIZE] = { 0 };
    while (true)
    {
        cmdRead = EVE_Hal_rd16(s_pHalContext, REG_CMD_READ);
        cmdWrite = EVE_Hal_rd16(s_pHalContext, REG_CMD_WRITE);

        if (cmdRead == cmdWrite)
        {
            EVE_Hal_wrMem(s_pHalContext, REG_CMDB_WRITE, (uint8_t*) emptyBuffer, EMPTY_BUFFER_SIZE);
            break;
        }
        else
        {
            availfreesz = EVE_Hal_rd32(s_pHalContext, REG_CMDB_SPACE);
            if (availfreesz > EMPTY_BUFFER_SIZE)
                availfreesz = EMPTY_BUFFER_SIZE;

            if (availfreesz > 0)
                EVE_Hal_wrMem(s_pHalContext, REG_CMDB_WRITE, (uint8_t*) emptyBuffer, availfreesz);
        }
        EVE_sleep(100);
    }
    eve_printf_debug("Ready\n");
}

/**
* @brief API to demonstrate Video display from Flash
*
*/
void SAMAPP_Video_fromFlash()
{
#if defined (BT81X_ENABLE)
    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }
    Draw_Text(s_pHalContext, "Example for: Video display from Flash");

    SAMAPP_INFO_START;
    EVE_CoCmd_text(s_pHalContext, 0, 150, 30, 0, "Video display from Flash");
    EVE_Cmd_waitFlush(s_pHalContext);
    EVE_CoCmd_flashSource(s_pHalContext, 4096);
    EVE_Cmd_wr32(s_pHalContext, CMD_PLAYVIDEO);
    EVE_Cmd_wr32(s_pHalContext, OPT_FLASH | OPT_SOUND | OPT_NOTEAR | OPT_OVERLAY);
    
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
#if defined(BT81X_ENABLE)
#define CMD_BUFFER 2*1024
    uint8_t pbuff[CMD_BUFFER];
    uint32_t filesz;

    Draw_Text(s_pHalContext, "Example for: Video display from file");

    filesz = FileIO_File_Open(TEST_DIR "\\Big buck bunny 240p 40s  adpcm_ima_wav.avi", FILEIO_E_FOPEN_READ);
    if (filesz <= 0)
    {
        printf("\nFailed to open file.\n");
        return;
    }

    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 155);
#if defined(BT81X_ENABLE)
    EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, 1); // restore default value
#endif

    EVE_Cmd_wr32(s_pHalContext, CMD_PLAYVIDEO);
    EVE_Cmd_wr32(s_pHalContext, OPT_SOUND | OPT_NOTEAR);
    APP_INF("Video playback starts.\n");
    
    while (filesz > 0)
    {
        uint32_t bytesread = filesz > CMD_BUFFER ? CMD_BUFFER : filesz;
        FileIO_File_Read(pbuff, bytesread);
        filesz -= bytesread;
        EVE_Cmd_wrMem(s_pHalContext, pbuff, bytesread);
    }

    helperStopVideoCmdFifo();
    EVE_Cmd_restore(s_pHalContext);
    EVE_Cmd_waitFlush(s_pHalContext);
#endif 
}

/**
* @brief Test AVI video playback full screen from flash
*
*/
void SAMAPP_Video_fromFlashFullScreen()
{
#if defined(EVE_FLASH_AVAILABLE) && defined(FT81X_ENABLE)// FT81X only
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
#if defined(FT81X_ENABLE) // FT81X only
    uint8_t g_scratch[SCRATCH_BUFF_SZ];
    uint32_t filesz;
    uint32_t bytesread;
#define VIDEO_HEIGHT         (480)
#define VIDEO_WIDTH          (854)
#define VIDEO_RAM_SPACE      (VIDEO_WIDTH * VIDEO_HEIGHT *2)
#define SPARE_RAM_G_SIZE     (RAM_G_SIZE - VIDEO_RAM_SPACE)
#define SPARE_RAM_G_ADDRESS  (RAM_G + VIDEO_RAM_SPACE)

    Draw_Text(s_pHalContext, "Example for: Video display via REG_CMDB_WRITE/REG_CMDB_SPACE");

    SAMAPP_INFO_START;
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(0));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(0));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(RGB565, 854 * 2L, 480));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(((854 * 2L) >> 10), ((480) >> 9)));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, 854, 480));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H((854 >> 9), (480 >> 9)));

    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(0));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0, 0));
    SAMAPP_INFO_END;

    //The video data is following: (through command buffer)
    filesz = FileIO_File_Open(TEST_DIR "\\chickens-4.avi", FILEIO_E_FOPEN_READ);

    if (filesz <= 0)
    {
        printf("\nFailed to open file.\n");
        return;
    }

    EVE_Cmd_wr32(s_pHalContext, CMD_PLAYVIDEO);
    EVE_Cmd_wr32(s_pHalContext, OPT_NODL);

    int32_t availfreesz = 0;
    int32_t freadbufffill = 0;
    const uint8_t* pbuff = g_scratch;
    while (filesz > 0)
    {
        availfreesz = EVE_Hal_rd32(s_pHalContext, REG_CMDB_SPACE);
        if (availfreesz <= 0)
        {
            continue;
        }
        if (0 == freadbufffill)
        {
            if (filesz > SCRATCH_BUFF_SZ)
            {
                freadbufffill = SCRATCH_BUFF_SZ;
            }
            else
            {
                freadbufffill = filesz;
            }
            bytesread = FileIO_File_Read(g_scratch, freadbufffill);
            filesz -= bytesread;
            pbuff = g_scratch;
        }

        if (availfreesz > freadbufffill)
        {
            availfreesz = freadbufffill;
        }

        if (availfreesz > 0)
        {
            EVE_Hal_wrMem(s_pHalContext, REG_CMDB_WRITE, pbuff, availfreesz);
        }
        pbuff += availfreesz;
        freadbufffill -= availfreesz;
    }
    helperStopVideoCmdFifo();
    EVE_Cmd_restore(s_pHalContext);
#endif
}

/**
* @brief AVI video playback via CMD buffer
*
*/
void SAMAPP_Video_fromCMDBuffer()
{
#if defined(BT81X_ENABLE)
    uint8_t g_scratch[SCRATCH_BUFF_SZ];
    uint32_t filesz;
    uint32_t bytesread;

#define LOGO_ADDRESS   (854 * 480 * 2)
#define PNG_W 182
#define PNG_H 103

    Draw_Text(s_pHalContext, "Example for: Video display via command buffer");

    /* construct the DL and display */
    uint32_t PicFileLen = FileIO_File_Open(
        TEST_DIR "\\Logo_182x103_182x103_ARGB4_Converted.png", FILEIO_E_FOPEN_READ);

    if (PicFileLen <= 0)
    {
        printf("\nFailed to open file.\n");
        return;
    }

    uchar8_t* Picpbuff = (uchar8_t*) malloc(8192);

    EVE_Cmd_waitFlush(s_pHalContext);
    EVE_Cmd_wr32(s_pHalContext, CMD_LOADIMAGE);
    EVE_Cmd_wr32(s_pHalContext, LOGO_ADDRESS); //destination address of jpg decode
    EVE_Cmd_wr32(s_pHalContext, OPT_NODL);//output format of the bitmap, supported options are OPT_MONO-bitmap to be monochrome, OPT_FULLSCREEN-attempt to scale to full screen size, and OPT_MEDIAFIFO-use media fifo for image data. If the bitmap format to be used is other than RGB565 and mono.  NODL option was chosen becuase the png has transparancy and we want to draw it by ARGB4 format

    while (PicFileLen > 0)
    {
        /* download the data into the command buffer by 2kb one shot */
        uint16_t Picblocklen = PicFileLen > 8192 ? 8192 : (uint16_t) PicFileLen;

        /* copy the data into pbuff and then transfter it to command buffer */
        FileIO_File_Read(Picpbuff, Picblocklen);
        PicFileLen -= Picblocklen;
        /* copy data continuously into command memory 
        alignment is already taken care by this api */
        EVE_Cmd_wrMem(s_pHalContext, Picpbuff, Picblocklen); 
    }
    /* close the opened binary zlib file */
    free(Picpbuff);
    EVE_Cmd_waitFlush(s_pHalContext);

    SAMAPP_INFO_START;
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(0));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(0));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(RGB565, 854 * 2L, 480));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(((854 * 2L) >> 10), ((480) >> 9)));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, 854, 480));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H((854 >> 9), (480 >> 9)));

    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(1));
    EVE_CoCmd_setBitmap(s_pHalContext, LOGO_ADDRESS, ARGB4, PNG_W, PNG_H);

    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(0));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0, 0));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(1));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(100 * 16, 100 * 16));
    SAMAPP_INFO_END;

    filesz = FileIO_File_Open(TEST_DIR "\\chickens-4.avi", FILEIO_E_FOPEN_READ);
    if (filesz <= 0)
    {
        printf("\nFailed to open file.\n");
        return;
    }

    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 155);
#if defined(BT81X_ENABLE)
    EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, 1); // restore default value
#endif        
    EVE_Cmd_wr32(s_pHalContext, CMD_PLAYVIDEO);
    EVE_Cmd_wr32(s_pHalContext, OPT_NODL);

    int32_t availfreesz = 0;
    int32_t freadbufffill = 0;
    const uint8_t* pbuff = g_scratch;
    APP_DBG("Video playback starts.\n");
    while (filesz > 0)
    {
        availfreesz = EVE_Hal_rd32(s_pHalContext, REG_CMDB_SPACE);
        if (availfreesz <= 0)
        {
            continue;
        }
        if (0 == freadbufffill)
        {
            if (filesz > SCRATCH_BUFF_SZ)
            {
                freadbufffill = SCRATCH_BUFF_SZ;
            }
            else
            {
                freadbufffill = filesz;
            }
            bytesread = FileIO_File_Read(g_scratch, freadbufffill);
            pbuff = g_scratch;
            filesz -= bytesread;
        }

        if (availfreesz > freadbufffill)
        {
            availfreesz = freadbufffill;
        }

        if (availfreesz > 0)
        {
            EVE_Hal_wrMem(s_pHalContext, REG_CMDB_WRITE, pbuff, availfreesz);
        }
        pbuff += availfreesz;
        freadbufffill -= availfreesz;
    }

    helperStopVideoCmdFifo();
    EVE_Cmd_restore(s_pHalContext);
    EVE_Cmd_waitFlush(s_pHalContext);
#endif 
}

/**
* @brief API to demonstrate Video display frame by frame from Flash
*
*/
void SAMAPP_Video_frameByFrameFromFlash()
{
#if defined(EVE_FLASH_AVAILABLE) && defined(FT81X_ENABLE)
    const uint32_t flashSource = 4096;

    // video 1st settings
    const uint32_t videoW = 462;
    const uint32_t videoH = 240;
    const uint32_t videoX = 0;
    const uint32_t videoY = 0;
    const uint32_t videoSource = 8;
    const uint32_t videoHANDLE = 1;
    const uint32_t completionPtr = 0;

    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }
    Draw_Text(s_pHalContext, "Example for: Video display frame by frame from Flash");

    Display_Start(s_pHalContext);
    // video 1st
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(videoHANDLE));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(videoSource));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(RGB565, videoW * 2L, videoH));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(((videoW * 2L) >> 10), ((videoH) >> 9)));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, videoW, videoH));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H((videoW >> 9), (videoH >> 9)));
    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(videoX * 16, videoY * 16));
    EVE_Cmd_wr32(s_pHalContext, END());
    Display_End(s_pHalContext);

    EVE_CoCmd_flashSource(s_pHalContext, flashSource);
    EVE_CoCmd_videoStartF(s_pHalContext);
    do
    {
        EVE_CoCmd_videoFrame(s_pHalContext, videoSource, completionPtr);
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
#if defined(FT81X_ENABLE) // FT81X only
    uint16_t aviw = 854;
    uint16_t avih = 480;
    uint16_t videoFrameStatusAddr;
    Fifo_t stFifo;
    uint32_t mediafifo;
    uint32_t mediafifolen;
    uint8_t g_scratch[SCRATCH_BUFF_SZ];
    uint32_t filesz;
    uint32_t currchunk;
    uint32_t bytesread;
    uint32_t cmdrd = 0;
    uint32_t cmdwr = 0;

    Draw_Text(s_pHalContext, "Example for: Video display frame by frame from Mediafifo");

    /* construct the DL and display */
    EVE_CoCmd_dlStart(s_pHalContext);
    EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(255, 255, 255));
    EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
    EVE_Cmd_wr32(s_pHalContext, DISPLAY());
    EVE_CoCmd_swap(s_pHalContext);
    EVE_Cmd_waitFlush(s_pHalContext);

    /* start video playback, load the data into media fifo */
    mediafifo = aviw * avih * 2L + 32L + 4; //the starting address of the media fifo, the begining space is for the decoded video frame,
    videoFrameStatusAddr = RAM_G;//the 4 byte address for the videoframe status
    mediafifolen = RAM_G_SIZE - mediafifo;
    EVE_CoCmd_mediaFifo(s_pHalContext, mediafifo, mediafifolen);//address of the media fifo buffer

    printf("Mediafifo: Start address and length %d %d\n", mediafifo, mediafifolen);
    EVE_Cmd_waitFlush(s_pHalContext);

    filesz = FileIO_File_Open(TEST_DIR "\\chickens-4.avi", FILEIO_E_FOPEN_READ);
    if (filesz <= 0)
    {
        printf("\nFailed to open file.\n");
        return;
    }

    printf("file size %x %d %d %d %d\n", filesz, mediafifo, mediafifolen, 
        EVE_Hal_rd32(s_pHalContext, REG_CMD_READ), EVE_Hal_rd32(s_pHalContext, REG_CMD_WRITE));
    Fifo_Init(&stFifo, mediafifo, mediafifolen, REG_MEDIAFIFO_READ, REG_MEDIAFIFO_WRITE);

    //address of the media fifo buffer - init of fifo done by coprocessor
    EVE_CoCmd_mediaFifo(s_pHalContext, mediafifo, mediafifolen); 

    SAMAPP_INFO_START;
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(0));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(4));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(RGB565, aviw * 2L, avih));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(((aviw * 2L) >> 10), ((avih) >> 9)));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, aviw, avih));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H((aviw >> 9), (avih >> 9)));
    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0, 0));
    SAMAPP_INFO_END;

    printf("media fifo init done \n");

    /* fill the complete fifo buffer before entering into steady state */
    printf("Start fifofill %d %x %x %x %x\n", filesz, cmdrd, cmdwr, 
        EVE_Hal_rd32(s_pHalContext, stFifo.HW_Read_Reg), 
        EVE_Hal_rd32(s_pHalContext, stFifo.HW_Write_Reg));
    currchunk = mediafifolen - SCRATCH_BUFF_SZ;
    currchunk = ALIGN_TWO_POWER_N(currchunk, SCRATCH_BUFF_SZ);
    while (currchunk > 0)
    {
        uint32_t tempchunk = currchunk;
        if (tempchunk > SCRATCH_BUFF_SZ)
        {
            tempchunk = SCRATCH_BUFF_SZ;
        }
        else
        {
            tempchunk = currchunk;
        }
        bytesread = FileIO_File_Read(g_scratch, tempchunk);
        EVE_Hal_wrMem(s_pHalContext, stFifo.fifo_buff + stFifo.fifo_wp, g_scratch, tempchunk);
        stFifo.fifo_wp += tempchunk;
        currchunk -= tempchunk;
    }

    /* update the data written */
    filesz -= stFifo.fifo_wp;

    /* update the read and write pointers of mediafifo */
    EVE_Hal_wr32(s_pHalContext, stFifo.HW_Write_Reg, stFifo.fifo_wp);
    printf("Media fifo rd wr %d %d %d %d \n", 
        EVE_Hal_rd32(s_pHalContext, stFifo.HW_Read_Reg), 
        EVE_Hal_rd32(s_pHalContext, stFifo.HW_Write_Reg), 
        EVE_Hal_rd32(s_pHalContext, REG_CMD_READ), 
        EVE_Hal_rd32(s_pHalContext, REG_CMD_WRITE));

    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 155);
    EVE_CoCmd_videoStart(s_pHalContext); //initialize AVI video decoder
    EVE_CoCmd_videoFrame(s_pHalContext, 4, videoFrameStatusAddr);//load 1 frame
    EVE_Cmd_waitFlush(s_pHalContext);

    cmdrd = EVE_Hal_rd32(s_pHalContext, REG_CMD_READ);
    cmdwr = EVE_Hal_rd32(s_pHalContext, REG_CMD_WRITE);

    while ((Fifo_GetFreeSpace(s_pHalContext, &stFifo) != (stFifo.fifo_len - 4)) 
        || (filesz > 0))//loop till end of the file
    {
        int32_t availfreesz = Fifo_GetFreeSpace(s_pHalContext, &stFifo);
        int32_t chunkfilled = 0;

        if (EVE_Hal_rd16(s_pHalContext, REG_CMD_READ) == EVE_Hal_rd16(s_pHalContext, REG_CMD_WRITE))
        {
            EVE_CoCmd_videoFrame(s_pHalContext, 4, videoFrameStatusAddr);
            EVE_Cmd_waitFlush(s_pHalContext);
        }
        while (filesz > 0)
        {
            if (availfreesz <= SCRATCH_BUFF_SZ)
            {
                break;
            }
            if (filesz > SCRATCH_BUFF_SZ)
            {
                currchunk = SCRATCH_BUFF_SZ;
            }
            else
            {
                currchunk = filesz;
            }
            bytesread = FileIO_File_Read(g_scratch, currchunk);
            //download the data and update
            chunkfilled = Fifo_Write(s_pHalContext, &stFifo, g_scratch, bytesread);
            if (chunkfilled != bytesread)
            {
                printf("filled sz and curr sz are wrong %d %d %d\n ", 
                    chunkfilled, currchunk, bytesread);
            }
            filesz -= chunkfilled;
            availfreesz = Fifo_GetFreeSpace(s_pHalContext, &stFifo);
        }

        if (EVE_Hal_rd32(s_pHalContext, stFifo.HW_Read_Reg) == 
            EVE_Hal_rd32(s_pHalContext, stFifo.HW_Write_Reg))
        {
            printf("end of videoframe %d %d %d %x %x %x %x\n", 
                filesz, currchunk, bytesread, cmdrd, cmdwr, 
                EVE_Hal_rd32(s_pHalContext, stFifo.HW_Read_Reg), 
                EVE_Hal_rd32(s_pHalContext, stFifo.HW_Write_Reg));
            break;
        }

        cmdrd = EVE_Hal_rd16(s_pHalContext, REG_CMD_READ);
        cmdwr = EVE_Hal_rd16(s_pHalContext, REG_CMD_WRITE);

    }

    printf("playback finished %d %x %x %x %x\n", filesz, cmdrd, cmdwr, 
        EVE_Hal_rd32(s_pHalContext, stFifo.HW_Read_Reg), 
        EVE_Hal_rd32(s_pHalContext, stFifo.HW_Write_Reg));

    EVE_Cmd_waitFlush(s_pHalContext);
#endif // FT81X
}

/**
* @brief API to demonstrate AVI video playback with ASTC overlay
*
*/
void SAMAPP_Video_ASTCOverlay()
{
#if defined(EVE_FLASH_AVAILABLE) && defined(FT81X_ENABLE) // FT81X only
#define VIDEO_HEIGHT         (480)
#define VIDEO_WIDTH          (854)
#define VIDEO_RAM_SPACE      (VIDEO_WIDTH * VIDEO_HEIGHT *2)
#define SPARE_RAM_G_SIZE     (RAM_G_SIZE - VIDEO_RAM_SPACE)
#define SPARE_RAM_G_ADDRESS   (RAM_G + VIDEO_RAM_SPACE)
#define ASTC_LOGO_FLASH_ADDR  (4096)

    Draw_Text(s_pHalContext, "Example for: Video display with ASTC overlay");

    // clear screen
    SAMAPP_INFO_START;
    SAMAPP_INFO_END;

    // Add one waiting screen while programming flash the logo image file
    SAMAPP_INFO_START;
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 80, 27, OPT_CENTER,
        "Loading video ...");
    //style 0 and scale 0.5
    EVE_CoCmd_spinner(s_pHalContext, (int16_t) (s_pHalContext->Width / 2),
        (int16_t) (s_pHalContext->Height / 2), 0, (int16_t) (float) 1.0 / 2);
    SAMAPP_INFO_END;
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

    SAMAPP_INFO_START;
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(0));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(0));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(RGB565, 854 * 2L, 480));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(((854 * 2L) >> 10), ((480) >> 9)));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, 854, 480));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H((854 >> 9), (480 >> 9)));

    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(1));
    EVE_CoCmd_setBitmap(s_pHalContext, 0x800000 | (ASTC_LOGO_FLASH_ADDR / 32),
        COMPRESSED_RGBA_ASTC_4x4_KHR, 480, 272);

    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(0));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0, 0));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(1));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0 * 16, 100 * 16));
    SAMAPP_INFO_END;

    //The video data is following: (through command buffer)
    uint8_t g_scratch[SCRATCH_BUFF_SZ];
    uint32_t filesz;
    uint32_t bytesread;
    
    filesz= FileIO_File_Open(TEST_DIR "\\chickens-4.avi", FILEIO_E_FOPEN_READ);

    if (filesz <= 0)
    {
        printf("\nFailed to open file.\n");
        return;
    }

#if defined(BT81X_ENABLE)
    EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, 1); // restore default value
#endif        
    EVE_Cmd_wr32(s_pHalContext, CMD_PLAYVIDEO);
    EVE_Cmd_wr32(s_pHalContext, OPT_NODL);

    int32_t availfreesz = 0;
    int32_t freadbufffill = 0;
    const uint8_t* pbuff = g_scratch;
    while (filesz > 0)
    {
        availfreesz = EVE_Hal_rd32(s_pHalContext, REG_CMDB_SPACE);
        if (availfreesz <= 0)
        {
            continue;
        }
        if (0 == freadbufffill)
        {
            memset(g_scratch, 0, sizeof(g_scratch));

            if (filesz > SCRATCH_BUFF_SZ)
            {
                freadbufffill = SCRATCH_BUFF_SZ;
            }
            else
            {
                freadbufffill = filesz;
            }
            bytesread = FileIO_File_Read(g_scratch, freadbufffill);
            pbuff = g_scratch;
            filesz -= bytesread;
        }

        if (availfreesz > freadbufffill)
        {
            availfreesz = freadbufffill;
        }

        if (availfreesz > 0)
        {
            EVE_Hal_wrMem(s_pHalContext, REG_CMDB_WRITE, pbuff, availfreesz);
        }
        pbuff += availfreesz;
        freadbufffill -= availfreesz;
    }
    helperStopVideoCmdFifo();
    EVE_Cmd_restore(s_pHalContext);
#endif
}

/**
* @brief Video playback with audio
*
*/
void SAMAPP_Video_audioEnabled()
{
#if defined(FT81X_ENABLE)// FT81X only
    uint16_t aviw = 462;
    uint16_t avih = 240;
    uint8_t g_scratch[SCRATCH_BUFF_SZ];
    Fifo_t stFifo;
    uint32_t mediafifo;
    uint32_t mediafifolen;
    uint32_t filesz;
    uint32_t currchunk;
    uint32_t bytesread;
    uint32_t cmdrd;
    uint32_t cmdwr;

    Draw_Text(s_pHalContext, "Example for: Video display with audio enable");

    filesz = FileIO_File_Open(TEST_DIR "\\Big buck bunny 240p 40s  adpcm_ima_wav.avi", FILEIO_E_FOPEN_READ);
    if (filesz <= 0)
    {
        SAMAPP_INFO_TEXT("Unable to open file.");
        return;
    }

    /* start video playback, load the data into media fifo */
    mediafifo = aviw * avih * 2L + 32 * 1024L; //starting address of the media fifo

    //how much memory will be allocated for the video playback fifo
    mediafifolen = RAM_G_SIZE - mediafifo;

    //address of the media fifo buffer
    EVE_CoCmd_mediaFifo(s_pHalContext, mediafifo, mediafifolen);
    printf("Mediafifo: Start address and length %d %d\n", mediafifo, mediafifolen);

    //initialize application media fifo structure
    Fifo_Init(&stFifo, mediafifo, mediafifolen, REG_MEDIAFIFO_READ, REG_MEDIAFIFO_WRITE);

    EVE_Cmd_waitFlush(s_pHalContext);
    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 155);
    EVE_Cmd_wr32(s_pHalContext, CMD_PLAYVIDEO);
    EVE_Cmd_wr32(s_pHalContext, OPT_MEDIAFIFO | OPT_NOTEAR | OPT_SOUND | OPT_FULLSCREEN);

    /* fill the complete fifo buffer before entering into steady state */
    currchunk = mediafifolen - SCRATCH_BUFF_SZ;
    currchunk = ALIGN_TWO_POWER_N(currchunk, SCRATCH_BUFF_SZ);
    while (currchunk > 0)
    {
        uint32_t tempchunk = currchunk;
        if (tempchunk > SCRATCH_BUFF_SZ)
        {
            tempchunk = SCRATCH_BUFF_SZ;
        }
        else
        {
            tempchunk = currchunk;
        }
        FileIO_File_Read(g_scratch, tempchunk);
        Fifo_WriteWait(s_pHalContext, &stFifo, g_scratch, tempchunk);
        currchunk -= tempchunk;
    }
    cmdrd = EVE_Hal_rd16(s_pHalContext, REG_CMD_READ);
    cmdwr = EVE_Hal_rd16(s_pHalContext, REG_CMD_WRITE);

    filesz -= stFifo.fifo_wp;
    while ((cmdrd != cmdwr) || (filesz > 0)) //loop till end of the file
    {
        if (filesz <= 0)
        {
            cmdrd = EVE_Hal_rd16(s_pHalContext, REG_CMD_READ);
            cmdwr = EVE_Hal_rd16(s_pHalContext, REG_CMD_WRITE);
            continue;
        }

        if (filesz > SCRATCH_BUFF_SZ)
        {
            currchunk = SCRATCH_BUFF_SZ;
        }
        else
        {
            currchunk = filesz;
        }
        bytesread = FileIO_File_Read(g_scratch, currchunk);
        //download the whole chunk into ring buffer - blocking call
        Fifo_WriteWait(s_pHalContext, &stFifo, g_scratch, bytesread);

        filesz -= currchunk;

        cmdrd = EVE_Hal_rd16(s_pHalContext, REG_CMD_READ);
        cmdwr = EVE_Hal_rd16(s_pHalContext, REG_CMD_WRITE);

    }
    EVE_Cmd_restore(s_pHalContext);

#endif
}

/**
* @brief Video playback frame by frame with pause/resume button
*
*/
void SAMAPP_Video_pauseResumeFBF()
{
#if defined(EVE_FLASH_AVAILABLE) && defined(FT81X_ENABLE)
    const uint32_t flashSource = 4096;

    // video 1st settings
    const uint32_t videoW = 462;
    const uint32_t videoH = 240;
    const uint32_t videoX = s_pHalContext->Width / 2 - videoW / 2;
    const uint32_t videoY = s_pHalContext->Height / 2 - videoH / 2;
    const uint32_t videoSource = 0;
    const uint32_t videoHANDLE = 1;
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
        EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(videoHANDLE));
        EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(videoSource));
        EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(RGB565, videoW * 2L, videoH));
        EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(((videoW * 2L) >> 10), ((videoH) >> 9)));
        EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, videoW, videoH));
        EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H((videoW >> 9), (videoH >> 9)));
        EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
        EVE_Cmd_wr32(s_pHalContext, VERTEX2F(videoX * 16, videoY * 16));
        EVE_Cmd_wr32(s_pHalContext, END());

        /*** Show a button ***/
        uint32_t btnW = 200;
        uint32_t btnH = 50;
        uint32_t btnX = s_pHalContext->Width/2 - btnW/2;
        uint32_t btnY = s_pHalContext->Height - btnH - 10;
        EVE_Cmd_wr32(s_pHalContext, TAG(btnPauseTag));
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
#if defined(EVE_FLASH_AVAILABLE) && defined(BT81X_ENABLE) // BT81X only
    const uint32_t flashSource = 4096;

    const uint32_t videoW = 462;
    const uint32_t videoH = 240;
    const uint32_t videoX = s_pHalContext->Width / 2 - videoW / 2;
    const uint32_t videoY = s_pHalContext->Height / 2 - videoH / 2;
    const uint32_t videoSource = 0;
    const uint32_t videoHANDLE = 1;
    const uint32_t txtOffset = RAM_G_SIZE - 1024;
    static uint8_t isPause = 0;
    const uint8_t btnPauseTag = 1;
    uint8_t txtPause[][20] = { "RESUME\n", "PAUSE\n" };

    Draw_Text(s_pHalContext, "Example for: Video+audio playback with pause/resume button");

    // clear screen
    SAMAPP_INFO_START;
    SAMAPP_INFO_END;

    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("SwitchFullMode failed");
        return;
    }

    SAMAPP_INFO_START;
    // video
    EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(videoHANDLE));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(videoSource));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(RGB565, videoW * 2L, videoH));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(((videoW * 2L) >> 10), ((videoH) >> 9)));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, videoW, videoH));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H((videoW >> 9), (videoH >> 9)));
    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(videoX * 16, videoY * 16));
    EVE_Cmd_wr32(s_pHalContext, END());

    /*** Show a button ***/
    uint32_t btnW = 300;
    uint32_t btnH = 50;
    uint32_t btnX = s_pHalContext->Width / 2 - btnW / 2;
    uint32_t btnY = s_pHalContext->Height - btnH - 10;
    EVE_Cmd_wr32(s_pHalContext, TAG(btnPauseTag));
    EVE_CoCmd_button(s_pHalContext, btnX, btnY, btnW, btnH, 30, 0, "Pause/Resume");
    /*** Done button ***/
    SAMAPP_INFO_END;
   

    EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, 1); // restore default value
    EVE_CoCmd_flashSource(s_pHalContext, flashSource);
    EVE_Cmd_wr32(s_pHalContext, CMD_PLAYVIDEO);
    EVE_Cmd_wr32(s_pHalContext, OPT_NODL | OPT_FLASH | OPT_SOUND | OPT_NOTEAR | OPT_OVERLAY);
    EVE_Cmd_waitFlush(s_pHalContext); //Video plays after this
    uint32_t lastCmdRead = EVE_Cmd_rp(s_pHalContext);
    EVE_CoCmd_nop(s_pHalContext);

    while (lastCmdRead == EVE_Cmd_rp(s_pHalContext))
    {
        Gesture_Renew(s_pHalContext);
        if (Gesture_Get()->tagReleased == btnPauseTag) {
            isPause = !isPause;
            EVE_Hal_wr8(s_pHalContext, REG_PLAY_CONTROL, isPause?0:1); // stop the video
        }
    }
    printf("Video end\n");
#endif
}

void SAMAPP_Video() {
    SAMAPP_Video_pauseResumeFBF();
    SAMAPP_Video_pauseResumeWithAudio();
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


