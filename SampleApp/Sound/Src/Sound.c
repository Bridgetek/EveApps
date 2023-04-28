/**
 * @file Sound.c
 * @brief Sample usage of sound
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
#include "Sound.h"

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void SAMAPP_Sound();

int main(int argc, char* argv[])
{
	s_pHalContext = &s_halContext;
	Gpu_Init(s_pHalContext);

	// read and store calibration setting
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
	Esd_Calibrate(s_pHalContext);
	Calibration_Save(s_pHalContext);
#endif

	EVE_Util_clearScreen(s_pHalContext);

    char *info[] =
    {  "EVE Sample Application",
        "This sample demonstrate the using of sound", 
        "",
        ""
    }; 

	while (TRUE) {
        WelcomeScreen(s_pHalContext, info);

        SAMAPP_Sound();

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

PROGMEM prog_uchar8_t SAMAPP_Snd_Array[5 * 58] =
"Slce\0Sqrq\0Sinw\0Saww\0Triw\0Beep\0Alrm\0Warb\0Crsl\0Pp01\0Pp02\0Pp03\0Pp04\0Pp05\0Pp06\0Pp07\0Pp08\0Pp09\0Pp10\0Pp11\0Pp12\0Pp13\0Pp14\0Pp15\0Pp16\0DMF#\0DMF*\0DMF0\0DMF1\0DMF2\0DMF3\0DMF4\0DMF5\0DMF6\0DMF7\0DMF8\0DMF9\0Harp\0Xyph\0Tuba\0Glok\0Orgn\0Trmp\0Pian\0Chim\0MBox\0Bell\0Clck\0Swth\0Cowb\0Noth\0Hiht\0Kick\0Pop\0Clak\0Chak\0Mute\0uMut\0";

PROGMEM prog_uchar8_t SAMAPP_Snd_TagArray[58] = { 0x63, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
0x23, 0x2a, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x40, 0x41, 0x42, 0x43,
0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x60,
0x61 };

void helperHighLightBtn(int32_t tagvalsnd, const prog_uchar8_t* pTagArray, int32_t wbutton,
    int32_t hbutton, const char8_t* StringArray, const prog_uchar8_t* pString)
{

    int32_t numbtnrow = 7;
    int32_t numbtncol = 8;

    for (int i = 0; i < numbtnrow; i++)
    {
        for (int j = 0; j < numbtncol; j++)
        {
            EVE_Cmd_wr32(s_pHalContext, TAG(pgm_read_byte(pTagArray)));
            if (tagvalsnd == pgm_read_byte(pTagArray))
            {
                /* red color for highlight effect */
                EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0x80, 0x00, 0x00));
                EVE_Cmd_wr32(s_pHalContext, BEGIN(RECTS));
                EVE_Cmd_wr32(s_pHalContext, TAG(pgm_read_byte(pTagArray)));
                EVE_Cmd_wr32(s_pHalContext,
                    VERTEX2F((j * wbutton + 2) * 16, (hbutton * i + 2) * 16));
                EVE_Cmd_wr32(s_pHalContext,
                    VERTEX2F(((j * wbutton) + wbutton - 2) * 16,
                        ((hbutton * i) + hbutton - 2) * 16));
                EVE_Cmd_wr32(s_pHalContext, END());
                /* reset the color to make sure font doesnt get impacted */
                EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0xff, 0xff, 0xff));
            }

            /* to make sure that highlight rectangle as well as font to take the same tag values */
#ifdef ARDUINO_PLATFORM
            strcpy_P(StringArray, (const prog_char8_t*)pString);
#endif
#if defined(FT900_PLATFORM) || defined(FT93X_PLATFORM)
            {
                /* Copy data from flash to RAM */
                int32_t idx = 0;
                int32_t slen;
                char8_t* pstring, tempchar;
                pstring = StringArray;
                do
                {
                    tempchar = pgm_read_byte_near(&pString[idx]);
                    *pstring++ = tempchar;
                    idx++;
                } while (tempchar);
            }
#endif
#if defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM)
            strcpy(StringArray, pString);
#endif
            EVE_CoCmd_text(s_pHalContext, (int16_t) ((wbutton / 2) + j * wbutton),
                (int16_t) ((hbutton / 2) + hbutton * i), 26, OPT_CENTER, StringArray);

            pString += (strlen(StringArray) + 1);
            pTagArray++;
        }
    }
}

void helperDrawBtn(const prog_uchar8_t* pTagArray, int32_t wbutton, int32_t hbutton)
{
    int32_t numbtnrow = 7;
    int32_t numbtncol = 8;

    for (int i = 0; i < numbtnrow; i++)
    {
        for (int j = 0; j < numbtncol; j++)
        {
            EVE_Cmd_wr32(s_pHalContext, TAG(pgm_read_byte(pTagArray)));
            EVE_Cmd_wr32(s_pHalContext, VERTEX2F((j * wbutton + 2) * 16, (hbutton * i + 2) * 16));
            EVE_Cmd_wr32(s_pHalContext,
                VERTEX2F(((j * wbutton) + wbutton - 2) * 16, ((hbutton * i) + hbutton - 2) * 16));
            pTagArray++;
        }
    }
}

