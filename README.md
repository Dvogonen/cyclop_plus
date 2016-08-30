#CYCLOP+ v1.5

###Introduction and Functions
The major function of this project (CYCLOP+) is to add the ability to manually select channels using only the standard button on the receiver of the Quanum Cyclops.
The second major function is to add support for an external OLED display that information can be presented to the user.
No hardware alteration is necessary to use CYCLOP+. The OLED display modification is optional. The buzzer modification is also optional.

###Version History
* 1.0 Initial dev version, not released
* 1.1 Functionly complete dev version, not released
* 1.2 Timing optimizations. First released version. 2016-06-20
* 1.3 Configration options added. Screensaver mode added. Battery meter added. 2016-07-15
* 1.4 SH1106 OLED support added. Button timing improved. Low battery alarm added. 2016-08-20
* 1.5 One click wakeup. Not Yet Released


###Informative Links
This is a short video introduction to the functionality (of v1.2):
https://youtu.be/mDhL1hS-EHk

This is a longer video (for v1.2) in which a modified Cyclops visor is demonstrated and programmed in two different ways:
https://youtu.be/C4XgYwpNXS0

This is an installation description written by RadianceNL. It is much more detailed than the description below.
https://radiance-fpv.nl/cyclop-plus-firmware-mod/

#Installation
###Solder programming pins to the PCB(optional)
- Solder a 2x3 block of pin headers into the holes for the ICSP port.
If you do not want to solder anything you can program the board by temporarily pushing programming pins into the PCB ICSP connector holes while you program the board.
The holes for the ICSP connector is found just to the right of the button switch.
The pins should point upwards on the same side as the receiver tin can is installed.
![ICSP pin header](/images/pcb_icsp.jpg)
The top left pin is VCC. 
 
###Attach an OLED display(optional)
- Solder a SSD1306 or SH1106 OLED 128x64 display with I2C interface to the I2C pins (Ground, VCC, SCL, SDA).

![128x64 OLED with I2C interface](/images/oled.jpg)

These little display modules where popularized by Adafruit. They are very common today and clones from various producers are sold on eBay for around 5$.
Do not buy one of the two-colored ones. 
You want a black and white display without any missing lines (the two-colored displays have slight space between the two color fields).
Please note that you want a display with 4 pins. These use the I2C interface. You can not use the 6 pin variants.

There are two different OLED screen types available that look the same.
One type has a SSD1306 controller built in, while the other uses a SH1106 controller. 
These are not compatible and need different software.
Chinese sellers on eBay often copy and paste text from other sellers and basically do not know what they sell.
A lot of SH1106 OLED screens have been sold as SSD1306 screens lately.
CYCLOP+ v1.4 is available in versions for both SSD1306 and SH1106.
You do have to select the right CYCLOP+ binary for your display to work, but since loading the wrong binary will not damage anything, this is nothing to stress out about.
If your OLED screen mostly displays white, whith a thin band of garbled graphics at the top, you have loaded the software for a SSD1306, but the display is actually a SH1106.

There are several pads available on the board, so you do not have to solder to the processor legs.
The board designer was kind enough to add three pads just to the left of display contact that are (from the left) Ground, SDA and SCL.
![ICSP pin header](/images/pcb_5v_display.jpg)
The OLED screens can use either 3.3 or 5 volts for VCC (even if the spec says 3.3-4.2 volts).
I recommend using 3.3 volts.
There is an easily accessible 3.3 volt pad next to the receiver module can.
Some OLED displays do not work on 3.3 volts.
If this is the case for your OLED, you can try to adjust the voltage regulator to deliver 5 volts rather than 6 and use the output from the regulator to drive your OLED instead.
The easiest point to find the regulator output voltage is on the rightmost pin of the main display contact.
A side effect of adjusting the regulator is that the main display is running on a lower voltage than intended. This seems to work for most people, but in some cases the main screen will not start up.
If you are among the unlucky ones who have gotten an OLED that refuses to work on 3.3 volts and a main display that only works above 6 volts, you have to get inventive.
Either buy another OLED or install a separate voltage regulator for the OLED.
![ICSP pin header](/images/pcb_33v_display.jpg)
Please note that if you instead want to connect to solder pads on the back of the PCB, there are two labeled SCL1 and SDA1. These are mislabeled. They should be swapped. 

