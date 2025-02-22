/*
 
  It is a sketch to control a receiver based on SI4735 with an I2C OLED device and 9 buttons. 

  This sketch works on DVE (David/Martins) KIT. See: https://davidmartinsengineering.wordpress.com/si4735-radio-kit/
  If you are using the DVE kit, set the OLED I2C bus address to 0X3D (see I2C_ADDRESS defined constan below)

  SI4735 all in one with SSB Support
  It is important to know the SSB support works on SI4735-D60 and SI4732-A10 devices. 

  This sketch has been successfully tested on:
  1) Pro Mini 3.3V;
  2) UNO (by using a voltage converter);
  3) Arduino Yún (by using a voltage converter);
  4) Arduino Micro (see the operating voltage of your Micro);
  5) Arduino Mega (by using a voltage converter); and

  This sketch uses I2C OLED/I2C, buttons and  Encoder.

  This sketch uses the Rotary Encoder Class implementation from Ben Buxton (the source code is included
  together with this sketch) and Tiny4kOLED Library (look for this library on Tools->Manage Libraries). 

  ABOUT DIGITAL pin 13 and INPUT PULL-UP on Arduino Pro Mini, UNO or similar:
  This pin has a LED and a resistor connected on the board. When this pin is set to HIGH the LED comes on. 
  If you use the internal pull-up resistor of the pin 13, you might experiment problem due to the drop voltage 
  caused by the LED circuit. If this occurs in your project you can do:
  1. use the pin 14. This pin is the A0 (Analog). But you have to refer it by 14 to use it as a digital pin (just change 13 by 14 on the sketch examples); 
  2. change the circuit and sketch to use external pull-up on pin 13;
  3. remove the LED or resitor connected to the led from the board (caution). 


  The  purpose  of  this  example  is  to  demonstrate a prototype  receiver based  on  the  SI4735-D60 or Si4732-A10  and  the
  "PU2CLR SI4735 Arduino Library". It is not the purpose of this prototype  to provide you a beautiful interface. You can do it better.


  ABOUT SSB PATCH:  
  This sketch will download a SSB patch to your SI4735-D60 or SI4732-A10 devices (patch_init.h). It will take about 8KB of the Arduino memory.

  In this context, a patch is a piece of software used to change the behavior of the SI4735 device.
  There is little information available about patching the SI4735. The following information is the understanding of the author of
  this project and it is not necessarily correct. A patch is executed internally (run by internal MCU) of the device.
  Usually, patches are used to fixes bugs or add improvements and new features of the firmware installed in the internal ROM of the device.
  Patches to the SI4735 are distributed in binary form and have to be transferred to the internal RAM of the device by
  the host MCU (in this case Arduino). Since the RAM is volatile memory, the patch stored into the device gets lost when you turn off the system.
  Consequently, the content of the patch has to be transferred again to the device each time after turn on the system or reset the device.

  ATTENTION: The author of this project does not guarantee that procedures shown here will work in your development environment.
  Given this, it is at your own risk to continue with the procedures suggested here.
  This library works with the I2C communication protocol and it is designed to apply a SSB extension PATCH to CI SI4735-D60.
  Once again, the author disclaims any liability for any damage this procedure may cause to your SI4735 or other devices that you are using.

  Features of this sketch:

  1) FM, AM (MW and SW) and SSB (LSB and USB);
  2) Audio bandwidth filter 0.5, 1, 1.2, 2.2, 3 and 4kHz;
  3) 22 commercial and ham radio bands pre configured;
  4) BFO Control; and
  5) Frequency step switch (1, 5 and 10kHz);

  Main Parts:
  Encoder with push button;
  Seven bush buttons;
  OLED Display with I2C device;
  Arduino Pro mini 3.3V;

  Prototype documentation: https://pu2clr.github.io/SI4735/
  PU2CLR Si47XX API documentation: https://pu2clr.github.io/SI4735/extras/apidoc/html/

  By Ricardo Lima Caratti, Nov 2019.
*/

#include <SI4735.h>
#include <Tiny4kOLED.h>
#include "Rotary.h"

#include "patch_init.h" // SSB patch for whole SSBRX initialization string

const uint16_t size_content = sizeof ssb_patch_content; // see ssb_patch_content in patch_full.h or patch_init.h

#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

// OLED Diaplay constants 
#define I2C_ADDRESS 0x3C  // Check your I2C bus address (0X3D is also very commom) 
#define RST_PIN -1        // Define proper RST_PIN if required.

