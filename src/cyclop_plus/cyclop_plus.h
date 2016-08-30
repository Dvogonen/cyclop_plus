/*******************************************************************************
  CYCLOP+ brings OLED support and manual channel selection to the
  HobbyKing Quanum Cyclops FPV googles.

  The rx5808-pro and rx5808-pro-diversity projects served as a starting
  point for the code base, even if little of the actual code remains.
  Without those projects CYCLOP+ would not have been created. All possible
  credit goes to the two mentioned projects and their contributors.

  I have used wonho-makers Adafruit_SH1106 library to add support for OLED
  screens with an SH1106 controller. The default is to use SSD1306 OLEDs.

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
********************************************************************************/
#ifndef cyclop_plus_h
#define cyclop_plus_h

// The OLED I2C address may be 0x2C or 0x3C. 0x3C is almost always used
//#define OLED_I2C_ADR      0x2C
#define OLED_I2C_ADR      0x3C

// SSD1306 and SH1106 OLED displays are supported. Select one.
#define SSD1306_OLED_DRIVER 
//#define SH1106_OLED_DRIVER

// This definition is used by the ADAFRUIT library
#define OLED_128x64_ADAFRUIT_SCREENS

// User Configuration Options
#define FLIP_SCREEN_OPTION        0
#define LIPO_2S_METER_OPTION      1
#define LIPO_3S_METER_OPTION      2
#define BATTERY_ALARM_OPTION      3
#define SHOW_STARTSCREEN_OPTION   4
#define SAVE_SCREEN_OPTION        5

#define FLIP_SCREEN_DEFAULT       1
#define LIPO_2S_METER_DEFAULT     0
#define LIPO_3S_METER_DEFAULT     1
#define BATTERY_ALARM_DEFAULT     1
#define SHOW_STARTSCREEN_DEFAULT  1
#define SAVE_SCREEN_DEFAULT       0

#define MAX_OPTIONS               6

// User Configuration Commands
#define TEST_ALARM_COMMAND        6
#define RESET_SETTINGS_COMMAND    7
#define EXIT_COMMAND              8
#define MAX_COMMANDS              3

// Number of lines in configuration menu
#define MAX_OPTION_LINES          7

// Delay after key click before screen save (in milli seconds)
#define SAVE_SCREEN_DELAY_MS      10000

// Alarm timing constants (in milli seconds)
#define ALARM_MAX_ON      50
#define ALARM_MAX_OFF     200
#define ALARM_MED_ON      100
#define ALARM_MED_OFF     1000
#define ALARM_MIN_ON      200
#define ALARM_MIN_OFF     3000

// Digital pin definitions
#define SPI_CLOCK_PIN     2
#define SLAVE_SELECT_PIN  3
#define SPI_DATA_PIN      4
#define BUTTON_PIN        5
#define ALARM_PIN         6
#define LED_PIN           13

// Analog pin definitions
#define VOLTAGE_METER_PIN A1
#define RSSI_PIN          A6

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
#define EEPROM_OPTIONS    1
#define EEPROM_CHECK      (EEPROM_OPTIONS + MAX_OPTIONS)

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

// Release information
#define VER_DATE_STRING   "UNFINISHED"
#define VER_INFO_STRING   "v1.5 by Dvogonen"

#endif // cyclop_plus_h
