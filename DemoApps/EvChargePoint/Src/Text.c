/**
 * @file Text.c
 * @brief Text definitions
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

#include "Def.h"

extern E_LANG g_language;

const char *s_languageEnglish = "ENGLISH";
const char *s_languageGerman = "DEUTSCH";
const char *s_languageChinese = "\x01\x02";

const char *s_chooseLanguage = "";
const char *s_tapYourCard = "";
const char *s_checkingTheCard = "";
const char *s_authenticated = "";
const char *s_plugIn = "";
const char *s_pleaseTouch = "";
const char *s_payWith = "";
const char *s_checkingConnection = "";
const char *s_connected = "";
const char *s_transactionEstimation = "";
const char *s_energy = "";
const char *s_time = "";
const char *s_cost = "";
const char *s_kWh = "";
const char *s_minutes = "";
const char *s_minutes_report = "";
const char *s_currency = "";
const char *s_startCharging = "";
const char *s_charging = "";
const char *s_pleaseUnplug = "";
const char *s_report = "";
const char *s_battery = "";
const char *s_batteryHealth = "";
const char *s_chargingTime = "";
const char *s_totalCost = "";
const char *s_healthy = "";
const char *s_exit = "";

void switch_english() {
    s_chooseLanguage = "\x09\x2a\x31\x31\x35\x27\x01\x2e\x23\x30\x29\x37\x23\x29\x27";
    s_tapYourCard = "\x17\x2e\x27\x23\x35\x27\x01\x36\x23\x32\x01\x3b\x31\x37\x34\x01\x25\x23\x34\x26\x01\x36\x31\x01\x25\x31\x30\x36\x2b\x30\x37\x27";
    s_checkingTheCard = "\x09\x2a\x27\x25\x2d\x2b\x30\x29\x01\x36\x2a\x27\x01\x25\x23\x34\x26\x05\x05\x05";
    s_authenticated = "\x07\x37\x36\x2a\x27\x30\x36\x2b\x25\x23\x36\x27\x26\x02";
    s_plugIn = "\x17\x13\x1c\x0e\x01\x10\x15";
    s_pleaseTouch = "\x17\x2e\x27\x23\x35\x27\x01\x36\x31\x37\x25\x2a\x01\x36\x2a\x27\x01\x35\x25\x34\x27\x27\x30\x01\x36\x31\x01\x25\x31\x30\x36\x2b\x30\x37\x27";
    s_payWith = "\x17\x23\x3b\x01\x39\x2b\x36\x2a";
    s_checkingConnection = "\x09\x2a\x27\x25\x2d\x2b\x30\x29\x01\x25\x31\x30\x30\x27\x25\x36\x2b\x31\x30\x05\x05\x05";
    s_connected = "\x09\x31\x30\x30\x27\x25\x36\x27\x26\x02";
    s_transactionEstimation = "\x1b\x34\x23\x30\x35\x23\x25\x36\x2b\x31\x30\x01\x0c\x35\x36\x2b\x2f\x23\x36\x2b\x31\x30";
    s_energy = "\x0c\x30\x27\x34\x29\x3b";
    s_time = "\x1b\x2b\x2f\x27";
    s_cost = "\x09\x31\x35\x36";
    s_kWh = "\x2d\x1e\x2a";
    s_currency = "\x03";
    s_startCharging = "START\nCHARGING"; //"\x1a\x36\x23\x34\x36\n\x09\x2a\x23\x34\x29\x2b\x30\x29";
    s_charging = "\x09\x2a\x23\x34\x29\x2b\x30\x29\x05\x05\x05";
    s_pleaseUnplug = "\x17\x2e\x27\x23\x35\x27\x01\x37\x30\x32\x2e\x37\x29\x01\x36\x2a\x27\x01\x25\x2a\x23\x34\x29\x27\x34\x01\x36\x31\x01\x35\x36\x31\x32";
    s_report = "\x19\x27\x32\x31\x34\x36";
    /*s_battery = "\x08\x23\x36\x36\x27\x34\x3b\x06";
    s_batteryHealth = "\x08\x23\x36\x36\x27\x34\x3b\x01\x2a\x27\x23\x2e\x36\x2a\x06";
    s_chargingTime = "\x09\x2a\x23\x34\x29\x2b\x30\x29\x01\x36\x2b\x2f\x27\x06";
    s_totalCost = "\x1b\x31\x36\x23\x2e\x01\x25\x31\x35\x36\x06";*/
    s_minutes = "\x2f\x2b\x30";
    s_battery = "Battery:";
    s_batteryHealth = "Battery health :";
    s_chargingTime = "Charging time :";
    s_totalCost = "Total cost :";
    s_minutes_report = "min";
    s_healthy = "\x0f\x27\x23\x2e\x36\x2a\x3b";
    s_exit = "\x0c\x1f\x10\x1b";
}

