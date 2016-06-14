#This is the development branch for v1.2 of CYCLOP+. The latest "stable" release is in the master branch.

#Introduction
The first goal of this project (CYCLOP+) is to add the ability to manually select channels using only the standard button on the receiver of the Quanum Cyclops.
The second goal is to add a display to the receiver so that The current channel and frequency can be displayed.
No hardware alteration should be necessary to run the firmware. If you solder a standard ICSP pin header to the PCB it becomes easy to program the receiver. It is however possible to program by just temporarily pushing programming pins into the PCB holes and then remove them when the programming is done. Neither is it necessary to attach a OLED screen. The screen is however a huge bonus and is recommended.

##Words of Warning
The project is in an early stage and there may be bugs that causes the receiver to fail without warning. Use of the FW is completely on your own risk.
To install the firmware you remove the receiver from the googles and program it using a ICSP programmer. It is possible to destroy the electronics if you wire up the programmer incorrectly.
It is most definitely possible to "Brick" your receiver module by tampering with the processor fuses. There is no need to change any fuses from their original values (0xE2 0xD9 0x07). Leave them alone.

## Hardware modifications
- (Optional)Solder a 2x3 block of pin headers into the ICSP port. This is found just under the button switch. The pins should point upwards on the same side as the receiver tin can is installed. The top right pin is VCC.
- (Optional)Connect a 128x64 OLED screen with I2C interface to the I2C pins (Ground, VCC, SCL, SDA). There are several pads available on the board, so you do not have to solder to the processor legs. The board designer was kind enough to add three pads just above the display contact that are (from the top) Ground, SCL and SDA. The OLED screens can use either 3.3 or 5 volts for VCC. I used 5 volts. The easiest point to find this is on the lowest pin on the main display contact. My 5v feed was calibrated to 6.15 volts. You might want to adjust this if you use 5 volt. Please note that the pads on the back of the board marked SCL1 and SDA1 are mislabeled. They should be swapped. 

##Building CYCLOP+
- The project is built using the Arduino development environment. Download the Arduino development environment from www.arduino.cc.
- Install the development environment.
- Download the CYCLOP+ source code from GitHub.
- Navigate to the cyclop_plus.ino file and open it in the Arduino development environment.
- Download the two external LCD libraries (Adafruit GFX and Adafruit SSD1306). This is done within the Arduino environment. 
- Specify "Arduino Pro or Pro Mini" as board. Then select "Atmega 328 (3.3 volt, 8 MHz)" as processor. These settings are found in the "Tool" menu.
- Build the project by pressing the v icon in the upper left corner of the Arduino window.

##Loading the FW
- Upload the sketch to the receiver board using an ICSP programmer. There are several to choose from. USBASPs are probably the cheapest alternative. See to it to buy a version that is switchable between 3.3 and 5 volt.
- Please note that you have to use the "Load with Programmer" menu option on the "Sketch" menu in the development environment. Pressing the upload arrow does _not_ work.
- The receiver board has both 3.3-volt and 5-volt power rails. The processor and receiver is however 3.3v and the VCC feeds are connected so you should definitely use a 3.3v programmer. You might be able to get away with using a 5-volt programmer, but you may also fry the receiver, which would be bad.
- The ICSP header of the board conforms to the standard 6 pin layout used for all Arduinos and most other Atmel Atmega hardware. Turn the board so that the tin can of the receiver is on the top side. The ICSP header is to the top right under the button switch. VCC is on the rightmost, upmost pin and MISO is on the leftmost, upmost pin.

##Using the FW
- A single click jumps upwards in frequency to the closest channel among the 40 available.
- A double click jumps downward in frequency
- A long click (0.6 - 1.5 seconds) triggers a autoscan for the best channel, just like a single click does on the original firmware.
- A long-long click (>1.5 seconds) triggers a graphical frequency scanner (not yet implemented).

####License
The MIT License (MIT)

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
