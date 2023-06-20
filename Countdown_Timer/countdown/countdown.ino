/*------------------------------------------------------------------------------------------------------
  Countdown timer for U.S. Coast Guard Communications Center Console. Arduino based application to count
  down from 15 or 30-minutes, standard patrol communications check-in intervals, or set any time between
  1 and 99 minutes. 15 or 30-minute times selected and count down start on a button press.

  By Bob Smith
  https://github.com/bethanysciences/countdown

  Components used (see readme for additional documentation)
  Arduino Nano Every https://content.arduino.cc/assets/Pinout-NANOevery_latest.pdf
    - ATMega48095 Microcontroller 
    - 7-21VDC VIN	
    - 20MHz Clock Speed	
    - ATSAMD11D14A based USB interface
    - GB4943, WEE, RCM, RoHS, CE, FCC, UKCA, REACH Conformities https://docs.arduino.cc/certifications
  Adafruit I2C Rotary Encoder Breakout
  Adafruit Rotary Encoder
  TM1637 based 0.56" high 7-segment LED display
  Passive Buzzer Module
  2 momentary pushbutton switches

  IMPROVEMENTS TO MAKE:
  - reset MCU function (15-min and 30-min press together to 15+seconds?)
  - dim display function
  - vary alarm volume
--------------------------------------------------------------------------------------------------------*/

#include <JC_Button.h>                                  // v2.1.2 https://github.com/JChristensen/JC_Button
#include <Adafruit_seesaw.h>                            // v1.7.0 https://github.com/adafruit/Adafruit_Seesaw
#include "CountDown.h"                                  // v0.3.1 https://github.com/RobTillaart/CountDown
                                                        // https://playground.arduino.cc/Main/CountDownTimer/
#include "TM1637.h"                                     // v0.3.7 https://github.com/RobTillaart/TM1637_RT

int buzzPin = 4;                                        // Passive Buzzer Module fire pin (GREEN Wire)
bool firstThrough = true;                               // track first loop so buzzer doesn't buzz

/*------------------------------------------------------------------------------------------------------*/
/*                    Instantiate JC Button Library and assign buttons                                  */
/*------------------------------------------------------------------------------------------------------*/
const byte PIN_15MIN(15);                               // production (BROWN wire)
const byte PIN_30MIN(14);                               // production (WHITE wire)
// const byte PIN_15MIN(10);                              // proto board
// const byte PIN_30MIN(6);                               // proto board
Button set15Min(PIN_15MIN);                             // instantiate 15-minute button as 'set15Min'
Button set30Min(PIN_30MIN);                             // instantiate 30-minute button as 'set30Min'

/*------------------------------------------------------------------------------------------------------*/
/*             Adafruit Encoder breakout board instantiation (uses separate SAMD09 MCU)                 */
/*------------------------------------------------------------------------------------------------------*/
#define SS_SWITCH     24                                // switch pin on SAM09 encoder breakout board
#define SEESAW_ADDR   0x36                              // i2c address (default - change for multiple)
                                                        // VIN (5VDC) - RED wire
                                                        // GND - BLACK wire
                                                        // Serial Clock (SCL) I2C YELLOW wire
                                                        // Serial Data (SDA) I2C BLUE wire
Adafruit_seesaw ss;                                     // instantiate encoder board / functions as 'ss'
int32_t encoder_position;                               // last encoder position
int32_t new_position;                                   // new position if changed
int switchState;                                        // current input pin reading
int lastSwitchState = LOW;                              // previous input pin reading
unsigned long lastDebounceTime = 0;                     // last time the output pin toggled
unsigned long debounceDelay = 50;                       // debounce encoder switch delay (ms)
int32_t minutes = 0;                                    // encoder position for minutes entry

/*------------------------------------------------------------------------------------------------------*/
/*                Instantiate Rob Tillaart's CountDown Library anb associated variables                 */
/*------------------------------------------------------------------------------------------------------*/
CountDown cd(CountDown::SECONDS);                       // instantiat countdown timer as 'cd'
bool onceThrough = true;                                // switch to finish
bool shutDown = false;                                  // deactivate timer to hold state

/*------------------------------------------------------------------------------------------------------*/
/*               Instantiate Rob Tillaart's 4-digit TM1637 based 7-segment display library              */
/*------------------------------------------------------------------------------------------------------*/
TM1637 tm;                                              // instantiat display as 'tm'
int CLK = 2;                                            // Clock Input - YELLOW wire
int DIO = 3;                                            // Data I/O - BLUE wire
int DIGITS = 4;                                         // this display is 4-digits

