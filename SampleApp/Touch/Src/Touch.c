/**
 * @file Touch.c
 * @brief Sample usage of touching
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
#include "Touch.h"

#define SAMAPP_DELAY_NEXT      EVE_sleep(2000);

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void SAMAPP_Touch();
static int8_t Volume;

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
        "This sample demonstrate the using of touch", 
        "",
        ""
    }; 

    while (TRUE) {
        WelcomeScreen(s_pHalContext, info);

        SAMAPP_Touch();

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
* @brief Calculate rectangle limits and positions
*
* @param context BouncingSquares context
* @param Arrayno Rectangle id
*/
static void helperRectangleCalc(SAMAPP_BouncingSquares* context, uint8_t Arrayno)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
    if(context->RectTouched[Arrayno] == 1)
	{
		//the limits for the smaller rectangles forward and backward movement is set here
		if (context->My[Arrayno] <= 0)
			context->Decrease[Arrayno] = 0; //increase
		else if (context->My[Arrayno] + 5 * 1 >= context->Ty[Arrayno])
			context->Decrease[Arrayno] = 1; //decrease

		// the smaller rectangles are moved accordingly according to the flags set above in this function call
		if (context->Decrease[Arrayno] == 1)
			context->My[Arrayno] -= 1; //the smaller rectangles are moved upwards
		else if (context->Decrease[Arrayno] == 0) // increase
			context->My[Arrayno] += 2 * 1; //the smaller rectangles are moved downwards slightly faster
	}
#endif // capacity EVE
}

/**
* @brief Beginning BouncingCircle section
*
* @param C Point size
*/
static void helperDrawConcentricCircles(float C)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
	EVE_CoDl_stencilFunc(s_pHalContext, NEVER, 0x00, 0x00);
	EVE_CoDl_stencilOp(s_pHalContext, INCR, INCR);
	EVE_CoDl_begin(s_pHalContext, POINTS);
	EVE_CoDl_pointSize(s_pHalContext, (uint16_t)((C - 5) * 16)); //inner circle
    EVE_CoDl_vertex2f(s_pHalContext, VP(240), VP(136));

    EVE_CoDl_stencilFunc(s_pHalContext, NOTEQUAL, 0x01, 0x01);
	EVE_CoDl_pointSize(s_pHalContext, (uint16_t)((C)*16)); //outer circle
    EVE_CoDl_vertex2f(s_pHalContext, VP(240), VP(136));

    EVE_CoDl_stencilFunc(s_pHalContext, EQUAL, 0x01, 0x01);
	EVE_CoDl_stencilOp(s_pHalContext, KEEP, KEEP);
	EVE_CoDl_colorRgb(s_pHalContext, 255, 0, 0);
	EVE_CoDl_pointSize(s_pHalContext, (uint16_t)((C)*16));
    EVE_CoDl_vertex2f(s_pHalContext, VP(240), VP(136));

    EVE_CoDl_stencilFunc(s_pHalContext, ALWAYS, 0x01, 0x01);
	EVE_CoDl_stencilOp(s_pHalContext, KEEP, KEEP);

	EVE_CoDl_end(s_pHalContext);
#endif // capacity EVE
}

/**
* @brief Draw touch points
*
* @param C1X Point X
* @param C1Y Point Y
* @param i Point number
*/
static void helperTouchPoints(int16_t CX, int16_t CY, uint8_t i)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
    /* Draw the five white circles for the Touch areas with their rescpective numbers*/
	EVE_CoDl_begin(s_pHalContext, POINTS);
	EVE_CoDl_pointSize(s_pHalContext, 14 * 16);
	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 0);
    EVE_CoDl_vertex2f(s_pHalContext, VP(CX), VP(CY));
	EVE_CoDl_end(s_pHalContext);
	EVE_CoDl_colorRgb(s_pHalContext, 155, 155, 0);
	EVE_CoCmd_number(s_pHalContext, CX, CY, 29, OPT_CENTERX | OPT_CENTERY, i);
#endif // capacity EVE
}

/**
* @brief check which circle has been touched based on the coordinates
*
* @param context BouncingSquares context
* @param val Touch value
* @param TouchNum Touch number
* @param i Circle number
*/
static void helperCheckCircleTouchCood(SAMAPP_BouncingCircles* context, int32_t val, uint8_t TouchNum, uint8_t i)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
	float CX = (float)(val >> 16);
	float CY = (float)(val & 0xffff);

    if (val == 0x80008000)
	{
		context->TN[i] = 0xFF;
		return;
	}

    if (context->TN[i] != TouchNum 
		&& (CX > (context->CX[i] - 15)) && (CX < (context->CX[i] + 15))
	    && (CY > (context->CY[i] - 30)) && (CY < context->CY[i] + 30))
    {
		context->CX[i] = CX;
		context->CY[i] = CY;
		context->TN[i] = TouchNum;
    }

    if (context->TN[i] == TouchNum)
	{
		context->CX[i] = CX;
		context->CY[i] = CY;
	}
#endif // capacity EVE
}

/**
* @brief calculate the radius of each circle according to the touch
*
* @param context BouncingSquares context
* @param X Touch X
* @param Y Touch Y
* @param Val Circle number
* @return
*/
static void helperCirclePlot(SAMAPP_BouncingCircles* context, uint8_t Val)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
	uint32_t Xsq;
	uint32_t Ysq;
	Xsq = (uint32_t)((uint16_t)context->CX[Val] - (s_pHalContext->Width / 2)) * ((uint16_t)context->CX[Val] - (s_pHalContext->Width / 2));
	Ysq = (uint32_t)((uint16_t)context->CY[Val] - (s_pHalContext->Height / 2)) * ((uint16_t)context->CY[Val] - (s_pHalContext->Height / 2));
	context->R[Val] = (float)(Xsq + Ysq);
	context->R[Val] = (float)sqrt(context->R[Val]);
