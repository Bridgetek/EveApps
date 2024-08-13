/**
 * @file Utility.c
 * @brief Sample usage of some useful command
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
#include "Utility.h"

#define SAMAPP_INFO_TEXT(str)  Draw_TextColor(s_pHalContext, str, (uint8_t[]) { 0x77, 0x77, 0x77 }, (uint8_t[]) { 255, 255, 255 })
#define SAMAPP_INFO_START      Display_StartColor(s_pHalContext,  (uint8_t[]) { 0x77, 0x77, 0x77 }, (uint8_t[]) { 255, 255, 255 })
#define SAMAPP_INFO_END        Display_End(s_pHalContext);
#define SAMAPP_DELAY_NEXT      EVE_sleep(2000);

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void SAMAPP_Utility();

/* Header of raw data containing properties of bitmap */
SAMAPP_Bitmap_header_t SAMAPP_Bitmap_RawData_Header[] =
{
    /* format,width,height,stride,arrayoffset */
    { RGB565      ,    40,      40,    40 * 2,    0 },
#ifdef FT81X_ENABLE
    { PALETTED4444,    40,     40 ,    40    ,    0 },
    { PALETTED8   ,    480,    272,    480   ,    0 },
    { PALETTED8   ,    802,    520,    802   ,    0 },
#else
    { PALETTED    ,    40 ,    40 ,    40    ,    0 },
    { PALETTED    ,    480,    272,    480   ,    0 },
#endif
};

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
        "This sample demonstrate the using of utilites of EVE", 
        "",
        ""
    }; 

	while (TRUE) {
        WelcomeScreen(s_pHalContext, info);

        SAMAPP_Utility();

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
* @brief Check Coprocesor fault
*
*/
void helperCoprocessorFaultReport()
{
    char8_t DiagMessage[128];
    if (EVE_Hal_rd16(s_pHalContext, REG_CMD_READ) == 0xfff) //Fault reported
    {
        EVE_Hal_rdMem(s_pHalContext, DiagMessage, 0x309800, 128);
        printf("%s", DiagMessage);
        printf("\n");
        DiagMessage[0] = '\0'; //Reset message
    }
}

/**
* @brief Restore Coprocessor after fault
*
*/
void helperCoprocessorFaultRecover()
{
    uint32_t save_REG_PCLK = EVE_Hal_rd32(s_pHalContext, REG_PCLK);

    /* 3 steps of recovery coprocessor sequence */
    /* Set REG_CPURESET to 1, to hold the coprocessor in the reset condition */
    EVE_Hal_wr32(s_pHalContext, REG_CPURESET, 1);
    /* Set REG_CMD_READ and REG_CMD_WRITE to zero */
    EVE_Hal_wr32(s_pHalContext, REG_CMD_READ, 0);
    EVE_Hal_wr32(s_pHalContext, REG_CMD_WRITE, 0);
    EVE_Hal_wr32(s_pHalContext, REG_CMD_DL, 0);
    EVE_Hal_wr32(s_pHalContext, REG_PCLK, save_REG_PCLK); /* coprocessor will set the pclk to 0 for that error case */
    Gpu_Hal_ResetCmdFifo(s_pHalContext);
    /* Set REG_CPURESET to 0, to restart the coprocessor */
    EVE_Hal_wr32(s_pHalContext, REG_CPURESET, 0);
    EVE_sleep(100);

    App_Set_CmdBuffer_Index(0);
    App_Set_DlBuffer_Index(0);

    EVE_CoCmd_flashFast(s_pHalContext, 0);
    EVE_Cmd_waitFlush(s_pHalContext);
    uint8_t status = EVE_Hal_rd8(s_pHalContext, REG_FLASH_STATUS);
    if (status != FLASH_STATUS_FULL)
    {
        printf("Flash is not able to get into full mode\n");
    }
}

/**
* @brief Flush a command to REG_CMDB_WRITE
*
* @param command Command value
*
*/
static void helperCMDBWrite(uint32_t command)
{
    EVE_Hal_wr32(s_pHalContext, REG_CMDB_WRITE, command);
}

/**
* @brief Flush a string to REG_CMDB_WRITE
*
* @param str string to write
*
*/
static void helperCMDBWriteString(uint8_t* str)
{
#define NUM_STRBUFFER 40
    int textLen = strlen(str);
    int padding = textLen % 4;
    /* Copy the command instruction into buffer */
    uint32_t v = 0;
    for (int i = 0; i < textLen/4; i++) 
    {
        v = str[i*4 + 0] << 0 |
            str[i*4 + 1] << 8 |
            str[i*4 + 2] << 16|
            str[i*4 + 3] << 24 ;
        EVE_Hal_wr32(s_pHalContext, REG_CMDB_WRITE, v);
    }

    if (padding)
    {
        v = 0;
        for (int i = 0; i < (4 - padding); i++) 
        {
            v |= str[textLen / 4*4 + i] << i * 8;
        }
        EVE_Hal_wr32(s_pHalContext, REG_CMDB_WRITE, v);
    }
}

/**
* @brief Write BMP image's header
*
* Only runs with window host
*
*/
static void helperBMPHeader(FILE* fp, int w, int h) {
#ifdef _MSC_VER
	// Write image header
	unsigned char file[14] = {
		'B','M', // magic
		0,0,0,0, // size in bytes
		0,0, // app data
		0,0, // app data
		40 + 14,0,0,0 // start of data offset
	};
	unsigned char info[40] = {
		40,0,0,0, // info hd size
		0,0,0,0, // width
		0,0,0,0, // heigth
		1,0, // number color planes
		24,0, // bits per pixel
		0,0,0,0, // compression is none
		0,0,0,0, // image bits size
		0x13,0x0B,0,0, // horz resoluition in pixel / m
		0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
		0,0,0,0, // #colors in pallete
		0,0,0,0, // #important colors
	};

	int padSize = (4 - (w * 3) % 4) % 4;
	int sizeData = w * h * 3 + h * padSize;
	int sizeAll = sizeData + sizeof(file) + sizeof(info);

	file[2] = (unsigned char)(sizeAll);
	file[3] = (unsigned char)(sizeAll >> 8);
	file[4] = (unsigned char)(sizeAll >> 16);
	file[5] = (unsigned char)(sizeAll >> 24);

	info[4] = (unsigned char)(w);
	info[5] = (unsigned char)(w >> 8);
	info[6] = (unsigned char)(w >> 16);
	info[7] = (unsigned char)(w >> 24);

	info[8] = (unsigned char)(h);
	info[9] = (unsigned char)(h >> 8);
	info[10] = (unsigned char)(h >> 16);
	info[11] = (unsigned char)(h >> 24);

	info[20] = (unsigned char)(sizeData);
	info[21] = (unsigned char)(sizeData >> 8);
	info[22] = (unsigned char)(sizeData >> 16);
	info[23] = (unsigned char)(sizeData >> 24);
	info[23] = (unsigned char)(sizeData >> 24);
	info[23] = (unsigned char)(sizeData >> 24);
	info[23] = (unsigned char)(sizeData >> 24);

	fwrite(file, 1, sizeof(file), fp);
	fwrite(info, 1, sizeof(info), fp);
#endif
}

/**
* @brief Convert raw image ARGB4 to an BMP image
*
* Only runs with window host
*
*/
static void helperRawARGB4ToBMP(char* rawFilePath, int w, int h, char* output) {
#ifdef _MSC_VER 
#define BMP_BPP  3   /// red, green, & blue
#define ARGB4_BPP 2 /// alpha4, red4, green4, & blue4
	int padSize = (4 - (w * 3) % 4) % 4;

	FILE* fpBMP = 0;
	FILE* fpRaw = 0;
	fpBMP = fopen(output, "wb");
	if (!fpBMP) {
		return;
	}

	fpRaw = fopen(rawFilePath, "rb");
	if (!fpRaw) {
		fclose(fpBMP);
		return;
	}

	// Prepare buffer
	unsigned char* rawLine = malloc(w * ARGB4_BPP);
	if (!rawLine) {
		APP_ERR("Malloc failed");
		fclose(fpBMP);
		fclose(fpRaw);
		return;
	}

	// Write image header
	helperBMPHeader(fpBMP, w, h);

	unsigned char pad[3] = { 0,0,0 };
	unsigned char raw[4], bmp[BMP_BPP];
	for (int i = h - 1; i >= 0; i--)
	{
		fseek(fpRaw, i * w * ARGB4_BPP, SEEK_SET);
		fread(rawLine, 1, w * ARGB4_BPP, fpRaw);
		for (int j = 0; j < w; j++)
		{
			raw[0] = rawLine[j * ARGB4_BPP + 0] & 0x0F;
			raw[1] = (rawLine[j * ARGB4_BPP + 0] & 0xF0) >> 4;
			raw[2] = rawLine[j * ARGB4_BPP + 1] & 0x0F;
			raw[3] = (rawLine[j * ARGB4_BPP + 1] & 0xF0) >> 4;

			bmp[2] = raw[2] << 4; ///red
			bmp[1] = raw[1] << 4; ///green
			bmp[0] = raw[0] << 4; ///blue

			fwrite(bmp, 1, BMP_BPP, fpBMP);
		}
		fwrite(pad, 1, padSize, fpBMP);
	}

	free(rawLine);
	fclose(fpBMP);
	fclose(fpRaw);
#endif
}

/**
* @brief Convert raw image ARGB8 to an BMP image
*
* Only runs with window host
*
*/
static void helperRawARGB8ToBMP(char* rawFilePath, int w, int h, char* output) {
#ifdef _MSC_VER 
#define BMP_BPP  3   /// red, green, & blue
#define ARGB8_BPP 4 /// alpha8, red8, green8, & blue8
	int padSize = (4 - (w * 3) % 4) % 4;

	FILE* fpBMP = 0;
	FILE* fpRaw = 0;
	fpBMP = fopen(output, "wb");
	if (!fpBMP) {
		return;
	}

	fpRaw = fopen(rawFilePath, "rb");
	if (!fpRaw) {
		fclose(fpBMP);
		return;
	}

	// Prepare buffer
	unsigned char* rawLine = malloc(w * ARGB8_BPP);
	if (!rawLine) {
		APP_ERR("Malloc failed");
		fclose(fpBMP);
		fclose(fpRaw);
		return;
	}

	// Write image header
	helperBMPHeader(fpBMP, w, h);

	unsigned char pad[3] = { 0,0,0 };
	unsigned char raw[ARGB8_BPP], bmp[BMP_BPP];
	for (int i = h - 1; i >= 0; i--)
	{
		fseek(fpRaw, i * w * ARGB8_BPP, SEEK_SET);
		fread(rawLine, 1, w * ARGB8_BPP, fpRaw);
		for (int j = 0; j < w; j++)
		{
			raw[0] = rawLine[j * ARGB8_BPP + 0];
			raw[1] = rawLine[j * ARGB8_BPP + 1];
			raw[2] = rawLine[j * ARGB8_BPP + 2];
			raw[3] = rawLine[j * ARGB8_BPP + 3];

			bmp[2] = raw[2]; ///red
			bmp[1] = raw[1]; ///green
			bmp[0] = raw[0]; ///blue

			fwrite(bmp, 1, BMP_BPP, fpBMP);
		}
		fwrite(pad, 1, padSize, fpBMP);
	}

	free(rawLine);
	fclose(fpBMP);
	fclose(fpRaw);
	APP_DBG("Done");
#endif
}

/**
* @brief Convert raw image RGB565 to an BMP image
*
* Only runs with window host
*
*/
static void helperRawRGB565ToBMP(char* rawFilePath, int w, int h, char* output) {
#ifdef _MSC_VER 
#define BMP_BPP 3    /// red, green, & blue
#define RGB565_BPP 2 /// red5, green6, & blue5
	int padSize = (4 - (w * 3) % 4) % 4;

	FILE* fpBMP = 0;
	FILE* fpRaw = 0;
	fpBMP = fopen(output, "wb");
	if (!fpBMP) {
		return;
	}

	fpRaw = fopen(rawFilePath, "rb");
	if (!fpRaw) {
		fclose(fpBMP);
		return;
	}

	// Prepare buffer
	unsigned char* rawLine = malloc(w * RGB565_BPP);
	if (!rawLine) {
		APP_ERR("Malloc failed");
		fclose(fpBMP);
		fclose(fpRaw);
		return;
	}

	// Write image header
	helperBMPHeader(fpBMP, w, h);

	unsigned char pad[3] = { 0,0,0 };
	unsigned char raw[2], bmp[3];
	for (int i = h - 1; i >= 0; i--)
	{
		fseek(fpRaw, i * w * RGB565_BPP, SEEK_SET);
		fread(rawLine, 1, w * RGB565_BPP, fpRaw);
		for (int j = 0; j < w; j++)
		{
			raw[0] = rawLine[j * RGB565_BPP + 0];
			raw[1] = rawLine[j * RGB565_BPP + 1];

			// big endian
			bmp[2] = raw[0] & 0xF8;                                    ///red
			bmp[1] = ((raw[0] & 0x07) << 5) | ((raw[1] & 0xE0) >> 3);  ///green
			bmp[0] = (raw[1] & 0x1F) << 3;                             ///blue

			// little endian
			bmp[0] = (raw[0] & 0x1F) << 3;                             ///blue
			bmp[1] = ((raw[1] & 0x07) << 5) | ((raw[0] & 0xE0) >> 3);  ///green
			bmp[2] = raw[1] & 0xF8;                                    ///red

			fwrite(bmp, 1, BMP_BPP, fpBMP);
		}
		fwrite((char*)pad, 1, padSize, fpBMP);
	}

	free(rawLine);
	fclose(fpBMP);
	fclose(fpRaw);
	APP_DBG("Done");
#endif
}

/**
* @brief Display a simple screen to be used by cmd_snapshot2
*
* Display an image repeated, and a text at the center
*
*/
static void helperSimpleScreen(char* title, uint8_t isImageEnable) {
	char* img = TEST_DIR "\\mandrill256.jpg";
	int w = 256;
	int h = 256;
	int x = (s_pHalContext->Width - w) / 2;
	int y = (s_pHalContext->Height - h) / 2;

	int xText = s_pHalContext->Width / 2;
	int yText = s_pHalContext->Height / 2;

	// Display an image repeated, and a text at the center
	Display_StartColor(s_pHalContext, (uint8_t[]) { 0x5F, 0x9E, 0xA0}, (uint8_t[]) { 255, 255, 255 });

	if (isImageEnable) {
		Gpu_Hal_LoadImageToMemory(s_pHalContext, img, 0, LOADIMAGE);
		EVE_CoCmd_setBitmap(s_pHalContext, 0, RGB565, w, h);
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		for (int i = 0; i < s_pHalContext->Width / w + 1; i++) {
			for (int j = 0; j < s_pHalContext->Height / h + 1; j++) {
				EVE_Cmd_wr32(s_pHalContext, VERTEX2F((i * w) * 16, (j * h) * 16));
			}
		}
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0 * 16, 0 * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(x * 16, y * 16));
	}

	EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0xFF, 0xFF, 0xE0));
	EVE_CoCmd_fillWidth(s_pHalContext, s_pHalContext->Width - 10);
	EVE_CoCmd_text(s_pHalContext, xText, yText, 30, OPT_CENTERX | OPT_FILL, title);
	Display_End(s_pHalContext);
}