##Attach a low battery alarm buzzer(optional)
Use a 5 volt piezzo buzzer. It does not matter if it is an active piezo buzzer or a passive piezo speaker.
A piezo buzzer looks like this this:
![Buzzer Example](/images/buzzer.jpg)

- Solder the red wire comming from your buzzer to the solder point marked D6. If your buzzer does not have a red wire instead solder the leg marked with a + to the D6 solder pad.
- Solder the other (black) wire to a ground point. The legs of the antenna contact as well as the square through hole solder island in the middle of the PCB are ground points.
- Enable the Low Battery Alarm option in the configuration menu.
![Alarm Speaker Connection](/images/pcb_buzzer.jpg)

###Build CYCLOP+ (optional)
- The project is built using the Arduino development environment. Download the Arduino development environment from www.arduino.cc.
- Install the development environment.
- Download the CYCLOP+ source code from GitHub.
- Navigate to the cyclop_plus.ino file and open it in the Arduino development environment.
- Download the two external LCD libraries (Adafruit GFX and Adafruit SSD1306). This is done within the Arduino environment.
- Select display type by editing Adafruit_SSD1306.h. Look at line 42 in the file.
- Specify "Arduino Pro or Pro Mini" as board. Then select "Atmega 328 (3.3 volt, 8 MHz)" as processor. These settings are found in the "Tool" menu.
- Build the project by pressing the v icon in the upper left corner of the Arduino window.

###Load CYCLOP+
- Build CYCLOP+ or download the latest stable version of CYCLOP+.
The SSD1306 version firmware file is called cyclop_plus.hex and can be downloaded via this link: https://raw.githubusercontent.com/Dvogonen/cyclop_plus/master/cyclop_plus.hex (right-click and download)

The SH1106 version firmware file is called cyclop_plus_sh1106.hex and can be downloaded via this link: https://raw.githubusercontent.com/Dvogonen/cyclop_plus/master/cyclop_plus_sh1106.hex (right-click and download)

If you do not install an OLED, you can use either binary.
Check the format of the downloaded file. Each line should start with a colon character and only contain letters and numbers like this:

:100000000C941F030C9447030C9447030C94470370

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
Select your CYCLOP+ firmware file in the Flash box.
Select "Intel Hex" as format in the Flash box.
Execute a write.
- If everything is set up correctly the LED on the receiver board next to the ISP pin header will light up for a minute or so.
When it goes black again the programming is done and the board can be mounted in the googles.

###Configure CYCLOP+
- Hold down the button during power up to enter the system options screen.
- Use single click and double click to navigate in the menu.
- Use long click to select or unselect an option.
- Examples of configurable options: Screen flip (up or down), 3s battery meter, 2s battery meter, screen saver, low level battery alarm.
- Enabling the screen saver option makes the display go out 10 seconds after the last button press. Use this if the display is mounted inside the visor.
- The settings are saved when the Exit option is selected. All changes are lost if the battery is disconnected before Exit has been selected.

###Use CYCLOP+
- A single click jumps up in frequency to the closest channel among the 40 available.
- A double click jumps down in frequency
- A long click (0.6 - 2 seconds) triggers a autoscan for the best channel, just like a single click does in the original firmware.
- A long-long click (> 2 seconds) triggers a manual frequency scanner. The receiver will start cycling through all channels quickly. Hold down the button again when the channel you want to use flickers onto the main display.

###Words of Warning
Use of the FW is completely on your own risk.
You have to dismantle the googles to program the receiver with a so called ISP (alternatively called ICSP) programmer.
It is naturally possible to destroy the receiver electronics if you connect VCC (the +3.3 volt wire coming from the programmer) to the wrong pin.
Other types of wiring errors will lead to the programmer not working, but will probably not destroy anything.
Always double check that VCC is connected correctly before you turn on power.

It is possible to "Brick" the processor in your receiver by tampering with the so called processor fuses. There is no need to change any fuses from their original values (0xE2 0xD9 0x07). Leave them alone.

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