#define RESET_PIN 12

// Enconder PINs
#define ENCODER_PIN_A 3
#define ENCODER_PIN_B 2

// Buttons controllers
#define MODE_SWITCH 4      // Switch MODE (Am/LSB/USB)
#define BANDWIDTH_BUTTON 5 // Used to select the banddwith. Values: 1.2, 2.2, 3.0, 4.0, 0.5, 1.0 kHz
#define VOL_UP 6           // Volume Up
#define VOL_DOWN 7         // Volume Down
#define BAND_BUTTON_UP 8   // Next band
#define BAND_BUTTON_DOWN 9 // Previous band
#define AGC_SWITCH 11      // Switch AGC ON/OF
#define STEP_SWITCH 10     // Used to select the increment or decrement frequency step (1, 5 or 10 kHz)
// #define BFO_SWITCH 13      // Used to select the enconder control (BFO or VFO)
#define BFO_SWITCH 14 // A0 (Alternative to the pin 13). Used to select the enconder control (BFO or VFO)

#define MIN_ELAPSED_TIME 100
#define MIN_ELAPSED_RSSI_TIME 150
#define DEFAULT_VOLUME 45 // change it for your favorite sound volume

#define FM 0
#define LSB 1
#define USB 2
#define AM 3
#define LW 4

#define SSB 1

const char *bandModeDesc[] = {"FM ", "LSB", "USB", "AM "};
uint8_t currentMode = FM;

bool bfoOn = false;
bool ssbLoaded = false;
bool fmStereo = true;

int currentBFO = 0;
long elapsedRSSI = millis();
long elapsedButton = millis();

// Encoder control variables
volatile int encoderCount = 0;

// Some variables to check the SI4735 status
uint16_t currentFrequency;
uint16_t previousFrequency;
uint8_t currentStep = 1;
uint8_t currentBFOStep = 25;

uint8_t bwIdxSSB = 2;
const char *bandwidthSSB[] = {"1.2", "2.2", "3.0", "4.0", "0.5", "1.0"};

uint8_t bwIdxAM = 1;
const char *bandwidthAM[] = {"6", "4", "3", "2", "1", "1.8", "2.5"};

// Atenuação and AGC
uint8_t agcIdx = 0;
uint8_t disableAgc = 0;
uint8_t agcNdx = 0;

/*
   Band data structure
*/
typedef struct
{
  uint8_t bandType;     // Band type (FM, MW or SW)
  uint16_t minimumFreq; // Minimum frequency of the band
  uint16_t maximumFreq; // maximum frequency of the band
  uint16_t currentFreq; // Default frequency or current frequency
  uint16_t currentStep; // Defeult step (increment and decrement)
} Band;

/*
 *  Band table
 */
Band band[] = {
    {FM_BAND_TYPE, 8400, 10800, 10570, 10},
    {LW_BAND_TYPE, 100, 510, 300, 1},
    {MW_BAND_TYPE, 520, 1720, 810, 10},
    {SW_BAND_TYPE, 1800, 3500, 1900, 1}, // 160 meters
    {SW_BAND_TYPE, 3500, 4500, 3700, 1}, // 80 meters
    {SW_BAND_TYPE, 4500, 5500, 4850, 5},
    {SW_BAND_TYPE, 5600, 6300, 6000, 5},
    {SW_BAND_TYPE, 6800, 7800, 7200, 5}, // 40 meters
    {SW_BAND_TYPE, 9200, 10000, 9600, 5},
    {SW_BAND_TYPE, 10000, 11000, 10100, 1}, // 30 meters
    {SW_BAND_TYPE, 11200, 12500, 11940, 5},
    {SW_BAND_TYPE, 13400, 13900, 13600, 5},
    {SW_BAND_TYPE, 14000, 14500, 14200, 1}, // 20 meters
    {SW_BAND_TYPE, 15000, 15900, 15300, 5},
    {SW_BAND_TYPE, 17200, 17900, 17600, 5},
    {SW_BAND_TYPE, 18000, 18300, 18100, 1},  // 17 meters
    {SW_BAND_TYPE, 21000, 21900, 21200, 1},  // 15 mters
    {SW_BAND_TYPE, 24890, 26200, 24940, 1},  // 12 meters
    {SW_BAND_TYPE, 26200, 27900, 27500, 1},  // CB band (11 meters)
    {SW_BAND_TYPE, 28000, 30000, 28400, 1}}; // 10 meters

