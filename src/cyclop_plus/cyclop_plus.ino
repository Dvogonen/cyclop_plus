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

  Copyright (c) 2017 Kjell Kernen (Dvogonen)

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
// Application includes
#include "cyclop_plus.h"
#include "rtc6715.h"

// Library includes
#include <avr/pgmspace.h>
#include <string.h>
#include <EEPROM.h>
#ifdef SSD1306_OLED_DRIVER
#include <Adafruit_SSD1306.h>
#endif
#ifdef SH1106_OLED_DRIVER
#include "libraries/adafruit_sh1106/Adafruit_SH1106.h"
#endif
#include <Adafruit_GFX.h>
#include <EnableInterrupt.h>

//******************************************************************************
//* File scope function declarations

void     activateScreenSaver( void );
uint16_t autoScan( uint16_t frequency );
uint16_t averageAnalogRead( uint8_t pin );
void     batteryMeter(void);
void     buttonPressInterrupt();
uint8_t  bestChannelMatch( uint16_t frequency );
void     dissolveDisplay(void);
void     drawAutoScanScreen(void);
void     drawBattery(uint8_t xPos, uint8_t yPos, uint8_t value );
void     drawChannelScreen( uint8_t channel, uint16_t rssi);
void     drawOptionsScreen(uint8_t option, uint8_t in_edit_state);
void     drawScannerScreen( void );
void     drawStartScreen(void);
uint8_t  getClickType(uint8_t buttonPin);
uint16_t getVoltage( void );
uint16_t graphicScanner( uint16_t frequency );
char    *longNameOfChannel(uint8_t channel, char *name);
uint8_t  nextChannel( uint8_t channel);
uint8_t  previousChannel( uint8_t channel);
bool     readEeprom(void);
void     resetOptions(void);
char    *shortNameOfChannel(uint8_t channel, char *name);
void     setOptions( void );
void     spi_0(void);
void     spi_1(void);
void     spiEnableHigh( void );
void     spiEnableLow( void );
int      spiRead( void );
void     testAlarm( void );
void     updateScannerScreen(uint8_t position, uint8_t value );
void     writeEeprom(void);

//******************************************************************************
//* Positions in the frequency table for the 48 channels
//* Direct access via array operations does not work since data is stored in
//* flash, not in RAM. Use getPosition to retrieve data

const uint8_t positions[] PROGMEM = {
  40, 41, 42, 43, 44, 45, 46, 47, 19, 32, 18, 17, 33, 16,  7, 34,
  8, 24,  6,  9, 25,  5, 35, 10, 26,  4, 11, 27,  3, 36, 12, 28,
  2, 13, 29, 37,  1, 14, 30,  0, 15, 31, 38, 20, 21, 39, 22, 23
};

uint16_t getPosition( uint8_t channel ) {
  return pgm_read_byte_near(positions + channel);
}

//******************************************************************************
//* Frequencies for the 48 channels
//* Direct access via array operations does not work since data is stored in
//* flash, not in RAM. Use getFrequency to retrieve data

const uint16_t channelFrequencies[] PROGMEM = {
  5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // Band A - Boscam A
  5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // Band B - Boscam B
  5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // Band E - DJI
  5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // Band F - FatShark \ Immersion
  5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // Band R - Raceband
  5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621  // Band L - Lowband
};

uint16_t getFrequency( uint8_t channel ) {
  return pgm_read_word_near(channelFrequencies + getPosition(channel));
}
//******************************************************************************
//* Reverse lookup for frequencies in position table
//* Direct access via array operations does not work since data is stored in
//* flash, not in RAM. Use getRevesePositions to retrieve data

const unsigned int reversePositions[] PROGMEM = {
  39, 36, 32, 28, 25, 21, 18, 14, // Band A - Boscam A
  16, 19, 23, 26, 30, 33, 37, 40, // Band B - Boscam B
  13, 11, 10,  8, 43, 44, 46, 47, // Band E - DJI
  17, 20, 24, 27, 31, 34, 38, 41, // Band F - FatShark \ Immersion
  9,  12, 15, 22, 29, 35, 42, 45, // Band R - Raceband
  0,   1,  2,  3,  4,  5,  6,  7  // Band L - Lowband
};

unsigned int getReversePosition( unsigned char position ) {
  return pgm_read_word_near(reversePositions + position);
}