/**
* @brief Copy raw image file to the host's storage, and convert it to bmp image 
*
* The convert is only runs with window host
* On Emulator host, user must click on the screen to 
*
*/
static void helperSnapshot2(uint32_t format, char* output, int linePerScan) {
	uint32_t bytesPerPixel = 2;
	const uint8_t ENABLE_PCLK_OFF = 1;
	uint32_t pclk = 0;
	uint32_t lcdW = s_pHalContext->Width;
	uint32_t cmdW = s_pHalContext->Width;

	// when fmt is 0x20, i.e. in ARGB8 format, the value of width shall be doubled
	if (format == 0x20) {
		cmdW *= 2;
		bytesPerPixel = 4;
	}

	uint32_t lcdH = s_pHalContext->Height;
	uint32_t bytePerLine = lcdW * bytesPerPixel;

	char outputBMP[100];
	char buff[1280 * 4]; // 1280 = max LCD size, 4 = maximum byte per pixel size

	snprintf(outputBMP, sizeof(outputBMP), "%s", output);
	int lenOutPut = strlen(outputBMP);
	snprintf(&outputBMP[lenOutPut - 4], 5, ".bmp");

	/* Disable the pclock */
	if (ENABLE_PCLK_OFF) {
		EVE_Cmd_waitFlush(s_pHalContext); 
		pclk = EVE_Hal_rd8(s_pHalContext, REG_PCLK);
		EVE_Hal_wr8(s_pHalContext, REG_PCLK, 0);
	}

	FileIO_File_Open(output, FILEIO_E_FOPEN_WRITE);
	for (int i = 0; i < lcdH; i += linePerScan) {
		int lineremain = lcdH - i;
		int lineNUm = linePerScan > lineremain ? lineremain : linePerScan;
		uint32_t chunk_size = bytePerLine * lineNUm;
		chunk_size = min(chunk_size, RAM_G_SIZE);// when chunk size > ram_g size, we will lost some part of image, because ram_g is not fit
		int address = RAM_G_SIZE - chunk_size;

		int cmdH = lineNUm;
		while (cmdH * cmdW * bytesPerPixel > RAM_G_SIZE) {
			cmdH--;
		}

		EVE_CoCmd_snapshot2(s_pHalContext, format, address, 0, i, cmdW, cmdH);
		EVE_Cmd_waitFlush(s_pHalContext);
		printf("\rReading %d line from line %d (%d %%)", lineNUm, i, i * 100 / lcdH);

		// download the raw image line by line
		for (int j = 0; j < lineNUm; j++) {
			int addressRd = address + j * bytePerLine;
			if (addressRd > RAM_G_SIZE) {
				APP_INF("raw image size is > ram_g");
			}
			EVE_Hal_rdMem(s_pHalContext, buff, address + j * bytePerLine, bytePerLine);
			int bytesWrite = FileIO_File_Write(buff, bytePerLine);
			if (bytesWrite < bytePerLine) {
				APP_ERR("error on writing file, line %d", i);
				break;
			}
		}
	}
	FileIO_File_Close();
	
	/* reconfigure the resolution wrt configuration */
	if (ENABLE_PCLK_OFF) {
		EVE_Cmd_waitFlush(s_pHalContext);
		EVE_Hal_wr8(s_pHalContext, REG_PCLK, pclk);
	}

	if (format == RGB565) {
		Draw_Text_Format(s_pHalContext, "Converting to BMP image file %s", outputBMP);
		helperRawRGB565ToBMP(output, lcdW, lcdH, outputBMP);
	}
	else if (format == ARGB4) {
		Draw_Text_Format(s_pHalContext, "Converting to BMP image file %s", outputBMP);
		helperRawARGB4ToBMP(output, lcdW, lcdH, outputBMP);
	}
	else if (format == 0x20) { // ARGB8
		Draw_Text_Format(s_pHalContext, "Converting to BMP image file %s", outputBMP);
		helperRawARGB8ToBMP(output, lcdW, lcdH, outputBMP);
	}
}