const int lastBand = (sizeof band / sizeof(Band)) - 1;
int bandIdx = 0;

uint8_t rssi = 0;
uint8_t stereo = 1;
uint8_t volume = DEFAULT_VOLUME;

// Devices class declarations
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
SI4735 si4735;

void setup()
{
  // Encoder pins
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  pinMode(BANDWIDTH_BUTTON, INPUT_PULLUP);
  pinMode(BAND_BUTTON_UP, INPUT_PULLUP);
  pinMode(BAND_BUTTON_DOWN, INPUT_PULLUP);
  pinMode(VOL_UP, INPUT_PULLUP);
  pinMode(VOL_DOWN, INPUT_PULLUP);
  pinMode(BFO_SWITCH, INPUT_PULLUP);
  pinMode(AGC_SWITCH, INPUT_PULLUP);
  pinMode(STEP_SWITCH, INPUT_PULLUP);
  pinMode(MODE_SWITCH, INPUT_PULLUP);

  oled.begin();
  oled.clear();
  oled.on();
  oled.setFont(FONT6X8);

  // Splash - Change it for your introduction text.
  oled.setCursor(40, 0);
  oled.print("SI4735");
  oled.setCursor(20, 1);
  oled.print("Arduino Library");
  delay(500);
  oled.setCursor(15, 2);
  oled.print("All in One Radio");
  delay(500);
  oled.setCursor(10, 3);
  oled.print("V1.1.5 - By PU2CLR");
  delay(5000);
  // end Splash

  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  // Uncomment the lines below if you experience some unstable behaviour. Default values were optimized to make the SSB patch load fast
  // si4735.setMaxDelayPowerUp(500);      // Time to the external crystal become stable after power up command (default is 10ms).
  // si4735.setMaxDelaySetFrequency(100); // Time needed to process the next frequency setup (default is 30 ms)

  si4735.getDeviceI2CAddress(RESET_PIN); // Looks for the I2C bus address and set it.  Returns 0 if error

  si4735.setup(RESET_PIN, FM_BAND_TYPE);

  delay(300);
  // Set up the radio for the current band (see index table variable bandIdx )
  useBand();

  currentFrequency = previousFrequency = si4735.getFrequency();

  si4735.setVolume(volume);
  oled.clear();
  showStatus();
}

// Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
void rotaryEncoder()
{ // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus)
  {
    if (encoderStatus == DIR_CW)
    {
      encoderCount = 1;
    }
    else
    {
      encoderCount = -1;
    }
  }
}

void cleanBfoRdsInfo()
{
  oled.setCursor(0, 2);
  oled.print("                    ");
}

// Show current frequency

void showFrequency()
{
  String freqDisplay;
  String unit;
  String bandMode;
  int divider = 1;
  int decimals = 3;
  if (band[bandIdx].bandType == FM_BAND_TYPE)
  {
    divider = 100;
    decimals = 1;
    unit = "MHz";
  }
  else if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE)
  {
    divider = 1;
    decimals = 0;
    unit = "kHz";
  }
  else
  {
    divider = 1000;
    decimals = 3;
    unit = "kHz";
  }

  if (!bfoOn)
    freqDisplay = String((float)currentFrequency / divider, decimals);
  else
    freqDisplay = ">" + String((float)currentFrequency / divider, decimals) + "<";

  oled.setCursor(38, 0);
  oled.print("        ");
  oled.setCursor(38, 0);
  oled.print(freqDisplay);

  if (currentFrequency < 520)
    bandMode = "LW  ";
  else
    bandMode = bandModeDesc[currentMode];

  oled.setCursor(0, 0);
  oled.print(bandMode);

  oled.setCursor(95, 0);
  oled.print(unit);
}

/*
    Show some basic information on display
*/
void showStatus()
{

  showFrequency();

  oled.setCursor(80, 1);
  oled.print("      ");
  oled.setCursor(80, 1);
  oled.print("St: ");
  oled.print(currentStep);

  oled.setCursor(0, 3);
  oled.print("           ");
  oled.setCursor(0, 3);

  if (currentMode == LSB || currentMode == USB)
  {
    oled.print("BW:");
    oled.print(String(bandwidthSSB[bwIdxSSB]));
    oled.print("kHz");
    showBFO();
  }
  else if (currentMode == AM)
  {
    oled.print("BW:");
    oled.print(String(bandwidthAM[bwIdxAM]));
    oled.print("kHz");
  }

  // Show AGC Information
  // si4735.getAutomaticGainControl();
  oled.setCursor(0, 1);
  if (agcIdx == 0 ) {
    oled.print("AGC ON");
  } else {
    oled.print("ATT: ");
    oled.print(agcNdx);
  }

  showRSSI();
  showVolume();
}

