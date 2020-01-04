# Nixie clock firmware
### Firmware for IN12/IN14 nixie clock ATmega8 based design

For other PCB design checkout current branch:

| Design name   | GIT branch    |
| ------------- |:-------------:|
| IN-12 (THT)   | IN12THT       |
| IN-12 (SMD)   | IN12SMD       |
| IN-14 (SMD)   | IN14SMD       |

Compiling project
------

#### Windows

Download latest Atmel Studio IDE [Atmel Studio 7](https://www.microchip.com/mplab/avr-support/atmel-studio-7)

#### GNU/Linux (tested on Debian)

install dependencies:

```
make install
```

compile project:

```
make
```

flash ATmega using USBasp programmer:

```
make flash
```


PCB project
------

[PCB IN-12 Nixie clock](https://circuitmaker.com/Projects/Details/Jakub-Dorda/IN-12-Nixie-clock)

Output gerber files are located in 'PCBgerber' folder.

Compiled firmware
------

Download latest compiled firmware from GitHub repository release tab:

[Latest release](https://github.com/jakdor/NixieClock/releases)

Documentation/manuals
------

Documentation is located in 'Docs' folder or under this link:

[ASSEMBLY GUIDE/USER MANUAL/PARTS LIST](https://goo.gl/EqvBFE)

FEATURES
------
* Full RGB backlight!
* Compact design
* HH:MM:SS format
* 24/12 h display format
* Alarm clock
* Backup power and settings memory
* Maintains time after power loss
* Backup battery lifetime up to 20 years
* accurate Real Time Clock
* Roulette Effect every hour (prevents cathode poisoning)
* Simple 3-buttons interface
* manual nixie brightness
* 3 separator modes (on, off, blinking)
* 6 RGB backlight modes including user defined and off
* night mode (turning off nixies and backlight between user set hours)

------

YouTube videos:

[https://www.youtube.com/watch?v=hwEY1RJ39X0](https://www.youtube.com/watch?v=hwEY1RJ39X0)

[https://www.youtube.com/watch?v=vacfdvKtQjE](https://www.youtube.com/watch?v=hwEY1RJ39X0)

![NixieClock](https://i.imgur.com/jiBY5re.jpg)
