/*******************************************************************************
  This is the header file for a a minimal library for communication with the
  RTC6715 circuit over SPI. The SPI interface is bit banged on three pins.

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
#ifndef rtc6715_h
#define rtc6715_h

class rtc6715
{
  public:
    rtc6715( unsigned int spi_clock_pin, unsigned int spi_slave_select_pin, unsigned int spi_data_pin );
    long readRegister( unsigned char reg );
    void setFrequency(unsigned int frequency);

  private:
    unsigned int calcFrequencyData( unsigned int frequency );
    void     spi_0(void);
    void     spi_1(void);
    void     spiEnableHigh( void );
    void     spiEnableLow( void );
    int      spiRead( void );

    unsigned int spi_clock_pin = 0;
    unsigned int spi_slave_select_pin = 0;
    unsigned int spi_data_pin = 0;
};

#endif // rtc6715_h
