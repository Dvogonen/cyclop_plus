/*******************************************************************************
  This is a minimal library for communication with the RTC6715 circuit over SPI.
  The SPI interface is bit banged on three pins.

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
#include "Arduino.h"
#include "rtc6715.h"

//******************************************************************************
//* function: rtc7615 constructor
//******************************************************************************
rtc6715::rtc6715(unsigned int clock_pin, unsigned int slave_select_pin, unsigned int data_pin)
{
  spi_clock_pin = clock_pin;
  spi_slave_select_pin = slave_select_pin;
  spi_data_pin = data_pin;

  // SPI pins for RX control
  pinMode (spi_slave_select_pin, OUTPUT);
  pinMode (spi_data_pin, OUTPUT);
  pinMode (spi_clock_pin, OUTPUT);
}

//******************************************************************************
//* function: calcFrequencyData
//*         : calculates the frequency value for the syntheziser register B of
//*         : the RTC6751 circuit that is used within the RX5808/RX5880 modules.
//*         : this value is inteded to be loaded to register at adress 1 via SPI
//*         :
//*  Formula: frequency = ( N*32 + A )*2 + 479
//******************************************************************************
unsigned int rtc6715::calcFrequencyData( unsigned int frequency )
{
  unsigned int N;
  unsigned char A;
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
void rtc6715::setFrequency(unsigned int frequency)
{
  unsigned int sRegB;
  unsigned char i;

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

  digitalWrite(spi_slave_select_pin, LOW);
  digitalWrite(spi_clock_pin, LOW);
  digitalWrite(spi_data_pin, LOW);
}


//******************************************************************************
//* function: spi_1
//******************************************************************************
void rtc6715::spi_1( void )
{
  digitalWrite(spi_clock_pin, LOW);
  delayMicroseconds(1);
  digitalWrite(spi_data_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(spi_clock_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(spi_clock_pin, LOW);
  delayMicroseconds(1);
}

//******************************************************************************
//* function: spi_0
//******************************************************************************
void rtc6715::spi_0( void )
{
  digitalWrite(spi_clock_pin, LOW);
  delayMicroseconds(1);
  digitalWrite(spi_data_pin, LOW);
  delayMicroseconds(1);
  digitalWrite(spi_clock_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(spi_clock_pin, LOW);
  delayMicroseconds(1);
}

//******************************************************************************
//* function: spiEnableLow
//******************************************************************************
void rtc6715::spiEnableLow( void )
{
  delayMicroseconds(1);
  digitalWrite(spi_slave_select_pin, LOW);
  delayMicroseconds(1);
}

//******************************************************************************
//* function: spiEnableHigh
//******************************************************************************
void rtc6715::spiEnableHigh( void )
{
  delayMicroseconds(1);
  digitalWrite(spi_slave_select_pin, HIGH);
  delayMicroseconds(1);
}

//******************************************************************************
//* Functions below are inteded for debuging and have not been tested
//******************************************************************************

//******************************************************************************
//* function: readRegister  - NOT TESTED!
//*         : Returns the contents of a given register as a long.
//*         : The 20 LSB of the long is the register content. The rest is zero
//*         : padding.
//*
//* SPI data: 4  bits  Register Address  LSB first
//*         : 1  bit   Read or Write     0=Read 1=Write
//*         : 20 bits  Register content
//******************************************************************************
long rtc6715::readRegister( unsigned char reg )
{
  long retVal = 0;
  unsigned char i;

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
  for (i = 20; i; i--, retVal = (retVal << 1 )) {
    if ( spiRead())
      retVal = retVal & 0x01;
  }
  // Disable SPI
  spiEnableHigh();

  return retVal;
}


//******************************************************************************
//* function: spiRead  - NOT TESTED!
//******************************************************************************
int rtc6715::spiRead( void )
{
  int retVal;
  digitalWrite(spi_clock_pin, LOW);
  delayMicroseconds(1);
  digitalWrite(spi_clock_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(spi_clock_pin, LOW);
  retVal = digitalRead(spi_data_pin);
  delayMicroseconds(1);
  return retVal;
}
