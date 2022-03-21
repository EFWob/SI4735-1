#ifndef config_h
#define config_h
// CONFIG-SECTION
//#define DEBUG                 // if defined, Serial output will show some info on button press events (at 115200 baud)
//#define DEBUG_BUTTONS_ONLY    // if defined (in addition to DEBUG), only button events will be reported on Serial, no radio function
                              // (use this to adjust the BUTTONTIME_XXXX-defines in include file "SimpleButton.h" to your liking)

// INTRO-CONFIG

char* introlines[] =   // Defines the intro lines to be shown at startup. At most 4 entries can be defined (excess will be ignored)
                              // If defined as empty array (introlines[] = {};), no intro screen will be shown
{
  "SI473X",
  "Arduino Library",
  "All in One Radio",
  "V3.0.8F-By PU2CLR"
};

#define INTRO_LINEDELAY         1200   // Delay (ms) before showing the next line of intro screen. If set to 0, all lines defined in 
                                       // introlines [] will be shown at once. This delay also applies to the last (normally 4th) line

#define INTRO_ENDDELAY          2000   // Delay (ms) to display full intro screen after last line has been shown 

// If both INTRO_LINEDELAY and INTRO_ENDDELAY are set to 0, intro screen will not show at all!

#define INTRO_LONGPRESSDELAY      2    // Defines how long encoder must be pressed for longpress detection during intro.
                                       //   -if 0, longpress is disabled (will be treated as shortpress and cancel the intro)
                                       //   -if enabled, a longpress with the defined timeout will erase EEPROM

#define INTRO_CLREEPROMDELAY    2000   // Delay (ms) to display "EEPROM erased" information if radio started with Encoder button pressed.
                                       // (If button is pressed longer, message will stay until button released)
                                       
#define INTRO_SILENT            0      // If set to != 0, Audio will start only after intro screen is done 
                                        // (otherwise already audible during intro)

// FUNCTION-CONFIG
/* Some of the defines refer to functions that can be reached by longpress on a specific button. The timings described here are the
 *  default timings as defined in include file "SimpleButton.h"-if you changed the settings there, the example timings described below
 *  will change of course according to your settings there.
 *  Relevant defines are BUTTONTIME_LONGPRESS1, BUTTONTIME_LONGPRESSREPEAT
 *  If you set the define BUTTON_2CLICKENABLED in "SimpleButton.h" to 0, no doubleclick/double longpress event will fire, no matter what
 *  you define below...
 *  If you disable Double-Click events (either globally or for a specific funtion only), upon doubleclick/doublelongpress the radio will 
 *  behave the same as with simple click/longpress.
 */

#define DEFAULT_VOLUME     45 // change it for your favorite sound volume (only used, if EEPROM is erased)

#define DEFAULT_BAND lastBand // default band to switch to, if no last station info is stored to EEPROM. 
                              //   -must be a valid index into the band table band[] (see below)
                              //   -can be set to "lastBand" (w/o the quotation marks!) to address the last band defined in the table

#define DISPLAY_OLDSTYLE   0 // Set to != 0 to use old style display
                             //  - Old style display: in BFO-Mode, the frequency is inverted. For every other function, the
                             //       function name preceeding the changing parameter is inverted (but not the parameter itself)
                             //  - New style display: both function name and parameter are inverted. Also for BFO setting
                             //       (in BFO-Mode frequency stays in not inverted display) 

#define RDS_OFF            0 // Set to 0/1, if RDS should be enabled/disabled for FM at startup (RDS can be toggled ON/OFF in 
                             // FM mode by shortpress on "Mode"-Button later)

#define ENCODER_ENTER      1 // If defined != 0, and any command is selected, Shortpress on Encoder will end the currently selected 
                             // command immediately (before default timeout of 4 seconds)
                                     
#define ENCODER_SEARCH     1 // If defined != 0, shortpress on encoder will start search (in AM/FM-Mode) (overriden by ENCODER_ENTER
                             // if applicable)
                                     
#define ENCODER_MUTEDELAY  2 // Controls how long the encoder button needs to be longpressed for Mute functionality:
                             //    -must be uint8_t (i. e. between 0 and 255)
                             //    -if Zero, Encoder longpresses will be ignored (treated same as shortpress)
                             //    -any other number 'x' specifies the timeout to be (roughly, in ms)
                             //       BUTTONTIME_LONGPRESS1 + x * BUTTONTIME_LONGPRESSREPEAT i. e.
                             //       320 + x * 48 if you did not change the default settings in "SimpleButton.h"
                             //    - with a setting of 2, this is roughly 400 ms

