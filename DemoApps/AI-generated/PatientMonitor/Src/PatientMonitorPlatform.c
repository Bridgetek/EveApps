/**
 * @file PatientMonitorPlatform.c
 * @brief Visible BT8XXEMU host window and normal hardware initialization.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Common.h"
#include "EVE_CoCmd.h"
#include "EVE_Util.h"
#include "PatientMonitor.h"
#include "PatientMonitorPlatform.h"

#if defined(BT8XXEMU_PLATFORM) && defined(_WIN32)

#define PM_FRAME_MESSAGE (WM_APP + 1)

typedef struct EmulatorWindow
{
    HWND Window;
    CRITICAL_SECTION FrameLock;
    argb8888 *Frame;
    uint32_t Width;
    uint32_t Height;
    BT8XXEMU_Emulator *Emulator;
    volatile LONG Running;
    int TouchX, TouchY;
    bool TouchPressed;
    bool TouchReady;
} EmulatorWindow;

static EmulatorWindow s_window;

void PatientMonitor_PlatformTouch(int16_t x, int16_t y, bool pressed)
{
    s_window.TouchX=x;
    s_window.TouchY=y;
    s_window.TouchPressed=pressed;
}

static bool commandWait(struct EVE_HalContext *phost)
{
    static uint32_t lastReport;
    uint32_t now = EVE_millis();
    (void)phost;
    if ((uint32_t)(now-lastReport)>1000U) {
        fprintf(stderr, "PatientMonitor: waiting for coprocessor\n");
        fflush(stderr);lastReport=now;
    }
    /* The HAL checks coprocessor faults. Keep the UI responsive during waits;
     * a process-lifetime timeout incorrectly aborts healthy later swaps. */
    return PatientMonitor_PlatformPump();
}

static void saveDiagnosticFrame(const argb8888 *buffer, uint32_t width, uint32_t height)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    HANDLE file;
    DWORD written;
    size_t pixelBytes = (size_t)width * height * sizeof(argb8888);

    memset(&fileHeader, 0, sizeof(fileHeader));
    memset(&infoHeader, 0, sizeof(infoHeader));
    fileHeader.bfType = 0x4D42;
    fileHeader.bfOffBits = sizeof(fileHeader) + sizeof(infoHeader);
    fileHeader.bfSize = fileHeader.bfOffBits + (DWORD)pixelBytes;
    infoHeader.biSize = sizeof(infoHeader);
    infoHeader.biWidth = (LONG)width;
    infoHeader.biHeight = -(LONG)height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 32;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage = (DWORD)pixelBytes;

    file = CreateFileW(L"PatientMonitor_Frame.bmp", GENERIC_WRITE, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return;
    WriteFile(file, &fileHeader, sizeof(fileHeader), &written, NULL);
    WriteFile(file, &infoHeader, sizeof(infoHeader), &written, NULL);
    WriteFile(file, buffer, (DWORD)pixelBytes, &written, NULL);
    CloseHandle(file);
}

static void updateTouch(LPARAM position, bool pressed)
{
    RECT client;
    int x;
    int y;

    if (!s_window.Emulator)
        return;
    if (!pressed)
    {
        PatientMonitor_PlatformTouch(0,0,false);
        return;
    }

    GetClientRect(s_window.Window, &client);
    if (client.right <= 0 || client.bottom <= 0)
        return;
    x = (int)(short)LOWORD(position) * PM_SCREEN_WIDTH / client.right;
    y = (int)(short)HIWORD(position) * PM_SCREEN_HEIGHT / client.bottom;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= PM_SCREEN_WIDTH) x = PM_SCREEN_WIDTH - 1;
    if (y >= PM_SCREEN_HEIGHT) y = PM_SCREEN_HEIGHT - 1;
    PatientMonitor_PlatformTouch((int16_t)x,(int16_t)y,true);
}

