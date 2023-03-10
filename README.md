# Tiny thermometer

## Description: 
A simple temperature and humidity meter displaying values on an 8x2 LCD using a DHT-22 sensor with a 5V external power supply
on a small Avr microcontroller Attiny 2313 with 2Kb program memory. The firmware is written in C language in Atmel Studio 7

Specifications:
- Power supply: 5V 100mA via Mini Usb;
- Temperature measure: 0 - 80 C real;
- Temperature measure: 0 - (-40) C int;
- Humidity measure: 0 - 100 % int;

Mcu works from 8MHz internal oscillator. But you should use an external crystal oscillator for more stability!
The source code is written in C language in Atmel Studio 7 IDE.
For flashing MCU I used AVR ISP USBasp programmer and desktop app Khazama 
You should set Fuse bits only for changing the oscillator's source (internal 8MHz or external 8MHz)! Leave the remaining bits at default!
If you use an internal generator, remove C3, C4, and Y1 from the circuit
R2 resistor needed to adjust the contrast of LCD
The device soldered on the solder breadboard and put on KRADEX Z70 Housing case

### NOTICE: Construction for home use only!

* The repository contains a project in Atmel Studio 7 with source code
* Simulation project in Proteus 8
* Program Memory Usage 	:	1684 bytes   82,2 % Full
* Data Memory Usage 		:	72 bytes   56,3 % Full

![](https://habrastorage.org/webt/wm/uf/z5/wmufz5xjcxuew-huxw3ny3s3f6g.jpeg)

Project on Hackaday - https://hackaday.io/project/169856-tiny-thermometer