#endif // capacity EVE
}

/**
* @brief Linear function
*
* @param p1
* @param p2
* @param t
* @param rate
* @return int16_t
*/
static int16_t helperLinear(float p1, float p2, uint16_t t, uint16_t rate)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
    float st = (float) t / rate;
    return (int16_t) (p1 + (st * (p2 - p1)));
#endif // capacity EVE
}

/**
* @brief Beginning MovingPoints section
*
* @param k Point set
* @param i POint number
*/
static void helperColorSelection(int16_t k, int16_t i)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
	if (k == 0)
	{
		if (i & 1)
			EVE_CoDl_colorRgb(s_pHalContext, 116, 27, 124); //purple
		else
			EVE_CoDl_colorRgb(s_pHalContext, 248, 134, 173); //pink
	}
	if (k == 1)
	{
		if (i & 1)
			EVE_CoDl_colorRgb(s_pHalContext, 232, 35, 25); //red
		else
			EVE_CoDl_colorRgb(s_pHalContext, 240, 135, 132); //light red
	}
	if (k == 2)
	{
		if (i & 1)
			EVE_CoDl_colorRgb(s_pHalContext, 248, 130, 58); //orange
		else
			EVE_CoDl_colorRgb(s_pHalContext, 255, 253, 85); //yellow
	}
	if (k == 3)
	{
		if (i & 1)
			EVE_CoDl_colorRgb(s_pHalContext, 0, 35, 245); //blue
		else
			EVE_CoDl_colorRgb(s_pHalContext, 115, 251, 253); //light blue
	}
	if (k == 4)
	{
		if (i & 1)
			EVE_CoDl_colorRgb(s_pHalContext, 55, 126, 71); //green
		else
			EVE_CoDl_colorRgb(s_pHalContext, 161, 251, 142); //light green
	}
#endif // capacity EVE
}

/**
* @brief Calculate offset movements
*
* @param context MovingPpoints context
* @param TouchNo Touch number
* @param X Touch X
* @param Y Touch Y
* @param t Point number
*/
static void helperPointsCalc(SAMAPP_MovingPoints* context)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
	uint8_t touchNo = 0;
    int16_t tempDeltaX;
    int16_t tempDeltaY;

    /* For total number of points calculate the offsets of movement */
	for (int16_t k = 0; k < NO_OF_POINTS * NO_OF_TOUCH; k++)
    {
		touchNo = k / NO_OF_POINTS;
		if (context->t[k] > NO_OF_POINTS)
        {
            context->t[k] = 0;
			context->X[k] = (context->val[touchNo] >> 16) & 0xffff;
			context->Y[k] = (context->val[touchNo] & 0xffff);
        }

        if ((context->X[k] != 0x8000) && (context->Y[k] != 0x8000))
        {
			tempDeltaX = helperLinear(context->X[k], context->StopX[touchNo], context->t[k], NO_OF_POINTS);
			tempDeltaY = helperLinear(context->Y[k], context->StopY, context->t[k], NO_OF_POINTS);
			helperColorSelection(touchNo, k);
            EVE_CoDl_vertex2f(s_pHalContext, VP(tempDeltaX), VP(tempDeltaY));
        }
		context->t[k]++;
    }
#endif // capacity EVE
}

/**
* @brief Beginning BouncingPoints section
*
* @param pBInst Blob instance
* @param TouchXY Touch value
*/
static void helperBlobColor(SAMAPP_BlobsInst* pBInst, int32_t TouchXY)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
    uint8_t j = 0;
    // if there is touch store the values
	if (TouchXY != 0x80008000)
    {
        pBInst->blobs[pBInst->CurrIdx].x = (TouchXY >> 16) & 0xffff;
        pBInst->blobs[pBInst->CurrIdx].y = (TouchXY & 0xffff);
    }
    else
    {
        pBInst->blobs[pBInst->CurrIdx].x = OFFSCREEN;
        pBInst->blobs[pBInst->CurrIdx].y = OFFSCREEN;
    }

    //calculate the current index
    pBInst->CurrIdx = (pBInst->CurrIdx + 1) & (NBLOBS - 1);

    EVE_CoDl_begin(s_pHalContext, POINTS);
	EVE_CoDl_colorRgb(s_pHalContext, 60, 166, 117);
    for (uint8_t i = 0; i < NBLOBS; i++)
    {
        // Blobs fade away and swell as they age
		EVE_CoDl_colorA(s_pHalContext, i << 1);
		EVE_CoDl_pointSize(s_pHalContext, 68 + (i << 3));

        // Random color for each blob, keyed from (blob_i + i)
        j = (pBInst->CurrIdx + i) & (NBLOBS - 1);

        // Draw it!
        if (pBInst->blobs[j].x != OFFSCREEN)
            EVE_CoDl_vertex2f(s_pHalContext,
                VP(pBInst->blobs[j].x), VP(pBInst->blobs[j].y));
    }
	EVE_CoDl_end(s_pHalContext);
#endif // capacity EVE
}

/**
* @brief touch test
*
* @param Sq Square positions
* @param TouchXY TouchXY value
* @param TouchNo Touch number order
*/
static void helperDrawTouchPt(SAMAPP_Squares_t *Sq, int32_t TouchXY, uint8_t TouchNo)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
	if (TouchXY != 0x80008000)
	{
		Sq->x = TouchXY >> 16;
		Sq->y = (TouchXY & 0xffff);
		Volume = (TouchNo + 1) * 255 / NO_OF_TOUCH;
	}
	else
	{
		Sq->x = OFFSCREEN;
		Sq->y = OFFSCREEN;
	}
	EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoCmd_setBitmap(s_pHalContext, 0, RGB565, 66, 66);
	EVE_CoDl_vertex2f(s_pHalContext, VP(Sq->x - 66 / 2), VP(Sq->y - 66 / 2));
	EVE_CoDl_end(s_pHalContext);