/* *******************************
   Shows RSSI status
*/
void showRSSI()
{
  char c[2] = ">";

  int bars = ((rssi / 10.0) / 2.0) + 1;

  oled.setCursor(80, 3);
  oled.print("       ");
  oled.setCursor(80, 3);
  oled.print("S:");
  if (bars > 5)
  {
    bars = 5;
    c[0] = '+';
  }
  for (int i = 0; i < bars; i++)
    oled.print(c);

  if (currentMode == FM)
  {
    oled.setCursor(0, 3);
    oled.print((si4735.getCurrentPilot()) ? "STEREO   " : "MONO     ");
  }
}

/*
   Shows the volume level on LCD
*/
void showVolume()
{
  oled.setCursor(60, 3);
  oled.print("  ");
  oled.setCursor(60, 3);
  oled.print(si4735.getCurrentVolume());
}

/*
   Shows the BFO current status.
   Must be called only on SSB mode (LSB or USB)
*/
void showBFO()
{

  String bfo;

  if (currentBFO > 0)
    bfo = "+" + String(currentBFO);
  else
    bfo = String(currentBFO);

  oled.setCursor(0, 2);
  oled.print("         ");
  oled.setCursor(0, 2);
  oled.print("BFO:");
  oled.print(bfo);
  oled.print("Hz ");

  oled.setCursor(80, 2);
  oled.print("       ");
  oled.setCursor(80, 2);
  oled.print("St: ");
  oled.print(currentBFOStep);
}

char *rdsMsg;
char *stationName;
char *rdsTime;
char bufferStatioName[50];
char bufferRdsMsg[100];
char bufferRdsTime[32];

void showRDSMsg()
{
  rdsMsg[35] = bufferRdsMsg[35] = '\0';
  if (strcmp(bufferRdsMsg, rdsMsg) == 0)
    return;
  delay(250);
}

void showRDSStation()
{
  // if (strcmp(bufferStatioName, stationName) == 0 ) return;
  // printValue(5, 110,bufferStatioName, stationName, COLOR_GREEN, 6);
  cleanBfoRdsInfo();
  oled.setCursor(0, 2);
  oled.print(stationName);
  // strcpy(bufferStatioName, stationName);
  delay(250);
}

void showRDSTime()
{

  if (strcmp(bufferRdsTime, rdsTime) == 0)
    return;
  // printValue(80, 110, bufferRdsTime, rdsTime, COLOR_GREEN, 6);
  delay(250);
}

void checkRDS()
{

  // tft.setFont(Terminal6x8);

  si4735.getRdsStatus();
  if (si4735.getRdsReceived())
  {
    if (si4735.getRdsSync() && si4735.getRdsSyncFound())
    {
      rdsMsg = si4735.getRdsText2A();
      stationName = si4735.getRdsText0A();
      rdsTime = si4735.getRdsTime();
      // if ( rdsMsg != NULL )   showRDSMsg();
      if (stationName != NULL)
        showRDSStation();
      // if ( rdsTime != NULL ) showRDSTime();
    }
  }
}

/*
   Goes to the next band (see Band table)
*/
void bandUp()
{
  // save the current frequency for the band
  band[bandIdx].currentFreq = currentFrequency;
  band[bandIdx].currentStep = currentStep;

  if (bandIdx < lastBand)
  {
    bandIdx++;
  }
  else
  {
    bandIdx = 0;
  }
  useBand();
}

/*
   Goes to the previous band (see Band table)
*/
void bandDown()
{
  // save the current frequency for the band
  band[bandIdx].currentFreq = currentFrequency;
  band[bandIdx].currentStep = currentStep;
  if (bandIdx > 0)
  {
    bandIdx--;
  }
  else
  {
    bandIdx = lastBand;
  }
  useBand();
}

