#CYCLOP+ v1.2
###Introduction and Goals
The first goal of this project (CYCLOP+) is to add the ability to manually select channels using only the standard button on the receiver of the Quanum Cyclops.
The second goal is to add support for connecting an OLED display to the receiver so that more information can be presented.
No hardware alteration is necessary to use CYCLOPS+. The OLED display is optional.

###Words of Warning
Use of the FW is completely on your own risk.
You have to dismantle the googles to program the receiver with a so called ISP (alternatively called ICSP) programmer.
It is naturally possible to destroy the receiver electronics if you connect VCC (the +3.3 volt wire coming from the programmer) to the wrong pin.
Other types of wiring errors will lead to the programmer not working, but will probably not destroy anything.
Always double check that VCC is connected correctly before you turn on power.

It is possible to "Brick" the processor in your receiver by tampering with the so called processor fuses. There is no need to change any fuses from their original values (0xE2 0xD9 0x07). Leave them alone.

###Solder programming pins to the PCB(optional)
- Solder a 2x3 block of pin headers into the holes for the ICSP port.
If you do not want to solder anything you can program the board by temporarily pushing programming pins into the PCB ICSP connector holes while you program the board.
The holes for the ICSP connector is found just to the right of the button switch.
The pins should point upwards on the same side as the receiver tin can is installed.
![ICSP pin header](/images/pcb_icsp.jpg)
The top left pin is VCC. 
 
###Attach an OLED display(optional)
- Solder an OLED 128x64 screen with I2C interface to the I2C pins (Ground, VCC, SCL, SDA). 
![128x64 OLED with I2C interface](/images/oled.jpg)
These little display modules where popularized by Adafruit. They are very common today and clones from various producers are sold on eBay for around 5$.
Do not buy one of the two-colored ones. 
You want a black and white display without any missing lines (the two-colored displays have slight space between the two color fields).
Please note that you want a display with 4 pins. These use the I2C interface. You can not use the 6 pin variants.
There are several pads available on the board, so you do not have to solder to the processor legs.
The board designer was kind enough to add three pads just to the left of display contact that are (from the left) Ground, SDA and SCL.
![ICSP pin header](/images/pcb_5v_display.jpg)
The OLED screens can use either 3.3 or 5 volts for VCC. I used 5 volts.
The easiest point to find this is on the rightmost pin of the main display contact.
My 5v feed was calibrated to 6.15 volts. You might want to adjust this if you use 5 volt.
If you would rather use 3.3 volt, there is an easily accessible pad next to the receiver module can.
![ICSP pin header](/images/pcb_33v_display.jpg)
Please note that if you instead want to connect to solder pads on the back of the PCB, there are two labeled SCL1 and SDA1. These are mislabeled. They should be swapped. 

###Build CYCLOP+ (optional)
- The project is built using the Arduino development environment. Download the Arduino development environment from www.arduino.cc.
- Install the development environment.
- Download the CYCLOP+ source code from GitHub.
- Navigate to the cyclop_plus.ino file and open it in the Arduino development environment.
- Download the two external LCD libraries (Adafruit GFX and Adafruit SSD1306). This is done within the Arduino environment. 
- Specify "Arduino Pro or Pro Mini" as board. Then select "Atmega 328 (3.3 volt, 8 MHz)" as processor. These settings are found in the "Tool" menu.
- Build the project by pressing the v icon in the upper left corner of the Arduino window.

###Load CYCLOP+
- Build CYCLOP+ or download the latest stable version of CYCLOP+.
The firmware file is called cyclop_plus.hex and can be downloaded via this link https://github.com/Dvogonen/cyclop_plus/blob/master/cyclop_plus.hex (right-click and download)
- To upload the sketch to the receiver board you must have an ISP programmer for AVR micro controllers.
The original is called AVR ISP mkII and is expensive.
The design is open source and copies can be had for around 20$.
But there are several other cheaper alternatives available to choose from.
An USBASP is the cheapest alternative (2-3$).
If you decide to get a USBASP, make sure to select one that can be switched between 3.3-volt and 5-volt output.
This is done by moving a jumper often labled JP1. If there is no jumper on the USBASP, you should not buy it.
![USBASP](/images/usbasp_and_adapter.jpg)
Almost all USBASPs come with a 10 wire flat cable with two 10 pin connectors.
This means that you will need an adapter between the 10 pole connector and the 6 pin ISCP connector.
These adapters are also available on eBay and cost about as much as the USBASP.
- Install the drivers for the programmer.
These are sometimes difficult to get to work.
Be prepared for a bit of a fight and use google a lot.
Google is your friend.
For the USBASP driver you may have to use a helper program called Zadig to install the driver.
- Set the programmer to 3.3 volts.
- Connect the programmer to the ICSP contact on the receiver board. Neither the display nor the battery should be connected simultaneously.
- The default programming tool from the processor manufacturer AVR is called AVRDUDE (seriously).
This is command line tool and is horrible to use.
There is a GUI version available that is called AVRDUDESS. This program is quite nice, so I recommend it instead of the bare bones original.
You find it here: http://blog.zakkemble.co.uk/avrdudess-a-gui-for-avrdude/
- Start AVRDUDESS.
Select your ISP programmer in the Programmer box.
Select the type of MCU (Atmega 328p).
Select your CYCLOP+ firmware file in the Flash box and execute a write.
- If everything is set up correctly the LED on the receiver board next to the ISP pin header will light up for a minute or so.
When it goes black again the programming is done and the board can be mounted in the googles.

###Use CYCLOP+
- A single click jumps up in frequency to the closest channel among the 40 available.
- A double click jumps down in frequency
- A long click (0.6 - 2 seconds) triggers a autoscan for the best channel, just like a single click does in the original firmware.
- A long-long click (> 2 seconds) triggers a manual frequency scanner. The receiver will start cycling through all channels quickly. Hold down the button again when the channel you want to use flickers onto the main display.

###License
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
