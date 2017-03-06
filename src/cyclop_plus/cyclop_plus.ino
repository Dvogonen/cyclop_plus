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

//******************************************************************************
//* File scope function declarations

void     activateScreenSaver( void );
uint16_t autoScan( uint16_t frequency );
uint16_t averageAnalogRead( uint8_t pin );
void     batteryMeter(void);
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
void     setRTC6715Frequency(uint16_t frequency);
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
  5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // Band C - Raceband
  5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621  // Band L - Lowband
};

uint16_t getFrequency( uint8_t channel ) {
  return pgm_read_word_near(channelFrequencies + getPosition(channel));
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

//******************************************************************************
//* function: setup
//******************************************************************************
void setup()
{
  // initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_ON);

  // initialize button pin
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, INPUT_PULLUP);

  // initialize alarm
  pinMode(ALARM_PIN, OUTPUT );

  // SPI pins for RX control
  pinMode (SLAVE_SELECT_PIN, OUTPUT);
  pinMode (SPI_DATA_PIN, OUTPUT);
  pinMode (SPI_CLOCK_PIN, OUTPUT);

  // Read current channel and options data from EEPROM
  if (!readEeprom()) {
    currentChannel = CHANNEL_MIN;
    resetOptions();
  }

  // Start receiver
  setRTC6715Frequency(getFrequency(currentChannel));

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

  return;
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

    case LONG_LONG_CLICK: // graphical band scanner
      currentChannel = bestChannelMatch(graphicScanner(getFrequency(currentChannel)));
      drawChannelScreen(currentChannel, 0);
      displayUpdateTimer = millis() +  RSSI_STABILITY_DELAY_MS ;
      break;

    case LONG_CLICK:      // auto search
      drawAutoScanScreen();
      currentChannel = bestChannelMatch(autoScan(getFrequency(currentChannel)));
      drawChannelScreen(currentChannel, 0);
      displayUpdateTimer = millis() +  RSSI_STABILITY_DELAY_MS ;
      break;

    case SINGLE_CLICK: // up the frequency
      currentChannel = nextChannel( currentChannel );
      setRTC6715Frequency(getFrequency(currentChannel));
      drawChannelScreen(currentChannel, 0);
      break;

    case DOUBLE_CLICK:  // down the frequency
      currentChannel = previousChannel( currentChannel );
      setRTC6715Frequency(getFrequency(currentChannel));
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
//* function: resetOptions
//*         : Resets all configuration settings to their default values
//******************************************************************************
void resetOptions(void) {
  options[FLIP_SCREEN_OPTION]      = FLIP_SCREEN_DEFAULT;
  options[LIPO_2S_METER_OPTION]    = LIPO_2S_METER_DEFAULT;
  options[LIPO_3S_METER_OPTION]    = LIPO_3S_METER_DEFAULT;
  options[BATTERY_ALARM_OPTION]    = BATTERY_ALARM_DEFAULT;
  options[ALARM_LEVEL_OPTION]      = ALARM_LEVEL_DEFAULT;
  options[BATTERY_CALIB_OPTION]    = BATTERY_CALIB_DEFAULT;
  options[SHOW_STARTSCREEN_OPTION] = SHOW_STARTSCREEN_DEFAULT;
  options[SAVE_SCREEN_OPTION]      = SAVE_SCREEN_DEFAULT;
  options[LOW_BAND_OPTION]         = LOW_BAND_DEFAULT;
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
  EEPROM.write(EEPROM_CHECK, 239);
}

//******************************************************************************
//* function: readEeprom
//*         : Reads all configuration settings from nonvolatile memory
//******************************************************************************
bool readEeprom(void) {
  uint8_t i;
  if (EEPROM.read(EEPROM_CHECK) != 239)
    return false;
  currentChannel =   EEPROM.read(EEPROM_CHANNEL);
  for (i = 0; i < MAX_OPTIONS; i++)
    options[i] = EEPROM.read(EEPROM_OPTIONS + i);
  return true;
}

//******************************************************************************
//* function: get_click_type
//*         : Polls the specified pin and returns the type of click that was
//*         : performed NO_CLICK, SINGLE_CLICK, DOUBLE_CLICK, LONG_CLICK,
//*         : LONG_LONG_CLICK or WAKEUP_CLICK
//******************************************************************************
uint8_t getClickType(uint8_t buttonPin) {
  uint16_t timer = 0;
  uint8_t click_type = NO_CLICK;

  // check if the key has been pressed
  if (digitalRead(buttonPin) == !BUTTON_PRESSED)
    return ( NO_CLICK );

  while (digitalRead(buttonPin) == BUTTON_PRESSED) {
    timer++;
    delay(5);
  }
  if (timer < 120)                  // 120 * 5 ms = 0.6s
    click_type = SINGLE_CLICK;
  if (timer >= 80 && timer < 300 )  // 300 * 5 ms = 1.5s
    click_type = LONG_CLICK;
  if (timer >= 300)
    click_type = LONG_LONG_CLICK;

  // If the screen saver is active the key press is just a wakeup call
  if (saveScreenActive) {
    saveScreenActive = 0;
    if ( click_type == SINGLE_CLICK )
      return ( WAKEUP_CLICK );
  }

  // Check if there is a second click
  timer = 0;
  while ((digitalRead(buttonPin) == !BUTTON_PRESSED) && (timer++ < 40)) {
    delay(5);
  }
  if (timer >= 40)                  // 40 * 5 ms = 0.2s
    return click_type;

  if (digitalRead(buttonPin) == BUTTON_PRESSED ) {
    click_type = DOUBLE_CLICK;
    while (digitalRead(buttonPin) == BUTTON_PRESSED) ;
  }
  return (click_type);
}

//******************************************************************************
//* function: nextChannel
//******************************************************************************
uint8_t nextChannel(uint8_t channel)
{
  if (channel > (CHANNEL_MAX - 1))
    return CHANNEL_MIN;
  else
    return channel + 1;
}

//******************************************************************************
//* function: previousChannel
//******************************************************************************
uint8_t previousChannel(uint8_t channel)
{
  if ((channel > CHANNEL_MAX) || (channel == CHANNEL_MIN))
    return CHANNEL_MAX;
  else
    return channel - 1;
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

  while ((clickType = getClickType(BUTTON_PIN)) == NO_CLICK) {
    scanFrequency += SCANNING_STEP;
    if (scanFrequency > FREQUENCY_MAX)
      scanFrequency = FREQUENCY_MIN;
    setRTC6715Frequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = averageAnalogRead(RSSI_PIN);
    rssiDisplayValue = (scanRssi - 140) / 10;    // Roughly 2 - 46
    updateScannerScreen(100 - ((FREQUENCY_MAX - scanFrequency) / SCANNING_STEP), rssiDisplayValue );
  }
  // Fine tuning
  scanFrequency = scanFrequency - 20;
  for (i = 0; i < 20; i++, scanFrequency += 2) {
    setRTC6715Frequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = averageAnalogRead(RSSI_PIN);
    if (bestRssi < scanRssi) {
      bestRssi = scanRssi;
      bestFrequency = scanFrequency;
    }
  }
  // Return the best frequency
  setRTC6715Frequency(bestFrequency);
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

  // Skip 10 MHz forward to avoid detecting the current channel
  scanFrequency = frequency + SCANNING_STEP;
  if (!(scanFrequency % 2))
    scanFrequency++;        // RTC6715 can only generate odd frequencies

  // Coarse tuning
  bestFrequency = scanFrequency;
  for (i = 0; i < 60 && (scanRssi < RSSI_TRESHOLD); i++) {
    if ( scanFrequency <= (FREQUENCY_MAX - 5))
      scanFrequency += 5;
    else
      scanFrequency = FREQUENCY_MIN;
    setRTC6715Frequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = averageAnalogRead(RSSI_PIN);
    if (bestRssi < scanRssi) {
      bestRssi = scanRssi;
      bestFrequency = scanFrequency;
    }
  }
  // Fine tuning
  scanFrequency = bestFrequency - 20;
  bestRssi = 0;
  for (i = 0; i < 20; i++, scanFrequency += 2) {
    setRTC6715Frequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = averageAnalogRead(RSSI_PIN);
    if (bestRssi < scanRssi) {
      bestRssi = scanRssi;
      bestFrequency = scanFrequency;
    }
  }
  // Return the best frequency
  setRTC6715Frequency(bestFrequency);
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
  else if (channelIndex < 32)
    name[0] = 'R';
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
//* function: readRTC6715Register  - NOT TESTED!
//*         : Returns the contents of a given register in  long.
//*         : The 20 LSB of the long is the register content. The rest is zero
//*         : padding.
//*
//* SPI data: 4  bits  Register Address  LSB first
//*         : 1  bit   Read or Write     0=Read 1=Write
//*         : 20 bits  Register content
//******************************************************************************
long readRTC6715Register( uint8_t reg )
{
  long retVal = 0;
  uint8_t i;

  // Enable SPI
  spiEnableHigh();
  spiEnableLow();

  // Address (4 LSB bits)
  for (i = 4; i; i--, reg >>= 1 ) {
    (reg & 0x1) ? spi_1() : spi_0();
  }
  // Read/Write bit
  spi_0(); // Read

  // Data (20 LSB bits)
  for (i = 20; i; i--, retVal <<= 1 ) {
    spiRead() ? retVal &= 0x01 : retVal &= 0x00;
  }
  // Disable SPI
  spiEnableHigh();

  return retVal;
}

//******************************************************************************
//* function: calcFrequencyData
//*         : calculates the frequency value for the syntheziser register B of
//*         : the RTC6751 circuit that is used within the RX5808/RX5880 modules.
//*         : this value is inteded to be loaded to register at adress 1 via SPI
//*         :
//*  Formula: frequency = ( N*32 + A )*2 + 479
//******************************************************************************
uint16_t calcFrequencyData( uint16_t frequency )
{
  uint16_t N;
  uint8_t A;
  frequency = (frequency - 479) / 2;
  N = frequency / 32;
  A = frequency % 32;
  return (N << 7) |  A;
}

//******************************************************************************
//* function: setRTC6715Frequency
//*         : for a given frequency the register setting for synth register B of
//*         : the RTC6715 circuit is calculated and bitbanged via the SPI bus
//*         : please note that the synth register A is assumed to have default
//*         : values.
//*
//* SPI data:  4 bits  Register Address  LSB first
//*         :  1 bit   Read or Write     0=Read 1=Write
//*         : 13 bits  N-Register Data   LSB first
//*         :  7 bits  A-Register        LSB first
//******************************************************************************
void setRTC6715Frequency(uint16_t frequency)
{
  uint16_t sRegB;
  uint8_t i;

  sRegB = calcFrequencyData(frequency);

  // Bit bang the syntheziser register

  // Enable SPI pin
  spiEnableHigh();
  delayMicroseconds(1);
  spiEnableLow();

  // Address (0x1)
  spi_1();
  spi_0();
  spi_0();
  spi_0();

  // Read/Write (Write)
  spi_1();

  // Data (16 LSB bits)
  for (i = 16; i; i--, sRegB >>= 1 ) {
    (sRegB & 0x1) ? spi_1() : spi_0();
  }
  // Data zero padding
  spi_0();
  spi_0();
  spi_0();
  spi_0();

  // Disable SPI pin
  spiEnableHigh();
  delayMicroseconds(1);

  digitalWrite(SLAVE_SELECT_PIN, LOW);
  digitalWrite(SPI_CLOCK_PIN, LOW);
  digitalWrite(SPI_DATA_PIN, LOW);
}

//******************************************************************************
//* function: spi_1
//******************************************************************************
void spi_1( void )
{
  digitalWrite(SPI_CLOCK_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(SPI_DATA_PIN, HIGH);
  delayMicroseconds(1);
  digitalWrite(SPI_CLOCK_PIN, HIGH);
  delayMicroseconds(1);
  digitalWrite(SPI_CLOCK_PIN, LOW);
  delayMicroseconds(1);
}

//******************************************************************************
//* function: spi_0
//******************************************************************************
void spi_0( void )
{
  digitalWrite(SPI_CLOCK_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(SPI_DATA_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(SPI_CLOCK_PIN, HIGH);
  delayMicroseconds(1);
  digitalWrite(SPI_CLOCK_PIN, LOW);
  delayMicroseconds(1);
}

//******************************************************************************
//* function: spiRead  - NOT TESTED!
//******************************************************************************
int spiRead( void )
{
  int retVal;
  digitalWrite(SPI_CLOCK_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(SPI_CLOCK_PIN, HIGH);
  delayMicroseconds(1);
  digitalWrite(SPI_CLOCK_PIN, LOW);
  retVal = digitalRead(SPI_DATA_PIN);
  delayMicroseconds(1);
  return retVal;
}

//******************************************************************************
//* function: spiEnableLow
//******************************************************************************
void spiEnableLow( void )
{
  delayMicroseconds(1);
  digitalWrite(SLAVE_SELECT_PIN, LOW);
  delayMicroseconds(1);
}

//******************************************************************************
//* function: spiEnableHigh
//******************************************************************************
void spiEnableHigh( void )
{
  delayMicroseconds(1);
  digitalWrite(SLAVE_SELECT_PIN, HIGH);
  delayMicroseconds(1);
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

  // Do no calculations if the meter should not be displayed
  if (!options[LIPO_3S_METER_OPTION] && !options[LIPO_2S_METER_OPTION])
    return;

  if (options[LIPO_3S_METER_OPTION])
  {
    minV = 102;
    maxV = 126;
  }
  if (options[LIPO_2S_METER_OPTION])
  {
    minV = 68;
    maxV = 84;
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
              options[BATTERY_CALIB_OPTION]+= 5;
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
              options[BATTERY_CALIB_OPTION]-=5;
          }
          else
            options[menuSelection] = !options[menuSelection];
          break;

        case LONG_CLICK:        // Execute command or toggle option
        case LONG_LONG_CLICK:
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
        case LONG_LONG_CLICK:
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
}

//******************************************************************************
//* function: testAlarm
//*         : Cycles through alarms, regardless of alarm settings
//******************************************************************************
void testAlarm( void ) {
  uint8_t i;

  for ( i = 0; i < 3; i++) {
    analogWrite( ALARM_PIN, 32 ); delay(ALARM_MIN_ON);
    analogWrite( ALARM_PIN, 0 );  delay(ALARM_MIN_OFF);
  }
  for (i = 0; i < 3; i++) {
    analogWrite( ALARM_PIN, 32 ); delay(ALARM_MED_ON);
    analogWrite( ALARM_PIN, 0 );  delay(ALARM_MED_OFF);
  }
  for (i = 0; i < 3; i++) {
    analogWrite( ALARM_PIN, 32 ); delay(ALARM_MAX_ON);
    analogWrite( ALARM_PIN, 0 );  delay(ALARM_MAX_OFF);
  }
  analogWrite( ALARM_PIN, 0 );
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
//* function: drawAutoScanScreen
//******************************************************************************
void drawAutoScanScreen( void ) {

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
  if ( options[LOW_BAND_OPTION] )
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
void drawOptionsScreen(uint8_t option, uint8_t in_edit_state ) {
  uint8_t i, j;

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
    display.setTextColor(WHITE, BLACK);
    if (j == option && !in_edit_state) {
      display.setTextColor(BLACK, WHITE);
    }
    switch (j) {
      case FLIP_SCREEN_OPTION:       display.print(F("Flip Screen     ")); break;
      case LIPO_2S_METER_OPTION:     display.print(F("LiPo 2s Meter   ")); break;
      case LIPO_3S_METER_OPTION:     display.print(F("LiPo 3s Meter   ")); break;
      case BATTERY_ALARM_OPTION:     display.print(F("Battery Alarm   ")); break;
      case ALARM_LEVEL_OPTION:       display.print(F("Alarm Level     ")); break;
      case BATTERY_CALIB_OPTION:     display.print(F("Battery Calib.  ")); break;
      case SHOW_STARTSCREEN_OPTION:  display.print(F("Show Startscreen")); break;
      case SAVE_SCREEN_OPTION:       display.print(F("Screen Saver    ")); break;
      case LOW_BAND_OPTION:          display.print(F("Display Low Band")); break;
      case TEST_ALARM_COMMAND:       display.print(F("Test Alarm      ")); break;
      case RESET_SETTINGS_COMMAND:   display.print(F("Reset Settings  ")); break;
      case EXIT_COMMAND:             display.print(F("Exit            ")); break;
    }
    display.setTextColor(WHITE, BLACK);
    if (j == option && in_edit_state) {
      display.setTextColor(BLACK, WHITE);
    }
    display.print(F(" "));
    if (j == FLIP_SCREEN_OPTION || j == LIPO_2S_METER_OPTION || j == LIPO_3S_METER_OPTION || j == SHOW_STARTSCREEN_OPTION || j == SAVE_SCREEN_OPTION || j == BATTERY_ALARM_OPTION || j == LOW_BAND_OPTION) {
      if (options[j])
        display.print(F("ON "));
      else
        display.print(F("OFF"));
    }
    else if ( j == ALARM_LEVEL_OPTION )
    {
      display.print(options[j]);
    }
    else if ( j == BATTERY_CALIB_OPTION )
    {
      display.print(getVoltage() / 10);
      display.print(F("."));
      display.print(getVoltage() % 10);
    }
    else
      display.print("    ");

    display.println();
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