//******************************************************************************
//* Other file scope variables
#ifdef SSD1306_OLED_DRIVER
Adafruit_SSD1306 display(4);
#endif
#ifdef SH1106_OLED_DRIVER
Adafruit_SH1106 display(4);
#endif
uint8_t lastClick = NO_CLICK;
uint8_t currentChannel = 0;
uint8_t lastChannel = 0;
uint8_t lastFunction = 255;
uint8_t clickType = NO_CLICK;
unsigned long pauseStart = 0;
uint16_t currentRssi = 0;
uint8_t ledState = LED_ON;
unsigned long saveScreenTimer;
unsigned long displayUpdateTimer = 0;
unsigned long eepromSaveTimer = 0;
unsigned long pulseTimer = 0;
unsigned long alarmTimer = 0;
uint8_t alarmSoundOn = 0;
uint16_t alarmOnPeriod = 0;
uint16_t alarmOffPeriod = 0;
uint8_t options[MAX_OPTIONS];
uint8_t saveScreenActive = 0;
rtc6715 receiver( SPI_CLOCK_PIN, SLAVE_SELECT_PIN, SPI_DATA_PIN );
unsigned char softPositions[48];

//******************************************************************************
//* function: setup
//******************************************************************************
void setup()
{
  int i;

  // initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_ON);

  // initialize button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  enableInterrupt(BUTTON_PIN, buttonPressInterrupt, CHANGE);

  // initialize alarm
  pinMode(ALARM_PIN, OUTPUT );

  // Read current channel and options data from EEPROM
  if (!readEeprom()) {
    currentChannel = CHANNEL_MIN;
    resetOptions();
  }

  // Start receiver
  receiver.setFrequency(getFrequency(currentChannel));

  // Initialize the display
#ifdef SSD1306_OLED_DRIVER
  display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADR);
#endif
#ifdef SH1106_OLED_DRIVER
  display.begin(SH1106_SWITCHCAPVCC, OLED_I2C_ADR);
#endif
  display.clearDisplay();
  if (options[FLIP_SCREEN_OPTION])
    display.setRotation(2);
  display.display();

  // Set Options
  if (digitalRead(BUTTON_PIN) == BUTTON_PRESSED ) {
    setOptions();
    writeEeprom();
    if (options[FLIP_SCREEN_OPTION])
      display.setRotation(2);
  }
  // Show start screen
  if (options[SHOW_STARTSCREEN_OPTION])
    drawStartScreen();

  // Wait at least the delay time before entering screen save mode
  saveScreenTimer = millis() + SAVE_SCREEN_DELAY_MS;
}

//******************************************************************************
//* function: loop
//******************************************************************************
void loop()
{
  switch (lastClick = getClickType( BUTTON_PIN ))
  {
    case NO_CLICK: // do nothing
      break;

    case WAKEUP_CLICK:  // do nothing
      break;

    case LONG_CLICK:
      while ( lastFunction )
        switch (lastFunction = selectFunction())
        {
          case 1:
            currentChannel = bestChannelMatch(graphicScanner(getFrequency(currentChannel)));
            drawChannelScreen(currentChannel, 0);
            displayUpdateTimer = millis() +  RSSI_STABILITY_DELAY_MS ;
            break;
          case 2:
            drawAutoScanScreen();
            currentChannel = bestChannelMatch(autoScan(getFrequency(currentChannel)));
            drawChannelScreen(currentChannel, 0);
            displayUpdateTimer = millis() +  RSSI_STABILITY_DELAY_MS ;
            break;
          case 3:
            setOptions();
            writeEeprom();
            if (options[FLIP_SCREEN_OPTION])
              display.setRotation(2);
            break;
        }
      lastFunction = 255;
      break;

    case SINGLE_CLICK: // up the frequency
      currentChannel = nextChannel( currentChannel );
      receiver.setFrequency(getFrequency(currentChannel));
      drawChannelScreen(currentChannel, 0);
      break;

    case DOUBLE_CLICK:  // down the frequency
      currentChannel = previousChannel( currentChannel );
      receiver.setFrequency(getFrequency(currentChannel));
      drawChannelScreen(currentChannel, 0);
      break;
  }
  // Reset screensaver timer after each key click
  if  (lastClick != NO_CLICK )
    saveScreenTimer = millis() + SAVE_SCREEN_DELAY_MS;

  // Check if the display needs updating
  if ( millis() > displayUpdateTimer ) {
    if ( options[SAVE_SCREEN_OPTION] && (saveScreenTimer < millis()))
      activateScreenSaver();
    else
    {
      currentRssi = averageAnalogRead(RSSI_PIN);
      drawChannelScreen(currentChannel, currentRssi);
      displayUpdateTimer = millis() + 1000;
    }
  }
  // Check if EEPROM needs a save. Reduce EEPROM writes by not saving to often
  if ((currentChannel != lastChannel) && (millis() > eepromSaveTimer))
  {
    writeEeprom();
    lastChannel = currentChannel;
    eepromSaveTimer = millis() + 10000;
  }
  // Check if it is time to switch LED state for the pulse led
  if ( millis() > pulseTimer ) {
    ledState = !ledState;
    pulseTimer = millis() + 500;
  }
  digitalWrite(LED_PIN, ledState);

  // Toggle alarm on or off
  if (options[BATTERY_ALARM_OPTION] && alarmOnPeriod) {
    if (millis() > alarmTimer) {
      alarmSoundOn = !alarmSoundOn;
      if (alarmSoundOn) {
        analogWrite( ALARM_PIN, 1 << options[ALARM_LEVEL_OPTION] );
        alarmTimer = millis() + alarmOnPeriod;
      }
      else {
        analogWrite( ALARM_PIN, 0 );
        alarmTimer = millis() + alarmOffPeriod;
      }
    }
  }
  else
    analogWrite( ALARM_PIN, 0 );
}

