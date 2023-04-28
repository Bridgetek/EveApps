/**
 * @file DemoMediaPlayer.c
 * @brief Manage and play media content on hard disk/SD card
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

#include "Platform.h"
#include "Common.h"
#include "App.h"
#include "FT_Util.h"
#if defined(EVE_FLASH_AVAILABLE)

static EVE_HalContext *s_pHalContext;

ft_int32_t MultimediaExplorer_Main(Ft_Gpu_Hal_Context_t* pHalContext, GuiManager* CleO, int command, void* _data);
ft_void_t Info(Ft_Gpu_Hal_Context_t* pHalContext);

void DemoMediaPlayer(EVE_HalContext* pHalContext) {
	s_pHalContext = pHalContext;
	FlashHelper_SwitchFullMode(s_pHalContext);

    init_guiobs(s_pHalContext);

    while(1) {
        MultimediaExplorer_Main(s_pHalContext, getGuiInstance(), 0, NULL);
    }
}
#endif
