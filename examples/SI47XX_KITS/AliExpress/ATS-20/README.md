# General Info

**This example is intended to be merged to https://github.com/pu2clr/SI4735/tree/master/examples/SI47XX_KITS/AliExpress/SI473X_ALL_IN_ONE_OLED_RDS_CHINESE_V8. This fork exists just for preparing the pull request. The work to be done is to finish this README and to fix some errors if found on the go...**

This example had the https://github.com/pu2clr/SI4735/tree/master/examples/SI47XX_KITS/AliExpress/SI473X_ALL_IN_ONE_OLED_RDS_CHINESE_V7 as starting point. The functionality has not changed in general but has some rework on the user-interface:
- The display does not flicker anymore.
- Buttons are now linked to more than one function (see section [User manual](#user-manual) below) by using i. e. double-click or longpress events.
- If the rotary encoder breaks, the radio can now be operated by using buttons only (see [Encoder-Simulation-Mode (ESM)](#encoder-simulation-mode) below)).
- The radio already starts the last station while the intro screen is still showing.


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
  - **LP(once)**/**2LP(once)**: toggle "Encoder-Simulation-Mode" (see below)

- assigned to "Rotary-Encoder-Button" (at normal play):
  - **2CLICK**/**2LP** will be ignored
  - **CLICK**: if any command is selected (to be changed by rotary encoder), cancel that command immediately (not waiting for timeout)
           else if in AM/FM-Mode: start search
           else if in SSB-Mode: toggle rotary encoder between BFO/VFO-setting
  - **LP(once)**: toggle Mute (if muted, "XX" will be displayed as current volume)

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

The essential timing definitions are:
- __#define BUTTONTIME_PRESSDEBOUNCE      1*16__: for how long pin readout must be __LOW__ for the button considered to be pressed. This is only for debouncing the button. 
- __#define BUTTONTIME_LONGPRESS1        20*16__: for how long (after debounce) a button needs to be pressed for being considered longpressed.
- __#define BUTTONTIME_LONGPRESSREPEAT    3*16__: the timing distance to report repeated longpress events while a button is still pressed. This setting is the "timing base" for repeated **LP/2LP**-events like longpress at "Vol+". The speed of change (the volume up in that example) can be configurad as this setting (minimum) or a multiple thereof (or disabled alltogether, see [Button Configuration](#configuration-of-button-functions))
- __#define BUTTONTIME_RELEASEDEBOUNCE    2*16__: for how long pin readout must be __HIGH__ for the button considered released again. This is only for debouncing the button. 
- __#define BUTTONTIME_2CLICKWAIT        12*16__: for how long to wait (after a first single __CLICK__) to see if another button press follows direct (thus starting a __2CLICK__ or __2LP__ event). This timing is somewhat critical:
    - if set too low, it might be impossible to generate __2CLICK__ events but two seperate __CLICK__ events are reported.
    - setting it too high is also problematic, as the single __CLICK__ will only be reported afther this time has elapsed and/or __2CLICK__/__2LP__ events are reported even though in fact two single __CLICKs__ were intended.
    
## Configuration of Button functions