/**
* @brief API to demonstrate CMD_WAIT
*
*/
void SAMAPP_Utility_wait()
{
#if EVE_SUPPORT_GEN == EVE4
    const uint32_t delayUs = 1000000;
    uint32_t sms = EVE_millis();
 
    Draw_Text(s_pHalContext, "Example for: Waiting/Sleeping");
    SAMAPP_INFO_TEXT("EVE is waiting for 1 second...");

    APP_INF("Time start= %u", sms);
    EVE_CoCmd_wait(s_pHalContext, delayUs);
    EVE_Cmd_waitFlush(s_pHalContext);
    uint32_t ems = EVE_millis();
    APP_INF("Time end= %u", ems);
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate CMD_APILEVEL
*
*/
void SAMAPP_Utility_apiLevel()
{
#if EVE_SUPPORT_GEN == EVE4
    Draw_Text(s_pHalContext, "Example for: Switch API level");

    SAMAPP_INFO_TEXT("Now set API level to 1 (BT815/6)");
    EVE_CoCmd_apiLevel(s_pHalContext, 1);
    EVE_Cmd_waitFlush(s_pHalContext);

    SAMAPP_INFO_TEXT("Now set API level to 2 (BT817/8)");
    EVE_CoCmd_apiLevel(s_pHalContext, 2);
    EVE_Cmd_waitFlush(s_pHalContext);

    SAMAPP_DELAY_NEXT;
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate for CMD_CALLLIST, CMD_NEWLIST, CMD_ENDLIST, CMD_RETURN
*
*/
void SAMAPP_Utility_callList()
{
#if EVE_SUPPORT_GEN == EVE4
    uint32_t endPtr = 4;
    uint32_t endPtrArr[5];
    const uint32_t w = 800;
    const uint32_t h = 480;
    const uint32_t x = 100;
    const uint32_t y = 100;
    uint32_t addrImg = RAM_G_SIZE - w * h * 2;
    char str[1000];

    Draw_Text(s_pHalContext, "Example for: CMD_CALLLIST, CMD_NEWLIST, CMD_ENDLIST and CMD_RETURN");

    SAMAPP_INFO_TEXT("Constructing 5 lists ...");
    for (int i = 0; i < 5; i++)
    {
        endPtrArr[i] = endPtr;

        EVE_CoCmd_newList(s_pHalContext, endPtr);
        EVE_CoCmd_setBitmap(s_pHalContext, addrImg, RGB565, w, h);

        EVE_Cmd_wr32(s_pHalContext, SAVE_CONTEXT());
        EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
        EVE_CoCmd_loadIdentity(s_pHalContext);
        EVE_CoCmd_rotateAround(s_pHalContext, w / 2, h / 2, i * 30 * 65536 / 360, 65536 * 1);
        EVE_CoCmd_setMatrix(s_pHalContext);
        EVE_Cmd_wr32(s_pHalContext, VERTEX2F((x) * 16, (y) * 16));
        EVE_Cmd_wr32(s_pHalContext, END());
        EVE_Cmd_wr32(s_pHalContext, RESTORE_CONTEXT());

        snprintf(str, 1000, "Displaying list number %u", i);
        EVE_CoCmd_text(s_pHalContext, (uint16_t) (x + i * 20), (uint16_t) (y + i * 20), 28, 0, str);
        EVE_CoCmd_endList(s_pHalContext);

        EVE_Cmd_waitFlush(s_pHalContext);
        uint32_t endAddr;
        EVE_CoCmd_getPtr(s_pHalContext, &endAddr);
        EVE_Cmd_waitFlush(s_pHalContext);

        snprintf(str, 1000, "Compiled list %u at %u, size: %u Bytes", i, endPtr, endAddr - endPtr);
        APP_INF("%s", str);
        endPtr = endAddr;
    }
    SAMAPP_INFO_TEXT("Constructed 5 lists");

    Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "\\flower_800x480.jpg", addrImg, LOADIMAGE);
    EVE_Cmd_waitFlush(s_pHalContext);
    for (int i = 0; i < 5; i++)
    {
        snprintf(str, 1000, "Calling List number %u", i);
        SAMAPP_INFO_TEXT(str);

        SAMAPP_INFO_START;
        EVE_CoCmd_callList(s_pHalContext, endPtrArr[i]);
        SAMAPP_INFO_END;
        SAMAPP_DELAY_NEXT;

        if (i == 1)
        {
            EVE_Cmd_waitFlush(s_pHalContext);
            uint32_t cmd = EVE_Hal_rd32(s_pHalContext, RAM_G + endPtrArr[i + 1] - 4);
            APP_INF("Return: 0x%08x", cmd);

            if (cmd == CMD_RETURN)
            {
                SAMAPP_INFO_TEXT("CMD_RETURN is appended into the list");
                SAMAPP_DELAY_NEXT;
            }
        }
    }
    SAMAPP_DELAY_NEXT;
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate for command list in RAM_G with alignment
*
*/
void SAMAPP_Utility_callListWithAlignment()
{
#if EVE_SUPPORT_GEN == EVE4
    Draw_Text(s_pHalContext, "Example for: Construct a command list in RAM_G to show a button");

    //Construct a command list in RAM_G to show a button
    EVE_Hal_wr32(s_pHalContext, RAM_G + 0 * 4, SAVE_CONTEXT());
    EVE_Hal_wr32(s_pHalContext, RAM_G + 1 * 4, COLOR_RGB(125, 125, 128));
    EVE_Hal_wr32(s_pHalContext, RAM_G + 2 * 4, CMD_BUTTON);

    EVE_Hal_wr16(s_pHalContext, RAM_G + 3 * 4, 160); //x coordinate of button
    EVE_Hal_wr16(s_pHalContext, RAM_G + 3 * 4 + 2, 160); //y coordinate of button
    EVE_Hal_wr16(s_pHalContext, RAM_G + 4 * 4, 324); //w
    EVE_Hal_wr16(s_pHalContext, RAM_G + 4 * 4 + 2, 234); //h
    EVE_Hal_wr16(s_pHalContext, RAM_G + 5 * 4, 31); //Font handle
    EVE_Hal_wr16(s_pHalContext, RAM_G + 5 * 4 + 2, 0); //option parameter of cmd_button
    EVE_Hal_wr8(s_pHalContext, RAM_G + 6 * 4 + 0, 'T');
    EVE_Hal_wr8(s_pHalContext, RAM_G + 6 * 4 + 1, 'E');
    EVE_Hal_wr8(s_pHalContext, RAM_G + 6 * 4 + 2, 'S');
    EVE_Hal_wr8(s_pHalContext, RAM_G + 6 * 4 + 3, 'T');

    EVE_Hal_wr8(s_pHalContext, RAM_G + 7 * 4, '\0');
    EVE_Hal_wr8(s_pHalContext, RAM_G + 7 * 4 + 1, '\0');
    EVE_Hal_wr8(s_pHalContext, RAM_G + 7 * 4 + 2, '\0');
    EVE_Hal_wr8(s_pHalContext, RAM_G + 7 * 4 + 3, '\0');

    EVE_Hal_wr32(s_pHalContext, RAM_G + 8 * 4, RESTORE_CONTEXT()); //Assume 3 bytes padding bytes for alignment 
    EVE_Hal_wr32(s_pHalContext, RAM_G + 9 * 4, CMD_RETURN); //return to the command buffer

    //Call cmd_list with data in RAM_G
    EVE_CoCmd_dlStart(s_pHalContext);
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 255, 255));
    EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
    EVE_CoCmd_callList(s_pHalContext, RAM_G);
    EVE_Cmd_wr32(s_pHalContext, DISPLAY());
    EVE_CoCmd_swap(s_pHalContext);
    EVE_Hal_flush(s_pHalContext);

    SAMAPP_DELAY_NEXT;
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate REG_UNDERRUN
*
*/
void SAMAPP_Utility_underRunDetection()
{
#if (EVE_SUPPORT_GEN == EVE4) && !defined(BT8XXEMU_PLATFORM)

    uint16_t w = 800;
    uint16_t h = 600;

    Draw_Text(s_pHalContext, "Example for: Underrun detection\n\n(Emulator is not support)");

    int i = 0;
    while (i++ < 10)
    {
        //load bitmap file into graphics RAM
        Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "//flower_800x600.jpg", RAM_G, LOADIMAGE);
        EVE_Cmd_waitFlush(s_pHalContext);

        //Start drawing bitmap
        SAMAPP_INFO_START;
        //RAM_G is starting address in graphics RAM, for example 00 0000h
        uint32_t x = s_pHalContext->Width / 2;
        uint32_t y = s_pHalContext->Height/ 2;
        EVE_CoCmd_text(s_pHalContext, x, y, 30, OPT_CENTER, "Triggering underrun ...");

        EVE_CoCmd_setBitmap(s_pHalContext, RAM_G, RGB565, w, h);
        EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
        for (int j = 0; j < 100; j++)
        {
            EVE_Cmd_wr32(s_pHalContext, VERTEX2F(i*16, i*16));
        }
        EVE_Cmd_wr32(s_pHalContext, END());
        SAMAPP_INFO_END;

        if (EVE_Hal_rd32(s_pHalContext, REG_UNDERRUN) != 0)
        {
            SAMAPP_INFO_TEXT("Underrun detected");
            return;
        }
    }
    SAMAPP_DELAY_NEXT;
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate Coprocessor fault and recover
*
*/
void SAMAPP_Utility_coprocessorFaultRecover()
{
#if defined (BT81X_ENABLE)
    int32_t bitmapWidth = 128;
    int32_t bitmapHeight = 128;
    Draw_Text(s_pHalContext, "Example for: Coprocessor fault and recover");

    //Fault case: enable interlace option
    Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "\\lenaface40_unsupported.png", RAM_G, LOADIMAGE);

    SAMAPP_INFO_START;
    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext,
        VERTEX2F((s_pHalContext->Width / 2 - bitmapWidth / 2) * 16,
            (s_pHalContext->Height / 2 - bitmapHeight / 2) * 16));
    EVE_Cmd_wr32(s_pHalContext, END());
    SAMAPP_INFO_END;
    delay(100);

    helperCoprocessorFaultReport();
    helperCoprocessorFaultRecover();
    
    //Fault case: change bit depth into 7
    Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "\\lenaface40_corrupted.png", RAM_G, LOADIMAGE);

    SAMAPP_INFO_START;
    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext,
        VERTEX2F((s_pHalContext->Width / 2 - bitmapWidth / 2) * 16,
            (s_pHalContext->Height / 2 - bitmapHeight / 2) * 16));
    EVE_Cmd_wr32(s_pHalContext, END());

    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 50, 30, OPT_CENTER,
        "This PNG images is loaded after\ncoprocessor fault and recovered");
    SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;