//******************************************************************************
//* function: writeEeprom
//*         : Writes all configuration settings to nonvolatile memory
//******************************************************************************
void writeEeprom(void) {
  uint8_t i;
  EEPROM.write(EEPROM_CHANNEL, currentChannel);
  for (i = 0; i < MAX_OPTIONS; i++)
    EEPROM.write(EEPROM_OPTIONS + i, options[i]);
  EEPROM.write(EEPROM_CHECK, VER_EEPROM);
}

//******************************************************************************
//* function: readEeprom
//*         : Reads all configuration settings from nonvolatile memory
//******************************************************************************
bool readEeprom(void) {
  uint8_t i;
  if (EEPROM.read(EEPROM_CHECK) != VER_EEPROM)
    return false;
  currentChannel =   EEPROM.read(EEPROM_CHANNEL);
  for (i = 0; i < MAX_OPTIONS; i++)
    options[i] = EEPROM.read(EEPROM_OPTIONS + i);
  return true;
}

//******************************************************************************
//* function: buttonPressInterrupt
//******************************************************************************
void buttonPressInterrupt() {
  static long clickStart = 0;

  if ( digitalRead(BUTTON_PIN) == BUTTON_PRESSED ) {// Button was pressed
    clickStart = millis();
  }
  else {   // Button was released
    if ( pauseStart) {
      clickType = DOUBLE_CLICK;
      clickStart = 0;
      pauseStart = 0;
    }
    else
    {
      if ( saveScreenActive ) {
        clickType = WAKEUP_CLICK;
        clickStart = 0;
        saveScreenActive = 0;
      }
      else if (( millis() - clickStart) > 350 )
        clickType = LONG_CLICK;
      else
        clickType = SINGLE_CLICK;
      clickStart = 0;
      pauseStart = millis();
    }
  }
}

//******************************************************************************
//* function: getClickType
//******************************************************************************
uint8_t getClickType(uint8_t buttonPin) {
  uint8_t tempClickType = NO_CLICK;

  if (pauseStart && (millis() - pauseStart) < 350)
    return NO_CLICK;

  pauseStart = 0;

  if ( clickType ) {
    tempClickType = clickType;
    clickType = NO_CLICK;
  }
  return tempClickType;
}

//******************************************************************************
//* function: nextChannel
//******************************************************************************
unsigned char incrementChannel(unsigned char channel)
{
  if (channel > (CHANNEL_MAX - 1))
    return CHANNEL_MIN;
  else
    return channel + 1;
}

unsigned char nextChannel(unsigned char channel)
{
  do {
    channel = incrementChannel( channel);
  } while (softPositions[channel] == 255);
  return channel;
}

//******************************************************************************
//* function: previousChannel
//******************************************************************************
unsigned char decrementChannel(unsigned char channel)
{
  if (channel > CHANNEL_MAX)
    return CHANNEL_MAX;

  if ( channel == CHANNEL_MIN )
    return CHANNEL_MAX;

  return channel - 1;
}

unsigned char previousChannel(unsigned char channel)
{
  do {
    channel = decrementChannel( channel );
  } while (softPositions[channel] == 255);
  return channel;
}

//******************************************************************************
//* function: bestChannelMatch
//*         : finds the best matching standard channel for a given frequency
//******************************************************************************
uint8_t bestChannelMatch( uint16_t frequency )
{
  int16_t comp;
  int16_t bestComp = 300;
  uint8_t bestChannel = CHANNEL_MIN;
  uint8_t i;

  for (i = CHANNEL_MIN; i <= CHANNEL_MAX; i++) {
    comp = abs( (int16_t)getFrequency(i) - (int16_t)frequency );
    if (comp < bestComp)
    {
      bestComp = comp;
      bestChannel = i;
    }
  }
  return bestChannel;
}

