/*
  This sketch SHOULD work with the Chinese KIT sold on AliExpress, eBay and Amazon
  The author of this sketch and Arduino Library does not know the seller of this kit and does not have a commercial relationship with any commercial product that uses the Arduino Library.
  It is important you understand that there is no guarantee that this sketch will work correctly in your current product.
  SO, DO NOT TRY IT IF YOU DON'T KNOW WHAT ARE YOU DOING. YOU MUST BE ABLE TO GO BACK TO THE PREVIOUS VERSION IF THIS SKETCH DOES NOT WORK FOR YOU.

  The user interface and commands are different if compared with the original sketch.
  Please, read the user_manual.txt for more details about this sketch before updating your receiver.
  PLEASE, READ THE user_manual.txt FOR MORE DETAILS ABOUT THIS SKETCH BEFORE UPDATING YOUR RECEIVER.

  ATTENTION: Turn your receiver on with the encoder push button pressed at first time to RESET the eeprom content.
  ARDUINO LIBRARIES:
  1) This sketch uses the Rotary Encoder Class implementation from Ben Buxton (the source code is included together with this sketch). You do not need to install it;
  2) Tiny4kOLED Library and TinyOLED-Fonts (on your Arduino IDE, look for this library on Tools->Manage Libraries).
  3) PU2CLR SI4735 Arduino Library (on your Arduino IDE look for this library on Tools->Manage Libraries).

  ABOUT THE EEPROM:

  ATMEL says the lifetime of an EEPROM memory position is about 100,000 writes.
  For this reason, this sketch tries to avoid save unnecessary writes into the eeprom.
  So, the condition to store any status of the receiver is changing the frequency,  bandwidth, volume, band or step  and 10 seconds of inactivity.
  For example, if you switch the band and turn the receiver off immediately, no new information will be written into the eeprom.
  But you wait 10 seconds after changing anything, all new information will be written.

  ABOUT SSB PATCH:

  First of all, it is important to say that the SSB patch content is not part of this library. The paches used here were made available by Mr.
  Vadim Afonkin on his Dropbox repository. It is important to note that the author of this library does not encourage anyone to use the SSB patches
  content for commercial purposes. In other words, this library only supports SSB patches, the patches themselves are not part of this library.

  In this context, a patch is a piece of software used to change the behavior of the SI4735 device.
  There is little information available about patching the SI4735 or Si4732 devices. The following information is the understanding of the author of
  this project and it is not necessarily correct. A patch is executed internally (run by internal MCU) of the device.
  Usually, patches are used to fixes bugs or add improvements and new features of the firmware installed in the internal ROM of the device.
  Patches to the SI473X are distributed in binary form and have to be transferred to the internal RAM of the device by
  the host MCU (in this case Arduino). Since the RAM is volatile memory, the patch stored into the device gets lost when you turn off the system.
  Consequently, the content of the patch has to be transferred again to the device each time after turn on the system or reset the device.

  Wire up on Arduino UNO, Pro mini and SI4735-D60

  | Device name               | Device Pin / Description      |  Arduino Pin  |
  | ----------------          | ----------------------------- | ------------  |
  | Display OLED              |                               |               |
  |                           | SDA                           |     A4        |
  |                           | CLK                           |     A5        |
  |     (*1) SI4735           |                               |               |
  |                           | RESET (pin 15)                |     12        |
  |                           | SDIO (pin 18)                 |     A4        |
  |                           | SCLK (pin 17)                 |     A5        |
  |                           | SEN (pin 16)                  |    GND        |
  |     (*2) Buttons          |                               |               |
  |                           | Switch MODE (AM/LSB/AM)       |      4        |
  |                           | Banddwith                     |      5        |
  |                           | Volume                        |      6        |
  |                           | Custom button 1 (*3)          |      7        |
  |                           | Band                          |      8        |
  |                           | Custom button 2 (*3)          |      9        |
  |                           | Step                          |     10        |
  |                           | AGC / Attentuation            |     11        |
  |                           | VFO/VFO Switch (Encoder)      |     14 / A0   |
  |    Encoder (*4)           |                               |               |
  |                           | A                             |       2       |
  |                           | B                             |       3       |

   1 - You can use the SI4732-A10. Check on the SI4732 package the pins: RESET, SDIO, SCLK and SEN.
   2 - Please, read the file user_manual.txt for more detail.
   3 - You can remove this buttons from your circuit and sketch if you dont want to use them.
   4 - Some encoder devices have pins A and B inverted. So, if the clockwise and counterclockwise directions
       are not correct for you, please, invert the settings for pins A and B.

  Prototype documentation: https://pu2clr.github.io/SI4735/
  PU2CLR Si47XX API documentation: https://pu2clr.github.io/SI4735/extras/apidoc/html/

  By Ricardo Lima Caratti, April  2021.
*/

#include <SI4735.h>
#include <EEPROM.h>
#include <Tiny4kOLED.h>
#include <font8x16atari.h> // Please, install the TinyOLED-Fonts library
#include "Rotary.h"
#include "SimpleButton.h"
#include "Config.h"
#include "patch_ssb_compressed.h" // Compressed SSB patch version (saving almost 1KB)






const uint16_t size_content = sizeof ssb_patch_content; // See ssb_patch_content.h
const uint16_t cmd_0x15_size = sizeof cmd_0x15;         // Array of lines where the 0x15 command occurs in the patch content.

#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

#define MIN_ELAPSED_RSSI_TIME 1000



#define FM 0
#define LSB 1
#define USB 2
#define AM 3
#define LW 4

#define SSB 1

#define STORE_TIME 10000 // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).
                         // uint16_t, maximum value is 0xffff of (uint16_t)65535 (dec) meaning a bit more than 65 seconds

//const uint8_t app_id = 43; // Useful to check the EEPROM content before processing useful data
const uint8_t app_id = 44;

const int eeprom_address = 0;
uint16_t storeTime; // = millis();

//const char *bandModeDesc[] = {"FM ", "LSB", "USB", "AM "};
const char bandModeDesc[] = {'F', 'L', 'U', 'A'};
uint8_t currentMode = FM;
uint8_t seekDirection = 1;

uint16_t millis16 = 0;    // replacement for call to millis(), stores the time of starting the most recent loop.
                         // can control timeouts of up to 70 seconds, which is enough for the cases used here 

bool bfoOn = false;
bool encoderMode = false;

bool ssbLoaded = false;
bool fmStereo = true;
bool rdsOff = 0 != RDS_OFF;

bool cmdVolume = false;   // if true, the encoder will control the volume.
bool cmdAgcAtt = false;   // if true, the encoder will control the AGC / Attenuation
bool cmdStep = false;     // if true, the encoder will control the step frequency
bool cmdBw = false;       // if true, the encoder will control the bandwidth
bool cmdBand = false;     // if true, the encoder will control the band
bool cmdSoftMute = false; // if true, the encoder will control the Soft Mute attenuation
bool cmdAvc = false;      // if true, the encoder will control Automatic Volume Control
bool attDirty = false;    // if true, AGC/Att needs to be redrawn on screen


bool cmdAnyOn = false;    // only true, if any of the cmdXxxx-flags above is true

uint8_t muteVolume = 0;   // restore volume for "un-mute"

uint8_t countRSSI = 0;

int currentBFO = 0;

// Button handlers for push buttons/encoder
SimpleButton  btn_Bandwidth(BANDWIDTH_BUTTON);
SimpleButton  btn_BandUp(BANDUP_BUTTON);
SimpleButton  btn_BandDn(BANDDN_BUTTON);
SimpleButton  btn_VolumeUp(VOLUMEUP_BUTTON);
SimpleButton  btn_VolumeDn(VOLUMEDN_BUTTON);
SimpleButton  btn_Encoder(ENCODER_BUTTON);
SimpleButton  btn_AGC(AGC_BUTTON);
SimpleButton  btn_Step(STEP_BUTTON);
SimpleButton  btn_Mode(MODE_BUTTON);

void showEncoderMode(void);
void doBandwidth(int8_t v, bool show = true);
void doStep(int8_t v);
void doAttenuation(int8_t v);

uint16_t elapsedRSSI;


// Encoder control variables
volatile int encoderCount = 0;

// Some variables to check the SI4735 status
uint16_t currentFrequency;
uint16_t previousFrequency;
// uint8_t currentStep = 1;
uint8_t currentBFOStep = 25;

// Datatype to deal with bandwidth on AM, SSB and FM in numerical order.
// Ordering by bandwidth values.
typedef struct
{
  uint8_t idx;      // SI473X device bandwidth index value
  uint8_t desc;     // bandwidth description
} Bandwidth;


#define NUM_INTROLINES (sizeof(introlines) / sizeof(introlines[0]))

#define BW_DEFAULT_SSB          4
#define BW_DEFAULT_AM           4
#define BW_DEFAULT_FM           0