#endif // defined (BT81X_ENABLE)
}

/**
* @brief API to demonstrate display bitmap by inflate data from flash
*
*/
void SAMAPP_Utility_cmdInflateFromFlash()
{
#if defined (BT81X_ENABLE)
    Draw_Text(s_pHalContext, "Example for: CMD_INFLATE2 with OPT_FLASH");

    /* INFLATED BITMAP information */
#define INFLATED_ADDR (4096)
#define INFLATED_BITMAP_WIDTH (63)  /* color_.bin */
#define INFLATED_BITMAP_HEIGHT (44)
#define INFLATED_BITMAP_FORMAT (ARGB4)
#define INFLATED_BITMAP_STRIDE (63*2)

    /*Load data to mediafifo */
    uchar8_t* imagefile = TEST_DIR "\\color_.bin";

    Ftf_Write_File_To_Flash_By_RAM_G(s_pHalContext, imagefile, INFLATED_ADDR);
    FlashHelper_SwitchState(s_pHalContext, FLASH_STATUS_FULL); // full mode

    /* Now use cmd_inflate */
    EVE_CoCmd_flashSource(s_pHalContext, INFLATED_ADDR);
    EVE_Hal_flush(s_pHalContext);
    EVE_CoCmd_inflate2(s_pHalContext, RAM_G, OPT_FLASH);
    EVE_Cmd_waitFlush(s_pHalContext);

    /* Display inflated image */
    SAMAPP_INFO_START;
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoDl_bitmapSource_ex(s_pHalContext, 0, 0);
    EVE_CoDl_bitmapLayout(s_pHalContext, INFLATED_BITMAP_FORMAT, INFLATED_BITMAP_STRIDE, INFLATED_BITMAP_HEIGHT);
    EVE_CoDl_bitmapSize(s_pHalContext, BILINEAR, BORDER, BORDER, INFLATED_BITMAP_WIDTH, INFLATED_BITMAP_HEIGHT);
    EVE_CoDl_vertex2f(s_pHalContext, 100 * 16, 100 * 16);
    EVE_CoDl_end(s_pHalContext);

    /*  Display the text information */
    EVE_CoDl_colorA(s_pHalContext, 255);
    EVE_CoCmd_text(s_pHalContext, 20, 50, 24, 0, "Display bitmap by inflate (OPT_FLASH)");
    SAMAPP_INFO_END;

    SAMAPP_DELAY_NEXT;
#endif // defined (BT81X_ENABLE)
}