#define ENCODERMODE_DELAY  1 // Controls "EncoderSimulationMode" ("ESM"):
                             //    -if enabled (>0), "EncoderSimulationMode" can be toggled by a longpress on button "Mode"
                             //    -in "ESM" a short/longpress to Band+ and Band- will be the same as rotating the encoder
                             //       right/left
                             //    -must be uint8_t (i. e. between 0 and 255)
                             //    -to disable "ESM", set this to Zero (0)
                             //    -any other number 'x' specifies (in ms), how long "Mode" must be pressed to toggle EncoderMode
                             //       x * BUTTONTIME_LONGPRESSREPEAT , with the first change happening after BUTTONTIME_LONGPRESS1
                             //    -with a setting of 2, this is roughly 400 ms 
                             //    -in "ESM":
                             //       - The encoder is not disabled, but simulated by buttons in addition
                             //       - Click or Longpress on AGC will be like pressing the Encoder
                             //       - Click or Longpress on "Band+"/"Band-" will be the same as rotating the encoder
                             //       - DoubleClick (instead of click) on "Band+" will start "Band-Change-Function" for 4 secs
                             //       - DoubleClick (instead of click) on "Band-" will start "SoftMute-Function" for 4 secs
                             //        -DoubleClick/Longclick on "AGC" allows for AGC-Settings

#define ENCODERMODE_SLOWDOWN 0 // Controls how fast repeated longpress on "Mode+"/"Mode-" will be translated into Encoder rotations
                               //   - must be uint_8, i. e. between 0 and 255
                               //   - time between simulated encoder rotations is (x + 1) * BUTTONTIME_LONGPRESSREPEAT (defined in 
                               //     "SimpleButton.h", defaults to roughly 50 ms)              
              
#define BAND_DELAY        2  // Controls how long the BAND+/- buttons need to be longpressed for functionality:
                             //    -must be uint8_t (i. e. between 0 and 255)
                             //    -if Zero, longpresses will be ignored (will be same as shortpress)
                             //    -any other number 'x' specifies the timeout (in ms) between Band changes as
                             //       x * BUTTONTIME_LONGPRESSREPEAT , with the first change happening after BUTTONTIME_LONGPRESS1
                             //    -with a setting of 2, this is roughly not below 100 ms 
                             //      (however, since band switching/scree updating consumes some time, it is OK to leave at 1)
                             //     (if you did not change the default settings in "SimpleButton.h")

#define BAND_2CLICKENABLE 1  // If defined != 0:
                             //     - DOUBLE-CLICK on "Band+"-button will toggle bandIdx between first and last band

#define BANDSWITCH1STEP_DELAY 1  // Controls "Next/Previous-Band" function:
                                 //    - must be uint8_t (i. e. between 0 and 255)
                                 //    - if set to "0", function is disabled.
                                 //    - if function is enabled, a short click on Band+ or Band- followed by a immediate longpress ("long
                                 //      doubleclick") will switch to Next or Previous Band respectively with wrapping around
                                 //    -any other number 'x' specifies how long the longpress following the first click must be:
                                 //       BUTTONTIME_LONGPRESS1 + x * BUTTONTIME_LONGPRESSREPEAT 
                                 //    -with a setting of 1, this is roughly 400 ms 
                                 //    -if enabled, function is anabled in both "EncoderMode" and "Normal Mode"

#define SOFTMUTE_2CLICKENABLE 1  // If defined != 0:
                                 //     - DOUBLE-CLICK on SoftMute("Band-")-button will toggle between min/max Softmute


#define VOLUME_DELAY          1  // Controls how long the Vol+/- buttons need to be longpressed for functionality:
                                 //    -must be uint8_t (i. e. between 0 and 255)
                                 //    -if Zero, longpresses will be ignored (will be same as shortpress)
                                 //    -any other number 'x' specifies the timeout (in ms) between value changes as
                                 //       x * BUTTONTIME_LONGPRESSREPEAT , with the first change happening after BUTTONTIME_LONGPRESS1
                                 //    -with a setting of 1 (the best IMHO), this is roughly 50 ms 
                                 //     (if you did not change the default settings in "SimpleButton.h")
                                     