#endif
}

/**
* @brief Check user touches
*
* @param context BouncingSquares context
* @param val Multi touch value
*/
static void helperCheckTouch(SAMAPP_BouncingSquares *context, int32_t val)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
	uint8_t Arrayno = -1;

	if (val != 0x80008000)
	{
		uint16_t Tx = (val >> 16) & 0xffff;

		// Check which rectangle is being touched according to the coordinates
		if (Tx >= 60 && Tx <= 105)
			Arrayno = 0;
		if (Tx >= 140 && Tx <= 185)
			Arrayno = 1;
		if (Tx >= 220 && Tx <= 265)
			Arrayno = 2;
		if (Tx >= 300 && Tx <= 345)
			Arrayno = 3;
		if (Tx >= 380 && Tx <= 425)
			Arrayno = 4;

		if (Arrayno != -1)
		{
			context->RectTouched[Arrayno] = 1; // rectangle touched

			//store the touch point's Y-coordinate
			context->Ty[Arrayno] = (val & 0xffff);

			//Limit the height of the larger rectangle to reserve space for the smaller one
			if (context->Ty[Arrayno] <= 60)
				context->Ty[Arrayno] = 60;
		}
	}

	//According to the bigger rectangle values move the smaller rectangles
	for (int i = 0; i < NO_OF_RECTS; i++)
	{
		helperRectangleCalc(context, (uint8_t)i);
	}
#endif // capacity EVE
}

/**
* @brief Beginning BouncingSquares section
*
* @param BRx
* @param BRy
* @param MovingRy
* @param SqNumber
*/
static void helperDrawBouncingSquares(SAMAPP_BouncingSquares *context, int16_t *RectX)
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
	int8_t R1 = 0;
	int8_t G1 = 0;
	int8_t B1 = 0;
	int8_t R2 = 0;
	int8_t G2 = 0;
	int8_t B2 = 0;

	for (int i = 0; i < NO_OF_RECTS; i++)
	{
		//different colours are set for the different rectangles
		if (i == 0)
		{
			R1 = 63;
			G1 = 72;
			B1 = 204;
			R2 = 0;
			G2 = 255;
			B2 = 255;
		}

		if (i == 1)
		{
			R1 = 255;
			G1 = 255;
			B1 = 0;
			R2 = 246;
			G2 = 89;
			B2 = 12;
		}

		if (i == 2)
		{
			R1 = 255;
			G1 = 0;
			B1 = 0;
			R2 = 200;
			G2 = 28;
			B2 = 36;
		}

		if (i == 3)
		{
			R1 = 131;
			G1 = 171;
			B1 = 9;
			R2 = 8;
			G2 = 100;
			B2 = 50;
		}

		if (i == 4)
		{
			R1 = 90;
			G1 = 40;
			B1 = 120;
			R2 = 177;
			G2 = 156;
			B2 = 217;
		}

		// Draw the rectanles here
		EVE_CoDl_begin(s_pHalContext, RECTS);
		EVE_CoDl_lineWidth(s_pHalContext, 2 * 16);

		EVE_CoDl_colorRgb(s_pHalContext, R1, G1, B1);
		EVE_CoDl_vertex2f(s_pHalContext, VP(RectX[i]), VP(context->Ty[i]));
		EVE_CoDl_vertex2f(s_pHalContext, VP(RectX[i] + 45), VP(s_pHalContext->Height / 2));

		EVE_CoDl_colorRgb(s_pHalContext, R2, G2, B2);
		EVE_CoDl_vertex2f(s_pHalContext, VP(RectX[i]), VP(context->My[i]));
		EVE_CoDl_vertex2f(s_pHalContext, VP(RectX[i] + 45), VP(context->My[i] + 2));
		EVE_CoDl_end(s_pHalContext);
	}
#endif // capacity EVE
}

/**
* @brief Draw plots
*
*/
static void helperDrawPlotXY()
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
	uint8_t i = 0;
	uint16_t PlotHt = 0;
	uint16_t PlotWth = 0;
	uint16_t X = 0;
	uint16_t Y = 0;

	PlotHt = (uint16_t)(s_pHalContext->Height / 10);
	PlotWth = (uint16_t)(s_pHalContext->Width / 10);

	EVE_CoDl_colorRgb(s_pHalContext, 36, 54, 125);
	EVE_CoDl_begin(s_pHalContext, LINES);
	/* Horizontal Lines */
	for (i = 1; i < 11; i++)
	{
		Y = i * PlotHt;
		EVE_CoDl_lineWidth(s_pHalContext, 1 * 16);
		EVE_CoDl_vertex2f(s_pHalContext, 0, VP(Y));
		EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width), VP(Y));
	}
	/* Vertical Lines */
	for (i = 1; i < 11; i++)
	{
		X = i * PlotWth;
		EVE_CoDl_lineWidth(s_pHalContext, 1 * 16);
		EVE_CoDl_vertex2f(s_pHalContext, VP(X), 0);
		EVE_CoDl_vertex2f(s_pHalContext, VP(X), VP(s_pHalContext->Height));
	}
	EVE_CoDl_end(s_pHalContext);
#endif // capacity EVE
}

/**
* @brief Draw set 6
*
*/
void SAMAPP_Touch_touchToPlaySong()
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
    Draw_Text(s_pHalContext, "Example for: Touch test\n\n\nPlease touch on screen (1-5 fingers)");
    uint32_t val[5];

    SAMAPP_Squares_t SqCall;

#if defined(EVE_SUPPORT_CAPACITIVE)
	EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_EXTENDED);