/**
* @brief API to demonstrate display bitmap by inflate data from media fifo
*
*/
void SAMAPP_Utility_CmdInflateFromFifo()
{
#if defined (BT81X_ENABLE)
    Draw_Text(s_pHalContext, "Example for: CMD_INFLATE2 with data from media fifo");

    /** Address in RAM_G */
#define MEDIAFIFO_SIZE (100*1024)
#define RAM_MEDIAFIFO_ADDR (RAM_G_SIZE - MEDIAFIFO_SIZE)

    /* INFLATED BITMAP information */
#define INFLATED_BITMAP_WIDTH (63)  /* color_.bin */
#define INFLATED_BITMAP_HEIGHT (44)
#define INFLATED_BITMAP_FORMAT (ARGB4)
#define INFLATED_BITMAP_STRIDE (63*2)

    /*Load data to mediafifo */
    uchar8_t* imagefile[] = { TEST_DIR "\\color_.bin", 0 };

    Ftf_Write_FileArr_To_RAM_G(s_pHalContext, imagefile, RAM_MEDIAFIFO_ADDR);
    EVE_CoCmd_mediaFifo(s_pHalContext, RAM_MEDIAFIFO_ADDR, MEDIAFIFO_SIZE); //address of the media fifo buffer
    EVE_Cmd_waitFlush(s_pHalContext);

    EVE_Hal_wr32(s_pHalContext, REG_MEDIAFIFO_WRITE, 890); // hardcoded for color_.bin file

    helperCoprocessorFaultReport();

    EVE_CoCmd_inflate2(s_pHalContext, RAM_G, OPT_MEDIAFIFO);
    EVE_Cmd_waitFlush(s_pHalContext);

    helperCoprocessorFaultReport();

    SAMAPP_INFO_START;
    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE2(0, 0));
    EVE_Cmd_wr32(s_pHalContext,
        BITMAP_LAYOUT(INFLATED_BITMAP_FORMAT, INFLATED_BITMAP_STRIDE, INFLATED_BITMAP_HEIGHT));
    EVE_Cmd_wr32(s_pHalContext,
        BITMAP_SIZE(BILINEAR, BORDER, BORDER, INFLATED_BITMAP_WIDTH, INFLATED_BITMAP_HEIGHT));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(100 * 16, 100 * 16));
    EVE_Cmd_wr32(s_pHalContext, END());

    /*  Display the text information */
    EVE_Cmd_wr32(s_pHalContext, COLOR_A(255));
    EVE_CoCmd_text(s_pHalContext, 20, 50, 23, 0, "Display bitmap by inflate from media fifo");
    SAMAPP_INFO_END;

    helperCoprocessorFaultReport();

    SAMAPP_DELAY_NEXT;
#endif // defined (BT81X_ENABLE)
}

/**
* @brief API to demonstrate the usage of inflate command - compression done via zlib
*
*/
void SAMAPP_Utility_CmdInflateFromCommand()
{
#define BUFFERSIZE 8192
    char pbuff[BUFFERSIZE];
    const SAMAPP_Bitmap_header_t* pBitmapHdr = NULL;
    char* file = TEST_DIR "\\lenaface40.bin";
    int16_t xoffset;
    int16_t yoffset;

    Draw_Text(s_pHalContext, "Example for: CMD_INFLATE with data from command fifo");

    if (0 >= FileIO_File_Open(file, FILEIO_E_FOPEN_READ))
    {
        printf("Error in opening file %s \n", file);
        return;
    }
    /**********************************************************************************/
    /* Below code demonstrates the usage of inflate function                          */
    /* Download the deflated data into command buffer and in turn coprocessor inflate */
    /* the deflated data and outputs at 0 location                                    */
    /**********************************************************************************/
    pBitmapHdr = &SAMAPP_Bitmap_RawData_Header[0];

    xoffset = (int16_t) ((s_pHalContext->Width - SAMAPP_Bitmap_RawData_Header[0].Width) / 2);
    yoffset = (int16_t) ((s_pHalContext->Height - SAMAPP_Bitmap_RawData_Header[0].Height) / 2);

    /* Clear the memory at location 0 - any previous bitmap data */
    EVE_Cmd_wr32(s_pHalContext, CMD_MEMSET);
    EVE_Cmd_wr32(s_pHalContext, 0L); //starting address of memset
    EVE_Cmd_wr32(s_pHalContext, 255L); //value of memset
                                       //number of elements to be changed
    EVE_Cmd_wr32(s_pHalContext, 1L * pBitmapHdr->Stride * pBitmapHdr->Height);

    /* Set the display list for graphics processor */
    /* Bitmap construction by MCU - display lena at 200x90 offset */
    /* Transfer the data into coprocessor memory directly word by word */
    SAMAPP_INFO_START;
    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(0));
    EVE_Cmd_wr32(s_pHalContext,
        BITMAP_LAYOUT(SAMAPP_Bitmap_RawData_Header[0].Format,
            SAMAPP_Bitmap_RawData_Header[0].Stride, SAMAPP_Bitmap_RawData_Header[0].Height));
    EVE_Cmd_wr32(s_pHalContext,
        BITMAP_SIZE(BILINEAR, BORDER, BORDER, SAMAPP_Bitmap_RawData_Header[0].Width,
            SAMAPP_Bitmap_RawData_Header[0].Height));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(xoffset * 16, yoffset * 16));
    EVE_Cmd_wr32(s_pHalContext, END());

    /*  Display the text information */
    EVE_Cmd_wr32(s_pHalContext, COLOR_A(255));
    xoffset -= 50;
    yoffset += 40;
    EVE_CoCmd_text(s_pHalContext, xoffset, yoffset, 26, 0, "Display bitmap by inflate from command fifo");
    SAMAPP_INFO_END;

    /* inflate the data read from binary file */
    EVE_Cmd_wr32(s_pHalContext, CMD_INFLATE);
    EVE_Cmd_wr32(s_pHalContext, 0); //destination address if inflate
    int bytes = FileIO_File_Read(pbuff, BUFFERSIZE);
    while (bytes)
    {
        /* download the data into the command buffer by 2kb one shot */
        uint16_t blocklen = bytes > BUFFERSIZE ? BUFFERSIZE : (uint16_t) bytes;

        /* copy data continuously into command memory */
        EVE_Cmd_wrMem(s_pHalContext, pbuff, blocklen); //alignment is already taken care by this api
        bytes = FileIO_File_Read(pbuff, BUFFERSIZE);
    }
    EVE_Cmd_waitFlush(s_pHalContext);
    SAMAPP_DELAY_NEXT;
}