#define AVC_2CLICKENABLE    1  // If defined != 0:
                               //     - DOUBLE-CLICK on AVC(==Vol-)-button will toggle betwin Min, Default and Max AVC (12/38/90)
                               //     - DOUBLE-LONGCLICK AVC(==Vol-)-button will "glide" between Min/Max AVC (direction changed at any press)
                               //     - the number defines the timeout between value changes (in ms) as 
                               //       x * BUTTONTIME_LONGPRESSREPEAT , with the first change happening after BUTTONTIME_LONGPRESS1
                               //     - with a setting of 1 (the best IMHO), this is roughly 50 ms 
                               //       (if you did not change the default settings in "SimpleButton.h")


#define STEP_DELAY          6  // Controls how long the Step button needs to be longpressed for functionality:
                               //    -must be uint8_t (i. e. between 0 and 255)
                               //    -if Zero, longpresses will be ignored (will be same as shortpress)
                               //    -any other number 'x' specifies the timeout (in ms) between value changes as
                               //       x * BUTTONTIME_LONGPRESSREPEAT , with the first change happening after BUTTONTIME_LONGPRESS1
                               //    -with a setting of 6, this is roughly 300 ms 
                               //     (if you did not change the default settings in "SimpleButton.h")

#define STEP_2CLICKENABLE   1  // If defined != 0:
                               //     - DOUBLE-CLICK on step-button will toggle between min/max step
                               //     - DOUBLE-LONGCLICK on step-button will set default step


#define BANDWIDTH_DELAY     9  // Controls how long the Bandwidth button needs to be longpressed for functionality:
                               //    -must be uint8_t (i. e. between 0 and 255)
                               //    -if Zero, longpresses will be ignored (will be same as shortpress)
                               //    -any other number 'x' specifies the timeout (in ms) between value changes as
                               //       x * BUTTONTIME_LONGPRESSREPEAT , with the first change happening after BUTTONTIME_LONGPRESS1
                               //    -with a setting of 9, this is roughly 450 ms
                               //     (if you did not change the default settings in "SimpleButton.h")
                                     
#define BANDWIDTH_2CLICKENABLE 1  // If defined != 0:
                                  //     - DOUBLE-CLICK on bandwidth-button will toggle between min/max bandwidth idx
                                  //     - DOUBLE-LONGCLICK on bw-button will set default bandwidth idx
                                     
#define AGC_DELAY              1  // Controls how long the AGC button needs to be longpressed for functionality:
                                  //    -must be uint8_t (i. e. between 0 and 255)
                                  //    -if Zero, longpresses will be ignored (will be same as shortpress)
                                  //    -any other number 'x' specifies the timeout (in ms) between value changes as
                                  //       x * BUTTONTIME_LONGPRESSREPEAT , with the first change happening after BUTTONTIME_LONGPRESS1
                                  //    -with a setting of 1, this is roughly 50 ms 
                                  //     (if you did not change the default settings in "SimpleButton.h")

#define AGC_2CLICKENABLE    1  // If defined != 0:
                               //     - DOUBLE-CLICK on AGC-button will toggle between Min and Max Attenuation (12/90)
                               //     - DOOUBLE_LONGCLICK on AGC-Button will set AGC


// Pin definitions

// Buttons controllers
#define MODE_BUTTON 4      // Switch MODE (Am/LSB/USB)
#define BANDWIDTH_BUTTON 5 // Used to select the banddwith.
#define VOLUMEUP_BUTTON 6    // Volume Up
#define VOLUMEDN_BUTTON 7     // **** Use thi button to implement a new function
#define BANDUP_BUTTON 8      // Next band
#define BANDDN_BUTTON 9     // **** Use thi button to implement a new function
#define AGC_BUTTON 11      // Switch AGC ON/OF
#define STEP_BUTTON 10     // Used to select the increment or decrement frequency step (see tabStep)
#define ENCODER_BUTTON 14  // Used to select the enconder control (BFO or VFO) and SEEK function on AM and FM modes

// OLED Diaplay constants
#define RST_PIN -1 // Define proper RST_PIN if required.

#define RESET_PIN 12

// Enconder PINs - if the clockwise and counterclockwise directions are not correct for you, please, invert this settings.
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3


#endif