uint8_t bwIdxSSB = BW_DEFAULT_SSB;
Bandwidth bandwidthSSB[] = {
  {4,  5}, //0 == "0.5"
  {5, 10}, //1 == "1.0"
  {0, 12}, //2 == "1.2"
  {1, 22}, //3 == "2.2"
  {2, 30}, //4 == "3.0"  - default
  {3, 40}  //5 == "4.0"
};              // 3 = 4kHz

#define BW_MAX_SSB (sizeof(bandwidthSSB) / sizeof(bandwidthSSB[0]) - 1)

uint8_t bwIdxAM = BW_DEFAULT_AM;
//const int maxFilterAM = 6;
Bandwidth bandwidthAM[] = {
  {4, 10}, //0 == "1.0"
  {5, 18}, //1 == "1.8"
  {3, 20}, //2 == "2.0"
  {6, 25}, //3 == "2.5"
  {2, 30}, //4 == "3.0" - default
  {1, 40}, //5 == "4.0"
  {0, 60}  //6 == "6.0"
};

#define BW_MAX_AM (sizeof(bandwidthAM) / sizeof(bandwidthAM[0]) - 1)


uint8_t bwIdxFM = BW_DEFAULT_FM;
Bandwidth bandwidthFM[] = {
  {0,   0}, //0 == "AUT" // Automatic - default
  {1, 110}, //1 == "110"}, // Force wide (110 kHz) channel filter.
  {2,  84}, //2 == " 84"},
  {3,  60}, //3 == " 60"},
  {4,  40}  //4 == " 40"}
};

#define BW_MAX_FM (sizeof(bandwidthFM) / sizeof(bandwidthFM[0]) - 1)

int8_t* bwFlag;
int8_t bwMax;
int8_t bwDefault;


// Atenuação and AGC
int8_t agcIdx = 0;
uint8_t disableAgc = 0;
uint8_t agcNdx = 0;
int8_t smIdx = 8;
int8_t avcIdx = 38;

int tabStep[] = {1,    // 0
                 5,    // 1
                 9,    // 2
                 10,   // 3
                 50,   // 4
                 100
                }; // 5

const int lastStep = (sizeof tabStep / sizeof(int)) - 1;
int8_t idxStep = 3;

/*
   Band data structure
*/
typedef struct
{
  uint8_t bandType;        // Band type (FM, MW or SW)
  uint16_t minimumFreq;    // Minimum frequency of the band
  uint16_t maximumFreq;    // Maximum frequency of the band
  uint16_t currentFreq;    // Default frequency or current frequency
  uint16_t currentStepIdx; // Idex of tabStep:  Defeult frequency step (See tabStep)
  int8_t bandwidthIdx;    // Index of the table bandwidthFM, bandwidthAM or bandwidthSSB;
} Band;

/*
   Band table
   YOU CAN CONFIGURE YOUR OWN BAND PLAN. Be guided by the comments.
   To add a new band, all you have to do is insert a new line in the table below. No extra code will be needed.
   You can remove a band by deleting a line if you do not want a given band.
   Also, you can change the parameters of the band.
   ATTENTION: You have to RESET the eeprom after adding or removing a line of this table.
              Turn your receiver on with the encoder push button pressed at first time to RESET the eeprom content.
*/
Band band[] = {
  {LW_BAND_TYPE, 100, 510, 300, 0, 4},
  {MW_BAND_TYPE, 520, 1720, 810, 3, 4},       // AM/MW from 520 to 1720kHz; default 810kHz; default step frequency index is 3 (10kHz); default bandwidth index is 4 (3kHz)
  {MW_BAND_TYPE, 531, 1701, 783, 2, 4},       // MW for Europe, Africa and Asia
  {SW_BAND_TYPE, 1700, 3500, 1900, 0, 4},     // 160 meters
  {SW_BAND_TYPE, 3500, 4500, 3700, 0, 5},     // 80 meters
  {SW_BAND_TYPE, 4500, 5600, 4850, 1, 4},
  {SW_BAND_TYPE, 5600, 6800, 6000, 1, 4},
  {SW_BAND_TYPE, 6800, 7300, 7100, 0, 4},     // 40 meters
  {SW_BAND_TYPE, 7200, 8500, 7200, 1, 4},     // 41 meters
  {SW_BAND_TYPE, 8500, 10000, 9600, 1, 4},
  {SW_BAND_TYPE, 10000, 11200, 10100, 0, 4},  // 30 meters
  {SW_BAND_TYPE, 11200, 12500, 11940, 1, 4},
  {SW_BAND_TYPE, 13400, 13900, 13600, 1, 4},
  {SW_BAND_TYPE, 14000, 14500, 14200, 0, 4},  // 20 meters
  {SW_BAND_TYPE, 15000, 15900, 15300, 1, 4},
  {SW_BAND_TYPE, 17200, 17900, 17600, 1, 4},
  {SW_BAND_TYPE, 18000, 18300, 18100, 0, 4},  // 17 meters
  {SW_BAND_TYPE, 21000, 21400, 21200, 0, 4},  // 15 mters
  {SW_BAND_TYPE, 21400, 21900, 21500, 1, 4},  // 13 mters
  {SW_BAND_TYPE, 24890, 26200, 24940, 0, 4},  // 12 meters
  {SW_BAND_TYPE, 26200, 28000, 27500, 0, 4},  // CB band (11 meters)
  {SW_BAND_TYPE, 28000, 30000, 28400, 0, 4},  // 10 meters
  {FM_BAND_TYPE, 6400, 8400, 7000, 3, 0},     // FM from 64 to 84MHz; default 70MHz; default step frequency index is 3; default bandwidth index AUTO
  {FM_BAND_TYPE, 8400, 10800, 10100, 3, 0}
};

const int lastBand = (sizeof band / sizeof(Band)) - 1;
int bandIdx = DEFAULT_BAND;

uint8_t rssi = 0;
uint8_t stereo = 1;
uint8_t volume = DEFAULT_VOLUME;

// Devices class declarations
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
SI4735 si4735;

void oledSpace(uint8_t n)
{
  for(uint8_t i = 0;i < n;i++)
    oled.print(' ');
}

/*
   Switch the radio to current band.
   The bandIdx variable points to the current band.
   This function change to the band referenced by bandIdx (see table band).
*/
void useBand(bool show = true)
{
  if (band[bandIdx].bandType == FM_BAND_TYPE)
  {
    currentMode = FM;
    si4735.setTuneFrequencyAntennaCapacitor(0);
    si4735.setFM(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, tabStep[band[bandIdx].currentStepIdx]);
    si4735.setSeekFmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
    si4735.setSeekFmSpacing(1);
    bfoOn = ssbLoaded = false;
    si4735.setRdsConfig(1, 2, 2, 2, 2);
    si4735.setFifoCount(1);
    bwIdxFM = band[bandIdx].bandwidthIdx;
    bwIdxFM = (bwIdxFM > BW_MAX_FM) ? BW_DEFAULT_FM : bwIdxFM;
    bwFlag = &bwIdxFM;
    bwMax = BW_MAX_FM;
    bwDefault = BW_DEFAULT_FM;
    //si4735.setFmBandwidth(bandwidthFM[bwIdxFM].idx);
  }
  else
  {
    if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE)
      si4735.setTuneFrequencyAntennaCapacitor(0);
    else
      si4735.setTuneFrequencyAntennaCapacitor(1);

    if (ssbLoaded)
    {
      si4735.setSSB(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, tabStep[band[bandIdx].currentStepIdx], currentMode);
      si4735.setSSBAutomaticVolumeControl(1);
      si4735.setSsbSoftMuteMaxAttenuation(0); // Disable Soft Mute for SSB
      bwIdxSSB = band[bandIdx].bandwidthIdx;
      bwIdxSSB = (bwIdxSSB > BW_MAX_SSB) ? BW_DEFAULT_SSB : bwIdxSSB;
      //Serial.print("useBand() bwIdxSSB= ");Serial.println(bwIdxSSB);
      /*
      si4735.setSSBAudioBandwidth(bandwidthSSB[bwIdxSSB].idx);
      // If audio bandwidth selected is about 2 kHz or below, it is recommended to set Sideband Cutoff Filter to 0.
      if (bandwidthSSB[bwIdxSSB].idx == 0 || bandwidthSSB[bwIdxSSB].idx == 4 || bandwidthSSB[bwIdxSSB].idx == 5)
        si4735.setSBBSidebandCutoffFilter(0);
      else
        si4735.setSBBSidebandCutoffFilter(1);
      */
      si4735.setSSBBfo(currentBFO);
      bwFlag = &bwIdxSSB;
      bwMax = BW_MAX_SSB;
      bwDefault = BW_DEFAULT_SSB;
    }
    else
    {
      currentMode = AM;
      si4735.setAM(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, tabStep[band[bandIdx].currentStepIdx]);
      si4735.setAutomaticGainControl(disableAgc, agcNdx);
      si4735.setAmSoftMuteMaxAttenuation(smIdx); // // Disable Soft Mute for AM
      bwIdxAM = band[bandIdx].bandwidthIdx;
      bwIdxAM = (bwIdxAM > BW_MAX_AM) ? BW_DEFAULT_AM : bwIdxAM;
      //si4735.setBandwidth(bandwidthAM[bwIdxAM].idx, 1);
      bfoOn = false;
      bwFlag = &bwIdxAM;
      bwMax = BW_MAX_AM;
      bwDefault = BW_DEFAULT_AM;
    }
    si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);                                       // Consider the range all defined current band
    si4735.setSeekAmSpacing((tabStep[band[bandIdx].currentStepIdx] > 10) ? 10 : tabStep[band[bandIdx].currentStepIdx]); // Max 10kHz for spacing
  }
  //delay(100);
  //oled.clear();
  previousFrequency = currentFrequency = band[bandIdx].currentFreq;
  idxStep = band[bandIdx].currentStepIdx;
  doBandwidth(0, false);
  if (show)
  {
    showStatus();
    if (FM == currentMode)
      cleanBfoRdsInfo();
  }
  resetEepromDelay();
}