/**
* @brief API to demonstrate CMD_FILLWIDTH
*
*/
void SAMAPP_Utility_fillWidth()
{
#if defined (BT81X_ENABLE)
    int16_t x;
    int16_t y;
    int16_t fill_w;

    Draw_Text(s_pHalContext, "Example for: CMD_FILLWIDTH");

    SAMAPP_INFO_START;
    fill_w = 80;
    EVE_CoCmd_fillWidth(s_pHalContext, fill_w);

    x = 100;
    y = 20;
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 255, 0));
    EVE_Cmd_wr32(s_pHalContext, BEGIN(RECTS));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(x * 16, y * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F((x + fill_w) * 16, (y + 150) * 16));
    EVE_Cmd_wr32(s_pHalContext, END());
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
    EVE_CoCmd_text(s_pHalContext, x, y, 30, OPT_FILL, "one two three four");

    x = 400;
    y = 70;
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 255, 0));
    EVE_Cmd_wr32(s_pHalContext, BEGIN(RECTS));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F((x - fill_w / 2) * 16, (y - 65) * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F((x + fill_w / 2) * 16, (y + 70) * 16));
    EVE_Cmd_wr32(s_pHalContext, END());
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
    EVE_CoCmd_text(s_pHalContext, x, y, 30, OPT_FILL | OPT_CENTER, "one two three four");

    y = 20 + 20 * 10;
    x = 100;
    fill_w = 2;
    EVE_CoCmd_fillWidth(s_pHalContext, fill_w);

    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 255, 0));
    EVE_Cmd_wr32(s_pHalContext, BEGIN(RECTS));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(x * 16, y * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F((x + 80) * 16, (y + 150) * 16));
    EVE_Cmd_wr32(s_pHalContext, END());
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
    EVE_CoCmd_text(s_pHalContext, x, y, 30, OPT_FILL, "one two three four");

    x = 400;
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 255, 0));
    EVE_Cmd_wr32(s_pHalContext, BEGIN(RECTS));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(x * 16, y * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F((x + 160) * 16, (y + 70) * 16));
    EVE_Cmd_wr32(s_pHalContext, END());
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
    EVE_CoCmd_text(s_pHalContext, x, y, 30, 0, "one two \nthree four");
    SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;

    // Cover the cmd_button,cmd_toggle too. 
    SAMAPP_INFO_START;
    EVE_Cmd_wr32(s_pHalContext, CLEAR(255, 255, 255));
    y = (int16_t) (s_pHalContext->Height / 2);
    fill_w = 200;
    EVE_CoCmd_fillWidth(s_pHalContext, fill_w);
    EVE_CoCmd_button(s_pHalContext, 100, y, fill_w, 100, 30, OPT_FILL, "one two three four");
    EVE_CoCmd_toggle(s_pHalContext, 400, y, fill_w, 30, OPT_FLAT, 0, "one two three four");
    SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;
#endif // defined (BT81X_ENABLE)
}

/**
* @brief API to demonstrate OTP_FORMAT
*
*/
void SAMAPP_Utility_printType() //must call after SAMAPP_ExtendedFormat_Font
{
#if defined (BT81X_ENABLE)
    Draw_Text(s_pHalContext, "Example for: OPT_FORMAT");
    uint8_t c = 51;
    uint32_t a = 0x12a000;
    int16_t mV = 1947;
    int32_t t = 680;

    SAMAPP_INFO_START;
    EVE_CoCmd_text(s_pHalContext, 10, 20, 31, 0, "%");
    EVE_CoCmd_text(s_pHalContext, 10, 20 + 50 * 1, 31, 0, "%%");
    EVE_CoCmd_text(s_pHalContext, 10, 20 + 50 * 2, 31, 0, "%d%%%");
    EVE_CoCmd_text(s_pHalContext, 10, 20 + 50 * 3, 31, OPT_FORMAT, "%d * %d = %d", 100, 200, 20000);
    EVE_CoCmd_text(s_pHalContext, 10, 20 + 50 * 4, 31, OPT_FORMAT, "%3d%% complete", c); /*result:  51 % complete */
    EVE_CoCmd_text(s_pHalContext, 10, 20 + 50 * 5, 31, OPT_FORMAT, "base address %06x", a); /*result:  base address 12a000 */
    EVE_CoCmd_text(s_pHalContext, 10, 20 + 50 * 6, 31, OPT_FORMAT, "%+5.3d mV", mV); /*result:  + 1947 mV */
    EVE_CoCmd_text(s_pHalContext, 10, 20 + 50 * 7, 31, OPT_FORMAT, "Temp. %d.%.1d degrees", t / 10,
        t % 10); /*result:  Temp. 68.0 degrees */

    EVE_Hal_wrMem(s_pHalContext, RAM_G + 4, "Hello\0", 6);
    EVE_CoCmd_text(s_pHalContext, 10, 20 + 50 * 8, 31, OPT_FORMAT, "%s %d times", RAM_G + 4, 5); /*result:  Temp. 68.0 degrees */

    SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;
#endif // defined (BT81X_ENABLE) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM))
}

/**
* @brief Change screen orientation from landscape to portrait mode.  
* 
* Setscratch command is also introduced, it sets the temporary bitmap handle for buttons, 
* keys, and graidents.
*
*/
void SAMAPP_Utility_screenRotate()
{
#if defined(FT81X_ENABLE) // FT81X only
    uint8_t text[100];

    Draw_Text(s_pHalContext, "Example for: Change screen orientation from landscape to portrait mode");

    for (uint32_t rotateMode = 0; rotateMode < 8; rotateMode++)
    {
        SAMAPP_INFO_START;
        EVE_CoCmd_setRotate(s_pHalContext, rotateMode);
        switch (rotateMode)
        {
        case 0:
        case 1:
        case 4:
        case 5:
            snprintf(text, 100, "Landscape Mode, rotate value= %d", rotateMode);
            EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 50, 29, OPT_CENTER,
                text);
            break;
        default:
            snprintf(text, 100, "Portrait Mode\nRotate value= %d", rotateMode);
            EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Height / 2), 50, 29, OPT_CENTER,
                text);
            break;
        }

        SAMAPP_INFO_END;
        SAMAPP_DELAY_NEXT;
    }

    //reset the rotation angle
    EVE_CoCmd_setRotate(s_pHalContext, 0);
#endif
}

/**
* @brief API to demonstrate number base
*
*/
void SAMAPP_Utility_numberBases()
{
#if defined(FT81X_ENABLE) // FT81X only
    Draw_Text(s_pHalContext, "Example for: Number base");

    SAMAPP_INFO_START;
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 50, 31, OPT_CENTER,
        "Built-in bases conversion");
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 100, 31, OPT_CENTER,
        "bases from 2 to 32:");

#ifdef DISPLAY_RESOLUTION_WVGA
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2) - 130, 150, 30, 0,
        "Binary(2):");
    EVE_CoCmd_setBase(s_pHalContext, 2);
    EVE_CoCmd_number(s_pHalContext, (int16_t) (s_pHalContext->Width / 2 + 80), 150, 30, 0, 1000);

    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2 - 130), 200, 30, 0,
        "Octal(8):");
    EVE_CoCmd_setBase(s_pHalContext, 8);
    EVE_CoCmd_number(s_pHalContext, (int16_t) (s_pHalContext->Width / 2 + 80), 200, 30, 0, 1000);

    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2 - 130), 250, 30, 0,
        "Decimal(10):");
    EVE_CoCmd_setBase(s_pHalContext, 10);
    EVE_CoCmd_number(s_pHalContext, (int16_t) (s_pHalContext->Width / 2 + 80), 250, 30, 0, 1000);

    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2 - 130), 300, 30, 0,
        "Hex(16):");
    EVE_CoCmd_setBase(s_pHalContext, 16);
    EVE_CoCmd_number(s_pHalContext, (int16_t) (s_pHalContext->Width / 2 + 80), 300, 30, 0, 1000);
#else
    EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2 - 130), 150, 30, 0, "Binary(2):");
    EVE_CoCmd_setBase(s_pHalContext, 2);
    EVE_CoCmd_number(s_pHalContext, s_pHalContext->Width / 2 + 80, 150, 30, 0, 1000);

    EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2 - 130), 180, 30, 0, "Octal(8):");
    EVE_CoCmd_setBase(s_pHalContext, 8);
    EVE_CoCmd_number(s_pHalContext, s_pHalContext->Width / 2 + 80, 180, 30, 0, 1000);

    EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2 - 130), 230, 30, 0, "Decimal(10):");
    EVE_CoCmd_setBase(s_pHalContext, 10);
    EVE_CoCmd_number(s_pHalContext, s_pHalContext->Width / 2 + 80, 230, 30, 0, 1000);

    EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2 - 130), 280, 30, 0, "Hex(16):");
    EVE_CoCmd_setBase(s_pHalContext, 16);
    EVE_CoCmd_number(s_pHalContext, s_pHalContext->Width / 2 + 80, 280, 30, 0, 1000);
#endif

    //Revert the base back to decimal because numeric base is part of the global context.
    EVE_CoCmd_setBase(s_pHalContext, 10);

    SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;
#endif
}