/*
   This function loads the contents of the ssb_patch_content array into the CI (Si4735) and starts the radio on
   SSB mode.
*/
void loadSSB()
{
  oled.setCursor(0, 2);
  oled.print("  Switching to SSB  ");

  si4735.reset();
  si4735.queryLibraryId(); // Is it really necessary here?  Just powerDown() maigh work!
  si4735.patchPowerUp();
  delay(50);
  si4735.setI2CFastMode(); // Recommended
  // si4735.setI2CFastModeCustom(500000); // It is a test and may crash.
  si4735.downloadPatch(ssb_patch_content, size_content);
  si4735.setI2CStandardMode(); // goes back to default (100kHz)
  cleanBfoRdsInfo();

  // delay(50);
  // Parameters
  // AUDIOBW - SSB Audio bandwidth; 0 = 1.2kHz (default); 1=2.2kHz; 2=3kHz; 3=4kHz; 4=500Hz; 5=1kHz;
  // SBCUTFLT SSB - side band cutoff filter for band passand low pass filter ( 0 or 1)
  // AVC_DIVIDER  - set 0 for SSB mode; set 3 for SYNC mode.
  // AVCEN - SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
  // SMUTESEL - SSB Soft-mute Based on RSSI or SNR (0 or 1).
  // DSP_AFCDIS - DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
  si4735.setSSBConfig(bwIdxSSB, 1, 0, 0, 0, 1);
  delay(25);
  ssbLoaded = true;
  oled.clear();
}

/*
   Switch the radio to current band.
   The bandIdx variable points to the current band. 
   This function change to the band referenced by bandIdx (see table band).
*/
void useBand()
{
  cleanBfoRdsInfo();
  if (band[bandIdx].bandType == FM_BAND_TYPE)
  {
    currentMode = FM;
    si4735.setTuneFrequencyAntennaCapacitor(0);
    si4735.setFM(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, band[bandIdx].currentStep);
    bfoOn = ssbLoaded = false;
    si4735.setRdsConfig(1, 2, 2, 2, 2);
  }
  else
  {
    if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE)
      si4735.setTuneFrequencyAntennaCapacitor(0);
    else
      si4735.setTuneFrequencyAntennaCapacitor(1);

    if (ssbLoaded)
    {
      si4735.setSSB(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, band[bandIdx].currentStep, currentMode);
      si4735.setSSBAutomaticVolumeControl(1);
      si4735.setSsbSoftMuteMaxAttenuation(0); // Disable Soft Mute for SSB
    }
    else
    {
      currentMode = AM;
      si4735.setAM(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, band[bandIdx].currentStep);
      si4735.setAutomaticGainControl(disableAgc, agcNdx);
      si4735.setAmSoftMuteMaxAttenuation(0); // // Disable Soft Mute for AM
      bfoOn = false;
    }
  }
  delay(100);
  currentFrequency = band[bandIdx].currentFreq;
  currentStep = band[bandIdx].currentStep;
  showStatus();
}

