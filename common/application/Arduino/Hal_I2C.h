/**
 * @file Hal_I2C.h
 * @brief This file contains apis related to i2c
 *
 * @author Bridgetek
 *
 * @date 2018
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

/* This file contains interface to I2C */
#ifndef HAL_I2C_H
#define HAL_I2C_H

#ifdef  ARDUINO_ATMEGA328P_I2C
#define ATMEGA328P_RTC_I2C_ADDRESS     (0x6F)//7 bit i2c address and last bit is for read/write information
#define ATMEGA328P_I2C_BURST_SIZE  (28)
#define ATMEGA328P_RTC_MFP  (2)
#endif

/* API prototypes */
int16_t hal_rtc_i2c_init();
int16_t hal_rtc_i2c_read(uint8_t addr, uint8_t *buffer,uint16_t length);
int16_t hal_rtc_i2c_write(uint8_t addr, uint8_t *buffer,uint16_t length);

#endif /* HAL_I2C_H */

/* Nothing beyond this */

