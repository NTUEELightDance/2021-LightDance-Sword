# 2021 LIGHTDANCE SWORD
LED sword controller based on ESP32. Communicate with the server and bright the LED lights of the sword according to timeline.

## main.ino
Get commands, call LedManager, response to the server.

## LedManager.h
LED setting:
* pin 27: 60 leds. The All leds are on the knife(刀身)
* pin 26: 26 leds. The first 18 are on the guard(刀鐔) and the others are on the handle(刀柄).
* pin 25: 8 leds. All on the handle.

## sword pattern
Use swordLedGen.js to generate png sword files. 

## Usage
After designing a pattern, add the led pattern in LedManager variable "pic_data_test". Use the swordLedGen.js to generate png images, and add the images to 編舞程式.