/*
   This function loads the contents of the ssb_patch_content array into the CI (Si4735) and starts the radio on
   SSB mode.
*/
void loadSSB(bool show = true)
{
  if (show)
  {
    oled.setCursor(0, 2);
    oledSpace(2);
    oled.print("Switching to SSB");
    oledSpace(2);
  }
  // si4735.setI2CFastModeCustom(700000); // It is working. Faster, but I'm not sure if it is safe.
  si4735.setI2CFastModeCustom(500000);
  si4735.queryLibraryId(); // Is it really necessary here? I will check it.
  si4735.patchPowerUp();
  delay(50);
  si4735.downloadCompressedPatch(ssb_patch_content, size_content, cmd_0x15, cmd_0x15_size);
  bwIdxSSB = (bwIdxSSB > BW_MAX_SSB) ? BW_DEFAULT_SSB : bwIdxSSB;
  //Serial.print("loadSSB bwIdxSSB=");Serial.println(bwIdxSSB);
  si4735.setSSBConfig(bandwidthSSB[bwIdxSSB].idx, 1, 0, 1, 0, 1);
  si4735.setI2CStandardMode();
  ssbLoaded = true;
  // oled.clear();
  cleanBfoRdsInfo();
}


/**
   reads the last receiver status from eeprom.
*/
void readAllReceiverInformation()
{
  int addr_offset;
  int bwIdx;
  volume = EEPROM.read(eeprom_address + 1); // Gets the stored volume;
  bandIdx = EEPROM.read(eeprom_address + 2);
  currentMode = EEPROM.read(eeprom_address + 3);
  currentBFO = EEPROM.read(eeprom_address + 4) << 8;
  currentBFO |= EEPROM.read(eeprom_address + 5);

  addr_offset = 6;
  for (int i = 0; i <= lastBand; i++)
  {
    band[i].currentFreq = EEPROM.read(addr_offset++) << 8;
    band[i].currentFreq |= EEPROM.read(addr_offset++);
    band[i].currentStepIdx = EEPROM.read(addr_offset++);
    band[i].bandwidthIdx = EEPROM.read(addr_offset++);
  }

  //previousFrequency = currentFrequency = band[bandIdx].currentFreq;
  idxStep = tabStep[band[bandIdx].currentStepIdx];
  bwIdx = band[bandIdx].bandwidthIdx;

  if (currentMode == LSB || currentMode == USB)
  {
    loadSSB(false);
    /*
    si4735.setSSBAudioBandwidth(bandwidthSSB[bwIdxSSB].idx);
    // If audio bandwidth selected is about 2 kHz or below, it is recommended to set Sideband Cutoff Filter to 0.
    if (bandwidthSSB[bwIdxSSB].idx == 0 || bandwidthSSB[bwIdxSSB].idx == 4 || bandwidthSSB[bwIdxSSB].idx == 5)
      si4735.setSBBSidebandCutoffFilter(0);
    else
      si4735.setSBBSidebandCutoffFilter(1);
    */
  }
  else if (currentMode == AM)
  {
    bwIdxAM = (bwIdx > BW_MAX_AM) ? BW_DEFAULT_AM : bwIdx;
    //si4735.setBandwidth(bandwidthAM[bwIdxAM].idx, 1);
  }
  else
  {
    bwIdxFM = (bwIdx > BW_MAX_FM) ? BW_DEFAULT_FM : bwIdx;
    //si4735.setFmBandwidth(bandwidthFM[bwIdxFM].idx);
  }
}


void doIntro(bool doClrEEPROM)
{
#define INTRO_SHOWLINE   1
#define INTRO_WAITLINE   2
#define INTRO_LINESDONE  3

#define INTRO_CLREEPROMSTART 4
#define INTRO_CLREEPROM      5
#define INTRO_WAIT           6
#define INTRO_BEFOREDONE     7
#define INTRO_DONE           8
#define INTRO_RELEASE        9

static uint8_t introState = 0;
uint8_t introLine = 0;
uint16_t stateTime;
  if (INTRO_DONE == introState)
    return;
  while (introState != INTRO_DONE) 
  {
    millis16 = millis();
    uint8_t event = btn_Encoder.checkEvent();
    if (introState && (introState < INTRO_CLREEPROM))
    { 
      if (BUTTONEVENT_FIRSTLONGPRESS == event)
      {
#if (0 < INTRO_LONGPRESSDELAY)
        if (introState < INTRO_CLREEPROMSTART)
        {
          introState = INTRO_CLREEPROMSTART;
          stateTime = millis16;
        }
        else
#endif
          event = BUTTONEVENT_SHORTPRESS;
      }      
      if (BUTTONEVENT_SHORTPRESS == event)
      {
        introState = INTRO_BEFOREDONE;
      }
    }  
    switch (introState) {
      case 0:
        stateTime = millis16;
        if (LOW == digitalRead(ENCODER_BUTTON) || doClrEEPROM)
        {
          introState = INTRO_CLREEPROM;
        }
        else
#if (((0 == INTRO_LINEDELAY) && (0 == INTRO_ENDDELAY)) || (0 == INTRO_BEFOREDONE))
          introState = INTRO_BEFOREDONE;
#else
          introState = INTRO_SHOWLINE;
#endif
        break;
      case INTRO_SHOWLINE:
        //oled.setCursor(introlines[introLine].x, introLine);
        stateTime = millis16;
        if ((NUM_INTROLINES > introLine) && (introLine < 4))
        {
          oled.setCursor(63 - strlen(introlines[introLine]) / 2 * 6, introLine);
          oled.print(introlines[introLine++]);
          introState = INTRO_WAITLINE;
        }
        else
        {
          introState = INTRO_LINESDONE;
        }
        break;
      case INTRO_WAITLINE:
        if (millis16 - stateTime > INTRO_LINEDELAY)
          introState = INTRO_SHOWLINE;
        break;
      case INTRO_LINESDONE:
        if (millis16 - stateTime > INTRO_ENDDELAY)
          introState = INTRO_BEFOREDONE;
        break;
#if (0 < INTRO_LONGPRESSDELAY)
      case INTRO_CLREEPROMSTART:
          if (BUTTON_IDLE == event)
            introState = INTRO_SHOWLINE;
          else if (millis16 - stateTime > ((uint16_t)INTRO_LONGPRESSDELAY * (uint16_t)500))
            introState = INTRO_CLREEPROM;
        break;
#endif
      case INTRO_CLREEPROM:
          oled.clear();
          EEPROM.write(eeprom_address, 0);
          oled.setCursor(0, 0);
          oled.print("EEPROM RESETED");
          introState = INTRO_WAIT;
          stateTime = millis16;
        break;
      case INTRO_WAIT:
          if (millis16 - stateTime > INTRO_CLREEPROMDELAY)
            if (HIGH == digitalRead(ENCODER_BUTTON))
              introState = INTRO_RELEASE;
        break;  
      case INTRO_RELEASE:
          if (btn_Encoder.checkEvent() == BUTTON_IDLE)
          {
            introState = INTRO_BEFOREDONE;
          }
        break;  
      case INTRO_BEFOREDONE:
#if (0 == INTRO_SILENT)
        showStatus();
        if (FM == currentMode)
          cleanBfoRdsInfo();
#else
        si4735.getDeviceI2CAddress(RESET_PIN); // Looks for the I2C bus address and set it.  Returns 0 if error

        si4735.setup(RESET_PIN, MW_BAND_TYPE); //
        si4735.setAvcAmMaxGain(48); // Sets the maximum gain for automatic volume control on AM/SSB mode (between 12 and 90dB)

        // Checking the EEPROM content
        if (EEPROM.read(eeprom_address) == app_id)
        {
          readAllReceiverInformation();
        }

        si4735.setVolume(volume);
        useBand();
        //if (FM == currentMode)
        //  cleanBfoRdsInfo();
#endif
        if (BUTTON_IDLE != event)
        {
          while (BUTTON_IDLE != btn_Encoder.checkEvent())
            millis16 = millis();
          delay(200);
        }
        introState = INTRO_DONE;
        break;
      default:
        introState = INTRO_DONE;        
        break;
    }
  }
}

