// Simple button library for Arduino.
#include "Arduino.h"

#ifndef simplebutton_h
#define simplebutton_h

/* 

Defines a class SimpleButton which must be linked to a GPIO to act as a button handler for this pin.

SimpleButton will debounce the pin and is enables the application to react on the follwing events:
- Shortpress (Button has pressed just once shortly, aka "click")
- Longpress (Button is pressed for a longer time), the application can distinguish between
    - Longpress starts
    - Continuous longpress
    - Longpress done
- DoubleClick (Button has pressed twice shortly, with only a short release time in between)
- DobleLongpress (Button has been pressed once shortly followed directly by a longpress)
    - DoubleLongpress starts/is still on/done are reported as well

Wether double click events are generated and the timings to distinguish for instance between short-/longpresses 
can be found below.

To keep the code (and RAM requirements) short , there are some limitations:
- Pin is set to Input with Internal Pullup. If the controller does not have internal pullups on tha specified pin, you need to add one
  in hardware. 
- The switch attached to the pin is considered to drive the pin to LOW if pressed.
- Valid pin numbers range from 0..63.
- Timings (for Debounce, Longpress-Events) are hardcoded at compile time (see BUTTONTIME_-#defines below) and can not be 
  changed by API or set to different values for different buttons!
- All Timings are significant as multiples of 16 only (i. e. 0..15 === 0, 16..31 === 16 etc.) and must not exceed 1023 (or 1008)

There are only two API-Calls:

SimpleButton::SimpleButton(uint8_t pin);

  - Constructor to create a "Button-Handler"
  - The given pin number must not exceed 63
  - The given pin is set to pinMode(INPUT_PULLUP). The allplication must not change the pinMode for the given pin!

uint8_t SimpleButton::checkEvent( uint8_t (*eventHandler)(uint8_t eventId, uint8_t pin)=NULL );
  - Must be called frequently (i. e. in loop()) to process the button events
  - will return one of the return codes described below (note that the return value can be changed by the callback) 
  - an optional callback-function can be passed as argument to SimpleButton::checkEvent():
     - signature of callback function is uint8_t (uint8_t eventId, uint8_t pin);
     - the first parameter is the event triggering the callback, equivalent to the return code of SimpleButton::checkEvent(), but 
       only the events coded as BUTTONEVENT_*-defines will be reported to the callback function
     - the second parameter is the pin that is attached to the button. This information can be used to use the same callback function
       for different buttons (i. e. for Volume+ or Volume- where the logic is the same but only the direction of change differs)
     - the callback will be called from inside checkEvent() before it returns to the main application loop
     - the return value of the callback will be returned by checkEvent(). When in doubt, just return the parameter event
     - for the application, events can be evaluated by the callback and/or by analysing the return value of ::checkEvent(). In theory 
       there is no difference on either approach, you can for instance split the action by handling longpress events within the callback
       and shortpresses from the return value of ::checkEvent().
 */

#define BUTTON_2CLICKENABLED 1                    // Comment out (or set to 0), if you do not want double-Click-Events.
                                                  //   Double Click-Events always start with a short click followed by another
                                                  //   short click (resulting in BUTTONEVENT_2CLICK) or longpress (resulting in
                                                  //   BUTTONEVENT_2FIRSTLONGPRESS followed by (any number of) 
                                                  //   BUTTONEVENT_2LONGPRESS concluded by BUTTONEVENT_2LONGPRESSDONE

#define BUTTONTIME_PRESSDEBOUNCE      1*16   // How long to debounce falling slope of pin (in ms), i. e. Button going to pressed
                                             // Zero is probably fine, if the button does not generate noise on changes (oscillates
                                             // between HIGH/LOW before going to a stable low reading) and you do not want to use 2Clicks.
#define BUTTONTIME_LONGPRESS1        20*16   // Time (ms) after debounce a button needs to be pressed to be considered longpressed
#define BUTTONTIME_LONGPRESSREPEAT    3*16   // Time (ms) between consecutive longpress events
#define BUTTONTIME_RELEASEDEBOUNCE    4*16   // How long to debounce rising slope of pin (in ms), i. e. Button going to released
#define BUTTONTIME_2CLICKWAIT        12*16   // How long to wait (after single short click) for start of next click (or longpress)