//******************************************************************************
//* function: graphicScanner
//*         : scans the 5.8 GHz band and draws a graphical representation.
//*         : when the button is pressed the current frequency is returned.
//******************************************************************************
uint16_t graphicScanner( uint16_t frequency ) {
  uint8_t i;
  uint16_t scanRssi;
  uint16_t bestRssi = 0;
  uint16_t scanFrequency = frequency;
  uint16_t bestFrequency = frequency;
  uint8_t clickType;
  uint8_t rssiDisplayValue;

  // Draw screen frame etc
  drawScannerScreen();

  while (digitalRead(BUTTON_PIN) != BUTTON_PRESSED) {
    scanFrequency += SCANNING_STEP;
    if (scanFrequency > FREQUENCY_MAX)
      scanFrequency = FREQUENCY_MIN;
    receiver.setFrequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = averageAnalogRead(RSSI_PIN);
    rssiDisplayValue = (scanRssi - 140) / 10;    // Roughly 2 - 46
    updateScannerScreen(100 - ((FREQUENCY_MAX - scanFrequency) / SCANNING_STEP), rssiDisplayValue );
  }
  // Fine tuning
  scanFrequency = scanFrequency - SCANNING_STEP * 4;
  for (i = 0; i < SCANNING_STEP * 4; i++, scanFrequency += 2) {
    receiver.setFrequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = averageAnalogRead(RSSI_PIN);
    if (bestRssi < scanRssi) {
      bestRssi = scanRssi;
      bestFrequency = scanFrequency;
    }
  }
  // Return the best frequency
  receiver.setFrequency(bestFrequency);
  return (bestFrequency);
}

//******************************************************************************
//* function: autoScan
//******************************************************************************
uint16_t autoScan( uint16_t frequency ) {
  uint8_t i;
  uint16_t scanRssi = 0;
  uint16_t bestRssi = 0;
  uint16_t scanFrequency;
  uint16_t bestFrequency;

  // Skip forward to avoid detecting the current channel
  scanFrequency = frequency + SCANNING_STEP;
  if (!(scanFrequency % 2))
    scanFrequency++;        // RTC6715 can only generate odd frequencies

  // Coarse tuning
  bestFrequency = scanFrequency;
  for (i = 0; i < 60 && (scanRssi < RSSI_TRESHOLD); i++) {
    if ( scanFrequency <= (FREQUENCY_MAX - SCANNING_STEP))
      scanFrequency += SCANNING_STEP;
    else
      scanFrequency = FREQUENCY_MIN;
    receiver.setFrequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = averageAnalogRead(RSSI_PIN);
    if (bestRssi < scanRssi) {
      bestRssi = scanRssi;
      bestFrequency = scanFrequency;
    }
  }
  // Fine tuning
  scanFrequency = bestFrequency - SCANNING_STEP * 4;
  bestRssi = 0;
  for (i = 0; i < SCANNING_STEP * 4; i++, scanFrequency += 2) {
    receiver.setFrequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = averageAnalogRead(RSSI_PIN);
    if (bestRssi < scanRssi) {
      bestRssi = scanRssi;
      bestFrequency = scanFrequency;
    }
  }
  // Return the best frequency
  receiver.setFrequency(bestFrequency);
  return (bestFrequency);
}
//******************************************************************************
//* function: averageAnalogRead
//*         : used to read from an anlog pin
//*         : returns an averaged value between (in theory) 0 and 1024
//*         : this function is called often, so it is speed optimized
//******************************************************************************
uint16_t averageAnalogRead( uint8_t pin)
{
  uint16_t rssi = 0;
  uint8_t i = 32;

  for ( ; i ; i--) {
    rssi += analogRead(pin);
  }
  return (rssi >> 5);
}

//******************************************************************************
//* function: shortNameOfChannel
//******************************************************************************
char *shortNameOfChannel(uint8_t channel, char *name)
{
  uint8_t channelIndex = getPosition(channel);
  if (channelIndex < 8)
    name[0] = 'A';
  else if (channelIndex < 16)
    name[0] = 'B';
  else if (channelIndex < 24)
    name[0] = 'E';
  else if (channelIndex < 32)
    name[0] = 'F';
  else if (channelIndex < 40)
    name[0] = 'C';
  else
    name[0] = 'L';
  name[1] = (channelIndex % 8) + '0' + 1;
  name[2] = 0;
  return name;
}

//******************************************************************************
//* function: longNameOfChannel
//******************************************************************************
char *longNameOfChannel(uint8_t channel, char *name)
{
  uint8_t len;
  uint8_t channelIndex = getPosition(channel);
  if (channelIndex < 8)
    strcpy(name, "Boscam A");
  else if (channelIndex < 16)
    strcpy(name, "Boscam B");
  else if (channelIndex < 24)
    strcpy(name, "Foxtech/DJI ");
  else if (channelIndex < 32)
    strcpy(name, "FatShark ");
  else if (channelIndex < 40)
    strcpy(name, "Raceband ");
  else
    strcpy(name, "Lowband  ");
  len = strlen( name );
  name[len] = (channelIndex % 8) + '0' + 1;
  name[len + 1] = 0;
  return name;
}