void setup()
{
  bool clearEEPROM = (LOW == digitalRead(ENCODER_BUTTON));

  // Encoder pins
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);
  oled.begin();
  oled.clear();
  oled.on();
  oled.setFont(FONT6X8);

#ifdef DEBUG
  Serial.begin(115200);
#ifdef DEBUG_BUTTONS_ONLY
  oled.setCursor(0,0);
  oled.print("DEBUG MODE!");
  oled.setCursor(0,1);
  oled.print("Please observe Serial");
  oled.setCursor(0,2);
  oled.print("Monitor for button");
  oled.setCursor(0,3);
  oled.print("events.");
  
  return;
#endif
#endif



  
  if (clearEEPROM)
    EEPROM.write(eeprom_address, 0);

  // Set up the radio for the current band (see index table variable bandIdx )
#if (0 == INTRO_SILENT)
  si4735.getDeviceI2CAddress(RESET_PIN); // Looks for the I2C bus address and set it.  Returns 0 if error

  si4735.setup(RESET_PIN, MW_BAND_TYPE); //
  si4735.setAvcAmMaxGain(48); // Sets the maximum gain for automatic volume control on AM/SSB mode (between 12 and 90dB)

  // Checking the EEPROM content
  if (EEPROM.read(eeprom_address) == app_id)
  {
    readAllReceiverInformation();
  }

  
  si4735.setVolume(volume);
  useBand(false);
#endif
  // Splash - Change orit for your introduction text or remove the splash code.

  doIntro(clearEEPROM);
    
  // end Splash

  //previousFrequency = currentFrequency;// =  si4735.getFrequency();

}

                                     

// Print information about detected button press events on Serial
// These events can be used to attach specific funtionality to a button
// DEBUG must be defined for the function to have any effect
#if defined(DEBUG)
uint8_t buttonEvent(uint8_t event, uint8_t pin) {
  Serial.print("Ev=");if (event < 10) Serial.print(' ');Serial.print(event);
  Serial.print(" Pin=");if (pin < 10) Serial.print(' ');Serial.print(pin);
#if defined(DEBUG_BUTTONS_ONLY)
struct {
  uint8_t pin;
  const char *name;
} pinNames[] = 
  {
    {MODE_BUTTON, "Mode"},
    {BANDWIDTH_BUTTON, "BW"},
    {VOLUMEUP_BUTTON, "Vol+"},
    {VOLUMEDN_BUTTON, "Vol-"},
    {BANDUP_BUTTON, "Band+"},
    {BANDDN_BUTTON, "Band-"},
    {AGC_BUTTON, "AGC"},
    {STEP_BUTTON, "Step"},
    {ENCODER_BUTTON, "Encoder"}
  };
#define KNOWN_BUTTONS (sizeof(pinNames) / sizeof(pinNames[0]))
  Serial.print(" (");
  int i;
  for(i = 0;i < KNOWN_BUTTONS;i++)
    if (pinNames[i].pin == pin)
      break;
  if (i < KNOWN_BUTTONS)
  {
    Serial.print('"');
    Serial.print(pinNames[i].name);
    Serial.print('\"');
  }
  else
    Serial.print("???undefined???");
  Serial.print(") ");
#endif  
  
  Serial.print(">>");
  if (BUTTONEVENT_ISDOUBLE(event))
    Serial.print("2");
  event = event & 0xef;
  if (BUTTONEVENT_ISLONGPRESS(event))
  { 
    if (BUTTONEVENT_FIRSTLONGPRESS == event)
      Serial.println("LP start");
    else if (BUTTONEVENT_LONGPRESS == event)
      Serial.println("LP repeat");
    else if (BUTTONEVENT_LONGPRESSDONE == event)
      Serial.println("LP done");
    else 
      Serial.println("LP: UNEXPECTED!!!!");
  }
  else if (event == BUTTONEVENT_SHORTPRESS)
    Serial.println("CLICK");
  else if (event == BUTTONEVENT_2CLICK)
    Serial.println("2CLICK");
  else
    Serial.println("UNEXPECTED!!!!");
  return event;
}

#else
#define buttonEvent NULL
#endif

void applyParameterChange(uint8_t *parameter, int8_t direction, uint8_t max, void (*changeFunction)(int8_t dir, bool show), bool show = true)
{
  if (((1 == direction) && (*parameter < max)) || ((1 != direction) && (*parameter > 0)) || (0 == direction))
  {
    //Serial.print("Do change. D:");Serial.print(direction);Serial.print(" P:");Serial.print(*parameter);Serial.print(" M:");Serial.println(max);
    changeFunction(direction, show);
  }
}




//Handle Actions of "Band+"/"Band-"-buttons
uint8_t bandEvent(uint8_t event, uint8_t pin) {
static uint8_t count;
bool direction = (BANDUP_BUTTON == pin);

#ifdef DEBUG
  buttonEvent(event, pin);
#endif

#if BANDSWITCH1STEP_DELAY != 0
  if (BUTTONEVENT_ISLONGPRESS(event))
    if (BUTTONEVENT_ISDOUBLE(event))
    {
      if (BUTTONEVENT_2LONGPRESS == event)
      {
        if (count < BANDSWITCH1STEP_DELAY)
          if (++count == BANDSWITCH1STEP_DELAY)
            if (direction)
            {
              if (bandIdx < lastBand)
                bandUp();
            }
            else
            {
              if (bandIdx)
                bandDown();          
            }
      }
      else
        count = 0;
      return BUTTON_IDLE;
    }
#endif
#if ENCODERMODE_DELAY != 0
  if (encoderMode)
  {
    if (BUTTONEVENT_ISDOUBLE(event))
      if (BUTTONEVENT_ISLONGPRESS(event))
      {
        event = BUTTONEVENT_UNDO_DOUBLE(event);
      }
    if (!BUTTONEVENT_ISDOUBLE(event))
    {
      if (!BUTTONEVENT_ISLONGPRESS(event) || (BUTTONEVENT_FIRSTLONGPRESS == event))
        count = 0;
      if (BUTTONEVENT_LONGPRESSDONE != event)
      {
#if (0 != ENCODERMODE_SLOWDOWN)
        if (!count++)
          encoderCount = direction?1:-1;
        count = count % ENCODERMODE_SLOWDOWN;
#else
        encoderCount = direction?1:-1;
#endif       
      }
      event = BUTTON_IDLE;
    }
    else
      event = BUTTONEVENT_UNDO_DOUBLE(event);
  }
#endif
#if (0 != SOFTMUTE_2CLICKENABLE)
  if (BANDDN_BUTTON == pin)
  {
    if (BUTTONEVENT_2CLICK == event)
    {
      if (FM != currentMode)
      {
        if (cmdAvc || cmdAgcAtt)
        {
          disableCommand(NULL, NULL);
          cmdSoftMute = true;
          }
        smIdx = (smIdx < 32)?32:0;
        doSoftMute(0);
        event = BUTTON_PRESSED; 
      }
      else 
        event = BUTTONEVENT_SHORTPRESS;
    }
  }
#endif
#if (0 != BAND_2CLICKENABLE)
  if (BANDUP_BUTTON == pin)
  {
    if (BUTTONEVENT_2CLICK == event)
    {
      if (bandIdx < lastBand)
        bandIdx = lastBand;
      else
        bandIdx = 0;
      useBand();
      event = BUTTON_PRESSED;
    }
  }
#endif

  event = BUTTONEVENT_UNDO_DOUBLE(event);
  /*
  if (BUTTONEVENT_2CLICK == event)
  {
    if (BAND_BUTTON == pin)
      bandUp();
    else
      bandDown();
    event = BUTTON_IDLE;
  }
  */
#if (0 != BAND_DELAY)
  if (BUTTONEVENT_ISLONGPRESS(event))           // longpress-Event?
    if (BUTTONEVENT_LONGPRESSDONE != event)
      {
        if (BUTTONEVENT_FIRSTLONGPRESS == event)
        {
          count = 0;
        }
        if (count++ == 0)
          if (BANDUP_BUTTON == pin)
          {
            if (bandIdx < lastBand)
              bandUp();
          }
          else
          {
           if (bandIdx)
            bandDown();
          }
        count = count % BAND_DELAY;
      }
#else
  if (BUTTONEVENT_FIRSTLONGPRESS == event)
    event = BUTTONEVENT_SHORTPRESS;
#endif
  if (BUTTONEVENT_SHORTPRESS == event)
    if (BANDUP_BUTTON == pin)
       disableCommand(&cmdBand, showBandDesc);
    else
      if (currentMode != FM) 
        disableCommand(&cmdSoftMute, showSoftMute);
  return BUTTON_IDLE;
}  //end of bandEvent()


