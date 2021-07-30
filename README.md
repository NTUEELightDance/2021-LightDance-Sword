# 2021 LIGHTDANCE SWORD
LED sword controller based on ESP32. Communicate with the server and bright the LED lights of the sword according to timeline.

## main.ino
Get commands, call LedManager, response to the server.

## LedManager.h
LED setting:
* Knife: 60 leds. Controlled by pin 27.
* Guard: 18+8 leds.  The first 18 are controlled by pin 26 and the others are controlled by pin 25.
* Handle: 8 leds. Controlled by pin 25.

## sword pattern
Use swordLedGen.js to generate png sword files. 

## Usage
After designing a pattern, add the led pattern in LedManager variable "pic_data_test". Use the swordLedGen.js to generate png images, and add the images to 編舞程式.

