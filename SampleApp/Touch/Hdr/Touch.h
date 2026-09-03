#ifndef APP_H_
#define APP_H_

#include "platform.h"

// Path to UI assets Folder
#if defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM)
#define TEST_DIR                            "..\\..\\..\\Test\\"
#else
#define TEST_DIR                            "/EveApps/SampleApp/Touch/Test"
#endif

#define GET_CALIBRATION                     1

//main windows
#define NO_OF_TOUCH (5)
// buffers
#define APPBUFFERSIZE (65536L)
#define APPBUFFERSIZEMINUSONE (APPBUFFERSIZE - 1)
#define OFFSCREEN (-16384)

#define NAMEARRAYSZ 500
typedef struct SAMAPP_Logo_Img {
	//prog_uchar8_t name[14];
	char8_t name[NAMEARRAYSZ];
	uint16_t image_height;
	uint8_t image_format;
	uint8_t filter;
	uint16_t sizex;
	uint16_t sizey;
	uint16_t linestride;
	uint32_t gram_address;
}SAMAPP_Logo_Img_t;

typedef struct SAMAPP_Squares {
	uint16_t x, y;
}SAMAPP_Squares_t;

//bouncing squares
#define NO_OF_RECTS (5)
typedef struct SAMAPP_BouncingSquares
{
	int16_t Ty[NO_OF_RECTS]; /**< Y-coordinate for touch point */
	int16_t My[NO_OF_RECTS]; /**< Y-coordinate for moving rectangle */
	uint8_t Decrease[NO_OF_RECTS]; /**< flag for decrease/increase */
	uint8_t RectTouched[NO_OF_RECTS]; /**< Touch flag */
} SAMAPP_BouncingSquares;

//Bouncing Circle macros
#define NO_OF_CIRCLE                        (5)

typedef struct SAMAPP_BouncingCircles
{
	float R[NO_OF_CIRCLE]; /**< circle radius */
	float CX[NO_OF_CIRCLE]; /**< X-coordinate for small circle (touch point) */
	float CY[NO_OF_CIRCLE]; /**< Y-coordinate for small circle (touch point) */
	uint8_t TN[NO_OF_CIRCLE]; /**< touch number */
} SAMAPP_BouncingCircles;

//bouncing pints structures
#define NBLOBS                              (64)
typedef struct SAMAPP_Blobs {
	int16_t x;
	int16_t y;
}SAMAPP_Blobs;
typedef struct SAMAPP_BlobsInst {
	SAMAPP_Blobs blobs[NBLOBS];
	uint8_t CurrIdx;
}SAMAPP_BlobsInst;

//moving points structures
#define NO_OF_POINTS (64)
typedef struct SAMAPP_MovingPoints {
	uint16_t StopX[NO_OF_TOUCH]; /**< The x-coordinate of the moving points is no longer updating */
	uint16_t StopY; /**< The y-coordinate of the moving points is no longer updating */
	uint32_t val[NO_OF_TOUCH]; /**< Touch location value */
	uint16_t X[(NO_OF_POINTS) * (NO_OF_TOUCH)]; /**< The x-coordinate of the moving points */
	uint16_t Y[(NO_OF_POINTS) * (NO_OF_TOUCH)]; /**< The y-coordinate of the moving points */
	uint8_t t[(NO_OF_POINTS) * (NO_OF_TOUCH)]; /**< point number */
}SAMAPP_MovingPoints;

#endif /* APP_H_ */