//Handle Actions of "Vol+"/"Vol-"-buttons
uint8_t volumeEvent(uint8_t event, uint8_t pin) {
#ifdef DEBUG
  buttonEvent(event, pin);
#endif
#if (0 != AVC_2CLICKENABLE)
  if (VOLUMEDN_BUTTON == pin)
    if (BUTTONEVENT_ISDOUBLE(event))
    {
      static uint8_t count = 0;
      static int8_t direction = 2;
      if (FM != currentMode)
      {
        if ((BUTTONEVENT_2CLICK == event) || (BUTTONEVENT_2FIRSTLONGPRESS == event))
        {
          if (cmdSoftMute || cmdAgcAtt)
          {
            disableCommand(NULL, NULL);
            cmdAvc = true;
          }
          if (BUTTONEVENT_2CLICK == event)
          {
            avcIdx = (90 == avcIdx)?12:(38 > avcIdx?38:90);
            doAvc(0);
          }
          else 
          {
            count = AVC_2CLICKENABLE;
            if (90 == avcIdx)
              direction = -2;
            else if (12 == avcIdx)
              direction = 2;
            else
              direction = -1 * direction;
          }
        }
        else if (BUTTONEVENT_2LONGPRESS == event)
          count++;
        if (AVC_2CLICKENABLE == count)
        {
          uint8_t x = avcIdx + direction;
          count = 0;
          if ((x >= 12) && (x <= 90))
          {
            avcIdx = x;
            doAvc(0);
          }
        }
      }
      event = BUTTON_PRESSED;
    }
#endif
  event = BUTTONEVENT_UNDO_DOUBLE(event);
  if (muteVolume)                               // currently muted?
    if (!BUTTONEVENT_ISDONE(event))             // Any event to change the volume?
      if ((BUTTONEVENT_SHORTPRESS != event) || (VOLUMEUP_BUTTON == pin))
        doVolume(1);                              // yes-->Unmute
#if (0 != VOLUME_DELAY)
#if (VOLUME_DELAY > 1)
  static uint8_t count;
  if (BUTTONEVENT_FIRSTLONGPRESS == event)
  {
    count = 0;
  }
#endif
  if (BUTTONEVENT_ISLONGPRESS(event))           // longpress-Event?
    if (BUTTONEVENT_LONGPRESSDONE != event)     // but not the final release of button?
    {
#if (VOLUME_DELAY > 1)
      if (count++ == 0)
#endif
        doVolume(VOLUMEUP_BUTTON == pin?1:-1); 
#if (VOLUME_DELAY > 1)
      count = count % VOLUME_DELAY;
#endif
    }
#else
  if (BUTTONEVENT_FIRSTLONGPRESS == event)
    event = BUTTONEVENT_SHORTPRESS;
#endif
    if (BUTTONEVENT_SHORTPRESS == event)
      if (VOLUMEUP_BUTTON == pin)
        disableCommand(&cmdVolume, showVolume);
      else
        if (currentMode != FM) 
          disableCommand(&cmdAvc, showAvc);
       
  return BUTTON_IDLE;
} // End of volumeEvent()


//Handle Actions of "Step"-button
uint8_t stepEvent(uint8_t event, uint8_t pin) {
#ifdef DEBUG
  buttonEvent(event, pin);
#endif
  if (FM == currentMode)
    return BUTTON_IDLE;
#if (0 != STEP_2CLICKENABLE)
  if (BUTTONEVENT_ISDOUBLE(event))
  {
    if (BUTTONEVENT_2CLICK == event)
    {
      idxStep = (idxStep == lastStep?0:lastStep);
      //applyParameterChange(&idxStep, 0, lastStep, doStep);
      doStep(0);
      event = BUTTON_PRESSED;
    }
  }
#endif
  event = BUTTONEVENT_UNDO_DOUBLE(event);
#if (0 != STEP_DELAY)
    if (BUTTONEVENT_ISLONGPRESS(event))
    {
      static uint8_t direction = 1;
      static uint8_t count;
      if (BUTTONEVENT_LONGPRESSDONE == event)
        direction = (1==direction)?-1:1;
      else
      {
        if (BUTTONEVENT_FIRSTLONGPRESS == event)
        {
          count = 0;
          if (0 == idxStep)
            direction = 1;
          else if (lastStep == idxStep)
            direction = -1;
        }
        if (0 == count++)
          if (bfoOn || (idxStep != ((1 == direction)?lastStep:0)))
            doStep(direction);
        count = count % STEP_DELAY;
      }
    }
#else
  if (BUTTONEVENT_FIRSTLONGPRESS == event)
    event = BUTTONEVENT_SHORTPRESS;
#endif
    if (BUTTONEVENT_SHORTPRESS == event)
      if (currentMode != FM)
      {
        disableCommand(&cmdStep, showStep);
        if (bfoOn)
          showBFO();
      }
  return BUTTON_IDLE;
} //End of stepEvent()


//Handle Actions of "BW"-button
uint8_t bandwidthEvent(uint8_t event, uint8_t pin) {
#ifdef DEBUG
  buttonEvent(event, pin);
#endif
#if (0 != BANDWIDTH_2CLICKENABLE)
  if (BUTTONEVENT_ISDOUBLE(event))
  {
    if ((BUTTONEVENT_2CLICK == event) || (BUTTONEVENT_2FIRSTLONGPRESS == event))
    {
      *bwFlag = BUTTONEVENT_2CLICK == event?(*bwFlag == bwMax?0:bwMax):bwDefault;
      applyParameterChange(bwFlag, 0, bwMax, doBandwidth);
    }
    event = BUTTON_PRESSED;
  }
#endif
  event = BUTTONEVENT_UNDO_DOUBLE(event);
#if (0 != BANDWIDTH_DELAY)
  if (BUTTONEVENT_ISLONGPRESS(event))
  {
    static uint8_t direction = 1;
    if (BUTTONEVENT_ISDONE(event))
      direction = (direction == 1)?-1:1;
    else
    {
      static uint8_t count;
      if (BUTTONEVENT_FIRSTLONGPRESS == event)
      {
        count = 0;
        if (0 == *bwFlag)
          direction = 1;
        else if (*bwFlag == bwMax)
          direction = -1;
      }
      if (0 == count)
      {
        applyParameterChange(bwFlag, direction, bwMax, doBandwidth);
      }
      count = (count + 1) % BANDWIDTH_DELAY;
    }
  }
#else
  if (BUTTONEVENT_FIRSTLONGPRESS == event)
    event = BUTTONEVENT_SHORTPRESS;
#endif
  if(BUTTONEVENT_SHORTPRESS == event)
      disableCommand(&cmdBw, showBandwidth);

  return BUTTON_IDLE;
} //End of handeBandwidth()


//Handle Actions of "AGC"-button
uint8_t agcEvent(uint8_t event, uint8_t pin) {
#ifdef DEBUG
  buttonEvent(event, pin);
#endif
#if (0 != ENCODER_MUTEDELAY)
  if (encoderMode)
    if (BUTTONEVENT_ISDOUBLE(event))
      event = BUTTONEVENT_UNDO_DOUBLE(event);
    else
    {
      encoderEvent(event, 0);
      return BUTTON_IDLE;
    }
#endif
#if (0 != AGC_2CLICKENABLE)
  if (BUTTONEVENT_ISDOUBLE(event))
  {
    if ((BUTTONEVENT_2CLICK == event) || (BUTTONEVENT_2FIRSTLONGPRESS == event))
    {
      if (FM != currentMode)
      {
        if (cmdSoftMute || cmdAvc)
        {
          disableCommand(NULL, NULL);
          cmdAgcAtt = true;
        }
        agcIdx = (BUTTONEVENT_2CLICK == event)?(37 == agcIdx?1:37):0;
        doAttenuation(0);
      }
    }
    event = BUTTON_PRESSED;
  }
#endif
#if (0 != AGC_DELAY)
  if (FM != currentMode)
    if (BUTTONEVENT_ISLONGPRESS(event))
    {
      static uint8_t direction = 1;
      static uint8_t count;
      if (BUTTONEVENT_LONGPRESSDONE == event)
        direction = (direction == 1)?-1:1;
      else
      {
        if (BUTTONEVENT_FIRSTLONGPRESS == event)
        {
          count = 0;
          if (0 == agcIdx)
            direction = 1;
          else if (37 == agcIdx)
            direction = -1;
        }
        if (0 == count++)
        {
          //applyParameterChange(&agcIdx, direction, 37, doAttenuation);
          int8_t x = agcIdx + direction;
          if ((x >= 0) && (x <= 37))
          {
            agcIdx = x;
            doAttenuation(0);
          }
        }
        count = count % AGC_DELAY;
      }
    }
#else
  if (BUTTONEVENT_FIRSTLONGPRESS == event)
    event = BUTTONEVENT_SHORTPRESS;
#endif
  if (BUTTONEVENT_SHORTPRESS == event) 
    if ( currentMode != FM)
      disableCommand(&cmdAgcAtt, showAttenuation);

  return BUTTON_IDLE;
}  //End of agcEvent()