/**
* @brief read back the result of a command
*
*/
void SAMAPP_Utility_crcCheck()
{
    const uint32_t memWrite = 0x66666666; // 66 = 'f'
    const uint32_t memSizeTest = 1024; // bytes
    const uint32_t crcExpected = 0x17D32620;
    uint32_t memcrcRet = 0;

    Draw_Text(s_pHalContext, "Example for: Check Crc32 for 1024 bytes on RAM_G");

    EVE_CoCmd_memSet(s_pHalContext, RAM_G, memWrite, memSizeTest);
    EVE_Cmd_waitFlush(s_pHalContext);/*reset cmd index*/

    uint16_t cmdbuff_write_ptr = EVE_Cmd_wp(s_pHalContext);
    uint32_t crc_result_addr = RAM_CMD + ((cmdbuff_write_ptr + 12) & 4095);

    EVE_CoCmd_memCrc(s_pHalContext, 0, memSizeTest, 0);
    EVE_Cmd_waitFlush(s_pHalContext);

    memcrcRet = EVE_Hal_rd32(s_pHalContext, crc_result_addr);
    printf("current CRC number [0,1023) is 0x%x \r\n", memcrcRet);

    if (memcrcRet == crcExpected)
    {
        printf("Crc return value is as expected");
        SAMAPP_INFO_TEXT( "Crc value is as expected");
    }
    SAMAPP_DELAY_NEXT;
}

/**
* @brief Sample app to demonstrate snapshot widget/functionality
*
*/
void SAMAPP_Utility_snapshot()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of snapshot function. Snapshot func */
    /* captures the present screen and dumps into bitmap with color formats  */
    /* argb4.  FT81x supports a simplier snapshot2 command to capture the    */
    /* screen, both methods can be used in FT81X.                            */
    /*************************************************************************/
    uint16_t bitmapWidth = 160;
    uint16_t bitmapHeight = 120;
    uint16_t RGB565RamOffset = 0;
    uint16_t ARGB4RamOffset = RGB565RamOffset + bitmapWidth * 2 * bitmapHeight;
#ifndef BT815_ENABLE
    fadeout(s_pHalContext);
#endif

    Draw_Text(s_pHalContext, "Example for: Snapshot widget/functionality");

#ifndef FT81X_ENABLE

#if !defined(ME800A_HV35R) && !defined(ME810A_HV35R)
    /* fadeout before switching off the pclock */
    /* Switch off the lcd */
    {
        uint8_t n = 0;
        EVE_Hal_wr8(s_pHalContext, REG_GPIO, 0x7f);
        EVE_sleep(100);
    }
#endif
    uint32_t WriteByte = 0;
    /* Disable the pclock */
    EVE_Hal_wr8(s_pHalContext, REG_PCLK, WriteByte);
    /* Configure the resolution to 160x120 dimention */
    WriteByte = bitmapWidth;
    EVE_Hal_wr16(s_pHalContext, REG_HSIZE, WriteByte);
    WriteByte = bitmapHeight;
    EVE_Hal_wr16(s_pHalContext, REG_VSIZE, WriteByte);
#endif

    /* Construct screen shot for snapshot */
    SAMAPP_INFO_START;
    /* captured snapshot */
    EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
    EVE_Cmd_wr32(s_pHalContext, COLOR_A(128));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(20 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0 * 16, 0 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(25 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(20 * 16, 10 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(30 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(40 * 16, 20 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(35 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(60 * 16, 30 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(40 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(80 * 16, 40 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(45 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(100 * 16, 50 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(50 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(120 * 16, 60 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(55 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(140 * 16, 70 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(60 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(160 * 16, 80 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(65 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0 * 16, 120 * 16));
    EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(70 * 16));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(160 * 16, 0 * 16));
    EVE_Cmd_wr32(s_pHalContext, END()); //display the bitmap at the center of the display
    EVE_Cmd_wr32(s_pHalContext, COLOR_A(255));
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(32, 32, 32));
    EVE_CoCmd_text(s_pHalContext, 80, 60, 26, OPT_CENTER, "Points");

    SAMAPP_INFO_END;
    EVE_sleep(100); //timeout for snapshot to be performed by coprocessor

#ifndef FT81X_ENABLE
    /* Take snap shot of the current screen */
    EVE_Cmd_wr32(s_pHalContext, CMD_SNAPSHOT);
    EVE_Cmd_wr32(s_pHalContext, 3200); //store the rgb content at location 3200

    //timeout for snapshot to be performed by coprocessor
    /* Wait till coprocessor completes the operation */
    EVE_Cmd_waitFlush(s_pHalContext);
    EVE_sleep(100); //timeout for snapshot to be performed by coprocessor

    /* reconfigure the resolution wrt configuration */
    WriteByte = s_pHalContext->Width;
    EVE_Hal_wr16(s_pHalContext, REG_HSIZE, WriteByte);
    WriteByte = s_pHalContext->Height;
    EVE_Hal_wr16(s_pHalContext, REG_VSIZE, WriteByte);
#else
    EVE_CoCmd_snapshot2(s_pHalContext, ARGB4, ARGB4RamOffset, 0, 0, bitmapWidth, bitmapHeight);
    /* Wait till coprocessor completes the operation */
    EVE_Cmd_waitFlush(s_pHalContext);

    EVE_sleep(100); //timeout for snapshot to be performed by coprocessor

    EVE_CoCmd_snapshot2(s_pHalContext, RGB565, RGB565RamOffset, 0, 0, bitmapWidth, bitmapHeight);
    /* Wait till coprocessor completes the operation */
    EVE_Cmd_waitFlush(s_pHalContext);

    EVE_sleep(100); //timeout for snapshot to be performed by coprocessor
#endif

    SAMAPP_INFO_START;
    /* captured snapshot */

#ifndef FT81X_ENABLE
    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(3200));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(ARGB4, bitmapWidth * 2, bitmapHeight));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(BILINEAR, BORDER, BORDER, bitmapWidth, bitmapHeight));
    EVE_Cmd_wr32(s_pHalContext, VERTEX2F(((s_pHalContext->Width - 160) / 2) * 16, ((s_pHalContext->Height - 120) / 2) * 16));
    EVE_Cmd_wr32(s_pHalContext, END()); //display the bitmap at the center of the display
                                        /* Display the text info */
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(32, 32, 32));
    EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 40, 27, OPT_CENTER, "Snap shot");
#else
    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(RGB565RamOffset));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(RGB565, bitmapWidth * 2, bitmapHeight));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, bitmapWidth, bitmapHeight));
    EVE_Cmd_wr32(s_pHalContext,
        VERTEX2F(((s_pHalContext->Width) / 6) * 16, (int16_t )((s_pHalContext->Height) / 2) * 16));
    EVE_Cmd_wr32(s_pHalContext, END()); //display the bitmap at the center of the display
    /* Display the text info */
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(32, 32, 32));
    EVE_CoCmd_text(s_pHalContext, (int16_t) ((s_pHalContext->Width) / 6),
        (int16_t) ((s_pHalContext->Height) / 2) - 20, 27, 0, "RGB565");

    EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(ARGB4RamOffset));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(ARGB4, bitmapWidth * 2, bitmapHeight));
    EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, bitmapWidth, bitmapHeight));
    EVE_Cmd_wr32(s_pHalContext,
        VERTEX2F(((s_pHalContext->Width) / 2) * 16, (int16_t )((s_pHalContext->Height) / 2) * 16));
    EVE_Cmd_wr32(s_pHalContext, END()); //display the bitmap at the center of the display
    
    /* Display the text info */
    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(32, 32, 32));
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2),
        (int16_t) ((s_pHalContext->Height) / 2) - 20, 27, 0, "ARGB4");

    EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(32, 32, 32));
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 40, 30, OPT_CENTER,
        "Snap Shot");
#endif
    SAMAPP_INFO_END;

#ifndef FT81X_ENABLE
    /* reenable the pclock */
    WriteByte = s_pHalContext->PCLK;
    EVE_Hal_wr8(s_pHalContext, REG_PCLK, WriteByte);
    EVE_sleep(60);
#if !defined(ME800A_HV35R) && !defined(ME810A_HV35R)
    /* Power on the LCD */
    {
        uint8_t n = 0x80;
        EVE_Hal_wr8(s_pHalContext, REG_GPIO, 0xff);
    }
