# U.S. Coast Guard Countdown Timer

by [Bob Smith](https://github.com/bethanysciences/almanac)

Rack mounted basic countdown timer to set quickly set and manager interval between patrol check-ins, usually 30-minutes or 15-minutes in bad weather or air operations. When complete loud, a loud customized note and duration alarm alerts to next call.

## Features

- Set and start 15 or 30-minute countdown using single buttons
- Dial in 1 to 99-minutes using rotary encoder
- Outputs minutes and seconds to 4-digit 7-segment LED display

## Components Used

- [Arduino Nano Every](https://store-usa.arduino.cc/products/arduino-nano-every)
- [Adafruit I2C Rotary Encoder Breakout](https://www.adafruit.com/product/4991)
- [Adafruit Rotary Encoder](https://www.adafruit.com/product/377)
- [TM1637 based 0.56" high 7-segment LED display](https://www.amazon.com/diymore-Display-Digital-Decimal-Segment/dp/B07MCGDST2/ref=sr_1_6?crid=2KB0SJVDRC588&keywords=tm1637+led+display&qid=1687116410&sprefix=tm1637+led+display%2Caps%2C203&sr=8-6)
- [Passive Buzzer Module](https://www.amazon.com/RLECS-Passive-Arduino-Raspberry-Speaker/dp/B07XDPXH7K/ref=asc_df_B07XDPXH7K/?tag=hyprod-20&linkCode=df0&hvadid=632016782313&hvpos=&hvnetw=g&hvrand=3559400587378035192&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=9012147&hvtargid=pla-1944578464961&psc=1)
- 2 momentary pushbutton switches

## Compiler

- [Using Arduino 2.1.0 IDE](https://github.com/arduino/arduino-ide)
- [Arduino platform-specification](https://arduino.github.io/arduino-cli/latest/platform-specification/)

## Core

- [Arduino megaAVR Boards 1.8.8](https://github.com/arduino/ArduinoCore-megaavr)

## Libraries

- [CountDown.h](//https://github.com/RobTillaart/CountDown)
- [Arduino Discussion](//https://playground.arduino.cc/Main/CountDownTimer/)
- [debounce.h](//https://github.com/kimballa/button-debounce)
- [TM1637.h](//https://github.com/RobTillaart/TM1637)