//Handle Actions of "Mode"-button
uint8_t modeEvent(uint8_t event, uint8_t pin) {
#ifdef DEBUG
  buttonEvent(event, pin);  
#endif

#if (0 != ENCODERMODE_DELAY)
static uint8_t count = 0;
#endif


  if (BUTTONEVENT_FIRSTLONGPRESS == event)
  {
#if (0 != ENCODERMODE_DELAY)
    count = 0;
#else
    event = BUTTONEVENT_SHORTPRESS;
#endif
  }
#if (0 != ENCODERMODE_DELAY)
  else if (BUTTONEVENT_LONGPRESS == event)
  {
    if (count < ENCODERMODE_DELAY)
      if (++count == ENCODERMODE_DELAY)
      {
        encoderMode = !encoderMode;
        showEncoderMode();
      }
  }
#endif
    if (BUTTONEVENT_SHORTPRESS == event)
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
        else// if (currentMode == USB)
        {
          currentMode = AM;
          ssbLoaded = false;
          bfoOn = false;
        }
        // Nothing to do if you are in FM mode
        band[bandIdx].currentFreq = currentFrequency;
        band[bandIdx].currentStepIdx = idxStep;
        useBand();
      }
      else
      {
        rdsOff = !rdsOff;
        cleanBfoRdsInfo();
      }
    }  
  return BUTTON_IDLE;
} //End of modeEvent()

//Handle Actions of Encoder-button
uint8_t encoderEvent(uint8_t event, uint8_t pin) {
#ifdef DEBUG
  buttonEvent(event, pin);
#endif
  if (BUTTONEVENT_SHORTPRESS == event)
  {
#if (0 != ENCODER_ENTER)
    if (cmdAnyOn)
    {
      disableCommand(NULL, NULL); // disable all command buttons
      return BUTTON_IDLE;      
    }
#endif
    if (currentMode == LSB || currentMode == USB)
    {
      bfoOn = !bfoOn;
#if (0 != DISPLAY_OLDSTYLE)
      showFrequency();
#endif
      if (cmdAnyOn)
        disableCommand(NULL, NULL); // disable all command buttons
      else
        showBFO();
    }
    else if (currentMode == FM || currentMode == AM)
    {
#if (0 != ENCODER_SEARCH)
      // Jumps up or down one space
      if (seekDirection)
        si4735.frequencyUp();
      else
        si4735.frequencyDown();
      si4735.seekStationProgress(showFrequencySeek, checkStopSeeking, seekDirection);
      delay(30);
      if (currentMode == FM)
      {
        float f = round(si4735.getFrequency() / 10.0);
        currentFrequency = (uint16_t)f * 10; // adjusts band space from 1 (10kHz) to 10 (100 kHz)
        si4735.setFrequency(currentFrequency);
      }
      else
      {
        currentFrequency = si4735.getFrequency(); //
      }
      showFrequency();
#endif      
    }
  }    


#if (0 != ENCODER_MUTEDELAY)
  static uint8_t waitBeforeMute;
  if (BUTTONEVENT_FIRSTLONGPRESS == event)
  {
    waitBeforeMute = ENCODER_MUTEDELAY;
  }
  else if ((BUTTONEVENT_LONGPRESS == event) && (0 != waitBeforeMute))
    if (0 == --waitBeforeMute)
    {
      uint8_t x = muteVolume;
      muteVolume = si4735.getVolume();
      si4735.setVolume(x);
      showVolume(); 
    }
#else
  if (BUTTONEVENT_FIRSTLONGPRESS == event)
    event = BUTTONEVENT_SHORTPRESS;
#endif
  return BUTTON_IDLE;
} //End of encoderEvent()






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

/*
   writes the conrrent receiver information into the eeprom.
   The EEPROM.update avoid write the same data in the same memory position. It will save unnecessary recording.
*/
void saveAllReceiverInformation()
{
  int addr_offset;
  EEPROM.update(eeprom_address, app_id);                 // stores the app id;
  EEPROM.update(eeprom_address + 1, muteVolume?muteVolume:si4735.getVolume()); // stores the current Volume
  EEPROM.update(eeprom_address + 2, bandIdx);            // Stores the current band
  EEPROM.update(eeprom_address + 3, currentMode);        // Stores the current Mode (FM / AM / SSB)
  EEPROM.update(eeprom_address + 4, currentBFO >> 8);
  EEPROM.update(eeprom_address + 5, currentBFO & 0XFF);

  addr_offset = 6;
  band[bandIdx].currentFreq = currentFrequency;

  for (int i = 0; i <= lastBand; i++)
  {
    EEPROM.update(addr_offset++, (band[i].currentFreq >> 8));   // stores the current Frequency HIGH byte for the band
    EEPROM.update(addr_offset++, (band[i].currentFreq & 0xFF)); // stores the current Frequency LOW byte for the band
    EEPROM.update(addr_offset++, band[i].currentStepIdx);       // Stores current step of the band
    EEPROM.update(addr_offset++, band[i].bandwidthIdx);        // table index (direct position) of bandwidth
  }
}

/*
   To store any change into the EEPROM, it is needed at least STORE_TIME  milliseconds of inactivity.
*/
void resetEepromDelay()
{
  storeTime = millis16;//millis();
  previousFrequency = 0;
}

/**
   Converts a number to a char string and places leading zeros.
   It is useful to mitigate memory space used by sprintf or generic similar function

   value  - value to be converted
   strValue - the value will be receive the value converted
   len -  final string size (in bytes)
   dot - the decimal or tousand separator position
   separator -  symbol "." or ","
*/
void convertToChar(uint16_t value, char *strValue, uint8_t len, uint8_t dot = 0, uint8_t separator = 0)
{
  char d;
  int i;
  for (i = (len - 1); i >= 0; i--)
  {
    d = value % 10;
    value = value / 10;
    strValue[i] = d + 48;
  }
  strValue[len] = '\0';
  if (dot > 0)
  {
    for (int i = len; i >= dot; i--)
    {
      strValue[i + 1] = strValue[i];
    }
    strValue[dot] = separator;
    len = dot;
  }
  i = 0;
  len--;
  while ((i < len) && ('0' == strValue[i]))
  {
    strValue[i++] = ' ';
  }
}

/**
  Show current frequency
*/
void showFrequency()
{
  //char *unit;
  char freqDisplay[10];

  if (FM == currentMode) //band[bandIdx].bandType == FM_BAND_TYPE)
  {
    convertToChar(currentFrequency, freqDisplay, 5, 3, ',');
    cleanBfoRdsInfo();

/*
    if (currentFrequency != previousFrequency)
      if (!rdsOff)
*/
    //unit = (char *)"MHz";
  }
  else
  {
//    unit = (char *)"kHz";
    if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE)
      convertToChar(currentFrequency, freqDisplay, 5, 0, '.');
    else
      convertToChar(currentFrequency, freqDisplay, 5, 2, '.');
  }
#if (0 != DISPLAY_OLDSTYLE)  
  oled.invertOutput(bfoOn);
#endif
  oled.setFont(FONT8X16ATARI);
  oled.setCursor(34, 0);
  oledSpace(6);
  //oled.print("      ");
  oled.setCursor(34, 0);
  oled.print(freqDisplay);
  oled.setFont(FONT6X8);
  oled.invertOutput(false);

  showEncoderMode();
}

/**
    This function is called by the seek function process.
*/
void showFrequencySeek(uint16_t freq)
{
  currentFrequency = freq;
  showFrequency();
}

/**
   Checks the stop seeking criterias.
   Returns true if the user press the touch or rotates the encoder during the seek process.
*/
bool checkStopSeeking()
{
  // Checks the touch and encoder
  return (bool)encoderCount || (digitalRead(ENCODER_BUTTON) == LOW); // returns true if the user rotates the encoder or press the push button
}


/**
   Shows band information
*/
void showBandDesc()
{
  char *bandMode;
  char buf[4];
  if (currentFrequency < 520)
    bandMode = (char *)"LW  ";
  else
  {
    bandMode = (char *)&buf;
    buf[0] = bandModeDesc[currentMode];
    if (buf[0] <= 'F')
    {
      buf[1] = 'M';buf[2] = ' ';
    }
    else
    {
      buf[1] = 'S';buf[2] = 'B';
    }
    buf[3] = 0;
    //bandMode = (char *)bandModeDesc[currentMode];
  }
  oled.setCursor(0, 0);
  oledSpace(4);
  //oled.print("    ");
  oled.setCursor(0, 0);
  oled.invertOutput(cmdBand);
  oled.print(bandMode);
  oled.invertOutput(false);
}

