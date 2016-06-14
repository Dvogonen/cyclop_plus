/*******************************************************************************
  CYCLOP+ brings OLED support and manual channel selection to the
  HobbyKing Quanum Cyclops FPV googles.

  The rx5808-pro and rx5808-pro-diversity projects served as a starting
  point for the code base, even if little of the actual code remains.
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

// Library includes
#include <avr/pgmspace.h>
#include <string.h>
#include <EEPROM.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// Application includes
#include "cyclop_plus.h"

//******************************************************************************
//* File scope function declarations

void saveEeprom(uint8_t channel, uint16_t rssiMin, uint16_t rssi_max);
bool readEeprom(uint8_t *channel, uint16_t *rssiMin, uint16_t *rssiMax);
void setRTC6715Frequency(uint16_t frequency);
void setReceiver(uint8_t receiver);
uint16_t readRssi();
uint16_t calcFrequencyData( uint16_t frequency );
char *shortNameOfChannel(uint8_t channel, char *name);
char *longNameOfChannel(uint8_t channel, char *name);
void disolveDisplay(void);
void flipDisplay();
void drawStartScreen(void);
void drawChannelScreen( uint8_t channel, uint16_t rssi);
void drawDialog(char *text);
uint8_t nextChannel( uint8_t channel);
uint8_t previousChannel( uint8_t channel);
uint8_t getClickType(uint8_t buttonPin);
uint16_t autoScan( uint16_t frequency );
uint8_t bestChannelMatch( uint16_t frequency );
void drawScannerScreen( void );
void updateScannerScreen(uint8_t position, uint8_t value );
uint16_t graphicScanner( uint16_t frequency );
#ifdef DEBUG
void drawDebugScreen(char *t1, uint16_t i1, char *t2,  uint16_t i2, char *t3,  uint16_t i3 );
#endif

//******************************************************************************
//* Positions in the table above for the 40 channels
//* Direct access via array operations does not work since data is stored in
//* flash, not in RAM. Use getPosition to retrieve data

const uint8_t positions[] PROGMEM = {
  19, 18, 32, 17, 33, 16,  7, 34,  8, 24,  6,  9, 25,  5, 35, 10, 26,  4, 11, 27,
  3, 36, 12, 28,  2, 13, 29, 37,  1, 14, 30,  0, 15, 31, 38, 20, 21, 39, 22, 23
};

uint16_t getPosition( uint8_t channel ) {
  return pgm_read_byte_near(positions + channel);
}

//******************************************************************************
//* Frequencies for the 40 channels
//* Direct access via array operations does not work since data is stored in
//* flash, not in RAM. Use getFrequency to retrieve data

const uint16_t channelFrequencies[] PROGMEM = {
  5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // Band A - Boscam A
  5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // Band B - Boscam B
  5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // Band E - DJI
  5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // Band F - FatShark \ Immersion
  5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917  // Band C - Raceband
};

uint16_t getFrequency( uint8_t channel ) {
  return pgm_read_word_near(channelFrequencies + getPosition(channel));
}

//******************************************************************************
//* Other file scope variables
Adafruit_SSD1306 display(4);
uint8_t currentChannel = 0;
uint8_t lastChannel = 0;
uint16_t currentRssi = 0;
uint8_t ledState = LED_ON;
unsigned long displayUpdateTimer = 0;
unsigned long eepromSaveTimer = 0;
unsigned long pulseTimer = 0;

//******************************************************************************
//* function: setup
//******************************************************************************
void setup()
{
  // initialize led pinb
  pinMode(LED_PIN, OUTPUT); // status pin
  digitalWrite(LED_PIN, LED_ON);

  // initialize button pin
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, INPUT_PULLUP);

  // SPI pins for RX control
  pinMode (SLAVE_SELECT_PIN, OUTPUT);
  pinMode (SPI_DATA_PIN, OUTPUT);
  pinMode (SPI_CLOCK_PIN, OUTPUT);

  // Initialize channel
  readEeprom( &currentChannel );
  if (currentChannel > CHANNEL_MAX)
    currentChannel = CHANNEL_MIN;

  // Start receiver
  setRTC6715Frequency(getFrequency(currentChannel));

  // Initialize and flip the display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADR);
  // display.setRotation(2);

  // Show start screen
  drawStartScreen();
  return;
}

//******************************************************************************
//* function: loop
//******************************************************************************
void loop()
{
  switch (getClickType( BUTTON_PIN ))
  {
    case NO_CLICK: // do nothing
      break;

    case LONG_LONG_CLICK: // graphical band scanner
      currentChannel = bestChannelMatch(graphicScanner(getFrequency(currentChannel)));
      drawChannelScreen(currentChannel, 0);
      displayUpdateTimer = millis() +  RSSI_STABILITY_DELAY_MS ;
      break;

    case LONG_CLICK:      // auto search
      drawDialog( "SCANNING");
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
  // Check if the display needs updating
  if ( millis() > displayUpdateTimer ) {
    currentRssi = readRssi();
    drawChannelScreen(currentChannel, currentRssi);
    displayUpdateTimer = millis() + 1000;
  }

  // Check if EEPROM needs a save. Reduce EEPROM writes by not saving to often
  if ((currentChannel != lastChannel) && (millis() > eepromSaveTimer))
  {
    saveEeprom( currentChannel );
    lastChannel = currentChannel;
    eepromSaveTimer = millis() + 10000;
  }

  // Check if it is time to switch LED state for the pulse led
  if ( millis() > pulseTimer ) {
    ledState = !ledState;
    pulseTimer = millis() + 500;
  }
  digitalWrite(LED_PIN, ledState);
}

//******************************************************************************
//* function: saveEeprom
//******************************************************************************
void saveEeprom(uint8_t channel) {
  EEPROM.write(EEPROM_CHANNEL, channel);
}

//******************************************************************************
//* function: readEeprom
//******************************************************************************
bool readEeprom(uint8_t *channel) {
  *channel =   EEPROM.read(EEPROM_CHANNEL);
}

//******************************************************************************
//* function: get_click_type
//*         : Polls the specified pin and returns the type of click that was
//*         : performed NO_CLICK, SINGLE_CLICK, DOUBLE_CLICK, LONG_CLICK
//*         : or LONG_LONG_CLICK
//******************************************************************************
uint8_t getClickType(uint8_t buttonPin) {
  uint16_t timer = 0;
  uint8_t click_type = NO_CLICK;

  // check if the key has been pressed
  if (digitalRead(buttonPin) == !BUTTON_PRESSED) {
    return ( NO_CLICK );
  }
  // Debounce to make sure it was a real key press
  delay(DEBOUNCE_MS);
  if (digitalRead(buttonPin) == !BUTTON_PRESSED) {
    return ( NO_CLICK );
  }
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

  // Check if there is a second click
  timer = 0;
  while ((digitalRead(buttonPin) == !BUTTON_PRESSED) && (timer++ < 40)) {
    delay(5);
  }
  if (timer == 40)                  // 40 * 5 ms = 0.2s
    return click_type;

  // Debounce to make sure it was a real key press
  delay( DEBOUNCE_MS);
  if (digitalRead(buttonPin) == BUTTON_PRESSED )
    click_type = DOUBLE_CLICK;

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
//*         : scans the 5.8 GHz band in 3 MHz increments and draws a graphical
//*         : representation. when the button is pressed the currently
//*         : scanned frequency is returned.
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
    scanFrequency += 3;
    if (scanFrequency > FREQUENCY_MAX)
      scanFrequency = FREQUENCY_MIN;
    setRTC6715Frequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = readRssi();
    rssiDisplayValue = (scanRssi - 140) / 10;    // Roughly 2 - 46
    updateScannerScreen(100 - ((FREQUENCY_MAX - scanFrequency) / 3), rssiDisplayValue );
  }
  // Fine tuning
  scanFrequency = scanFrequency - 20;
  for (i = 0; i < 20; i++, scanFrequency += 2) {
    setRTC6715Frequency(scanFrequency);
    delay( RSSI_STABILITY_DELAY_MS );
    scanRssi = readRssi();
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
  scanFrequency = frequency + 10;
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
    scanRssi = readRssi();
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
    scanRssi = readRssi();
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
//* function: readRssi
//*         : returns an averaged rssi value between (in theory) 0 and 1024
//*         : this function is called often, so it is speed optimized
//******************************************************************************
uint16_t readRssi()
{
  uint16_t rssi = 0;
  uint8_t i = 32;

  for ( ; i ; i--) {
    rssi += analogRead(RSSI_PIN);
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
  else
    name[0] = 'C';
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
  else
    strcpy(name, "Raceband ");
  len = strlen( name );
  name[len] = (channelIndex % 8) + '0' + 1;
  name[len + 1] = 0;
  return name;
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
//* SPI data: 4  bits  Register Address  LSB first
//*         : 1  bit   Read or Write     0=Read 1=Write
//*         : 13 bits  N-Register Data   LSB first
//*         : 7  bits  A-Register        LSB first
//******************************************************************************
void setRTC6715Frequency(uint16_t frequency)
{
  uint16_t sRegB;
  uint8_t i;

  sRegB = calcFrequencyData(frequency);

  // Bit bang the syntheziser register

  // Clock
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

  // Clock
  spiEnableHigh();
  delayMicroseconds(1);

  digitalWrite(SLAVE_SELECT_PIN, LOW);
  digitalWrite(SPI_CLOCK_PIN, LOW);
  digitalWrite(SPI_DATA_PIN, LOW);
}

//******************************************************************************
//* function: spi_1
//******************************************************************************
void spi_1()
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
void spi_0()
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
//* function: spiEnableLow
//******************************************************************************
void spiEnableLow()
{
  delayMicroseconds(1);
  digitalWrite(SLAVE_SELECT_PIN, LOW);
  delayMicroseconds(1);
}

//******************************************************************************
//* function: spiEnableHigh
//******************************************************************************
void spiEnableHigh()
{
  delayMicroseconds(1);
  digitalWrite(SLAVE_SELECT_PIN, HIGH);
  delayMicroseconds(1);
}

//******************************************************************************
//* Screen functions
//******************************************************************************
//******************************************************************************
//* function: disolveDisplay
//*         : fancy graphics stuff that disolves the screen into black
//*         : unnecessary, but fun
//******************************************************************************
void disolveDisplay(void)
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
  display.print("CYCLOP+");
  display.drawLine(0, 27, 127, 27, WHITE);
  display.setCursor(15, 35);
  display.setTextSize(1);
  display.print(VER_INFO_STRING);
  display.setCursor(33, 50);
  display.print(VER_DATE_STRING);
  display.display();

  // Return after 2000 ms or when button is pressed
  for (i = 200; i; i--)
  {
    if (digitalRead(BUTTON_PIN) == BUTTON_PRESSED )
      return;
    delay(10);
  }
  disolveDisplay();
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
  display.setCursor(0, 0);
  display.setTextSize(3);
  display.print(getFrequency(channel));
  display.setCursor(64, 7);
  display.setTextSize(2);
  display.print(" MHz");
  display.drawLine(0, 24, 127, 24, WHITE);
  display.setCursor(0, 27);
  display.setTextSize(1);
  display.print(" Channel     RSSI");
  display.setCursor(0, 39);
  display.setTextSize(2);
  display.print(" ");
  display.print(" ");
  display.print(shortNameOfChannel(channel, buffer));
  display.print("  ");
  display.print(rssi);
  display.setCursor(0, 57);
  display.setTextSize(1);
  longNameOfChannel(channel, buffer);
  i = (21 - strlen(buffer)) / 2;
  for (; i; i--) {
    display.print(" ");
  }
  display.print( buffer );
  display.display();
}

//******************************************************************************
//* function: drawDialog
//*         : draw a floating dialog. 9 character maximum!!!
//******************************************************************************
void drawDialog( char *text ) {
  uint8_t width = 5 + 2 + (strlen(text) * 12) + 5;
  uint8_t height = 5 + 2 + 14 + 2 + 5;
  display.fillRect(63 - width / 2, 31 - height / 2, width, height, BLACK);
  display.fillRect(63 - width / 2 + 3 , 31 - height / 2 + 3, width - 6, height - 6, WHITE);
  display.fillRect(63 - width / 2 + 5,  31 - height / 2 + 5, width - 10, height - 10, BLACK);
  display.setCursor(63 - width / 2 + 7, 31 - height / 2 + 7);
  display.setTextSize(2);
  display.print(text);
  display.display();
}

//******************************************************************************
//* function: drawScannerScreen
//******************************************************************************
void drawScannerScreen( void ) {
  display.clearDisplay();
  display.drawLine(0, 55, 127, 55, WHITE);
  updateScannerScreen(0, 0);
}

//******************************************************************************
//* function: updateScannerScreen
//*         : position = 0 to 99
//*         : value = 0 to 53
//******************************************************************************
void updateScannerScreen(uint8_t position, uint8_t value ) {
  uint8_t i;
  static uint8_t errase_position = 14;
  static uint8_t errase_value = 53;
  position = position + 14;
  display.drawLine( errase_position, 0, errase_position, errase_value, BLACK );
  display.drawLine( errase_position, errase_value + 1, errase_position, 53, WHITE );
  display.fillRect(0, 56, 128, 8, BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 57);
  display.print("5.65     5.8     5.95");
  for ( i = 0; i < 12; i++) {
    display.drawLine( position, i * 4, position, i * 4 + 1, WHITE );
    display.drawLine( position, i * 4 + 2, position, i * 4 + 3,  BLACK );
  }
  display.drawLine( position    , 55, position    , 63, WHITE );
  display.drawLine( position - 1, 58, position - 1, 63, WHITE );
  display.drawLine( position + 1, 58, position + 1, 63, WHITE );
  display.drawLine( position - 2, 60, position - 2, 63, WHITE );
  display.drawLine( position + 2, 60, position + 2, 63, WHITE );
  display.drawLine( position - 3, 62, position - 3, 63, WHITE );
  display.drawLine( position + 3, 62, position + 3, 63, WHITE );

  errase_position = position;
  if (value > 53)
    value = 53;
  errase_value = 53 - value;
  display.display();
}

//******************************************************************************
//* function: drawDebugScreen
//******************************************************************************
#ifdef DEBUG
void drawDebugScreen(char *t1, uint16_t i1, char *t2,  uint16_t i2, char *t3,  uint16_t i3 )
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);

  if ( strlen(t1)) {
    display.print(t1);
    display.print(": ");
    display.print(i1);
    display.println();
  }

  if ( strlen(t2)) {
    display.print(t2);
    display.print(": ");
    display.print(i2);
    display.println();
  }

  if ( strlen(t3)) {
    display.print(t3);
    display.print(": ");
    display.print(i3);
    display.println();
  }

  display.display();

  while (digitalRead(BUTTON_PIN) != BUTTON_PRESSED);
  while (digitalRead(BUTTON_PIN) == BUTTON_PRESSED);
}
#endif // DEBUG