//******************************************************************************
//* function: getVoltage
//*         : returns battery voltage as an unsigned integer.
//*         : The value is multiplied with 10, 12volts => 120, 7.2Volts => 72
//*
//*         : Measured voltage values:
//*         : 12.6v = 639   10.8v = 546   8.4v = 411   7.2v = 359
//*         : The result is not linear...
//*         : A rough estimation is that 5 steps correspond to 0.1 volts
//******************************************************************************
uint16_t getVoltage( void )
{
  return ( 50 + (((averageAnalogRead(VOLTAGE_METER_PIN) - 250 + (options[BATTERY_CALIB_OPTION] - 128))) / 5 ));
}
//******************************************************************************
//* function: batteryMeter
//******************************************************************************
void batteryMeter( void )
{
  uint16_t voltage;
  uint8_t value;
  uint16_t minV;
  uint16_t maxV;

  if (options[BATTERY_TYPE_OPTION])
  { /* 2s lipo battery*/
    minV = 68;
    maxV = 84;
  }
  else
  { /* 3s lipo battery */
    minV = 102;
    maxV = 126;
  }
  voltage = getVoltage();

  if (voltage >= maxV)
    value = 99;
  else if (voltage <= minV)
    value = 0;
  else
    value = (uint8_t)((voltage - minV) / (float)(maxV - minV) * 100.0);

  // Set alarm periods
  if (value < 5)
  {
    alarmOnPeriod = ALARM_MAX_ON;
    alarmOffPeriod = ALARM_MAX_OFF;
  }
  else if (value < 15)
  {
    alarmOnPeriod = ALARM_MED_ON;
    alarmOffPeriod = ALARM_MED_OFF;
  }
  else if (value < 25)
  {
    alarmOnPeriod = ALARM_MIN_ON;
    alarmOffPeriod = ALARM_MIN_OFF;
  }
  else
  {
    alarmOnPeriod = 0;
    alarmOffPeriod = 0;
  }
  drawBattery(58, 32, value);
}

//******************************************************************************
//* function: updateSoftPositions
//******************************************************************************
void updateSoftPositions( void ) {
  unsigned char i;

  // Mark blocked channels in the softPositions array
  for (i = 0; i < 48; i++)
    softPositions[i] = positions[i];

  if (!options[A_BAND_OPTION]) {
    for (i = 0; i < 8; i++)
      softPositions[getReversePosition( i )] = 255;
  }
  if (!options[B_BAND_OPTION]) {
    for (i = 8; i < 16; i++)
      softPositions[getReversePosition( i )] = 255;
  }
  if (!options[E_BAND_OPTION]) {
    for (i = 16; i < 24; i++)
      softPositions[getReversePosition( i )] = 255;
  }
  if (!options[F_BAND_OPTION]) {
    for (i = 24; i < 32; i++)
      softPositions[getReversePosition( i )] = 255;
  }
  if (!options[R_BAND_OPTION]) {
    for (i = 32; i < 40; i++)
      softPositions[getReversePosition( i )] = 255;
  }
  if (!options[L_BAND_OPTION]) {
    for (i = 40; i < 48; i++)
      softPositions[getReversePosition( i )] = 255;
  }
}

//******************************************************************************
//* function: resetOptions
//*         : Resets all configuration settings to their default values
//******************************************************************************

//******************************************************************************
//* function: resetOptions
//*         : Resets all configuration settings to their default values
//******************************************************************************
void resetOptions(void) {
  options[FLIP_SCREEN_OPTION]      = FLIP_SCREEN_DEFAULT;
  options[BATTERY_ALARM_OPTION]    = BATTERY_ALARM_DEFAULT;
  options[ALARM_LEVEL_OPTION]      = ALARM_LEVEL_DEFAULT;
  options[BATTERY_TYPE_OPTION]     = BATTERY_TYPE_DEFAULT;
  options[BATTERY_CALIB_OPTION]    = BATTERY_CALIB_DEFAULT;
  options[SHOW_STARTSCREEN_OPTION] = SHOW_STARTSCREEN_DEFAULT;
  options[SAVE_SCREEN_OPTION]      = SAVE_SCREEN_DEFAULT;
  options[A_BAND_OPTION]           = A_BAND_DEFAULT;
  options[B_BAND_OPTION]           = B_BAND_DEFAULT;
  options[E_BAND_OPTION]           = E_BAND_DEFAULT;
  options[F_BAND_OPTION]           = F_BAND_DEFAULT;
  options[R_BAND_OPTION]           = R_BAND_DEFAULT;
  options[L_BAND_OPTION]           = L_BAND_DEFAULT;

  updateSoftPositions();
}

