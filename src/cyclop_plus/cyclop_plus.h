/*******************************************************************************
 CYCLOP+ brings OLED support and manual channel selection to the
 HobbyKing Quanum Cyclops FPV googles.

 The rx5808-pro and rx5808-pro-diversity projects served as a starting
 point for the code base, even if very little of the actual code remains.
 Without those projects CYCLOP+ would not have been created. All possible
 credit goes to the two mentioned projects and their contributors.

 The MIT License (MIT)

 Copyright (c) 2016 Kjell Kernen (Dvogonen)

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*******************************************************************************/

#ifndef cyclop_plus_h
#define cyclop_plus_h
// This definition is used by the ADAFRUIT library
#define OLED_128x64_ADAFRUIT_SCREENS

// Digital pin definitions
#define BUTTON_PIN        5
#define SPI_DATA_PIN      4
#define SLAVE_SELECT_PIN  3
#define SPI_CLOCK_PIN     2
#define LED_PIN           13

// Analog pin definitions
#define RSSI_PIN          A6

// button debounce delay in ms
#define DEBOUNCE_MS       75

// Minimum delay between setting a channel and trusting the RSSI values
#define RSSI_STABILITY_DELAY_MS 25

// RSSI threshold for accepting a channel
#define RSSI_TRESHOLD     250

// Channels in use 
#define CHANNEL_MIN       0
#define CHANNEL_MAX       39

// Max and Min frequencies
#define FREQUENCY_MIN     5645
#define FREQUENCY_MAX     5945

//EEPROM addresses
#define EEPROM_CHANNEL    0

// click types
#define NO_CLICK          0
#define SINGLE_CLICK      1
#define DOUBLE_CLICK      2
#define LONG_CLICK        3
#define LONG_LONG_CLICK   4

// Button pins go low or high on button clicks
#define BUTTON_PRESSED    LOW

// LED state defines
#define LED_OFF           LOW
#define LED_ON            HIGH

// The OLED screens have either got address 0x2C or 0x3C
//#define OLED_I2C_ADR      0x2C
#define OLED_I2C_ADR      0x3C

// Relese information
#define VER_DATE_STRING   "2016-06-20"
#define VER_INFO_STRING   "v1.2 by Dvogonen"

#endif // cyclop_plus_h
