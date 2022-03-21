#include "Arduino.h"
#include "SimpleButton.h"


#define BUTTONSTATE_IDLE          0           // Button not pressed (initial state) 
                                              // DO NOT CHANGE!!! BUTTONSTATE_IDLE must always be defined as 0 (Zero)!
#define BUTTONSTATE_DEBOUNCE      1           // Button press detected, waiting for debounce
#define BUTTONSTATE_RELEASE       2           // Button was released again

// BUTTONSTATE_RELEASE is the last state (in numerical order) for which the button is assumed not pressed
// All states which are (numerical) higher than BUTTONSTATE_RELEASE are representing states with pressed button
// In total 16 different states can be coded (0..15), apart from the rule with BUTTONSTATE_RELEASE the order is irrelevant

#define BUTTONSTATE_PRESSED       3           // Button pressed. Measure time to see if it is a Longpress
#define BUTTONSTATE_LONGPRESS     4           // Press is a longpress
#define BUTTONSTATE_LONGRELEASE   5           // Button released after longpress but wait for debounce the release
#define BUTTONSTATE_SHORTRELEASE  6           // Button released after shortpress but wait for debounce the release
#define BUTTONSTATE_2PRESSED      7           // Button pressed again after preceeding first click. Measure time to see if it is a Longpress
#define BUTTONSTATE_2SHORTRELEASE 8           // Button released for a 2click (short), but wait for debounce.
#define BUTTONSTATE_2LONGPRESS    9           // Button press after first click is a longpress
#define BUTTONSTATE_2LONGRELEASE 10           // Button is released after 2longpress, but waits for debounce.

//#define BUTTONSTATE_2DEBOUNCE     7


SimpleButton::SimpleButton(uint8_t pin)
{
  pinMode(pin, INPUT_PULLUP);
  _PinDebounceState = ((uint16_t)pin << 10);
}


uint8_t SimpleButton::checkEvent(uint8_t (*_event)(uint8_t event, uint8_t pin)) {
uint8_t ret = 0;
#if (0 != USE_MILLIS16)
extern uint16_t millis16;
uint16_t currentTime = millis16 & 0x3f0;
#else
uint16_t currentTime = millis() & 0x3f0;
#endif
uint16_t state = _PinDebounceState & 0xf;
uint16_t debounce = _PinDebounceState & 0x3f0;
uint8_t pin = _PinDebounceState >> 10;
uint8_t pinState = digitalRead(pin);
uint16_t elapsed;

  if (currentTime < debounce)
    currentTime = currentTime + 0x400;

  elapsed = currentTime - debounce;
  switch(state)
  {
    case BUTTONSTATE_IDLE:
      if (!pinState)
      {
        state = BUTTONSTATE_DEBOUNCE;
        debounce = currentTime;        
      }
      break;  
    case BUTTONSTATE_DEBOUNCE:
      if (pinState)
      {
        state = BUTTONSTATE_IDLE;
      }
      else if (elapsed >= BUTTONTIME_PRESSDEBOUNCE)
      {
        state = BUTTONSTATE_PRESSED;
      }
      break;  
    case BUTTONSTATE_PRESSED:
    case BUTTONSTATE_2PRESSED:
      if (pinState)
      {
        debounce = currentTime;
        state = (BUTTONSTATE_PRESSED==state)?BUTTONSTATE_SHORTRELEASE:BUTTONSTATE_2SHORTRELEASE;
      }
      else if (elapsed >= BUTTONTIME_LONGPRESS1)
      {
        if (BUTTONSTATE_PRESSED == state)
        {
          ret = BUTTONEVENT_FIRSTLONGPRESS;
          state = BUTTONSTATE_LONGPRESS;
        }
        else
        {
          ret = BUTTONEVENT_2FIRSTLONGPRESS;
          state = BUTTONSTATE_2LONGPRESS;
        }
        debounce = currentTime;
      }
      break;
    case BUTTONSTATE_LONGPRESS:
    case BUTTONSTATE_2LONGPRESS:
      if (pinState)
      {
        state = (BUTTONSTATE_LONGPRESS==state)?BUTTONSTATE_LONGRELEASE:BUTTONSTATE_2LONGRELEASE;
      }
      else if (elapsed >= BUTTONTIME_LONGPRESSREPEAT)
      {
        debounce = currentTime;
        ret = (BUTTONSTATE_LONGPRESS==state)?BUTTONEVENT_LONGPRESS:BUTTONEVENT_2LONGPRESS;
      }
      break;
    case BUTTONSTATE_LONGRELEASE:
    case BUTTONSTATE_2LONGRELEASE:
      if (pinState)
      {
        ret = (BUTTONSTATE_LONGRELEASE == state)?BUTTONEVENT_LONGPRESSDONE:BUTTONEVENT_2LONGPRESSDONE;
        state = BUTTONSTATE_RELEASE;
        debounce = currentTime;
      }
      else
      {
        state = (BUTTONSTATE_LONGRELEASE == state)?BUTTONSTATE_LONGPRESS:BUTTONSTATE_2LONGPRESS;
      }
      break;
    case BUTTONSTATE_SHORTRELEASE:
#if (0 != BUTTON_2CLICKENABLED)
    case BUTTONSTATE_2SHORTRELEASE:
      if (pinState)
      {
        if (BUTTONSTATE_2SHORTRELEASE == state)
        {
          ret = BUTTONEVENT_2CLICK;
          state = BUTTONSTATE_RELEASE;
        }
        else if (elapsed >= BUTTONTIME_2CLICKWAIT)
        {
          ret = BUTTONEVENT_SHORTPRESS;
          state = BUTTONSTATE_RELEASE;          
        }
      }
      else if (elapsed >= BUTTONTIME_RELEASEDEBOUNCE)
      {
        state = BUTTONSTATE_2PRESSED;
      }
#else
      if (pinState)
      {
        ret = BUTTONEVENT_SHORTPRESS;
        state = BUTTONSTATE_RELEASE;
      }
      else
      {
        state = BUTTONSTATE_PRESSED;
      }
#endif
      break;
    case BUTTONSTATE_RELEASE:
      if (pinState)
      {
        if (elapsed >= BUTTONTIME_RELEASEDEBOUNCE)//(millis() - (_debounce > (BUTTON_DEBOUNCE)))
          state = BUTTONSTATE_IDLE;
      }
      else 
      {
        debounce = currentTime;
      }
      break;  
    default:
      break;
  }
  _PinDebounceState = (_PinDebounceState & 0xfc00) | (debounce & 0x3f0) | state;
  if (ret)
  {
    if (_event)
      ret = _event(ret, pin); 
  }
  else
  {
    if (state > BUTTONSTATE_RELEASE)
      ret = BUTTON_PRESSED;
  }
  return ret;
}