//******************************************************************************
//* function: setOptions
//******************************************************************************
void setOptions()
{
  uint8_t exitNow = false;
  uint8_t menuSelection = 0;
  uint8_t click = NO_CLICK;
  uint8_t in_edit_state = 0;

  // Display option screen
  drawOptionsScreen( menuSelection, in_edit_state );

  // Let the user release the button
  getClickType( BUTTON_PIN );

  while ( !exitNow )
  {
    drawOptionsScreen( menuSelection, in_edit_state );
    click = getClickType( BUTTON_PIN );

    if (in_edit_state)
      switch ( click )
      {
        case NO_CLICK:          // do nothing
          break;

        case SINGLE_CLICK:      // Increase Option
          if (menuSelection == ALARM_LEVEL_OPTION)
          {
            if (options[ALARM_LEVEL_OPTION] < 8 )
              options[ALARM_LEVEL_OPTION]++;
          }
          else if (menuSelection == BATTERY_CALIB_OPTION)
          {
            if (options[BATTERY_CALIB_OPTION] < 250)
              options[BATTERY_CALIB_OPTION] += 5;
          }
          else
            options[menuSelection] = !options[menuSelection];
          break;


        case DOUBLE_CLICK:      // Decrease Option
          if (menuSelection == ALARM_LEVEL_OPTION)
          {
            if (options[ALARM_LEVEL_OPTION] > 1)
              options[ALARM_LEVEL_OPTION]--;
          }
          else if (menuSelection == BATTERY_CALIB_OPTION)
          {
            if (options[BATTERY_CALIB_OPTION] > 5)
              options[BATTERY_CALIB_OPTION] -= 5;
          }
          else
            options[menuSelection] = !options[menuSelection];
          break;

        case LONG_CLICK:        // Execute command or toggle option
          in_edit_state = 0;
          break;
      }
    else
      switch ( click )
      {
        case NO_CLICK:          // do nothing
          break;

        case SINGLE_CLICK:      // Move to next option
          menuSelection++;
          if (menuSelection >= MAX_OPTIONS + MAX_COMMANDS)
            menuSelection = 0;
          break;

        case DOUBLE_CLICK:      // Move to previous option
          if (menuSelection == 0)
            menuSelection = MAX_OPTIONS + MAX_COMMANDS - 1;
          else
            menuSelection--;
          break;

        case LONG_CLICK:        // Execute command or edit option
          if (menuSelection == EXIT_COMMAND)
            exitNow = true;
          else if (menuSelection == RESET_SETTINGS_COMMAND)
            resetOptions();
          else if (menuSelection == TEST_ALARM_COMMAND) {
            testAlarm();
          }
          else
            in_edit_state = 1;
          break;
      }
  }
  updateSoftPositions();
}

//******************************************************************************
//* function: testAlarm
//*         : Cycles through alarms, regardless of alarm settings
//******************************************************************************
void testAlarm( void ) {
  unsigned char i;

  while (getClickType(BUTTON_PIN) == NO_CLICK) {
    for (i = 0; i < 3; i++) {
      analogWrite( ALARM_PIN, 1 << options[ALARM_LEVEL_OPTION] );
      delay(ALARM_MAX_ON);
      analogWrite( ALARM_PIN, 0 );
      delay(ALARM_MAX_OFF);
    }
  }
}

//******************************************************************************
//* function: selectFunction
//******************************************************************************
uint8_t selectFunction(void)
{
  uint8_t function = 0;
  uint8_t lastClick = NO_CLICK;
  do
  {
    drawFunctionScreen( function );
    lastClick = getClickType( BUTTON_PIN );
    if (lastClick == SINGLE_CLICK)
      function == 3 ? function = 0 : function++;
    if (lastClick == DOUBLE_CLICK)
      function == 0 ? function = 3 : function--;
  }
  while ( lastClick != LONG_CLICK );
  return ( function );
}

//******************************************************************************
//* Screen functions
//******************************************************************************
//******************************************************************************
//* function: dissolveDisplay
//*         : fancy graphics stuff that dissolves the screen into black
//*         : unnecessary, but fun
//******************************************************************************
void dissolveDisplay(void)
{
  uint8_t x, y, i = 30;
  uint16_t j;
  while (i--) {
    if (digitalRead(BUTTON_PIN) == BUTTON_PRESSED) // Return if button pressed
      return;
    j = 250;
    while (j--) {
      x = random(128);
      y = random(64);
      display.drawPixel( x, y, BLACK);
    }
    display.display();
  }
  display.clearDisplay();
  display.display();
}