static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case PM_FRAME_MESSAGE:
        InvalidateRect(window, NULL, FALSE);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        RECT client;
        BITMAPINFO info;
        HDC dc = BeginPaint(window, &paint);
        GetClientRect(window, &client);
        memset(&info, 0, sizeof(info));
        info.bmiHeader.biSize = sizeof(info.bmiHeader);
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;

        EnterCriticalSection(&s_window.FrameLock);
        if (s_window.Frame && s_window.Width && s_window.Height)
        {
            info.bmiHeader.biWidth = (LONG)s_window.Width;
            info.bmiHeader.biHeight = -(LONG)s_window.Height;
            StretchDIBits(dc, 0, 0, client.right, client.bottom,
                0, 0, (int)s_window.Width, (int)s_window.Height,
                s_window.Frame, &info, DIB_RGB_COLORS, SRCCOPY);
        }
        else
        {
            FillRect(dc, &client, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }
        LeaveCriticalSection(&s_window.FrameLock);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_LBUTTONDOWN:
        SetCapture(window);
        updateTouch(lParam, true);
        return 0;
    case WM_MOUSEMOVE:
        if (wParam & MK_LBUTTON)
            updateTouch(lParam, true);
        return 0;
    case WM_LBUTTONUP:
        updateTouch(lParam, false);
        ReleaseCapture();
        return 0;
    case WM_CAPTURECHANGED:
    case WM_CANCELMODE:
        updateTouch(0, false);
        return 0;
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        InterlockedExchange(&s_window.Running, 0);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}