/* *******************************
   Shows RSSI status
*/
void showRSSI(bool force = false)
{  
static int8_t lastBars = -1;
  int8_t bars = (rssi / 16);
  //Serial.print("RSSI:");Serial.println(rssi);
  bars = (bars>4)?4:bars;
  if (currentMode == FM)
  {
    oled.setCursor(18, 0);
    if (si4735.getCurrentPilot())
    {
      oled.invertOutput(true);
      oled.print("s");
      oled.invertOutput(false);
    }
    else
      oledSpace(2);
  }
  
  if (force || (bars != lastBars))
  {
    lastBars = bars;
    oled.setCursor(90, 3);
    oledSpace(6);
  //oled.print("      ");
    oled.setCursor(90, 3);
    oled.print(".");
    for (int i = 0; i < bars; i++)
      oled.print('_');
    oled.print('|');
  }
}

/*
   Shows the volume level on LCD
*/
void showVolume()
{
  char s[3];
//  oled.setCursor(62, 3);
//  oledSpace(2);
  //oled.print("  ");
  oled.invertOutput(cmdVolume);
#if (0 != DISPLAY_OLDSTYLE)
  oled.setCursor(55, 3);
  oled.print(' ');
  oled.invertOutput(false);
#else
  oled.setCursor(62, 3);
#endif
  if (muteVolume)
  {
    oled.print("XX");
  }
    
  else
  {
    convertToChar(si4735.getCurrentVolume(), s, 2, 0, 0);
    oled.print(s);
  }
#if (0 == DISPLAY_OLDSTYLE)
  oled.invertOutput(false);
#endif
}

void showStep()
{
  if (bfoOn)
    return;
  oled.setCursor(93, 1);
  oledSpace(5);
  //oled.print("     ");
  if (currentMode != FM)
  {
    oled.setCursor(93, 1);
    oled.invertOutput(cmdStep);
    oled.print("S:");
#if (0 != DISPLAY_OLDSTYLE)
    oled.invertOutput(false);
#endif
    oled.print(tabStep[idxStep]);
#if (0 == DISPLAY_OLDSTYLE)
    oled.invertOutput(false);
#endif
  }
    
}

/**
   Shows bandwidth on AM,SSB and FM mode
*/
void showBandwidth()
{
  char buf[4];
  char *bw = (char *)&buf;
  if (currentMode == FM)
  {
    if (0 != bwIdxFM)
      convertToChar(bandwidthFM[bwIdxFM].desc, buf,3);
    else
      bw = "AUT";    
  }
  else
  {
    if (currentMode == LSB || currentMode == USB)
    {
      convertToChar(bandwidthSSB[bwIdxSSB].desc, buf,2, 1, '.');
      showBFO();
    }
    else
    {
      convertToChar(bandwidthAM[bwIdxAM].desc, buf,2, 1, '.');
    }
    
  }
  oled.setCursor(0, 3);
  oled.setCursor(0, 3);
  oled.invertOutput(cmdBw);
  oled.print("BW: ");
#if (0 != DISPLAY_OLDSTYLE)
  oled.invertOutput(false);
#endif
  oled.print(bw);
#if (0 == DISPLAY_OLDSTYLE)
  oled.invertOutput(false);
#endif
}

/*
   Shows AGCC and Attenuation
*/
void showAttenuation()
{
  // Show AGC Information
  oled.setCursor(0, 1);
  oledSpace(5);
  //oled.print("     ");
  oled.setCursor(0, 1);
  if ( currentMode != FM ) {
    oled.invertOutput(cmdAgcAtt);
    if (agcIdx == 0)
    {
      oled.print("AGC");
    }
    else
    {
      oled.print("At");
#if (0 != DISPLAY_OLDSTYLE)
      oled.invertOutput(false);
#endif
      oled.print(agcNdx);
    }
    oled.invertOutput(false);
  }
  attDirty = false;
}

void showSoftMute() {
  if ( currentMode != FM ) {
    oled.setCursor(0, 1);
    oledSpace(5);
    //oled.print("     ");
    oled.setCursor(0, 1);
    oled.invertOutput(cmdSoftMute);
    oled.print("SM");
#if (0 != DISPLAY_OLDSTYLE)
    oled.invertOutput(false);
#endif
    oled.print(smIdx);
#if (0 == DISPLAY_OLDSTYLE)
    oled.invertOutput(false);
#endif
  }
}


void showAvc() {
  if ( currentMode != FM ) {
    oled.setCursor(0, 1);
    oledSpace(5);
    //oled.print("     ");
    oled.setCursor(0, 1);
    oled.invertOutput(cmdAvc);
    oled.print("AVC");
#if (0 != DISPLAY_OLDSTYLE)
    oled.invertOutput(false);
#endif
    oled.print(avcIdx);
#if (0 == DISPLAY_OLDSTYLE)
    oled.invertOutput(false);
#endif
  }
}

/*
   Shows the BFO current status.
   Must be called only on SSB mode (LSB or USB)
*/
void showBFO()
{
  oled.setCursor(0, 2);
  oledSpace(14);
  oled.setCursor(0, 2);
#if (0 == DISPLAY_OLDSTYLE)  
  oled.invertOutput(bfoOn  && !cmdAnyOn);
#endif
  oled.print("BFO: ");
  oled.print(currentBFO);
  oled.print("Hz");
#if (0 != DISPLAY_OLDSTYLE)  
  oled.invertOutput(false);
#endif

  oled.invertOutput(cmdStep);
  oled.setCursor(93, 2);
  oled.setCursor(93, 2);
  oled.print("S:");
#if (0 != DISPLAY_OLDSTYLE)
  oled.invertOutput(false);
#endif
  oled.print(currentBFOStep);
#if (0 == DISPLAY_OLDSTYLE)
  oled.invertOutput(false);
#endif
}

void showEncoderMode()
{
  oled.setCursor(93, 0);
  oled.invertOutput(encoderMode);
  oled.print(currentMode == FM?'M':'k');
  oled.print("Hz");
  oled.invertOutput(false);
}

/**
    Shows some basic information on display
*/
void showStatus()
{
  oled.clear();
  showFrequency();
  showBandDesc();
  showStep();
  showBandwidth();
  showAttenuation();
  showRSSI(true);
  showVolume(); 
}


char *stationName;
char bufferStatioName[20];
long rdsElapsed = millis();

char oldBuffer[15];

/*
   Clean the content of the third line (line 2 - remember the first line is 0)
*/
void cleanBfoRdsInfo()
{
  oled.setCursor(0, 2);
  oledSpace(20);
  if ((currentMode == FM) && rdsOff)
  {
    oled.setCursor(0, 2);
    oled.print("RDS off");
  }
  //oled.print("                    ");
}

/*
   Show the Station Name.
*/
void showRDSStation()
{
  char *po, *pc;
  int col = 0;
  po = oldBuffer;
  pc = stationName;
  while (*pc)
  {
    if (*po != *pc)
    {
      oled.setCursor(col, 2);
      oled.print(*pc);
    }
    *po = *pc;
    po++;
    pc++;
    col += 10;
  }
  // strcpy(oldBuffer, stationName);
  //delay(100);
}



