# General Info

**This example is intended to be merged to https://github.com/pu2clr/SI4735/tree/master/examples/SI47XX_KITS/AliExpress/SI473X_ALL_IN_ONE_OLED_RDS_CHINESE_V8. This fork exists just for preparing the pull request. The work to be done is to finish this README and to fix some errors if found on the go...**

This example had the https://github.com/pu2clr/SI4735/tree/master/examples/SI47XX_KITS/AliExpress/SI473X_ALL_IN_ONE_OLED_RDS_CHINESE_V7 as starting point. The functionality has not changed in general but has some rework on the user-interface:
- The display does not flicker anymore if no values are changed (will be especially visible in AM/SSB-display).
- Buttons are now linked to more than one function (see section [User manual](#user-manual) below) by using i. e. double-click or longpress events.
- The display style while changing a parameter has changed. Not only the parameter name but also the changed value are displayed in inverted style. This change is especially noticeable if the BFO setting is changed this way. Here the BFO-line (and no longer the frequency of the VFO) will be inverted. The frequency will be inverted if changed by turing the rotary encoder.
- If the rotary encoder breaks, the radio can now be operated by using buttons only (see [Encoder-Simulation-Mode (ESM)](#encoder-simulation-mode) below)).
- The radio already starts the last station while the intro screen is still showing.
- Of the changed functionality can be configured to your liking (even disabled alltogether), see [configuration](#configuration) below.


# Features

* Modes: FM, AM and SSB
* Two FM bands covering from 64 to 108 MHz
* LW band
* Two MW bands (one for Europe, Africa and Asia)
* Twenty three bands. Two VHF(FM); one LW; two MW; and Nineteen SW bands covering from 1800 to 30000 kHz
* FM/RDS
* Bandwidth control on FM mode (Auto, 110, 84, 60 and 40 kHz)
* Bandwidth control on AM mode (1, 2, 2.5, 3, 4 and 6 kHz)
* Bandwidth control on SSB mode (0.5, 1, 1.2, 2.2, 3 and 4 kHz)
* Receiver status stored into the EEPROM
* The final code (HEX file) is about 2.5K smaller than the previous one (now  you have more memory to add new features)
* All the previous status of the receiver can be rescued when you turn it on (including SSB mode, bandwidth, volume, frequency, BFO etc)
* The bandwidth is a property of the band (you can use different bandwidth for different bands)
* The seek function was improved (it is more precise on FM mode). The seek direction is controlled by the encoder (clockwise or counter-clockwise . Press encoder push button for seeking
* Steps: 1, 5, 9, 10 and 50 kHz
* Now you can configure MW band space to 9 or 10 kHz
* Added a MW band for Europe, Africa and Asia
* The frequency on Display is bigger than the previous version
* Now the bandwidth sequence is ordered by bandwidth values
* After about 4 seconds, all command buttons are all disabled and the encoder control goes back to the frequency


# User manual
## General behaviour

* Some device come with an encoder that breaks easily. If that happens, the device can now be fully operated by using buttons only (see [Encoder-Simulation-Mode](#encoder-simulation-mode-esm) below).
* Buttons can now react on the follwing events:
  - Shortclick (or just **CLICK**) occurs if a button has been shortpressed.
  - A Doubleclick (or **2CLICK**) is two shortclicks in very quick succession.
  - a Longpress (or **LP**) is a long press and hold of a button.
  - a Double-Longpress (or **2LP**) is a short click followed by a longpress in very quick succession.
  - **LP**/**2LP**-events have a continuous functionality attached normally (like constantly increase the volume as long as "Vol+" is pressed.
    There are functions that react only once during a longpress episode (like **LP** on encoder to Mute/Unmute the radio).
    To denote this, such events will be labelled as ****LP**(once)** or ****2LP**(once)**
* The behaviour described below is the default behaviour, that might be changed to your liking (see [Button Configuration](#configuration-of-button-functions))


## Button Commands

- assigned to "BAND+":
  - **CLICK**: toggle command to change the Band by rotating the encoder
  - **2CLICK**: toggle between first and last band defined
  - **LP**: Switch to next band as long as button is pressed, stop at the last defined band.
  - **2LP(once)**: Switch (just once) to next band (again no wrap around)

- assigned to "BAND-":
  - **CLICK**: toggle command to change SoftMute by rotating the encoder (ignored in FM-Mode)
  - **2CLICK**: toggle between min and max value of SoftMute (ignored in FM-Mode)
  - **LP**: Switch to previous band as long as button is pressed, stop at the first band defined.
  - **2LP(once)**: Switch (just once) to previous band (again no wrap around)

- assigned to "VOL+":
  - **CLICK**/**2CLICK**: toggle command to change the Volume by rotating the encoder
  - **LP**/**2LP**: continuously increase Volume up to max (63)

- assigned to "VOL-"
  - **CLICK**: toggle command to change AVC by rotating the encoder (ignored in FM-Mode)
  - **2CLICK**: toggle AVC value between min (12), average (38) and max (90) (not in FM-mode)
  - **LP**: continuously decrease Volume up to min (0)
  - **2LP**: continuously increase/decrease AVC between 12/90 (direction changes with every **2LP**) (not in FM-mode)  

- assigned to "STEP" (not in FM-Mode):
  - **CLICK**: toggle command to change Step by rotating the encoder
  - **2CLICK**: toggle Step-Value between Min/Max-step defined (for specific AM/SSB band)
  - **LP**/**2LP**: continuously increase/decrease Step between min/max (direction changes with every press)

- assigned to "BW":
  - **CLICK**: toggle command to change Bandwidth by rotating the encoder
  - **2CLICK**: toggle Bandwidth between min/max defined for the current band
  - **LP**: continuously increase/decrease Bandwidth between min/max (direction changes with every press)
  - **2LP(once)**: set the default Bandwidth defined for the current band.

- assigned to "AGC" (not in FM-Mode): 
  - **CLICK**: toggle command to change AGC/Attenuation by rotating the encoder
  - **2CLICK**: toggle Attenuation between min (0) and max (36)
  - **LP**: continuously increase/decrease AGC/Attenuation between AGC/0..36 (direction changes with every press)
  - **2LP(once)**: set AGC

- assigned to "MODE":
  - **CLICK**/**2CLICK**: in FM-Mode: toggle RDS off/on, else toggle between AM/LSB/USB-mode
    - the RDS setting applies to all FM bands and is stored to EEPROM
  - **LP(once)**/**2LP(once)**: toggle "Encoder-Simulation-Mode" (see below)

- assigned to "Rotary-Encoder-Button" (at normal play):
  - **2CLICK**/**2LP** will be ignored
  - **CLICK**: if any command is selected (to be changed by rotary encoder), cancel that command immediately (not waiting for timeout)
           else if in AM/FM-Mode: start search
           else if in SSB-Mode: toggle rotary encoder between BFO/VFO-setting
  - **LP(once)**: toggle Mute (if muted, "XX" will be displayed as current volume). Radio will resume at last volume, if any volume change function (i. e. by __LP__ on "Vol+" or "Vol-") or the rotary encoder is turned to change the current frequency.

- assigned to "Rotary-Encoder-Button" (at startup):
  - if pressed when powered on, EEPROM will be cleared
  - **CLICK**: if intro is still running will cancel the intro screen direction
  - **LP**: if intro is still running will also clear the EEPROM 
  - **2CLICK**/**2LP** will be ignored

## Encoder-Simulation-Mode (ESM)

* Some devices come with a rotary encoder that is broken easily. This mode is intended to simulate the encoder by buttons only.
* If "Encoder-Simulation-Mode" is active, the display of the frequency unit (kHz/MHz) at the upper right will be inverted. (To activate/deactivate ESM you have to **LP** the Mode-Button).
* In Active encoder mode (full button event change description is below):
  - **CLICK**/**LP** on "BAND+" will be the same as rotating the encoder right 
  - **CLICK**/**LP** on "BAND-" will be the same as rotating the encoder left
  - **CLICK**/**LP** on "AGC" will be the same as **CLICK**/**LP** on the encoder Button
* Note that the encoder itself will stay fully operational in this mode

- behaviour of button "BAND+" in ESM:
  - **CLICK**: identical to one turn right of the encoder
  - **2CLICK**: toggle command to change the Band by rotating the encoder (like **CLICK** in "normal" mode)
  - **LP**: identical to continously turning the encoder right
  - **2LP(once)**: Switch (just once) to next band (as in "normal mode")

- behaviour of button "BAND-" in ESM:
  - **CLICK**: identical to one turn left of the encoder
  - **2CLICK**: toggle command to change SoftMute by rotating the encoder (like **CLICK** in "normal" mode)
  - **LP**: identical to continously turning the encoder left
  - **2LP(once)**: Switch (just once) to previous band (as in "normal mode")

- behaviour of button "AGC" in ESM:
  - **CLICK**: identical to **CLICK** on the encoder button
  - **2CLICK**: toggle command to change AGC/Att by rotating the encoder (like "normal" **CLICK**, not in FM-Mode)
  - **LP**: identical to **LP** on the encoder button
  - **2LP**: Change AGC/Att as long as pressed (like "normal" **LP**, not in FM-Mode)


# Configuration
## General
- The configuration can only be done at compile time, if you want to change some behaviour as described below you have to recompile and reflash your device.
- To support debugging, two flags can be set in **Config.h**:
    - **#DEBUG** if this flag is defined 
        - Serial output is enabled at 115200 baud
        - Button events will be displayed on Serial monitor
        - **LP**/**2LP** episodes will show multiple events in the following order:
            - **LP/2LP start** Longpress/Double longpress just started
            - **LP/2LP repeat** Longpress/Double longpress ongoing
            - **LP/2LP done** Longpress/Double longpress finished
    - **#DEBUG_BUTTONS_ONLY** if this flag is defined (but only if **#DEBUG** is defined as well)
        - Same info on Serial as before (with clear name of button pressed) 
        - Radio will not play
            - this mode is especially useful to [adjust button timings](#configuration-of-button-timings)
    - if **#DEBUG** is set (but not **#DEBUG_BUTTONS_ONLY**) the ressources are maxed out: flash is at 99% then 
- The functionalities (of the buttones) can be altered by changing some [#defines](#configuration-of-button-functions) in **Config.h** 
- The button timings itself (i. e. timeout to distinguish between longpress and doublepress) can be changed by altering some [#defines](#configuration-of-button-timings) in **SimpleButton.h**


## Configuration of Button timings
General:
- The button timings can be configured to personal likings and/or capabilities of the device used.
- To support the configuration, uncomment the line __#define DEBUG__ in __Config.h__ for getting some informations on the button events on Serial monitor (@115200 baud)
- If you also uncomment the line __#define DEBUG_BUTTONS_ONLY__ just below in __Config.h__ you will get slightly more verbose output on Serial (including the "clear names" of the buttons so you can check the correct pin assignment) and all radio functions will be turned off. The readout could be something like this:
```
22:35:44.253 -> Ev= 1 Pin=11 ("AGC") >>CLICK
22:35:45.148 -> Ev= 1 Pin= 4 ("Mode") >>CLICK
22:35:53.363 -> Ev= 1 Pin= 8 ("Band+") >>CLICK
22:35:54.357 -> Ev=17 Pin= 7 ("Vol-") >>2CLICK
22:35:56.809 -> Ev= 3 Pin=10 ("Step") >>LP start
22:35:56.875 -> Ev= 7 Pin=10 ("Step") >>LP repeat
22:35:56.909 -> Ev= 7 Pin=10 ("Step") >>LP repeat
22:35:56.942 -> Ev= 7 Pin=10 ("Step") >>LP repeat
22:35:57.008 -> Ev= 7 Pin=10 ("Step") >>LP repeat
22:35:57.041 -> Ev= 7 Pin=10 ("Step") >>LP repeat
22:35:57.107 -> Ev= 7 Pin=10 ("Step") >>LP repeat
22:35:57.141 -> Ev= 7 Pin=10 ("Step") >>LP repeat
22:35:57.207 -> Ev= 7 Pin=10 ("Step") >>LP repeat
22:35:57.207 -> Ev=11 Pin=10 ("Step") >>LP done
22:36:04.728 -> Ev=19 Pin=14 ("Encoder") >>2LP start
22:36:04.761 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:04.827 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:04.861 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:04.927 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:04.960 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:04.993 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:05.060 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:05.093 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:05.159 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:05.192 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:05.259 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:05.292 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:05.358 -> Ev=23 Pin=14 ("Encoder") >>2LP repeat
22:36:05.358 -> Ev=27 Pin=14 ("Encoder") >>2LP done
22:40:23.294 -> Encoder turned right
22:40:24.122 -> Encoder turned left
```

- The button timings self must be configured in **SimpleButton.h** (not Config.h)
- All timings are compile-time settings (preprocessor __#defines__) that apply to all buttons
- All timings are in milliseconds (ms) and must be defined as multiples of 16 with a maximum of 1008 (0, 16, 32, ..., 1008), other values are rounded down (0..15==0, 16..31==16, ... 1008..1023==1008) and wrapped around and calculated modulo 1024
- Timings are not guaranteed, as especially an Arduino Nano is not a RT system, but they are close enough :-)

The essential timing definitions are:
- __#define BUTTONTIME_PRESSDEBOUNCE      1*16__: for how long pin readout must be __LOW__ for the button considered to be pressed. This is only for debouncing the button. 
- __#define BUTTONTIME_LONGPRESS1        20*16__: for how long (after debounce) a button needs to be pressed for being considered longpressed.
- __#define BUTTONTIME_LONGPRESSREPEAT    3*16__: the timing distance to report repeated longpress events while a button is still pressed. This setting is the "timing base" for repeated **LP/2LP**-events like longpress at "Vol+". The speed of change (the volume up in that example) can be configurad as this setting (minimum) or a multiple thereof (or disabled alltogether, see [Button Configuration](#configuration-of-button-functions))
- __#define BUTTONTIME_RELEASEDEBOUNCE    2*16__: for how long pin readout must be __HIGH__ for the button considered released again. This is only for debouncing the button. 
- __#define BUTTONTIME_2CLICKWAIT        12*16__: for how long to wait (after a first single __CLICK__) to see if another button press follows direct (thus starting a __2CLICK__ or __2LP__ event). This timing is somewhat critical:
    - if set too low, it might be impossible to generate __2CLICK__ events but two seperate __CLICK__ events are reported.
    - setting it too high is also problematic, as the single __CLICK__ will only be reported afther this time has elapsed and/or __2CLICK__/__2LP__ events are reported even though in fact two single __CLICKs__ were intended.

There is another setting in **SimpleButton.h** that does not define the timings but enables (default) or disables double-click events:
- __#define BUTTON_2CLICKENABLED 1__: if this define is set to 0, no __2CLICK__ and/or __2LP__ events will be generated. A double-click will result in two __CLICKs__ and a click followed by a longpress will result in __CLICK__ and __LP__.

## Configuration of Button functions

Basically, the "old" behaviour is represented in this version by a single __CLICK__ on the button. All extended button functions are linked to __LP__ (including __LP(once)__) or double-click events (__2CLICK__, __2LP__, __2LP(once)__).

With the following defines, the extended functions can be adjusted to your liking (enabled or disabled or timings for __LPx__-events can be adjusted). You don't need to program direct but just change some __#defines__ in **Config.h** before recompiling/flashing. Within **Config.h**, search for the __FUNCTIONCONFIG__ section.

Please keep in mind, that the base timing for "repeating" __LP__-events is defined by __#define BUTTONTIME_LONGPRESSREPEAT__ in **SimpleButton.h** as described [above](#configuration-of-button-timings).

The following applies in general:
- each functions is controlled by a number, that number must be of uint8-type (valid range is 0..255)
- the timings are rough estimates (its not a realtime system) and might be longer if the device is busy

The following should apply in general (I might have not tested all variants):
- if a function is disabled, the associated click-event (__2CLICK__, __LP__ etc.) will be treated as a normal __CLICK__

- __#define BAND_DELAY  2__ controls the __LP__ behaviour of the "Band+"/"Band-"-buttons:
    - if set to 0 __LP__ event (continuously switch band up/down) will be disabled
    - any other number __n__ will set the timeout between band switches to be __n * BUTTONTIME_LONGPRESSREPEAT__ (in ms)
- __#define BANDSWITCH1STEP_DELAY 1__ controls the __2LP(once)__ behaviour of the "Band+"/"Band-"-buttons:
    - if set to 0 __2LP(once)__ event switch one band up/down) will be disabled
    - any other number __n__ will set the timeout of the longpress following the first shortpress to be __(n - 1) * BUTTONTIME_LONGPRESSREPEAT + BUTTONTIME_LONGPRESS1__ (in ms). Leaving 1 (minumum) should be fine.
- __#define BANDUP_2CLICKENABLE 1__ controls the __2CLICK__ behaviour of the "Band+" button:
    - if set to != 0, __2CLICK__ on "Band+" toggles between last/first band
- __#define BANDDN_2CLICKENABLE 1__ controls the __2CLICK__ behaviour of the "Band-" button:
    - if set to != 0, __2CLICK__ on "Band-" toggles between min/max softmute (not in FM)
- __#define VOLUME_DELAY 1__ controls the __LP__ bevaviour of "Vol+"/"Vol-" buttons:
    - if set to 0 __LP__ event (continuously change volume up/down) will be disabled
    - any other number __n__ will set the timeout between volume changes to be __n * BUTTONTIME_LONGPRESSREPEAT__ (in ms)
- __#define VOLUMEDN_2CLICKENABLE 1__ controls the __2CLICK__ and the __2LP__ behaviour of the "Vol-" button:
    - if set to 0, both __2CLICK__ and __2LP__ will be disabled (change AVC in non FM-Mode)
    - if set to any number __n__:
        - __2CLICK__ will be enabled (toggle AVC between min, default and max, i. e. 12, 38 and 90)
        - __2LP__ will be enabled (continuously change AVC between min and max) with the timeout between value changes defined as __n * BUTTONTIME_LONGPRESSREPEAT__ (in ms)
- __#define STEP_DELAY 6__ controls the __LP__ behaviour of the "Step" button:
    - if set to 0, __LP__ will be disabled
    - any other number __n__ will set the timeout between band switches to be __n * BUTTONTIME_LONGPRESSREPEAT__ (in ms)
- __#define STEP_2CLICKENABLE 1__ controls the __2CLICK__ and the __2LP(once)__ behaviour of the "Step" button:
    - if set to 0, both __2CLICK__ and __2LP(once)__ will be disabled (change AVC in non FM-Mode)
    - if set to any other valid number: 
        - __2CLICK__ will be enabled (to toggle Step between min and max, depending on current band)
        - __2LP(once)__ will be enabled (to set the default step for the current band)
- __#define BANDWIDTH_DELAY 9__ controls the __LP__ behaviour of the "BW" button:
    - if set to 0, __LP__ will be disabled
    - any other number __n__ will set the timeout between bandwidth changes to be __n * BUTTONTIME_LONGPRESSREPEAT__ (in ms)
- __#define BANDWIDTH_2CLICKENABLE 1__ controls the __2CLICK__ and the __2LP(once)__ behaviour of the "Step" button:
    - if set to 0, both __2CLICK__ and __2LP(once)__ will be disabled
    - if set to any other valid number: 
        - __2CLICK__ will be enabled (toggle between between min and max, depending on current mode)
        - __2LP(once)__ will be enabled (set default for current mode)   
- __#define AGC_DELAY 1__ controls the __LP__ behaviour of the "AGC" button:
    - if set to 0, __LP__ will be disabled
    - any other number __n__ will set the timeout between AGC changes to be __n * BUTTONTIME_LONGPRESSREPEAT__ (in ms)
- __#define BANDWIDTH_2CLICKENABLE 1__ controls the __2CLICK__ and the __2LP(once)__ behaviour of the "AGC" button:
    - if set to 0, both __2CLICK__ and __2LP(once)__ will be disabled
    - if set to any other valid number: 
        - __2CLICK__ will be enabled (to toggle between between min and max attenuation, i. e. 12 and 90)
        - __2LP(once)__ will be enabled (to switch on AGC)   
- __#define MODE_DELAY 15__ controls the __LP(once)__ behaviour of the "Mode" button (to switch on [Encoder Simulation Mode ESM](#encoder-simulation-mode-esm)):
    - if set to 0 __LP(once)__ event to switch on ESM will be disabled
    - any other number __n__ will set the timeout to start ESM  to be __(n - 1) * BUTTONTIME_LONGPRESSREPEAT + BUTTONTIME_LONGPRESS1__ (in ms). Leaving 1 (minumum) should be fine.
    - as ESM can be confusing if switched on unintendedly, __MODE_DELAY__ should be set to a high value. In order to quit ESM faster (than entering it), the __#define ESM_DONE_SPEEDUP__ can be set in addition to shorten the longpress needed to cancel active ESM:
      - if set to 0, same longpress length is required to cancel ESM
      - shortest is to set to the same value as __MODE_DELAY__ or above (so setting it to 255 will always lead to the shortest possible longpress time to cancel ESM, no matter how __ENCODER_MODE__ is defined)
    - within ESM, the timeout between two simulated "roatation clicks" by __LP__ on "Band+" (for simulating turns right) or "Band-" (left) are set by __ESM_SLOWDOWN 0__ and calculated as __(n + 1) *  BUTTONTIME_LONGPRESSREPEAT__ (in ms) with the default value 0 as the fastest possible.
- __#define ENCODER_DELAY 2__ controls the __LP(once)__ behaviour of the "Encoder" button (to Mute/Unmute the device):
    - if set to 0 __LP(once)__ event to Mute/Unmute will be disabled
    - any other number __n__ will define the timeout to Mute/Unmute  to be __(n - 1) * BUTTONTIME_LONGPRESSREPEAT + BUTTONTIME_LONGPRESS1__ (in ms). 
- __#define ENCODER_SEARCH 1__ allows to enable/disable search in AM and FM mode:
    - if set to 0, __CLICK__ on encoder will not start search (in AM/FM mode), any other number will enable search mode in AM/FM
- __#define ENCODER_CANCEL 1__ allows to enable cancellation of (encoder controlled) commands by __CLICK__ on encoder:
    - if set to !=0, __CLICK__ on encoder will cancel any command that has been started by a shortpress of a button before the timeout (about 4 seconds) will automatically cancel the encoder assignment.

## Configuration of Intro screen

Within **Config.h**, search for the __INTROCONFIG__ section to adjust the settings for the intro screen to your liking.

- the lines to be shown can be defined using the array __char introlines = {...}__. 
  - at most 4 lines can be defined, excess lines will be ignored
  - an empty array can be defined (as __char* introlines[]={};__) to disable the intro screen altogether
  - The intro will show line by line with the delay (in ms) after each lines specified by __#define INTRO_LINEDELAY__
  - The additional delay after all lines have been displayed can be specified by __#define INTRO_ENDDELAY__
    - if both defines (__INTRO_LINEDELAY__ and __INTRO_ENDDELAY__) are set to 0, intro screen will not be shown (setting empty __char* introlines[] == {}__ is the better option as it will save some memory)
  - if the __#define INTRO_LONGPRESSDELAY__ is set to any number n > 0 (as uint8_t), a longpress on the encoder while the info screen is still showing will also erase EEPROM (no need to press the encoder already on power up). The required longpress duration is calculated as __(n - 1) * BUTTONTIME_LONGPRESSREPEAT + BUTTONTIME_LONGPRESS1__ (in ms). (if set to 0, longpress on encoder will be treated the same as shortpress and cancel the intro)
  - if EEPROM was deleted (either by longpress of eduring intro or at power up), the intro will be cancelled and the screen will show "EEPROM erased". The timeout (in ms) can be specified by __#define INTRO_CLREEPROMDELAY__. After that timeout (or button release, whatever condition applies longer) the EEPROM clear message will disapear and the normal radio screen will show
  - the device will already play the last station while the intro screen is still showing. If you do not want this you can set the __#define INTRO_SILENT__ to any non-zero value to return to the "old" behaviour of starting audio only after the intro screen has been finished.

## Miscellaneous configuration

Some configuration settings that do not fit into any other category can be defined in the __MISCCONFIG__ section of __Config.h__

- __#define DEFAULT_VOLUME__ sets the default volume (if no valid entry was saved to EEPROM before), must be between 0 and 63
- __#define DEFAULT_BAND__ sets the index of the band to be played (if no valid entry was saved to EEPROM before), must be between 0 and __lastBand__
- __#define RDS_OFF__ disables RDS if set to non-zero or enables RDS if set to 0 (if no valid entry was saved to EEPROM before)
- __#define DISPLAY_OLDSTYLE__ will set display behaviour to old mode, if set to non-zero. Main differences in the (default) new mode are:
  - if a parameter is changed by encoder, not only the parameter name but also the parameter value self is displayed in inverted style
  - if the BFO setting is linked to the encode, the BFO-line (and no longer the frequency) are displayed in inverted mode


## Configuration of Pin assignments

Within **Config.h**, search for the __PINCONFIG__ section to change the pin assignments.

- the assignments itself have not changed compared to prior versions, they are just moved here for convenience.
- a few literals have been changed to better match to the button description (on the ATS-20 specifically) i. e. __#define BANDUP_BUTTON__ or __#define BANDDN_BUTTON__

# Some programming hints

On an Arduino Nano, the Flash/RAM usage is 29292 bytes (95%) and 1113 bytes (54%). So while RAM is still fine, program space is limited.

With the __#define DEBUG__ set in __Config.h__ this increases to 30666 bytes (99%, only 54 bytes left) and 1388 bytes (67%):
- the device will still be stable, just there is almost no room for added functionality in this setting
- note that for instance string constants are stored in program memory. So if you change the array __introlines[]__ you will probably increase/decrease Flash size

As described [above](#configuration-of-button-functions), most functions can be configured. If a function is disabled, some parts of the code will not be compiled in thus reducing the Flash usage. This is achieved by using the __#defines__ as conditional compile guard.

Consider the following example:
```
//Handle Actions of "Step"-button
uint8_t stepEvent(uint8_t event, uint8_t pin) {
#ifdef DEBUG
  buttonEvent(event, pin);
#endif

...
}
```
This function gets called, whenever a button event (__CLICK__, __LP__ etc.) happens on the "Step"-button. If __DEBUG__ is defined, this function will call __buttonEvent(event, pin);__ (which will print some Debug information on Serial as described [here](#configuration-of-button-timings)). If __DEBUG__ is not set, this call is not compiled thus saving some program space.
(In total, if all of the options are [disabled](#configuration-of-button-functions), the used Flash/RAM size goes down to 27664/1099 bytes).

If you want to change the function assignment to a button event, you have to change the programming. The most likely point to start are the "button handlers". For each button you will find a line in __loop()__ that looks like the following:
```
   btn_VolumeUp.checkEvent(volumeEvent);
```

Here, __btn_VolumeUp__ is the object that handles the "Vol+" button. __.checkEvent__ is the method that checks, if any button event is due and, if so, calls the callback-function __volumeEvent__ 
For a timely processing of the events, a frequent call of the __checkEvent()__ in loop() is needed.

The callback function must be defined with the following signature:
```
   uint8_t volumeEvent(uint8_t event, uint8_t pin);
```
(The return-value is ignored in this application).
The parameter __pin__ specifies the GPIO that is linked to the specific button. This can be use the same callback function for more than one button with similar funtionality (i. e. "Vol+"/"Vol-" where the reaction on a longpress is the same just the direction of change depends on which button was actually pressed).

The parameter __event__ is a constant defined in __SimpleButton.h__ that specifies the event that triggered the call:
- __BUTTONEVENT_SHORTPRESS__: Shortpress-Event (aka __CLICK__) detected
- __BUTTONEVENT_2CLICK__: Double-click-Event (aka __2CLICK__)
- __BUTTONEVENT_FIRSTLONGPRESS__: Button is longpressed, longpress just started. 
- __BUTTONEVENT_LONGPRESS__: Button is still longpressed (event will be generated every x ms as defined by 
                            __BUTTONTIME_LONGPRESSREPEAT__ in __SimpleButton.h__
- __BUTTONEVENT_LONGPRESSDONE__: Button is released after longpress. (Normally this is not an event that should be linked to any user function but allows the application to do some cleanup if needed)
- __BUTTONEVENT_2FIRSTLONGPRESS__: Button is longpressed (after preceeding shortpress), "double-longpress" just started. 
- __BUTTONEVENT_2LONGPRESS__: Button is still "double-longpressed" (event will be generated every x ms as defined by __BUTTONTIME_LONGPRESSREPEAT__ in __SimpleButton.h__
- __BUTTONEVENT_2LONGPRESSDONE__: Button is released after "double-longpress". (Normally this is not an event that should be linked to any user function but allows the application to do some cleanup if needed)

One longpress-episode will result in the following sequence:
- first the callback will be called with event set to __BUTTONEVENT_FIRSTLONGPRESS__. 
- after this, zero or multiple calls with event __BUTTONEVENT_LONGPRESS__ will follow (will be 0 if button was released before timeout __BUTTONTIME_LONGPRESSREPEAT__ elapsed)
- after the button has been released, callback will be called with event set to __BUTTONEVENT_LONGPRESSDONE__.
Same applies for "double-longpress", just the events will change to __BUTTONEVENT_2FIRSTLONGPRESS__, __BUTTONEVENT_2LONGPRESS__ and __BUTTONEVENT_2LONGPRESSDONE__.

With __BUTTONEVENT_SHORTPRESS__ and __BUTTONEVENT_2CLICK__ there is a direct link to the "application events" __CLICK__ and __2CLICK__. The __LP__, __LP(once)__, __2LP__ or __2LP(once)__ events must be designed using the respective __BUTTONEVENTS__ listed above.

Consider the following example how this is handled for changing the Volume up or down by longpress on "Vol+" or "Vol-" button:

```
//Handle Actions of "Vol+"/"Vol-"-buttons
uint8_t volumeEvent(uint8_t event, uint8_t pin) {

//...
#if (0 != VOLUME_DELAY)                         // Longpress on "Vol+"/"Vol-" allowed?
#if (VOLUME_DELAY > 1)                          // react on every "nth"- LP_REPEAT event only?
  static uint8_t count;                           // -->Keep count of LP-events to skip
  if (BUTTONEVENT_FIRSTLONGPRESS == event)        // -->if LP-episode starts
  {
    count = 0;                                      // --->set count to 0
  }
#endif
  if (BUTTONEVENT_ISLONGPRESS(event))           // longpress-Event? (can be LP_START, LP_DONE, LP_REPEAT
    if (BUTTONEVENT_LONGPRESSDONE != event)       // -->but not the final release? (i. e. LP_START or LP_REPEAT)
    {
#if (VOLUME_DELAY == 1)                           // -->change at any LONGPRESS_REPEAT-event (or LP_START)?
      doVolume(VOLUMEUP_BUTTON == pin?1:-1);        // -->do the change (direction depending on parameter "pin")
#else                                             // -->skip a few repeats for slower change of volume
      if (!count)                                   // --> time to change the volume?
        doVolume(VOLUMEUP_BUTTON == pin?1:-1);        // ---> do the change (direction depending on parameter "pin")
      count = (count + 1) % VOLUME_DELAY;           // -->increase & wrap around the counter
#endif
    }

//...

#endif

//...


```
- First thing to notice is the conditional compile starting with __#if (0 != VOLUME_DELAY)__ that requires __VOLUME_DELAY__ to be >= 1 for the funtionality to be built in
- then there are two possible cases:
  - __VOLUME_DELAY__ is precise 1 or
  - __VOLUME_DELAY__ is greater than 1
- in case greater than one, __static uint8_t count;__ is defined and set to 0 to keep track of the number of events that occured to skip (discard) events if needed. Notice the __static__ scope of the variable that allows to retain the value even if the function has returned.  
- after this it is checked, if the event is a longpress-event but not a longpress-release-event (so valid are 
 __BUTTONEVENT_FIRSTLONGPRESS__ or __BUTTONEVENT_LONGPRESS__ ), if so again the 2 cases from above need to be considered:
  - __VOLUME_DELAY__ is precise 1, only the call __doVolume(VOLUMEUP_BUTTON == pin?1:-1);__ will result that volume is increased (if the pin is equal to __VOLUMEUP_BUTTON__) or decreased (pin can only be __VOLUME_DN_BUTTON__ in that case)
  - if __VOLUME_DELAY__ is greater than one, __count__ is checked to be 0. If so, __doVolume()__ will be called. After this, count is increased by one and wrapped around (to 0) if it reaches __VOLUME_DELAY__

 You might have noticed that there is a "flaw" in the algorithm: __count__ is used for either pin. In practice that makes no difference. If you longpress both buttons at the same time, you will see that the Volume stays at the same level (may be just toggles between two values).