void switch_german() {
    s_chooseLanguage = "\x1a\x32\x34\x23\x25\x2a\x27\x01\x39\x41\x2a\x2e\x27\x30";
    s_tapYourCard = "\x08\x2b\x36\x36\x27\x01\x36\x2b\x32\x32\x27\x30\x01\x1a\x2b\x27\x01\x23\x37\x28\x01\x10\x2a\x34\x27\x01\x12\x23\x34\x36\x27,\x01\x37\x2f\x01\x28\x31\x34\x36\x3c\x37\x28\x23\x2a\x34\x27\x30";
    s_checkingTheCard = "\x3f\x24\x27\x34\x32\x34\x43\x28\x27\x30\x01\x26\x27\x34\x01\x12\x23\x34\x36\x27\x01\x05\x05\x05";
    s_authenticated = "\x07\x37\x36\x2a\x27\x30\x36\x2b\x28\x2b\x3c\x2b\x27\x34\x36\x02";
    s_plugIn = "\x07\x15\x1a\x09\x0f\x13\x10\x0c\x40\x0c\x15";
    s_pleaseTouch = "\x08\x2b\x36\x36\x27\x01\x24\x27\x34\x43\x2a\x34\x27\x30\x01\x1a\x2b\x27\x01\x26\x27\x30\x01\x08\x2b\x2e\x26\x35\x25\x2a\x2b\x34\x2f,\x01\x37\x2f\x01\x28\x31\x34\x36\x3c\x37\x28\x23\x2a\x34\x27\x30";
    //s_payWith = "\x08\x27\x3c\x23\x2a\x2e\x27\x30\x01\x2f\x2b\x36";
    s_payWith = "Bezahlen mit";
    s_checkingConnection = "\x1d\x27\x34\x24\x2b\x30\x26\x37\x30\x29\x01\x32\x34\x43\x28\x27\x30\x01\x05\x05\x05";
    s_connected = "\x1d\x27\x34\x24\x2b\x30\x26\x37\x30\x29\x01\x2a\x27\x34\x29\x27\x35\x36\x27\x2e\x2e\x36\x02";
    s_transactionEstimation = "\x1b\x34\x23\x30\x35\x23\x2d\x36\x2b\x31\x30\x35\x35\x25\x2a\x41\x36\x3c\x37\x30\x29";
    s_energy = "\x0c\x30\x27\x34\x29\x2b\x27";
    s_time = "\x21\x27\x2b\x36";
    s_cost = "\x12\x31\x35\x36\x27\x30";
    s_kWh = "\x2d\x1e\x2a";
    
    s_currency = "\x03";
    //s_startCharging = "\x13\x07\x0b\x0c\x1d\x16\x19\x0e\x07\x15\x0e\x01\x08\x0c\x0e\x10\x15\x15\x0c\x15";
    s_startCharging = "LADEVORGANG\nBEGINNEN";
    s_charging = "\x13\x23\x26\x27\x30\x01\x05\x05\x05";
    s_pleaseUnplug = "\x08\x2b\x36\x36\x27\x01\x36\x34\x27\x30\x30\x27\x30\x01\x1a\x2b\x27\x01\x26\x23\x35\x01\x13\x23\x26\x27\x29\x27\x34\x41\x36\x01\x37\x2f\x01\x23\x30\x3c\x37\x2a\x23\x2e\x36\x27\x30";
    s_report = "\x08\x27\x34\x2b\x25\x2a\x36";
    /*s_battery = "\x08\x23\x36\x36\x27\x34\x2b\x27\x06";
    s_batteryHealth = "\x08\x23\x36\x36\x27\x34\x2b\x27\x01\x0e\x27\x35\x37\x30\x26\x2a\x27\x2b\x36\x06\n\x08";
    s_chargingTime = "\x13\x23\x26\x27\x3c\x27\x2b\x36\x06";
    s_totalCost = "\x0e\x27\x35\x23\x2f\x36\x2d\x31\x35\x36\x27\x30\x06";*/
    s_minutes = "\x2f\x2b\x30";
    s_battery = "Batterie:";
    s_batteryHealth = "Batterie Gesundheit:";
    s_chargingTime = "Ladezeit:";
    s_totalCost = "Gesamtkosten:";
    s_minutes_report = "min";
    s_healthy = "\x0e\x27\x35\x37\x30\x26";
    s_exit = "\x08\x0c\x0c\x15\x0b\x0c\x15";
}