#endif
    EVE_sleep(30);

	EVE_Util_loadRawFile(s_pHalContext, 0, TEST_DIR "\\yellow_66x66_RGB565.raw");
    /*Audio*/
	EVE_Util_loadRawFile(s_pHalContext, 1024 * 100, TEST_DIR "\\Devil_Ride_30_44100_ulaw.wav");

    EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_FREQ, 44100);
	EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_START, 1024 * 100);
    EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_FORMAT, ULAW_SAMPLES);
    EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_LENGTH, APPBUFFERSIZE);
    EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_LOOP, 1);
    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 0);
    EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_PLAY, 1);

    for (int j = 0; j < 1500; j++)
    {
		Display_StartColor(s_pHalContext, (uint8_t[]) { 0, 0, 0 }, (uint8_t[]) { 255, 255, 255 });
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 30, 26, OPT_CENTER,
            "Touch to play song"); //text info

        val[0] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH0_XY);
        val[1] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH1_XY);
        val[2] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH2_XY);
        val[3] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH3_XY);
        val[4] = (EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_X) << 16)
            | (EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_Y));

        for (int8_t i = 0; i < NO_OF_TOUCH; i++)
        {
			helperDrawTouchPt(&SqCall, (int32_t)val[i], i);
        }
        if ((val[0] == 0x80008000) && (val[1] == 0x80008000) && (val[2] == 0x80008000)
            && (val[3] == 0x80008000) && (val[4] == 0x80008000))
            Volume = 0;
		EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, Volume);
		Display_End(s_pHalContext);
    }
    EVE_Hal_wr8(s_pHalContext, REG_VOL_PB, 0);
	EVE_Hal_wr32(s_pHalContext, REG_PLAYBACK_LENGTH, 0);
    EVE_Hal_wr8(s_pHalContext, REG_PLAYBACK_PLAY, 1);

#if defined(EVE_SUPPORT_CAPACITIVE)
	EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_COMPATIBILITY);
#endif
    SAMAPP_DELAY_NEXT;
#endif
}

/**
* @brief Draw Bouncing squares
*
*/
void SAMAPP_Touch_BouncingSquares()
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
    int16_t RectX[5];
    int32_t val[5];

    SAMAPP_BouncingSquares context;

    Draw_Text(s_pHalContext, "Example for: Draw Bouncing squares\n\n\nPlease touch on screen (1-5 fingers)");

    //Calculate the X vertices where the five rectangles have to be placed
    for (int i = 1; i < 5; i++)
    {
        RectX[0] = 60;
        RectX[i] = RectX[i - 1] + 80;
    }

    for (int i = 0; i < 5; i++)
    {
        context.Ty[i] = 0;
        context.My[i] = 0;
		context.RectTouched[i] = 0;
		context.Decrease[i] = 0;
    }

#if defined(EVE_SUPPORT_CAPACITIVE)
    EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_EXTENDED);
#endif
    EVE_sleep(30);
    for (int k = 0; k < 300; k++)
    {
        /* first touch*/
        val[0] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH0_XY);
        /*second touch*/
        val[1] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH1_XY);
        /*third touch*/
        val[2] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH2_XY);
        /*fourth  touch*/
        val[3] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH3_XY);
        /*fifth  touch*/
        val[4] = ((uint32_t) EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_X) << 16L)
            | (EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_Y) & 0xffffL);

        //Check which rectangle is being touched using the coordinates and move the respective smaller rectangle
        for (int8_t i = 0; i < NO_OF_TOUCH; i++)
        {
            helperCheckTouch(&context, val[i]);
        }

        Display_StartColor(s_pHalContext, (uint8_t[]) { 0, 0, 0 }, (uint8_t[]) { 255, 255, 255 });
		helperDrawBouncingSquares(&context, RectX);
		Display_End(s_pHalContext);
    }
#if defined(EVE_SUPPORT_CAPACITIVE)
    EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_COMPATIBILITY);
#endif
#endif // capacity EVE
}
/* End BouncingSquares section */

/**
* @brief Draw Bouncing Circles
*
*/
void SAMAPP_Touch_BouncingCircles()
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
    int32_t Touchval[NO_OF_CIRCLE];
    SAMAPP_BouncingCircles context;

    Draw_Text(s_pHalContext, "Example for: Draw Bouncing Circles\n\n\nPlease touch on screen (1-5 fingers)");

#if defined(EVE_SUPPORT_CAPACITIVE)
    EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_EXTENDED);
#endif
    EVE_sleep(30);
    /* calculate the intital radius of the circles before the touch happens*/
    context.R[0] = 50;
    context.CX[0] = 190;
    context.CY[0] = 136;
    for (int8_t i = 1; i < NO_OF_CIRCLE; i++)
    {
        context.R[i] = context.R[i - 1] + 30;
        context.CX[i] = context.CX[i - 1] - 30;
        context.CY[i] = 136;
    }

    for (int32_t k = 0; k < 150; k++)
    {
        /* values of the five touches are stored here */
        Touchval[0] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH0_XY);
        Touchval[1] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH1_XY);
        Touchval[2] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH2_XY);
        Touchval[3] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH3_XY);
        Touchval[4] = ((int32_t) EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_X) << 16)
            | (EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_Y));

		Display_StartColor(s_pHalContext, (uint8_t[]) { 100, 255, 100 }, (uint8_t[]) { 255, 255, 255 });
		/* The plot is drawn here */
		helperDrawPlotXY();

        /* check which circle has been touched based on the coordinates and store the[0] number of the circle touched*/

        for (int8_t i = 0; i < NO_OF_CIRCLE; i++)
        {
			for (int8_t j = 0; j < NO_OF_TOUCH; j++)
			{
				helperCheckCircleTouchCood(&context, Touchval[j], j, i);
			}
        }
        /* calculate the radius of each circle according to the touch of each individual circle */

        for (int8_t i = 0; i < NO_OF_CIRCLE; i++)
        {
            helperCirclePlot(&context, i);
        }
        /* with the calculated radius draw the circles as well as the Touch points */

        for (int8_t i = 0; i < (NO_OF_CIRCLE); i++)
        {
            helperDrawConcentricCircles(context.R[i]);
            helperTouchPoints((int16_t) context.CX[i], (int16_t) context.CY[i], i + 1);
        }

        Display_End(s_pHalContext);
    }