/**
* @brief Sample app api to demonstrate sound
* @brief ple app api to demonstrate sound
* @brief  mple app api to demonstrate sound
*
*/
void SAMAPP_Sound_builtin()
{
    int32_t LoopFlag = 1000;
    int32_t wbutton;
    int32_t hbutton;
    int32_t tagval;
    int32_t tagvalsnd = -1;
    int32_t numbtnrow;
    int32_t numbtncol;
    int32_t prevtag = -1;
    uint32_t freqtrack = 0;
    uint32_t currfreq = 0;
    uint32_t prevcurrfreq;
    const PROGMEM prog_uchar8_t* pString;
    const PROGMEM prog_uchar8_t* pTagArray;
    char8_t StringArray[8] = { 0 };

    Draw_Text(s_pHalContext, "Example for: Play built-in sound\n\n\nPlease touch on screen");

    /*************************************************************************/
    /* Below code demonstrates the usage of sound function. All the supported*/
    /* sounds and respective pitches are put as part of keys/buttons, by     */
    /* choosing particular key/button the sound is played                    */
    /*************************************************************************/
    numbtnrow = 7; //number of rows to be created - note that mute and unmute are not played in this application
    numbtncol = 8; //number of colomns to be created
    wbutton = (s_pHalContext->Width - 40) / numbtncol;
    hbutton = s_pHalContext->Height / numbtnrow;

    /* set the volume to maximum */
    EVE_Hal_wr8(s_pHalContext, REG_VOL_SOUND, 0xFF);
    /* set the tracker to track the slider for frequency */

    EVE_CoCmd_track(s_pHalContext, (int16_t) (s_pHalContext->Width - 15), 20, 8,
        (int16_t) (s_pHalContext->Height - 40), 100);
    EVE_Cmd_waitFlush(s_pHalContext);
    while (LoopFlag--)
    {
        tagval = EVE_Hal_rd8(s_pHalContext, REG_TOUCH_TAG);
        freqtrack = EVE_Hal_rd32(s_pHalContext, REG_TRACKER);

        if (100 == (freqtrack & 0xff))
        {
            currfreq = (freqtrack >> 16);
            currfreq = (88 * currfreq) / 65536;
            if (currfreq > 108)
                currfreq = 108;
        }
        if (tagval > 0)
        {
            if (tagval <= 99)
            {
                tagvalsnd = tagval;
            }
            if (0x63 == tagvalsnd)
            {
                tagvalsnd = 0;
            }
            if ((prevtag != tagval) || (prevcurrfreq != currfreq))
            {
                /* Play sound wrt pitch */
                EVE_Hal_wr16(s_pHalContext, REG_SOUND,
                    (int16_t) (((currfreq + 21) << 8) | tagvalsnd));
                EVE_Hal_wr8(s_pHalContext, REG_PLAY, 1);
            }
            if (0 == tagvalsnd)
                tagvalsnd = 99;
        }
        /* start a new display list for construction of screen */

        EVE_CoCmd_dlStart(s_pHalContext);
        EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(64, 64, 64));
        EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
        /* line width for the rectangle */
        EVE_Cmd_wr32(s_pHalContext, LINE_WIDTH(1 * 16));

        /* custom keys for sound input */
        pTagArray = SAMAPP_Snd_TagArray;
        /* First draw all the rectangles followed by the font */
        /* yellow color for background color */
        EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0x80, 0x80, 0x00));
        EVE_Cmd_wr32(s_pHalContext, BEGIN(RECTS));

        helperDrawBtn(pTagArray, wbutton, hbutton);

        EVE_Cmd_wr32(s_pHalContext, END());
        EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0xff, 0xff, 0xff));
        /* draw the highlight rectangle and text info */
        pTagArray = SAMAPP_Snd_TagArray;
        pString = SAMAPP_Snd_Array;

        helperHighLightBtn(tagvalsnd, pTagArray, wbutton, hbutton, StringArray, pString);

        /* Draw vertical slider bar for frequency control */
        StringArray[0] = '\0';
        strcat(StringArray, "Pt ");
        Gpu_Hal_Dec2Ascii(StringArray, (int32_t) (currfreq + 21));
        EVE_Cmd_wr32(s_pHalContext, TAG_MASK(0));
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width - 20), 10, 26, OPT_CENTER,
            StringArray);
        EVE_Cmd_wr32(s_pHalContext, TAG_MASK(1));
        EVE_Cmd_wr32(s_pHalContext, TAG(100));
        EVE_CoCmd_slider(s_pHalContext, (int16_t) (s_pHalContext->Width - 15), 20, 8,
            (int16_t) (s_pHalContext->Height - 40), 0, (int16_t) currfreq, 88);

        EVE_Cmd_wr32(s_pHalContext, DISPLAY());
        EVE_CoCmd_swap(s_pHalContext);
        prevtag = tagval;

        prevcurrfreq = currfreq;
        /* Wait till coprocessor completes the operation */
        EVE_Cmd_waitFlush(s_pHalContext);
        EVE_sleep(10);
    }

    EVE_Hal_wr16(s_pHalContext, REG_SOUND, 0);
    EVE_Hal_wr8(s_pHalContext, REG_PLAY, 1);
}

