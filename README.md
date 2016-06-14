#Introduction - v1.1
The first goal of this project (CYCLOP+) is to add the ability to manually select channels using the single button on the receiver of the Quanum Cyclops.
The second goal is to add a display to the receiver so that channel and frequency can be displayed.
No hardware alteration is needed to run the firmware. Even if it is easier to program the receiver if you solder a standard ICSP pin header in place, you may be able to program by just temporarily pushing pins into the holes and then remove them. Neither is it necessary to attach a OLED screen. The screen is however a huge bonus and is recommended.

##Words of Warning
The project is in an early stage and there may be bugs that causes the receiver to fail without warning. Use of the FW is completely on your own risk.
To install the firmware, you have to remove the receiver PCB from the googles and program it using a ICSP programmer. It is possible to destroy the electronics wiring up the programmer incorrectly.
It is most definitely possible to "Brick" your receiver module by tampering with the processor fuses. There is no need to change any fuses from their original values (0xE2 0xD9 0x07).

##Building CYCLOP+
- The project is built using the Arduino development environment, so download the Arduino development environment from www.arduino.cc.
- Install the development environment.
- Download the CYCLOP+ source code from GitHub.
- Navigate to the cyclop_plus.ino file and start it.
- Download the two external LCD libraries (Adafruit GFX and Adafruit SSD1306).
- Build the project. Use "Arduino Pro or Pro Mini" as board. Then select "Atmega 328 (3.3 volt, 8 MHz)" as processor.

## Hardware modification
- (Optional)Solder a 2x3 block of pin headers into the ICSP port just under the button switch. The pins should point upwards on the same side as the receiver tin can is installed.
- (Optional)Connect a 128x64 OLED screen with I2C interface to Ground, VCC, SCL, SDA. The board designer was kind enough to add three display contact pads clearly marked "display 1". The pads are (from the top) Ground, SCL and SDA. They are found above the contact that goes off to the screen. The OLED screens can use either 3.3 or 5 volts. I used 5 volts. The easiest point to find this is on the lowest pin on the main display contact. Please note that the pads on the back of the board marked SCL1 and SDA1 are mislabeled. They should be swapped. 

##Loading the FW
- Upload the sketch to the receiver board using a ICSP programmer. There are several to choose from and you have to find out for yourself how to do this.
- Please note that you have to use the "Load with Programmer" menu option on the "Sketch" menu. Pressing the upload arrow does _not_ work.
- The receiver board has both 3.3 and 5-volt power rails. The processor and receiver is however 3.3v and the VCC feeds are connected so you should definitely use a 3.3v programmer. You might be able to get away with using a 5-volt programmer, but you may also fry the receiver, which would be bad.
- The ICSP header of the board conforms to the standard 6 pin layout used for all Arduinos and most other Atmel Atmega hardware. Turn the board so that the tin can of the receiver is on the top side. The ICSP header is to the top right under the button switch. VCC is on the rightmost, upmost pin and MISO is on the leftmost, upmost pin.

##Using the FW
- A single click jumps upwards in frequency to the closest channel among the 40 available.
- A double click jumps downward in frequency
- A long click (0.6 - 1.5 seconds) triggers a autoscan for the best channel, just like a single click does on the original firmware.
- A long-long click (>1.5 seconds) also triggers an autoscan, but this wmay be changed to something else later on.

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