void switch_chinese() {
    s_chooseLanguage = "\x46\x22\x40\x3e";
    s_tapYourCard = "\x41\x33\x23\x1e\x38\x12\x34\x3a\x3b";
    s_checkingTheCard = "\x2e\x2d\x12\x03\x03\x03";
    s_authenticated = "\x47\x43\x42\x08\x4b\x3f\x4c";
    s_plugIn = "\x26\x0e";
    s_pleaseTouch = "\x41\x3d\x27\x16\x18\x3a\x3b";
    s_payWith = "\x06\x2f\x29\x1b";
    s_checkingConnection = "\x2e\x2d\x44\x25\x03\x03\x03";
    s_connected = "\x17\x44\x25";
    s_transactionEstimation = "\x05\x2b\x09\x39";
    s_energy = "\x3c\x32";
    s_time = "\x2a\x4a";
    s_cost = "\x1f\x2c";
    s_kWh = "\x11\x36\x2a";
    s_minutes = "\x10\x49";
    s_minutes_report = "\x10\x49";
    s_currency = "\x02";
    s_startCharging = "\x1a\x15\n\x0d\x37";
    s_charging = "\x0d\x37\x04\x03\x03\x03";
    s_pleaseUnplug = "\x41\x21\x24\x0d\x37\x14\x07\x0b\x30";
    s_report = "\x20\x13";
    s_battery = "\x37\x31\x37\x48\x4d";
    s_batteryHealth = "\x37\x31\x35\x1c\x4d";
    s_chargingTime = "\x0d\x37\x2a\x4a\x4d";
    s_totalCost = "\x1d\x1f\x2c\x4d";
    s_healthy = "\x0c\x19";
    s_exit = "\x45\x0f";
}

void switch_language() {
    switch (g_language) {
        case LANG_EN: switch_english(); break;
        case LANG_DE: switch_german();  break;
        case LANG_CN: switch_chinese(); break;
        default:   break;
    }
    font_init();
    sync_up((uint8_t)(g_language));
}

uint32_t f_num = 0;
void font_init() {
    if (g_language == LANG_CN) {
        FontTitle.xf_addr = FontTitleCH.xf_addr;
        FontSmall.xf_addr = FontSmallCH.xf_addr;
        FontBottom.xf_addr = FontBottomCH.xf_addr;
    } else {
        FontTitle.xf_addr = FontTitleEN.xf_addr;
        FontSmall.xf_addr = FontSmallEN.xf_addr;
        FontBottom.xf_addr = FontBottomEN.xf_addr;
    }
}