#endif
#endif
    EVE_sleep(200); //give some time for the lcd to switchon - hack for one perticular panel

    /* set the display pwm back to 128 */
#ifndef BT815_ENABLE
    fadein(s_pHalContext);
#endif
    SAMAPP_DELAY_NEXT;
}

/**
* @brief Sample app to demonstrate cmd_snapshot2
* 
* Snapshot2 can generate RGB565 / ARGB8 / ARGB4 image into ram_g
* If the EVE is using ram_g to display, it will be conflict with cmd_snapshot2, be careful
* In case of ARGB8, the width value must be double
*
*/
void SAMAPP_Utility_snapshot2()
{
	int noImage = 0;
	int haveImage = 1;

    Draw_Text(s_pHalContext, "Example for: cmd_Snapshot2");

    helperSimpleScreen("Capture the screen line by line", haveImage); /// construct a screen to be captured
	/// 1 = Capture the screen line by line, height = Capture the whole screen
	helperSnapshot2(RGB565, TEST_DIR "SAMAPP_Utility_snapshot2_line_by_line_rgb565.raw", 1); 
	Draw_Text_Format(s_pHalContext, "Cmd_snapshot2 line by line finished");

	helperSimpleScreen("Cmd_snapshot2 with ARGB8 need to double the Width value", haveImage); /// construct a screen to be captured
	helperSnapshot2(0x20, TEST_DIR "SAMAPP_Utility_snapshot2_argb8.raw", s_pHalContext->Height/20); /// 20 lines per scan
	Draw_Text_Format(s_pHalContext, "Cmd_snapshot2 with ARGB8 finished");

	helperSimpleScreen("Cmd_snapshot2 with RGB565", haveImage);
	helperSnapshot2(RGB565, TEST_DIR "SAMAPP_Utility_snapshot2_rgb565.raw", s_pHalContext->Height / 20); /// 20 lines per scan
	Draw_Text_Format(s_pHalContext, "Cmd_snapshot2 with RGB565 finished");
	
	helperSimpleScreen("Cmd_snapshot2 with ARGB4", haveImage); /// construct a screen to be captured
	helperSnapshot2(ARGB4, TEST_DIR "SAMAPP_Utility_snapshot2_argb4.raw", s_pHalContext->Height/20); /// 20 lines per scan
	Draw_Text_Format(s_pHalContext, "Cmd_snapshot2 with ARGB4 finished");
}

/**
* @brief Draw set 11
*
*/
void SAMAPP_Utility_bulkTransfer()
{
    SAMAPP_Circle_t circles[100];
    uint32_t precision = 16;
    uint32_t CNUM = 100; // Disable circles
    uint32_t ImgW = 256;
    uint32_t ImgH = 256;
    uint32_t xoffset = (s_pHalContext->Width - ImgW) / 2;
    uint32_t yoffset = (s_pHalContext->Height - ImgH) / 2;
    int count = 0;

    Draw_Text(s_pHalContext, "Example for: Bulk transfer");

    for (int i = 0; i < (int) CNUM; i++)
    {
        circles[i].visible = 0;
        circles[i].opacity = 0;
        circles[i].radius = 0;
        circles[i].step = 0;
        circles[i].visible = 0;
        circles[i].x = 0;
        circles[i].y = 0;
        circles[i].color.b = 0;
        circles[i].color.g = 0;
        circles[i].color.r = 0;
    }

    Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "\\mandrill256.jpg", 0, LOADIMAGE);

    while (count++ < 60 * 10)
    { // wait 10 seconds, 60 FPS
      /*Display List start*/
        helperCMDBWrite(CMD_DLSTART);
        helperCMDBWrite(CLEAR_COLOR_RGB(0, 0, 0));
        helperCMDBWrite(CLEAR(1, 1, 1));
        // draw circles
        for (int i = 0; i < (int) CNUM; i++)
        {
            int visible = rand() % 3;
            int x = rand() % s_pHalContext->Width;
            int y = rand() % s_pHalContext->Height;
            int radius = rand() % 20;
            int r = rand() % 255;
            int g = rand() % 255;
            int b = rand() % 255;
            int step = rand() % 100 / 20 + 1;

            if (visible != 1)
            {
                visible = 0;
            }

            if (circles[i].visible == 0 && visible == 1)
            {
                circles[i].visible = 1;
                circles[i].x = x;
                circles[i].y = y;
                circles[i].radius = radius;
                circles[i].color.r = r;
                circles[i].color.g = g;
                circles[i].color.b = b;
                circles[i].opacity = 0;
                circles[i].step = step;
            }
            else if (circles[i].opacity >= 255)
            {
                circles[i].step = -circles[i].step;
            }
            else if (circles[i].opacity <= 0)
            {
                circles[i].visible = 0;
            }
            circles[i].opacity += circles[i].step;

            if (circles[i].visible)
            {
                helperCMDBWrite(COLOR_A(circles[i].opacity));
                helperCMDBWrite(
                    COLOR_RGB(circles[i].color.r, circles[i].color.g, circles[i].color.b));

                helperCMDBWrite(BEGIN(FTPOINTS));
                helperCMDBWrite(POINT_SIZE(circles[i].radius * precision));
                helperCMDBWrite(VERTEX2F(circles[i].x * precision, circles[i].y * precision));
                helperCMDBWrite(END());
            }
        }
        // reset coloring
        helperCMDBWrite(COLOR_A(255));
        helperCMDBWrite(COLOR_RGB(255, 255, 255));

        // draw image
        helperCMDBWrite(BITMAP_HANDLE(2));
        helperCMDBWrite(BEGIN(BITMAPS));
        helperCMDBWrite(BITMAP_SOURCE(0));
        helperCMDBWrite(BITMAP_LAYOUT(RGB565, ImgW * 2, ImgH));
		helperCMDBWrite(BITMAP_LAYOUT_H((ImgW * 2) >> 10, ImgH >> 9));
        helperCMDBWrite(BITMAP_SIZE(BILINEAR, BORDER, BORDER, ImgW, ImgH));
		helperCMDBWrite(BITMAP_SIZE_H(ImgW >> 9, ImgH >> 9));
        helperCMDBWrite(VERTEX2F(xoffset * precision, yoffset * precision));
        helperCMDBWrite(END());

        // Draw the text in top of screen
        uint32_t x = s_pHalContext->Width / 2 - 120;
        uint32_t y = 10;
        uint32_t font = 28;
        uint32_t opt = 0;
        helperCMDBWrite(CMD_TEXT);
        helperCMDBWrite(((y << 16) | (x & 0xffff)));
        helperCMDBWrite(((opt << 16) | (font & 0xffff)));
        helperCMDBWriteString("Bulk transfer demostration");

        y = 50;
        x -= 200;
        helperCMDBWrite(CMD_TEXT);
        helperCMDBWrite(((y << 16) | (x & 0xffff)));
        helperCMDBWrite(((opt << 16) | (font & 0xffff)));
        helperCMDBWriteString("Commands are transferd into REG_CMDB_WRITE instead of RAM_CMD");

        helperCMDBWrite(DISPLAY()); //send command display - to end display commands
        helperCMDBWrite(CMD_SWAP); // draw the new screen
        
        // Flush all the command
        EVE_Cmd_waitFlush(s_pHalContext); // Wait until EVE is free
    }
}

void SAMAPP_Utility() {
    SAMAPP_Utility_snapshot2();
    SAMAPP_Utility_wait();
    SAMAPP_Utility_apiLevel();
    SAMAPP_Utility_callList();
    SAMAPP_Utility_callListWithAlignment();
    SAMAPP_Utility_underRunDetection();
    SAMAPP_Utility_coprocessorFaultRecover();
	SAMAPP_Utility_bulkTransfer();
    SAMAPP_Utility_cmdInflateFromFlash();
    SAMAPP_Utility_CmdInflateFromFifo();
    SAMAPP_Utility_CmdInflateFromCommand();
    SAMAPP_Utility_fillWidth();
    SAMAPP_Utility_printType();
    SAMAPP_Utility_screenRotate();
    SAMAPP_Utility_numberBases();
    SAMAPP_Utility_crcCheck();
	SAMAPP_Utility_snapshot();
}