//******************************************************************************
//* function: drawStartScreen
//*         : displays a boot image for a short time
//******************************************************************************
void drawStartScreen( void ) {
  uint8_t i;

  display.clearDisplay();
  display.drawLine(0, 0, 127, 0, WHITE);
  display.setTextColor(WHITE);
  display.setCursor(1, 4);
  display.setTextSize(3);
  display.print(F("CYCLOP+"));
  display.drawLine(0, 27, 127, 27, WHITE);
  display.setCursor(15, 35);
  display.setTextSize(1);
  display.print(F(VER_INFO_STRING));
  display.setCursor(33, 50);
  display.print(F(VER_DATE_STRING));
  display.display();

  // Return after 2000 ms or when button is pressed
  for (i = 200; i; i--)
  {
    if (digitalRead(BUTTON_PIN) == BUTTON_PRESSED )
      return;
    delay(10);
  }
  dissolveDisplay();
  return;
}

//******************************************************************************
//* function: drawChannelScreen
//*         : draws the standard screen with channel information
//******************************************************************************
void drawChannelScreen( uint8_t channel, uint16_t rssi) {
  char buffer[22];
  uint8_t i;

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.setTextSize(3);
  display.print(getFrequency(channel));
  display.setCursor(75, 7);
  display.setTextSize(2);
  display.print(F(" MHz"));
  display.drawLine(0, 24, 127, 24, WHITE);
  display.setCursor(0, 27);
  display.setTextSize(1);
  display.print(F("  Channel    RSSI"));
  display.setCursor(0, 39);
  display.setTextSize(2);
  display.print(F(" "));
  display.print(F(" "));
  display.print(shortNameOfChannel(channel, buffer));
  display.print(F("  "));
  display.print(rssi);
  display.setCursor(0, 57);
  display.setTextSize(1);
  longNameOfChannel(channel, buffer);
  i = (21 - strlen(buffer)) / 2;
  for (; i; i--) {
    display.print(F(" "));
  }
  display.print( buffer );
  batteryMeter();
  display.display();
}

//******************************************************************************
//* function: drawFunctionScreen
//******************************************************************************
#define XPOS  14
#define YPOS  14
uint8_t drawFunctionScreen( uint8_t function )
{
  display.fillRect(9, 9, 110, 46, BLACK);
  display.drawRect(10, 10, 108, 44, WHITE);
  display.setTextSize(1);
  display.setCursor(XPOS, YPOS);
  display.setTextColor(function == 0 ? BLACK : WHITE, function == 0 ? WHITE : BLACK);
  display.print(F(" Exit            "));
  display.setCursor(XPOS, YPOS + 9);
  display.setTextColor(function == 1 ? BLACK : WHITE, function == 1 ? WHITE : BLACK);
  display.print(F(" Graphic Scanner "));
  display.setCursor(XPOS, YPOS + 18);
  display.setTextColor(function == 2 ? BLACK : WHITE, function == 2 ? WHITE : BLACK);
  display.print(F(" Auto Scanner    "));
  display.setCursor(XPOS, YPOS + 27);
  display.setTextColor(function == 3 ? BLACK : WHITE, function == 3 ? WHITE : BLACK);
  display.print(F(" Options         "));
  display.display();
}

//******************************************************************************
//* function: drawAutoScanScreen
//******************************************************************************
void drawAutoScanScreen( void )
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.setTextSize(3);
  display.print(F("SCAN"));
  display.setCursor(75, 7);
  display.setTextSize(2);
  display.print(F(" MHz"));
  display.drawLine(0, 24, 127, 24, WHITE);
  display.setCursor(0, 27);
  display.setTextSize(1);
  display.print(F("  Channel    RSSI"));
  batteryMeter();
  display.display();
}

//******************************************************************************
//* function: drawScannerScreen
//******************************************************************************
void drawScannerScreen( void ) {

  display.clearDisplay();
  display.drawLine(0, 55, 127, 55, WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 57);
  if ( options[L_BAND_OPTION] )
    display.print(F("5.35     5.6     5.95"));
  else
    display.print(F("5.65     5.8     5.95"));
  updateScannerScreen(0, 0);
}

//******************************************************************************
//* function: updateScannerScreen
//*         : position = 0 to 99
//*         : value = 0 to 53
//*         : must be fast since there are frequent updates
//******************************************************************************
void updateScannerScreen(uint8_t position, uint8_t value ) {
  // uint8_t i;
  static uint8_t last_position = 14;
  static uint8_t last_value = 0;

  // The scan graph only uses the 100 positions in the middle of the screen
  position = position + 14;

  // Errase the scan line from the last pass
  display.drawFastVLine( last_position, 0, 54 - last_value, BLACK );

  // Draw the current scan line
  display.drawFastVLine( position, 0, 54, WHITE );

  // Save position and value for the next pass
  last_position = position;
  if (value > 53)
    last_value = 53;
  else
    last_value = value;
  display.display();
}