/*
   Checks the station name is available
*/
void checkRDS()
{
  if (millis() - rdsElapsed < 100)
    return;
  si4735.getRdsStatus();
  if (si4735.getRdsReceived())
  {
    if (si4735.getRdsSync() && si4735.getRdsSyncFound() && !si4735.getRdsSyncLost() && !si4735.getGroupLost())
    {
      stationName = si4735.getRdsText0A();
      if (stationName != NULL /* && si4735.getEndGroupB()  && (millis() - rdsElapsed) > 10 */)
      {
        showRDSStation();
        // si4735.resetEndGroupB();
        rdsElapsed = millis();
      }
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
  band[bandIdx].currentStepIdx = idxStep; // currentStep;

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
  band[bandIdx].currentStepIdx = idxStep;
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


/**
   Changes the step frequency value based on encoder rotation
*/
void doStep(int8_t v)
{
  // This command should work only for SSB mode
  if ((currentMode == LSB || currentMode == USB) && bfoOn)
  {
    currentBFOStep = (currentBFOStep == 25) ? 10 : 25;
    //Serial.println("bfoOn");
    showBFO();
  }
  else
  {
    if (v)
    {
      idxStep = (v == 1) ? idxStep + 1 : idxStep - 1;
      if (idxStep > lastStep)
        idxStep = 0;
      else if (idxStep < 0)
        idxStep = lastStep;
    }
    si4735.setFrequencyStep(tabStep[idxStep]);
    band[bandIdx].currentStepIdx = idxStep;
    si4735.setSeekAmSpacing((tabStep[idxStep] > 10) ? 10 : tabStep[idxStep]); // Max 10kHz for spacing
    showStep();
  }
}

/**
   Changes the volume based on encoder rotation
*/
void doVolume(int8_t v)
{
  if (muteVolume)
  {
    si4735.setVolume(muteVolume);
    muteVolume = 0;    
  }
  else
    if (v == 1)
      si4735.volumeUp();
    else
      si4735.volumeDown();
  showVolume();
}

/**
   Switches the AGC/Attenuation based on encoder rotation
*/
void doAttenuation(int8_t v)
{
  if (FM != currentMode)
  {
    if (v)
    {
      agcIdx = (v == 1) ? agcIdx + 1 : agcIdx - 1;
      if (agcIdx < 0)
        agcIdx = 37;
      else if (agcIdx > 37)
        agcIdx = 0;
    }

    disableAgc = (agcIdx > 0); // if true, disable AGC; esle, AGC is enable

    agcNdx = (agcIdx > 1)?agcIdx - 1:0;

    // Sets AGC on/off and gain
    si4735.setAutomaticGainControl(disableAgc, agcNdx);
    showAttenuation();
  }
  //delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
}

/**
   SoftMute control
*/
void doSoftMute(int8_t v)
{
  if ( currentMode != FM ) {
    if (v)
      smIdx = (v == 1) ? smIdx + 1 : smIdx - 1;
    else
    {
      countRSSI = 0;
      attDirty = true;
    }    
    if (smIdx > 32)
      smIdx = 0;
    else if (smIdx < 0)
      smIdx = 32;
    si4735.setAmSoftMuteMaxAttenuation(smIdx);

    showSoftMute();
  }
}

/**
   Automatic Volume Control
*/
void doAvc(int8_t v)
{
  if ( currentMode != FM ) {
    if (v)
      avcIdx = (v == 1) ? avcIdx + 2 : avcIdx - 2;
    else
    {
      countRSSI = 0;
      attDirty = true;
    }
    if (avcIdx > 90)
      avcIdx = 12;
    else if (avcIdx < 12)
      avcIdx = 90;

    si4735.setAvcAmMaxGain(avcIdx);
    showAvc();
  }
}


/**
   Switches the bandwidth based on encoder rotation
*/
void doBandwidth(int8_t v, bool show = true)
{
  if (v != 0)
  {
    if (v > 0) 
      *bwFlag = *bwFlag + 1;
    else
      *bwFlag = *bwFlag - 1;
    if (*bwFlag < 0)
      *bwFlag = bwMax;
    else if (*bwFlag > bwMax)
      *bwFlag = 0;
  }
  band[bandIdx].bandwidthIdx = *bwFlag;
  if (currentMode == AM)
  {
    si4735.setBandwidth(bandwidthAM[bwIdxAM].idx, 1);
  }
  else if (currentMode == FM)
  {
    si4735.setFmBandwidth(bandwidthFM[bwIdxFM].idx);
  }
  else //if (currentMode == LSB || currentMode == USB)
  {
    si4735.setSSBAudioBandwidth(bandwidthSSB[bwIdxSSB].idx);
    // If audio bandwidth selected is about 2 kHz or below, it is recommended to set Sideband Cutoff Filter to 0.
    if (bandwidthSSB[bwIdxSSB].idx == 0 || bandwidthSSB[bwIdxSSB].idx == 4 || bandwidthSSB[bwIdxSSB].idx == 5)
      si4735.setSBBSidebandCutoffFilter(0);
    else
      si4735.setSBBSidebandCutoffFilter(1);
  }
  if (show)
    showBandwidth();
}

/**
   disble command buttons and keep the current status of the last command button pressed
*/
void disableCommand(bool *b, void (*showFunction)())
{
bool wasAnyOn = cmdAnyOn;
bool value = false;
  if (b != NULL)
    *b = value = !*b;

  if (wasAnyOn)
  {  
    cmdVolume = false;
    cmdAgcAtt = false;
    cmdStep = false;
    cmdBw = false;
    cmdBand = false;
    cmdSoftMute = false;
    cmdAvc = false;
    
    cmdAnyOn = false;
    
  }
  if (b != NULL) // rescues the last status of the last command only the parameter is not null
    if (*b = cmdAnyOn = value)
    {
      if (bfoOn)
        if (!wasAnyOn)
          showBFO();
      if ((b == &cmdAvc) || (b == &cmdAgcAtt) || (b == &cmdSoftMute))
        attDirty = false;
    }

  if (wasAnyOn)
  {
    if (bfoOn)
      showBFO();
    showVolume();
    showStep();
    showAttenuation();
    showBandwidth();
    showBandDesc();
  }

  if (showFunction != NULL) //  show the desired status only if it is necessary.
    if (cmdAnyOn)
      showFunction();
  if (attDirty)
  {
    showAttenuation();
  }

  elapsedRSSI = millis16;
  countRSSI = 0;
}

void doEncoderAction()
{
  if (encoderCount != 0)
  {
#if defined(DEBUG)
    Serial.print("Encoder turned ");
    if (encoderCount > 0)
      Serial.println("right");
    else
      Serial.println("left");
#if defined(DEBUG_BUTTONS_ONLY)
    encoderCount = 0;
    return;
#endif
#endif
    if (cmdVolume)
      doVolume(encoderCount);
    else if (cmdAgcAtt)
      doAttenuation(encoderCount);
    else if (cmdStep)
      doStep(encoderCount);
    else if (cmdBw)
      doBandwidth(encoderCount);
    else if (cmdSoftMute)
      doSoftMute(encoderCount);
    else if (cmdAvc)
      doAvc(encoderCount);
    else if (cmdBand)
    {
      if (encoderCount == 1)
        bandUp();
      else
        bandDown();
    }
    else if (bfoOn)
    {
      currentBFO = (encoderCount == 1) ? (currentBFO + currentBFOStep) : (currentBFO - currentBFOStep);
      si4735.setSSBBfo(currentBFO);
      resetEepromDelay();
      //previousFrequency = 0; // Forces eeprom update
      showBFO();
    }
    else
    {
      if (encoderCount == 1)
      {
        si4735.frequencyUp();
        seekDirection = 1;
      }
      else
      {
        si4735.frequencyDown();
        seekDirection = 0;
      }
      // Show the current frequency only if it has changed
      currentFrequency = si4735.getFrequency();
      showFrequency();
    }
    encoderCount = 0;
    resetEepromDelay(); // if you moved the encoder, something was changed
    elapsedRSSI = millis16; //millis();
    countRSSI = 0;
  }
}

void loop()
{
uint8_t x;
  millis16 = millis();                   // looses 16 bits, but we are just fine with the lower word...
#if defined(DEBUG) && defined(DEBUG_BUTTONS_ONLY)
  doEncoderAction();
  btn_BandUp.checkEvent(buttonEvent) ;
  btn_BandDn.checkEvent(buttonEvent) ;
  btn_VolumeUp.checkEvent(buttonEvent);
  btn_VolumeDn.checkEvent(buttonEvent);
  btn_Bandwidth.checkEvent(buttonEvent) ;
  btn_AGC.checkEvent(buttonEvent) ;
  btn_Step.checkEvent(buttonEvent) ;
  btn_Mode.checkEvent(buttonEvent) ;
  btn_Encoder.checkEvent(buttonEvent) ;
#else

  // Check if the encoder has moved.
  doEncoderAction();
  // Check button commands
    // check if some button is pressed
    // Events are handled by the specific "event-handler", i. e. the callback function passed to checkEvent() 

   btn_BandUp.checkEvent(bandEvent);
   btn_BandDn.checkEvent(bandEvent);
   btn_VolumeUp.checkEvent(volumeEvent);
   btn_VolumeDn.checkEvent(volumeEvent);
   btn_Step.checkEvent(stepEvent);
   btn_Bandwidth.checkEvent(bandwidthEvent);
   btn_AGC.checkEvent(agcEvent);
   btn_Mode.checkEvent(modeEvent);

   btn_Encoder.checkEvent(encoderEvent);

  
  // Show RSSI status only if this condition has changed
  if ((millis16 - elapsedRSSI) > MIN_ELAPSED_RSSI_TIME)// * 9)
  {
    si4735.getCurrentReceivedSignalQuality();
    int aux = si4735.getCurrentRSSI();
    if (rssi != aux)
    {
      rssi = aux;
      showRSSI();
    }

    if (countRSSI++ > 3)
    {
      disableCommand(NULL, NULL); // disable all command buttons
    }
    else
      elapsedRSSI = millis16;
  }


  if (currentMode == FM)
    if (!rdsOff)
        {
          checkRDS();
        }

  // Show the current frequency only if it has changed
  if (currentFrequency != previousFrequency)
  {
    if ((millis16 - storeTime) > STORE_TIME)
    {
      saveAllReceiverInformation();
      storeTime = millis16;
      previousFrequency = currentFrequency;
    }
  }
#endif
}