#if defined(EVE_SUPPORT_CAPACITIVE)
	EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_COMPATIBILITY);
#endif
#endif // capacity EVE
}
/* End BouncingCircle section */

/**
* @brief Draw Bouncing points
*
*/
void SAMAPP_Touch_BouncingPoints()
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
    int32_t val[5];
	SAMAPP_BlobsInst gBlobsInst[NO_OF_TOUCH];
	SAMAPP_BlobsInst *pBInst;

    Draw_Text(s_pHalContext, "Example for: Draw Bouncing points\n\n\nPlease touch on screen (1-5 fingers)");

#if defined(EVE_SUPPORT_CAPACITIVE)
    EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_EXTENDED);
#endif
    EVE_sleep(30);
    pBInst = &gBlobsInst[0];

    //set all coordinates to OFFSCREEN position
	for (uint8_t j = 0; j < NO_OF_TOUCH; j++)
    {
        for (uint8_t i = 0; i < NBLOBS; i++)
        {
            pBInst->blobs[i].x = OFFSCREEN;
            pBInst->blobs[i].y = OFFSCREEN;
        }
        pBInst->CurrIdx = 0;
        pBInst++;
    }

    for (uint16_t k = 0; k < 150; k++)
    {
        val[0] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH0_XY);
        val[1] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH1_XY);
        val[2] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH2_XY);
        val[3] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH3_XY);
        val[4] = (((int32_t) EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_X) << 16)
            | (EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_Y) & 0xffff));

        Display_StartColor(s_pHalContext, (uint8_t[]) { 43, 73, 59 }, (uint8_t[]) { 255, 255, 255 });
		EVE_CoDl_blendFunc(s_pHalContext, SRC_ALPHA, ONE);
		EVE_CoDl_colorMask(s_pHalContext, 1, 1, 1, 0);

        // draw blobs according to the number of touches
		for (uint16_t j = 0; j < NO_OF_TOUCH; j++)
        {
            helperBlobColor(&gBlobsInst[j], val[j]);
        }
        Display_End(s_pHalContext);
    }

#if defined(EVE_SUPPORT_CAPACITIVE)
    EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_COMPATIBILITY);
#endif
#endif // capacity EVE
}
/* End BouncingPoints section */

/**
* @brief Move points
*
*/
void SAMAPP_Touch_MovingPoints()
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
    SAMAPP_MovingPoints context;

    Draw_Text(s_pHalContext, "Example for: Draw Moving points\n\n\nPlease touch on screen (1-5 fingers)");

#if defined(EVE_SUPPORT_CAPACITIVE)
    EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_EXTENDED);
#endif
    EVE_sleep(30);
    /* Initialize all coordinates */
	for (uint16_t j = 0; j < NO_OF_TOUCH; j++)
    {
        for (uint16_t i = 0; i < NO_OF_POINTS; i++)
        {
            context.t[i + j * NO_OF_POINTS] = (uint8_t) i;
			context.X[i + j * NO_OF_POINTS] = OFFSCREEN;
			context.Y[i + j * NO_OF_POINTS] = OFFSCREEN;
        }
    }

	context.StopY = 20;
	for (uint16_t i = 0; i < NO_OF_TOUCH; i++)
    {
		context.StopX[i] = 180 + i * 50;
    }

#if defined(FT900_PLATFORM) || defined(FT93X_PLATFORM)
    for (uint16_t k = 0; k < 800; k++)
#else
    for (uint16_t k = 0; k < 300; k++)
#endif
    {
        context.val[0] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH0_XY);
        context.val[1] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH1_XY);
        context.val[2] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH2_XY);
        context.val[3] = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH3_XY);
		context.val[4] = (((int32_t)EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_X) << 16)
		    | (EVE_Hal_rd16(s_pHalContext, REG_CTOUCH_TOUCH4_Y) & 0xffff));

		Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 0, 0, 0 });
		EVE_CoDl_begin(s_pHalContext, POINTS);
		EVE_CoDl_pointSize(s_pHalContext, 20 * 16);
		EVE_CoDl_colorA(s_pHalContext, 120);
        helperPointsCalc(&context);
		EVE_CoDl_end(s_pHalContext);
		Display_End(s_pHalContext);
    }
#if defined(EVE_SUPPORT_CAPACITIVE)
    EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_COMPATIBILITY);
#endif
#endif // capacity EVE
}
/* End MovingPoints section */

/**
* @brief Multi touch on a single tracked object can be individually tracked to save the MCU calculations on rotary and linear tracking. Maximum of 5 trackers.
*
*/
void SAMAPP_Touch_multiTracker()
{
#if (EVE_CHIPID & 0x01) == 0x01 // capacity EVE
#if defined(FT81X_ENABLE) // FT81X only
    uint32_t trackers[5];
    uint32_t delayLoop = 300;
    uint32_t trackerVal;
    uint8_t tagval;
    uint8_t RDialTag = 100;
    uint8_t GDialTag = 101;
    uint8_t BDialTag = 102;
    uint8_t ADialTag = 103;
    uint8_t DialR = (uint8_t) (s_pHalContext->Width / 10);
    uint8_t rectRed = 0;
    uint8_t rectGreen = 0;
    uint8_t rectBlue = 0;
    uint8_t rectAlpha = 255;

    uint16_t RDialX = DialR + 20;
    uint16_t RDialY = (uint16_t) s_pHalContext->Height / 4;
    uint16_t GDialX = DialR + 20;
    uint16_t GDialY = RDialY * 3;
    uint16_t BDialX = (uint16_t) (s_pHalContext->Width - 20 - DialR);
    uint16_t BDialY = (uint16_t) (s_pHalContext->Height / 4);
    uint16_t ADialX = (uint16_t) (s_pHalContext->Width - 20 - DialR);
    uint16_t ADialY = BDialY * 3;
    uint16_t rectWidth = (uint16_t) (s_pHalContext->Width / 2.6);
    uint16_t rectHeight = (uint16_t) (s_pHalContext->Height / 2);
    uint16_t rectX = (uint16_t) (s_pHalContext->Width / 2 - rectWidth / 2);
    uint16_t rectY = (uint16_t) (s_pHalContext->Height / 2 - rectHeight / 2);
    uint16_t RDialTrackVal = 0;
    uint16_t GDialTrackVal = 0;
    uint16_t BDialTrackVal = 0;
    uint16_t ADialTrackVal = 65535;

    Draw_Text(s_pHalContext, "Example for: Multi touch on a single tracked object\n\n\nPlease touch on screen (1-5 fingers)");

#if defined(EVE_SUPPORT_CAPACITIVE)
	EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_EXTENDED);