static bool createWindow(void)
{
    static const wchar_t className[] = L"EveAppsPatientMonitorWindow";
    WNDCLASSW windowClass;
    RECT workArea;
    RECT bounds;
    int clientWidth = PM_SCREEN_WIDTH;
    int clientHeight = PM_SCREEN_HEIGHT;
    int windowWidth;
    int windowHeight;
    int windowX;
    int windowY;

    memset(&s_window, 0, sizeof(s_window));
    InitializeCriticalSection(&s_window.FrameLock);
    InterlockedExchange(&s_window.Running, 1);

    memset(&windowClass, 0, sizeof(windowClass));
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClass.lpszClassName = className;
    if (!RegisterClassW(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    if (clientWidth > (workArea.right - workArea.left) * 9 / 10)
    {
        clientWidth = (workArea.right - workArea.left) * 9 / 10;
        clientHeight = clientWidth * PM_SCREEN_HEIGHT / PM_SCREEN_WIDTH;
    }
    if (clientHeight > (workArea.bottom - workArea.top) * 9 / 10)
    {
        clientHeight = (workArea.bottom - workArea.top) * 9 / 10;
        clientWidth = clientHeight * PM_SCREEN_WIDTH / PM_SCREEN_HEIGHT;
    }
    bounds.left = 0;
    bounds.top = 0;
    bounds.right = clientWidth;
    bounds.bottom = clientHeight;
    AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE);
    windowWidth = bounds.right - bounds.left;
    windowHeight = bounds.bottom - bounds.top;
    windowX = workArea.left + ((workArea.right - workArea.left) - windowWidth) / 2;
    windowY = workArea.top + ((workArea.bottom - workArea.top) - windowHeight) / 2;
    s_window.Window = CreateWindowExW(0, className,
        L"BT817 Patient Monitor - 1280 x 800 Emulator",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, windowX, windowY,
        windowWidth, windowHeight,
        NULL, NULL, windowClass.hInstance, NULL);
    if (!s_window.Window)
    {
        fprintf(stderr, "PatientMonitor: CreateWindowExW failed (%lu).\n", (unsigned long)GetLastError());
        fflush(stderr);
        return false;
    }
    ShowWindow(s_window.Window, SW_SHOW);
    UpdateWindow(s_window.Window);
    BringWindowToTop(s_window.Window);
    SetForegroundWindow(s_window.Window);
    fprintf(stderr, "PatientMonitor: emulator window created.\n");
    fflush(stderr);
    return true;
}

static int emulatorGraphics(BT8XXEMU_Emulator *sender, void *context, int output,
    const argb8888 *buffer, uint32_t width, uint32_t height, BT8XXEMU_FrameFlags flags)
{
    static bool firstFrame = true;
    static uint32_t frameNumber;
    EmulatorWindow *window = (EmulatorWindow *)context;
    size_t size;
    argb8888 *frame;
    (void)sender;


    if (!InterlockedCompareExchange(&window->Running, 1, 1))
        return 0;
    if (!output || !buffer || !width || !height || !(flags & BT8XXEMU_FrameBufferComplete))
        return 1;

    ++frameNumber;

    size = (size_t)width * height * sizeof(argb8888);
    EnterCriticalSection(&window->FrameLock);
    if (window->Width != width || window->Height != height)
    {
        frame = (argb8888 *)realloc(window->Frame, size);
        if (!frame)
        {
            LeaveCriticalSection(&window->FrameLock);
            return 0;
        }
        window->Frame = frame;
        window->Width = width;
        window->Height = height;
    }
    memcpy(window->Frame, buffer, size);
    LeaveCriticalSection(&window->FrameLock);
    if (firstFrame)
    {
        fprintf(stderr, "PatientMonitor: first emulator frame is %lu x %lu.\n",
            (unsigned long)width, (unsigned long)height);
        fflush(stderr);
        firstFrame = false;
    }
    if (frameNumber == 120U)
        saveDiagnosticFrame(buffer, width, height);
    PostMessage(window->Window, PM_FRAME_MESSAGE, 0, 0);
    return 1;
}

bool PatientMonitor_PlatformInit(EVE_HalContext *phost)
{
    EVE_HalParameters params;
    BT8XXEMU_EmulatorParameters emulatorParams;

    if (!createWindow())
        return false;

    EVE_Hal_initialize();
    EVE_Hal_defaultsEx(&params, (size_t)-1);
    params.CbCmdWait = commandWait;
    EVE_Util_emulatorDefaults(&params, &emulatorParams, EVE_SUPPORT_CHIPID);
    emulatorParams.Graphics = emulatorGraphics;
    /* The application owns the host window lifetime. */
    emulatorParams.Close = NULL;
    emulatorParams.UserContext = &s_window;
    params.EmulatorFlashParameters = NULL; /* This sample has no flash assets. */

    if (!EVE_Hal_open(phost, &params))
        return false;
    fprintf(stderr, "PatientMonitor: BT8XXEMU opened.\n");
    fflush(stderr);
    s_window.Emulator = phost->Emulator;
    EVE_Util_bootupConfig(phost);
    fprintf(stderr, "PatientMonitor: EVE boot configuration complete.\n");
    fflush(stderr);

    EVE_Hal_wr8(phost, REG_ADAPTIVE_FRAMERATE, 0);
    EVE_Hal_wr8(phost, REG_CPURESET, 2);
    EVE_Hal_wr16(phost, REG_TOUCH_CONFIG, 0x05D0);
    EVE_Hal_wr8(phost, REG_CPURESET, 0);
    EVE_sleep(300);
    EVE_CoCmd_apiLevel(phost, 2);
    if (!EVE_Cmd_waitFlush(phost)) return false;
    fprintf(stderr, "PatientMonitor: API level ready\n");
    fflush(stderr);
    Setup_Precision(0);
    s_window.TouchReady=true;
    return true;
}

bool PatientMonitor_PlatformPump(void)
{
    MSG message;
    unsigned int count = 0;
    while (count++ < 32 && PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
            InterlockedExchange(&s_window.Running, 0);
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    /* Custom-output emulators require touch coordinates to be refreshed,
     * not only delivered once on the Windows mouse-down message. */
    if(s_window.Emulator && s_window.TouchReady) {
        if(s_window.TouchPressed)
            BT8XXEMU_touchSetXY(s_window.Emulator,0,s_window.TouchX,s_window.TouchY,1);
        else
            BT8XXEMU_touchResetXY(s_window.Emulator,0);
    }
    return InterlockedCompareExchange(&s_window.Running, 1, 1) != 0;
}

void PatientMonitor_PlatformRelease(EVE_HalContext *phost)
{
    Gpu_Release(phost);
    if (s_window.Window)
        DestroyWindow(s_window.Window);
    EnterCriticalSection(&s_window.FrameLock);
    free(s_window.Frame);
    s_window.Frame = NULL;
    LeaveCriticalSection(&s_window.FrameLock);
    DeleteCriticalSection(&s_window.FrameLock);
}

#else

bool PatientMonitor_PlatformInit(EVE_HalContext *phost)
{
    Gpu_Init(phost);
    return true;
}

bool PatientMonitor_PlatformPump(void)
{
    return true;
}

void PatientMonitor_PlatformRelease(EVE_HalContext *phost)
{
    Gpu_Release(phost);
}

#endif