/*------------------------------------------------------------------------------------------------------*/
/*                                   Application setup - runs once                                      */
/*------------------------------------------------------------------------------------------------------*/
void setup() {
    pinMode(buzzPin, OUTPUT);                           // fire up buzzer module
    // tone(buzzPin, 2000, 500);                        // indicate startup (2000hz for 500ms)

    set15Min.begin();                                   // fire up 15-minute switch
    set30Min.begin();                                   // fire up 30-minute switch

    ss.begin(SEESAW_ADDR);
    ss.pinMode(SS_SWITCH, INPUT_PULLUP);                // fire encoder switch pin
    delay(10);                                          // give MCU a chance to catch up
    ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);  // set switch interrupt
    ss.enableEncoderInterrupt();                        // fire encoder interrupt
    encoder_position = new_position;                    // reset encoder

    tm.begin(CLK, DIO, DIGITS);                         // fire up and assign pins 4-digit display
    tm.setBrightness(7);                                // set brightness (0 - 7)
    tm.displayClear();                                  // clear display
    tm.displayTime(00, 00, true);                       // light 4-zeros and colon

    cd.setResolution(CountDown::SECONDS);               // set countdown timer to track seconds
    cd.stop();                                          // make sure countdown timer is stopped
}


/*------------------------------------------------------------------------------------------------------*/
/*                                   Application loop - runs forever                                    */
/*------------------------------------------------------------------------------------------------------*/

void loop() {
    if (PollEncoderSwitch() == true) resetTimer();      // reset when NOT in minutes entry function  

    new_position = ss.getEncoderPosition();             // did encoder position change since last loop?

/*                 if timer NOT running and encoder changed goto set minutes routine                    */
    if ((cd.isStopped() == true) && (encoder_position != new_position)) {
        encoder_position = new_position;                
        setMinutes();
    }

/*  if timer NOT running check if 15 or 30-min switch pressed (and released) then go set and run timer  */
    set15Min.read();
    if(set15Min.wasReleased()) set15Mins();

    set30Min.read();
    if(set30Min.wasReleased()) set30Mins();
    
/*                 if timer running (not shutdown) display mins & secs left on 4-digit display          */
    if (shutDown == false) {
        if (cd.remaining() >= 1) {
            uint8_t mm = cd.remaining() / 60;
            uint8_t ss = cd.remaining() - mm * 60;
            bool colon;
            tm.displayTime(mm, ss, true);
        }

/*                 if timer NOT running (run down) display 00:00                                        */
        if (cd.isStopped() == true) {
            if (onceThrough == false) {
                tm.displayTime(00, 00, false);
                onceThrough = true;
                shutDown = true;
            }

/*                 if FIRST TIME through loop DO NOT buzz                                               */
            if (firstThrough == true) {
                return;
            }
            else {
                buzzerBeep(3);
            }   
        }
    }
    delay(10);                                          // let's not overwhelm the MCU
}

bool PollEncoderSwitch() {
    int encoderSW = ss.digitalRead(SS_SWITCH);
    if (encoderSW != lastSwitchState) lastDebounceTime = millis();
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (encoderSW != switchState) {
            switchState = encoderSW;
            if (switchState == HIGH) return true;
        }
    }
    lastSwitchState = encoderSW;
    return false;
}

void set15Mins() {
    onceThrough = false;
    cd.start(15);                                       // prototype 15-second  
    // cd.start(900);                                   // production 15-min
    shutDown = false;
    firstThrough = false;                               // first time through change switch to FALSE
}

void set30Mins() {
    onceThrough = false;
    cd.start(1800);
    shutDown = false;
    firstThrough = false;                               // first time through change switch to FALSE

}

void setMinutes() {
    minutes = 0;
    while (PollEncoderSwitch() == false) {
        minutes = ss.getEncoderPosition();
        if (minutes <=0) minutes = 0;
        if (minutes >99) minutes = 99;
        int32_t mm = minutes;
        int32_t ss = 0;
        tm.displayTime(mm, ss, true);
    }
    onceThrough = false;
    cd.start(minutes * 60);
    shutDown = false;
    firstThrough = false;                               // first time through change switch to FALSE
}

void resetTimer() {
    cd.stop();
    onceThrough = false;
    firstThrough = false;                               // first time through change switch to FALSE
}

void buzzerBeep(int beeps) {
    for(int x = 0; x < beeps; x++){
        tone(buzzPin, 2000, 500);                   // 2.0khz for 500ms
        delay(500);
        tone(buzzPin, 2100, 500);                   // 2.1khz for 500ms
        delay(500);
    }
}