/* 
Return codes for SimpleButton::eventCheck() Do not mess around with the numbers used in the follwing defines!!!

Each event is reported only once. I. e. if a BUTTONEVENT_SHORTPRESS is returned, the next call to eventCheck() will
return something else (most likely BUTTON_IDLE) if called again directly (and no other press has happened).

Note that the values are somewhat bitcoded:
- if value is 0 (Zero), button is currently not pressed and no event is due...
- b0 is the "event" flag, if set, a button event has occured (pressed, longpressed)
- if b1 is set (currently only if b0 is set as well), this is a longpress event
- b2 is the "repeat" flag (currently only for continuous longpress, so b1 and b0 are set as well)
- b3 is the "done" flag. 
   * if no other bit is set, this signals the button is still active (pressed) but no event condition applies
   * if b1 and b0 are also set, longpress has just been finished
- b4 is the double-click flag. If this flag is set, this is either a double click or a longpress following the first short click
   * double-click-events are generated only if the #define BUTTON_2CLICKENABLED is set
   * the timeout to wait for the second click following the short click can be set by #define BUTTONTIME_2CLICKWAIT
      - if this timeout is too short, you will not be able to generate doubleclick-events
      - the longer this timeout is, the longer it will take to propagate a single short press

*/

// DO NOT CHANGE THE FOLLOWING DEFINES IF YOU ARE NOT ABSOLUTELY SURE WHAT YOU ARE DOING AND WHY!

#define BUTTON_IDLE                     0         // Button is currently not pressed.
#define BUTTONEVENT_SHORTPRESS     _BV(0)         // Shortpress-Event detected!
#define BUTTONEVENT_2CLICK         (_BV(4)|_BV(0))// Double-click-Event
#define BUTTONEVENT_FIRSTLONGPRESS (_BV(1)|_BV(0))// Button is longpressed (Longpress just started)
#define BUTTONEVENT_LONGPRESS (_BV(2)|_BV(1)|_BV(0))// Button is still longpressed (event will be generated every x ms as defined by 
                                                    // BUTTONTIME_LONGPRESSREPEAT (see below), if SimpleButton::checkEvent() is called 
                                                    // often enough.
                                                    // The application self must do something if longer period is needed (i. e. like
                                                    //  delaying calls to ::checkEvent())
#define BUTTONEVENT_LONGPRESSDONE (_BV(3)|_BV(1)|_BV(0))   // Button is released after longpress. The application must not treat this event
                                                           // as if the button is still pressed but either use it for some cleanup (if needed)
                                                           // or simply ignore it.
#define BUTTONEVENT_2FIRSTLONGPRESS (_BV(4)|_BV(1)|_BV(0)) // Button is longpressed (Longpress just started) after a short click
#define BUTTONEVENT_2LONGPRESS (_BV(4)|_BV(2)|_BV(1)|_BV(0))
                                                  // Button is still longpressed after a preceding short click (event will be generated 
                                                  // every x ms as defined by BUTTONTIME_LONGPRESSREPEAT (see below), if 
                                                  // SimpleButton::checkEvent() is called often enough.
                                                  // The application self must do something if longer period is needed (i. e. like
                                                  //  delaying calls to ::checkEvent())
#define BUTTONEVENT_2LONGPRESSDONE (_BV(4)|_BV(3)|_BV(1)|_BV(0))        
                                                  // Button is released after longpress following a preceding short click. 
                                                  // The application must not treat this event
                                                  // as if the button is still pressed but either use it for some cleanup (if needed)
                                                  // or simply ignore it.
#define BUTTON_PRESSED               _BV(3)       // No event, but the button is pressed (so either a BUTTONEVENT_SHORTPRESS or 
                                                  // any of the longpress-Events might follow but time for this is not yet due).

// The following is evaluated as true, if the event passed as argument is a LONGPRESS-event (single or double, 
//  start/continuing/done)
#define BUTTONEVENT_ISLONGPRESS(x)      ((_BV(0)|_BV(1)) == (x & (_BV(0)|_BV(1))))                                                  

//TODO: needed?
#define BUTTONEVENT_ISDONE(x)           (_BV(3) == (x & _BV(3)))       

// The following is evaluated as true, if the event passed as argument is a DOUBLE (click- or longpress-)event
#define BUTTONEVENT_ISDOUBLE(x)        ((_BV(4)|_BV(0)) == (x & (_BV(4)|_BV(0))))       

// The following converts the event passed as argument from double-click/longpress to single click/longpress event
#define BUTTONEVENT_UNDO_DOUBLE(x)          (x & ~_BV(4))


#define USE_MILLIS16      1  // if defined (!= 0) an external variable uint16_t millis16; is used instead of millis()

class SimpleButton
{
  public:
    SimpleButton(uint8_t pin);
    uint8_t checkEvent(uint8_t (*_event)(uint8_t event, uint8_t pin) = NULL);
  private:
    uint16_t _PinDebounceState;
    // The data is stored as Bitfield:
    //   - b15..b10 (6 bit): pin number 
    //   -  b9.. b4 (6 bit): timestamp (in units of 16ms) to remember last state change in checkEvent
    //   -  b3.. b0 (4 bit): current state of checkEvent
};
#endif
//https://www.electronjs.org/docs/latest/development/pull-requests