void loop()
{
  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    if (bfoOn)
    {
      currentBFO = (encoderCount == 1) ? (currentBFO + currentBFOStep) : (currentBFO - currentBFOStep);
      si4735.setSSBBfo(currentBFO);
      showBFO();
    }
    else
    {
      if (encoderCount == 1)
        si4735.frequencyUp();
      else
        si4735.frequencyDown();

      // Show the current frequency only if it has changed
      currentFrequency = si4735.getFrequency();
    }
    encoderCount = 0;
  }

  // Check button commands
  if ((millis() - elapsedButton) > MIN_ELAPSED_TIME)
  {
    // check if some button is pressed
    if (digitalRead(BANDWIDTH_BUTTON) == LOW)
    {
      if (currentMode == LSB || currentMode == USB)
      {
        bwIdxSSB++;
        if (bwIdxSSB > 5)
          bwIdxSSB = 0;
        si4735.setSSBAudioBandwidth(bwIdxSSB);
        // If audio bandwidth selected is about 2 kHz or below, it is recommended to set Sideband Cutoff Filter to 0.
        if (bwIdxSSB == 0 || bwIdxSSB == 4 || bwIdxSSB == 5)
          si4735.setSBBSidebandCutoffFilter(0);
        else
          si4735.setSBBSidebandCutoffFilter(1);
      }
      else if (currentMode == AM)
      {
        bwIdxAM++;
        if (bwIdxAM > 6)
          bwIdxAM = 0;
        si4735.setBandwidth(bwIdxAM, 1);
      }
      showStatus();
      delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
    }
    else if (digitalRead(BAND_BUTTON_UP) == LOW)
      bandUp();
    else if (digitalRead(BAND_BUTTON_DOWN) == LOW)
      bandDown();
    else if (digitalRead(VOL_UP) == LOW)
    {
      si4735.volumeUp();
      volume = si4735.getVolume();
      showVolume();
      delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
    }
    else if (digitalRead(VOL_DOWN) == LOW)
    {
      si4735.volumeDown();
      volume = si4735.getVolume();
      showVolume();
      delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
    }
    else if (digitalRead(BFO_SWITCH) == LOW)
    {
      if (currentMode == LSB || currentMode == USB)
      {
        bfoOn = !bfoOn;
        if (bfoOn)
          showBFO();
        showStatus();
      }
      else if (currentMode == FM)
      {
        si4735.seekStationUp();
        delay(30);
        currentFrequency = si4735.getFrequency();
      }
      delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
    }
    else if (digitalRead(AGC_SWITCH) == LOW)
    {
      if (agcIdx == 0)
      {
        disableAgc = 0; // Turns AGC ON
        agcNdx = 0;
        agcIdx = 1;
      } else if (agcIdx == 1)
      {
        disableAgc = 1; // Turns AGC OFF
        agcNdx = 0;     // Sets minimum attenuation
        agcIdx = 2;
      } else if (agcIdx == 2)
      {
        disableAgc = 1; // Turns AGC OFF
        agcNdx = 10;    // Increases the attenuation AM/SSB AGC Index  = 10
        agcIdx = 3;
      } else if (agcIdx == 3)
      {
        disableAgc = 1; // Turns AGC OFF
        agcNdx = 20;    // Increases the attenuation AM/SSB AGC Index  = 20
        agcIdx = 4;
      } else if (agcIdx == 4)
      {
        disableAgc = 1; // Turns AGC OFF
        agcNdx = 36;    // Sets maximum attenuation
        agcIdx = 0;
      }
      // Sets AGC on/off an gain
      si4735.setAutomaticGainControl(disableAgc, agcNdx);
      showStatus();
    }
    else if (digitalRead(STEP_SWITCH) == LOW)
    {
      if (currentMode == FM)
      {
        fmStereo = !fmStereo;
        if (fmStereo)
          si4735.setFmStereoOn();
        else
          si4735.setFmStereoOff(); // It is not working so far.
      }
      else
      {

        // This command should work only for SSB mode
        if (bfoOn && (currentMode == LSB || currentMode == USB))
        {
          currentBFOStep = (currentBFOStep == 25) ? 10 : 25;
          showBFO();
        }
        else
        {
          if (currentStep == 1)
            currentStep = 5;
          else if (currentStep == 5)
            currentStep = 10;
          else
            currentStep = 1;
          si4735.setFrequencyStep(currentStep);
          band[bandIdx].currentStep = currentStep;
          showStatus();
        }
        delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
      }
    }
    else if (digitalRead(MODE_SWITCH) == LOW)
    {
      if (currentMode != FM)
      {
        if (currentMode == AM)
        {
          // If you were in AM mode, it is necessary to load SSB patch (avery time)
          loadSSB();
          currentMode = LSB;
        }
        else if (currentMode == LSB)
        {
          currentMode = USB;
        }
        else if (currentMode == USB)
        {
          currentMode = AM;
          ssbLoaded = false;
          bfoOn = false;
        }
        // Nothing to do if you are in FM mode
        band[bandIdx].currentFreq = currentFrequency;
        band[bandIdx].currentStep = currentStep;
        useBand();
      }
    }
    elapsedButton = millis();
  }

  // Show RSSI status only if this condition has changed
  if ((millis() - elapsedRSSI) > MIN_ELAPSED_RSSI_TIME * 9)
  {
    si4735.getCurrentReceivedSignalQuality();
    int aux = si4735.getCurrentRSSI();
    if (rssi != aux)
    {
      rssi = aux;
      showRSSI();
    }
    elapsedRSSI = millis();
  }

  if (currentMode == FM)
  {
    if (currentFrequency != previousFrequency)
    {
      cleanBfoRdsInfo();
    }
    checkRDS();
  }

  // Show the current frequency only if it has changed
  if (currentFrequency != previousFrequency)
  {
    previousFrequency = currentFrequency;
    showFrequency();
  }

  delay(10);
}