#endif
	EVE_sleep(30);

    EVE_CoCmd_track(s_pHalContext, RDialX, RDialY, 1, 1, RDialTag);
    EVE_CoCmd_track(s_pHalContext, GDialX, GDialY, 1, 1, GDialTag);
    EVE_CoCmd_track(s_pHalContext, BDialX, BDialY, 1, 1, BDialTag);
    EVE_CoCmd_track(s_pHalContext, ADialX, ADialY, 1, 1, ADialTag);

    while (delayLoop != 0)
    {
        trackers[0] = EVE_Hal_rd32(s_pHalContext, REG_TRACKER);
        trackers[1] = EVE_Hal_rd32(s_pHalContext, REG_TRACKER_1);
        trackers[2] = EVE_Hal_rd32(s_pHalContext, REG_TRACKER_2);
        trackers[3] = EVE_Hal_rd32(s_pHalContext, REG_TRACKER_3);

        for (uint8_t i = 0; i < 4; i++)
        {
            tagval = (trackers[i] & 0xff);
            trackerVal = (trackers[i] >> 16) & 0xffff;
            if (tagval == RDialTag)
            {
                rectRed = (uint8_t) (trackerVal * 255 / 65536);
                RDialTrackVal = (uint16_t) trackerVal;
            }
            else if (tagval == GDialTag)
            {
                rectGreen = (uint8_t) (trackerVal * 255 / 65536);
                GDialTrackVal = (uint16_t) trackerVal;
            }
            else if (tagval == BDialTag)
            {
                rectBlue = (uint8_t) (trackerVal * 255 / 65536);
                BDialTrackVal = (uint16_t) trackerVal;
            }
            else if (tagval == ADialTag)
            {
                rectAlpha = (uint8_t) (trackerVal * 255 / 65536);
                ADialTrackVal = (uint16_t) trackerVal;
            }
        }
		Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 255, 255, 255 });
		EVE_CoDl_colorA(s_pHalContext, 255);
		EVE_CoDl_tagMask(s_pHalContext, 1);
		EVE_CoDl_tag(s_pHalContext, RDialTag);
		EVE_CoCmd_dial(s_pHalContext, RDialX, RDialY, DialR, 0, RDialTrackVal);
		EVE_CoDl_tag(s_pHalContext, GDialTag);
		EVE_CoCmd_dial(s_pHalContext, GDialX, GDialY, DialR, 0, GDialTrackVal);
		EVE_CoDl_tag(s_pHalContext, BDialTag);
		EVE_CoCmd_dial(s_pHalContext, BDialX, BDialY, DialR, 0, BDialTrackVal);
		EVE_CoDl_tag(s_pHalContext, ADialTag);
		EVE_CoCmd_dial(s_pHalContext, ADialX, ADialY, DialR, 0, ADialTrackVal);
		EVE_CoDl_tagMask(s_pHalContext, 0);

        EVE_CoCmd_text(s_pHalContext, RDialX, RDialY, 28, OPT_CENTER, "Red");//text info
        EVE_CoCmd_text(s_pHalContext, GDialX, GDialY, 28, OPT_CENTER, "Green");//text info
        EVE_CoCmd_text(s_pHalContext, BDialX, BDialY, 28, OPT_CENTER, "Blue");//text info
        EVE_CoCmd_text(s_pHalContext, ADialX, ADialY, 28, OPT_CENTER, "Alpha");//text info

        EVE_CoDl_begin(s_pHalContext, RECTS);
		EVE_CoDl_colorRgb(s_pHalContext, rectRed, rectGreen, rectBlue);
		EVE_CoDl_colorA(s_pHalContext, rectAlpha);
		EVE_CoDl_vertex2f(s_pHalContext, VP(rectX), VP(rectY));
		EVE_CoDl_vertex2f(s_pHalContext, VP(rectX + rectWidth), VP(rectY + rectHeight));
		EVE_CoDl_end(s_pHalContext);

		Display_End(s_pHalContext);

        delayLoop--;
    }
#if defined(EVE_SUPPORT_CAPACITIVE)
	EVE_Hal_wr8(s_pHalContext, REG_CTOUCH_EXTENDED, CTOUCH_MODE_COMPATIBILITY);
#endif
#endif
#endif
}