//******************************************************************************
//* function: drawBattery
//*         : value = 0 to 100
//******************************************************************************
void drawBattery(uint8_t xPos, uint8_t yPos, uint8_t value ) {

  display.drawRect(3 + xPos,  0 + yPos, 4, 2, WHITE);
  display.drawRect(0 + xPos, 2 + yPos, 10, 20, WHITE);
  display.drawRect(2 + xPos,  4 + yPos, 6, 16, BLACK);
  if (value > 85)
    display.drawRect(3 + xPos,  5 + yPos, 4, 2, WHITE);
  if (value > 65)
    display.drawRect(3 + xPos,  8 + yPos, 4, 2, WHITE);
  if (value > 45)
    display.drawRect(3 + xPos,  11 + yPos, 4, 2, WHITE);
  if (value > 25)
    display.drawRect(3 + xPos, 14 + yPos, 4, 2, WHITE);
  if (value > 5)
    display.drawRect(3 + xPos, 17 + yPos, 4, 2, WHITE);
}

//******************************************************************************
//* function: drawOptionsScreen
//******************************************************************************
void drawOption(uint8_t option)
{
  if ( option == ALARM_LEVEL_OPTION )
  {
    display.print("   ");
    display.print(options[option]);
  }
  else if ( option == BATTERY_TYPE_OPTION )
  {
    if (options[option])
      display.print(F("  2s"));
    else
      display.print(F("  3s"));
  }
  else if ( option == BATTERY_CALIB_OPTION )
  {
    display.print(getVoltage() / 10);
    display.print(F("."));
    display.print(getVoltage() % 10);
  }
  else if (option == TEST_ALARM_COMMAND || option == RESET_SETTINGS_COMMAND || option == EXIT_COMMAND )
  {
    display.print("    ");
  }
  else
  {
    if (options[option])
      display.print(F("  ON"));
    else
      display.print(F(" OFF"));
  }
}

void drawOptionsScreen(uint8_t option, uint8_t in_edit_state ) {
  uint8_t i, j;
  if ( in_edit_state ) {
    display.setCursor( 17 * 6, 1 * 8 );
    display.setTextColor(BLACK, WHITE);
    drawOption( option );
  }
  else
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);

    if (option != 0)
      j = option - 1;
    else
      j = MAX_OPTIONS + MAX_COMMANDS - 1;

    for (i = 0; i < MAX_OPTION_LINES; i++, j++)
    {
      if (j >= (MAX_OPTIONS + MAX_COMMANDS))
        j = 0;

      if (j == option)
        display.setTextColor(BLACK, WHITE);
      else
        display.setTextColor(WHITE, BLACK);

      switch (j) {
        case FLIP_SCREEN_OPTION:       display.print(F("Flip Screen     ")); break;
        case BATTERY_ALARM_OPTION:     display.print(F("Battery alarm   ")); break;
        case ALARM_LEVEL_OPTION:       display.print(F("Alarm level     ")); break;
        case BATTERY_TYPE_OPTION:      display.print(F("Battery Type    ")); break;
        case BATTERY_CALIB_OPTION:     display.print(F("Battery Calib.  ")); break;
        case SHOW_STARTSCREEN_OPTION:  display.print(F("Show Startscreen")); break;
        case SAVE_SCREEN_OPTION:       display.print(F("Screen Saver    ")); break;
        case A_BAND_OPTION:            display.print(F("Boscam A band   ")); break;
        case B_BAND_OPTION:            display.print(F("Boscam B band   ")); break;
        case E_BAND_OPTION:            display.print(F("Foxtech/DJI band")); break;
        case F_BAND_OPTION:            display.print(F("Fatshark band   ")); break;
        case R_BAND_OPTION:            display.print(F("Race Band       ")); break;
        case L_BAND_OPTION:            display.print(F("Low Band        ")); break;
        case TEST_ALARM_COMMAND:       display.print(F("Test Alarm      ")); break;
        case RESET_SETTINGS_COMMAND:   display.print(F("Reset Settings  ")); break;
        case EXIT_COMMAND:             display.print(F("Exit            ")); break;
      }
      display.setTextColor(WHITE, BLACK);
      display.print(F(" "));
      drawOption( j );
      display.println();
    }
  }
  display.display();
}

//******************************************************************************
//* function: activateScreenSaver
//******************************************************************************
void activateScreenSaver( void)
{
  display.clearDisplay();
  display.display();
  saveScreenActive = 1;
}

