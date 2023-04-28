/**
 * @file Hal_I2C.cpp
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

/* Standard includes */
#include "Platform.h"
#include <Wire.h>
#include "Hal_I2C.h"

/* mainly assosiated to rtc - need to make it generic */

int16_t hal_rtc_i2c_init()
{
  Wire.begin();
  delay(100);  
  return 0;
}

/* api to read n bytes from addr */
int16_t hal_rtc_i2c_read(uint8_t addr, uint8_t *buffer,uint16_t length)
{
  uint16_t i;
  short count = 0;
  uint8_t writeResult = 0;
   while (length > 28)
  {
    hal_rtc_i2c_read(addr,buffer,28);
    buffer += 28;
    addr += 28;
    length -= 28;
  }
  
  Wire.beginTransmission(0x6f); // transmit to device (0x23)
  
 /* address bytes for rtc are from 00 to 0xff */
  Wire.write(addr);        // sends value byte  
  
  /* end the transmission but do not release the bus - usage is random data read use case from rtc */
  writeResult = Wire.endTransmission(false);//hold the bus to read the next data

  if (0 != writeResult)
  {
    return -1;//error case
  }

  Wire.requestFrom(0x6f, length);// request length bytes from slave device and end the transmission after this
  for(i=0;i<length;i++)
  {
    /* need to consider timout here */
    while(0 == Wire.available());//blocking call - at least one byte must be available
    buffer[i] = Wire.read();
  }
  
  return 0;
}

/* API to write data into particular location */
int16_t hal_rtc_i2c_write(uint8_t addr, uint8_t *buffer,uint16_t length)
{
  uint16_t i;
  byte writeResult = 0;
  if(0 == length)
  {
    return -1;
  }
  
  /* for read the lower bit must be set to 1 and for write set to 0 */
  Wire.beginTransmission(0x6f);

  /* address bytes for rtc are from 00 to 0xff */
  Wire.write(addr);        // sends value byte  
  
  /* check for each byte */
  for(i=0;i<length;i++)
  {
    Wire.write(*buffer++);//send the data to slave
  }

  /* end the transmission by stop bit */
  
  writeResult = Wire.endTransmission();//end the transmission by setting the stop bit
  
  if(0 != writeResult)
  {
    return writeResult;
  }
  return 0;
}

/* Nothing beyond this */