/**
* @brief explain the usage of touch engine of EVE
* 
*/
void SAMAPP_Touch_touchInfo()
{
	Draw_Text(s_pHalContext, "Example for: Touch raw, touch screen, touch tag, raw adc\n\n\nPlease touch on screen");
    int32_t LoopFlag = 0;
    int32_t wbutton;
    int32_t hbutton;
    int32_t tagval;
    int32_t tagoption;
    char8_t StringArray[100];
    uint32_t ReadWord;
    uint16_t xvalue;
    uint16_t yvalue;
    uint16_t pendown;

    /*************************************************************************/
    /* Below code demonstrates the usage of touch function. Display info     */
    /* touch raw, touch screen, touch tag, raw adc and resistance values     */
    /*************************************************************************/
    LoopFlag = 300;
    wbutton = s_pHalContext->Width / 8;
    hbutton = s_pHalContext->Height / 8;
    while (LoopFlag--)
    {
		Display_StartColor(s_pHalContext, (uint8_t[]) { 64, 64, 64 }, (uint8_t[]) { 255, 255, 255 });
		EVE_CoDl_tagMask(s_pHalContext, 0);
        /* Draw informative text at width/2,20 location */
        StringArray[0] = '\0';
		strcat_s(StringArray, sizeof(StringArray), "Touch Raw XY (");
        ReadWord = EVE_Hal_rd32(s_pHalContext, REG_TOUCH_RAW_XY);
        yvalue = (uint16_t) (ReadWord & 0xffff);
        xvalue = (uint16_t) ((ReadWord >> 16) & 0xffff);
        Gpu_Hal_Dec2Ascii(StringArray, (uint32_t) xvalue);
		strcat_s(StringArray, sizeof(StringArray), ",");
        Gpu_Hal_Dec2Ascii(StringArray, (uint32_t) yvalue);
		strcat_s(StringArray, sizeof(StringArray), ")");
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 10, 26, OPT_CENTER,
            StringArray);

        StringArray[0] = '\0';
		strcat_s(StringArray, sizeof(StringArray), "Touch RZ (");
        ReadWord = EVE_Hal_rd16(s_pHalContext, REG_TOUCH_RZ);
        Gpu_Hal_Dec2Ascii(StringArray, ReadWord);
		strcat_s(StringArray, sizeof(StringArray), ")");
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 25, 26, OPT_CENTER,
            StringArray);

        StringArray[0] = '\0';
		strcat_s(StringArray, sizeof(StringArray), "Touch Screen XY (");
        ReadWord = EVE_Hal_rd32(s_pHalContext, REG_TOUCH_SCREEN_XY);
        yvalue = (int16_t) (ReadWord & 0xffff);
        xvalue = (int16_t) ((ReadWord >> 16) & 0xffff);
        Gpu_Hal_Dec2Ascii(StringArray, (int32_t) xvalue);
		strcat_s(StringArray, sizeof(StringArray), ",");
        Gpu_Hal_Dec2Ascii(StringArray, (int32_t) yvalue);
		strcat_s(StringArray, sizeof(StringArray), ")");
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 40, 26, OPT_CENTER,
            StringArray);

        StringArray[0] = '\0';
		strcat_s(StringArray, sizeof(StringArray), "Touch TAG (");
        ReadWord = EVE_Hal_rd8(s_pHalContext, REG_TOUCH_TAG);
        Gpu_Hal_Dec2Ascii(StringArray, ReadWord);
		strcat_s(StringArray, sizeof(StringArray), ")");
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 55, 26, OPT_CENTER,
            StringArray);
        tagval = ReadWord;
        StringArray[0] = '\0';
		strcat_s(StringArray, sizeof(StringArray), "Touch Direct XY (");
        ReadWord = EVE_Hal_rd32(s_pHalContext, REG_TOUCH_DIRECT_XY);
        yvalue = (int16_t) (ReadWord & 0x03ff);
        xvalue = (int16_t) ((ReadWord >> 16) & 0x03ff);
        Gpu_Hal_Dec2Ascii(StringArray, (int32_t) xvalue);
		strcat_s(StringArray, sizeof(StringArray), ",");
        Gpu_Hal_Dec2Ascii(StringArray, (int32_t) yvalue);
        pendown = (int16_t) ((ReadWord >> 31) & 0x01);
		strcat_s(StringArray, sizeof(StringArray), ",");
        Gpu_Hal_Dec2Ascii(StringArray, (int32_t) pendown);
		strcat_s(StringArray, sizeof(StringArray), ")");
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 70, 26, OPT_CENTER,
            StringArray);

        StringArray[0] = '\0';
		strcat_s(StringArray, sizeof(StringArray), "Touch Direct Z1Z2 (");
        ReadWord = EVE_Hal_rd32(s_pHalContext, REG_TOUCH_DIRECT_Z1Z2);
        yvalue = (int16_t) (ReadWord & 0x03ff);
        xvalue = (int16_t) ((ReadWord >> 16) & 0x03ff);
        Gpu_Hal_Dec2Ascii(StringArray, (int32_t) xvalue);
		strcat_s(StringArray, sizeof(StringArray), ",");
        Gpu_Hal_Dec2Ascii(StringArray, (int32_t) yvalue);
		strcat_s(StringArray, sizeof(StringArray), ")");

        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 85, 26, OPT_CENTER,
            StringArray);

        EVE_CoCmd_fgColor(s_pHalContext, 0x008000);
		EVE_CoDl_tagMask(s_pHalContext, 1);
        tagoption = 0;
        if (12 == tagval)
        {
            tagoption = OPT_FLAT;
        }

        EVE_CoDl_tag(s_pHalContext, 12);
        EVE_CoCmd_button(s_pHalContext, (int16_t) ((s_pHalContext->Width / 4) - (wbutton / 2)),
            (int16_t) ((s_pHalContext->Height * 2 / 4) - (hbutton / 2)), (int16_t) wbutton,
            (int16_t) hbutton, 26, (int16_t) tagoption, "Tag12");
		EVE_CoDl_tag(s_pHalContext, 13);
        tagoption = 0;
        if (13 == tagval)
        {
            tagoption = OPT_FLAT;
        }
        EVE_CoCmd_button(s_pHalContext, (int16_t) ((s_pHalContext->Width * 3 / 4) - (wbutton / 2)),
            (int16_t) ((s_pHalContext->Height * 3 / 4) - (hbutton / 2)), (int16_t) wbutton,
            (int16_t) hbutton, 26, (int16_t) tagoption, "Tag13");

        /* Wait till coprocessor completes the operation */
        Display_End(s_pHalContext);
        EVE_sleep(30);
    }
}