/**
* @brief API to demonstrate music playback in streaming way
*
*/
void SAMAPP_Sound_musicStreaming()
{
    uint32_t filesz = 0;
    uint32_t chunksize = 16 * 1024;
    uint32_t totalbufflen = 64 * 1024;
    uint32_t currreadlen = 0;
    const uint8_t* pBuff = NULL;
    const uint8_t* music_playing = 0;
    uint32_t wrptr = RAM_G;
    uint32_t rdptr;
    uint32_t freebuffspace;

    Draw_Text(s_pHalContext, "Example for: Play music streaming way");

    filesz = FileIO_File_Open(TEST_DIR "\\Devil_Ride_30_11025hz.raw", FILEIO_E_FOPEN_READ);
    if (filesz <= 0)
    {
        APP_ERR("Cannot open Devil_Ride_30_11025hz.raw");
        return;
    }

    pBuff = (uint8_t*) malloc(totalbufflen);

    EVE_CoCmd_dlStart(s_pHalContext);
    EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(0xff, 0xff, 0xff)); //set the background to white
    EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(32, 32, 32));//black color text

    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2),
        (int16_t) (s_pHalContext->Height / 2), 27, OPT_CENTER, "Now you will hear the music");
    EVE_Cmd_wr32(s_pHalContext, DISPLAY());
    /* Wait till coprocessor completes the operation */
    EVE_Cmd_waitFlush(s_pHalContext);
    GPU_DLSwap(s_pHalContext, DLSWAP_FRAME);

    while (filesz > 0)
    {
        currreadlen = filesz;

        if (currreadlen > chunksize)
        {
            currreadlen = chunksize;
        }
        FileIO_File_Read(pBuff, currreadlen);
        EVE_Hal_wrMem(s_pHalContext, wrptr, pBuff, currreadlen);
        wrptr += currreadlen;
        wrptr = wrptr % (RAM_G + totalbufflen);

        filesz -= currreadlen;

        if (music_playing == 0)
        {
            EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_START, RAM_G); //Audio playback start address
            EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_LENGTH, totalbufflen);//Length of raw data buffer in bytes
            EVE_Hal_wr16(s_pHalContext, REG_PLAYBACK_FREQ, 11025);//Frequency
            EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_FORMAT, LINEAR_SAMPLES);//Current sampling frequency
            EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_LOOP, 1);
            EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 255);
            EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_PLAY, 1);
            music_playing = 1;
        }

        //Check the freespace if the file has more
        do
        {
            uint32_t fullness = 0;
            rdptr = EVE_Hal_rd32(s_pHalContext, REG_PLAYBACK_READPTR);
            fullness = (wrptr - rdptr) % totalbufflen;
            freebuffspace = totalbufflen - fullness;
        }while (freebuffspace < chunksize);

        //if the file is sent over and there is one more chunk size free space.
        if (filesz == 0)
        {
            //Clear the chunksize to make mute sound.
            EVE_CoCmd_memSet(s_pHalContext, wrptr, 0, chunksize);
            EVE_Cmd_waitFlush(s_pHalContext);
        }
    }

    //if read pointer is already passed over write pointer
    if (EVE_Hal_rd32(s_pHalContext, REG_PLAYBACK_READPTR) > wrptr)
    {
        //wait till the read pointer will be wrapped over
        while (EVE_Hal_rd32(s_pHalContext, REG_PLAYBACK_READPTR) > wrptr)
            ;
    }

    //wait till read pointer pass through write pointer
    while (EVE_Hal_rd32(s_pHalContext, REG_PLAYBACK_READPTR) < wrptr)
        ;

    //The file is done , mute the sound first.
    EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_LOOP, 0);
    EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_LENGTH, 0);//Length of raw data buffer in bytes
    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 0);
    EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_PLAY, 1);

    FileIO_File_Close();
    free(pBuff);
}

/**
* @brief API to demonstrate music playback
*
*/
void SAMAPP_Sound_fromEABConvertedRaw()
{
    Draw_Text(s_pHalContext, "Example for: Play sound from EAB converted audio");

    Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "\\Devil_Ride_30_11025hz.raw", 0, LOAD);
    EVE_Cmd_waitFlush(s_pHalContext);

    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 255);
    EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_START, 0); //Audio playback start address
    EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_LENGTH, 334707);//Length of raw data buffer in bytes
    EVE_Hal_wr16(s_pHalContext, REG_PLAYBACK_FREQ, 11025);//Frequency
    EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_FORMAT, LINEAR_SAMPLES);//Current sampling frequency
    EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_LOOP, 0);
    EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_PLAY, 1);
    EVE_Cmd_waitFlush(s_pHalContext);
    while (EVE_Hal_rd8(s_pHalContext, REG_PLAYBACK_PLAY) == 1);
    EVE_Cmd_waitFlush(s_pHalContext);
}

void SAMAPP_Sound() {
    SAMAPP_Sound_builtin();
    SAMAPP_Sound_musicStreaming();
    SAMAPP_Sound_fromEABConvertedRaw();
}