/**
* @brief Sample app api to demonstrate track widget funtionality
*
*/
void SAMAPP_Touch_objectTrack()
{
    /*************************************************************************/
    /* Below code demonstrates the usage of track function. Track function   */
    /* tracks the pen touch on any specific object. Track function supports  */
    /* rotary and horizontal/vertical tracks. Rotary is given by rotation    */
    /* angle and horizontal/vertucal track is offset position.               */
    /*************************************************************************/
    int32_t LoopFlag = 0;
    uint32_t TrackRegisterVal = 0;
    uint16_t angleval = 0;
    uint16_t slideval = 0;
    uint16_t scrollval = 0;

    /* Set the tracker for 3 bojects */

    Draw_Text(s_pHalContext, "Example for: Rotary and horizontal/vertical tracks\n\n\nPlease touch on screen");

    EVE_CoCmd_track(s_pHalContext, (int16_t) (s_pHalContext->Width / 2),
        (int16_t) (s_pHalContext->Height / 2), 1, 1, 10);
    EVE_CoCmd_track(s_pHalContext, 40, (int16_t) (s_pHalContext->Height - 40),
        (int16_t) (s_pHalContext->Width - 80), 8, 11);
    EVE_CoCmd_track(s_pHalContext, (int16_t) (s_pHalContext->Width - 40), 40, 8,
        (int16_t) (s_pHalContext->Height - 80), 12);
    /* Wait till coprocessor completes the operation */
    EVE_Cmd_waitFlush(s_pHalContext);

    LoopFlag = 600;
    /* update the background color continuously for the color change in any of the trackers */
    while (LoopFlag--)
    {
        uint8_t tagval = 0;
        TrackRegisterVal = EVE_Hal_rd32(s_pHalContext, REG_TRACKER);
        tagval = TrackRegisterVal & 0xff;

        if (10 == tagval)
        {
            angleval = TrackRegisterVal >> 16;
        }
        else if (11 == tagval)
        {
            slideval = TrackRegisterVal >> 16;
        }
        else if (12 == tagval)
        {
            scrollval = TrackRegisterVal >> 16;
            if ((scrollval + 65535 / 10) > (9 * 65535 / 10))
            {
                scrollval = (8 * 65535 / 10);
            }
            else if (scrollval < (1 * 65535 / 10))
            {
                scrollval = 0;
            }
            else
            {
                scrollval -= (1 * 65535 / 10);
            }
        }

        /* Display a rotary dial, horizontal slider and vertical scroll */
        int32_t tmpval0;
        int32_t tmpval1;
        int32_t tmpval2;
        uint8_t angval;
        uint8_t sldval;
        uint8_t scrlval;

        tmpval0 = (int32_t) angleval * 255 / 65536;
        tmpval1 = (int32_t) slideval * 255 / 65536;
        tmpval2 = (int32_t) scrollval * 255 / 65536;

        angval = tmpval0 & 0xff;
        sldval = tmpval1 & 0xff;
        scrlval = tmpval2 & 0xff;

        Display_StartColor(s_pHalContext, (uint8_t[]) { angval, sldval, scrlval }, (uint8_t[]) { 255, 255, 255 });

        /* Draw dial with 3d effect */
        EVE_CoCmd_fgColor(s_pHalContext, 0x00ff00);
        EVE_CoCmd_bgColor(s_pHalContext, 0x800000);
        EVE_CoDl_tag(s_pHalContext, 10);
        EVE_CoCmd_dial(s_pHalContext, (int16_t) (s_pHalContext->Width / 2),
            (int16_t) (s_pHalContext->Height / 2), (int16_t) (s_pHalContext->Width / 8), 0,
            angleval);

        /* Draw slider with 3d effect */
        EVE_CoCmd_fgColor(s_pHalContext, 0x00a000);
        EVE_CoCmd_bgColor(s_pHalContext, 0x800000);
        EVE_CoDl_tag(s_pHalContext, 11);
        EVE_CoCmd_slider(s_pHalContext, 40, (int16_t) (s_pHalContext->Height - 40),
            (int16_t) (s_pHalContext->Width - 80), 8, 0, slideval, 65535);

        /* Draw scroll with 3d effect */
        EVE_CoCmd_fgColor(s_pHalContext, 0x00a000);
        EVE_CoCmd_bgColor(s_pHalContext, 0x000080);
        EVE_CoDl_tag(s_pHalContext, 12);
        EVE_CoCmd_scrollbar(s_pHalContext, (int16_t) (s_pHalContext->Width - 40), 40, 8,
            (int16_t) (s_pHalContext->Height - 80), 0, scrollval, (uint16_t) (65535 * 0.2), 65535);

        EVE_CoCmd_fgColor(s_pHalContext, TAG_MASK(0));
		EVE_CoDl_colorRgb(s_pHalContext, 0xff, 0xff, 0xff);
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2),
            (int16_t) ((s_pHalContext->Height / 2) + (s_pHalContext->Width / 8) + 8), 26,
            OPT_CENTER, "Rotary track");
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2),
            (int16_t) (s_pHalContext->Height - 40 + 8 + 8), 26, OPT_CENTER, "Horizontal track");
        EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width - 50), 20, 26, OPT_CENTER,
            "Vertical track");

        Display_End(s_pHalContext);

        EVE_sleep(10);
    }
}

void SAMAPP_Touch() {
    SAMAPP_Touch_touchToPlaySong();
    SAMAPP_Touch_BouncingSquares();
    SAMAPP_Touch_BouncingCircles();
    SAMAPP_Touch_BouncingPoints();
    SAMAPP_Touch_MovingPoints();
    SAMAPP_Touch_multiTracker();
    SAMAPP_Touch_touchInfo();
    SAMAPP_Touch_objectTrack();
}


