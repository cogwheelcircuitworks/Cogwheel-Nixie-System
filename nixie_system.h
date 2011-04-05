/*

Copyright (C) 2011 by Cogwheel, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

///
/// nixie_system.h
///
/// Common definitions for Cogwheel Nixie System
///
/// (C)2010 Cogwheel, Inc. All Rights Reserved
///

/// formatting brought to you by astyle -A1 --indent=spaces=2

// ADDING AN OPTION
// - Create a new location in NVMem by adding a symbol to enum nvaddr
// - If the variable will be accessed allot, you should create a regular variable in ram which shadows it.
// - Create an entry in enum options 
// - Create a CLI command entry in CmdTab[],  
// - Add a help entry in HelpCLI[]
// - Implement a cmdXXX in DoCmd() and ShowCmds();
// - Set nvadXXX default value in NVMemInitialize();
// - Implement cmdXXX in DoCmd();
// - Implement optXXX entries in OptGet(), and OptSaveOption()
// - Implement entry in HelpButtons[]

#ifndef nixie_system_h
#define nixie_system_h

#include <avr/pgmspace.h>  // for PSTR() and PROGMEM() (strings in flash)

#include <avr/io.h> // for WDT Reset
#include <avr/wdt.h>

#define EOL "\r\n"
#define WHATNOTICE PSTR(": Cogwheel Nixie System")
#define COPYRIGHTNOTICE1 PSTR(": (C)2011 Cogwheel,Inc. Software licensed under terms of LGPLv3 and CC(BY)")

#ifdef MAIN
#define Extern
#define DS(x) =x
#else
#define Extern extern
#define DS(x)
#endif

//
// Display modes
//
enum dispModes
{
  // Norm
  dispOff            = 0x10, // top level mode cycle
  dispHrsMinsSecs    = 0x11,
  dispYear           = 0x12,
  dispDM             = 0x13,
  dispMD             = 0x14,

  // Set
  dispSetHrsMinsSecs = 0x22, // Set time mode
  dispSetOptions     = 0x43, // Set options mode

  // Test
  dispTests          = 0x80, // Test mode
  dispNone           = 0x00
};

// 
// useful bit tests of CurDispMode  
//
enum dispMasks
{
  dispIsNorm       = 0x10,
  dispIsSetHMS     = 0x20,
  dispIsSetOptions = 0x40,
  dispIsTest       = 0x80
};

Extern unsigned char CurDispMode;

Extern unsigned char DispCycle[8];
Extern unsigned char DispCycleCur;

//
// major configurations
//
enum configtypes
{
  confTst              = -1,
  confNoChange         =  0,
  confIN17x7           =  1,  // 7 on-board IN17
  conf8422x6IN17x1     =  2, // off-board 8422 (or IN12s) x 4 plus IN17x1
  confIN18x6,                // 4 IN18s
  confB7971x4,               // 4 B7971
  confB7971x8,               // 8 B7971
  // add more here
  confMAX
};
#define CONF_MIN confTst
#define CONF_MAX confIN17x7

const boolean OrderPrevails = 1;

//
// addresses of things stored in non-vol memory
//
//
enum nvaddrs
{
    nvadVerNumber,              // verifies nvm has been intialized
    nvadVerNumberHiByte,        
    
    nvadCheckSum,               // verifies nvm has been intialized
    nvadCheckSumHiByte,        

    nvadConfigType,

    nvadHVUserVal,              // user brightness setting
    nvadHVUserValHiByte,        // is a word so we need another slot here.

    nvadHVDutyCycleMinV,        // max/min duty cycle (high voltage)
    nvadHVDutyCycleMinVHiByte,  

    nvadHVDutyCycleMaxV,
    nvadHVDutyCycleMaxVHiByte,

    nvadCrossFade,              // fade between display changes
    nvadDateOrdMDY,             // date order should be MDY ?

    nvadOnTime,                 // turn display off hour
    nvadOffTime,                // turn display on hour

    nvadDSTOn,                  // Daylight Savings Time Enable
    nvadDSTFellBack,            // prevents repeats

    nvadDSTStartDayOfWeekCount, // dst: 1st, 2nd, 3rd, 4th, etc.
    nvadDSTStartDayOfWeek,      // dst: 1=sun, 2=mon, etc..
    nvadDSTStartMonth,          // dst: Month

    nvadDSTEndDayOfWeekCount,   // dst: 1st, 2nd, 3rd, 4th, etc.
    nvadDSTEndDayOfWeek,        // dst: 1=sun, 2=mon, etc..
    nvadDSTEndMonth,            // dst: Month

    nvadTransFx,                // transition effect 
    nvadWipeStyle,              // wipe effects style
    nvadWipeRate,               // wipe rate

    nvadDisplayCycle,           // display cycle

    nvadChime,                  // Audible periodic chime option

    nvadAlarmHrs,
    nvadAlarmMins,

    

#if 0
// Future soft options:   
   nvadHVMaxVal,               // Abs Max High voltage setting
    nvadHVMinVal,               // Abs Min High voltage setting
    nvadScanRateVal,            // Scan rate
    nvadNumDevsVal,             // Number of devices (tubes)
    nvadNumElemsVal,            // Numer of display elems per device
#endif
    nvadMAX
};

#define NV_VERSION 1   // determines initialization


//
// Microcontroller pin assignments
//
// NB: Arduino pin nomenclature
// PortD:  0-7
// PortB:  8-12
// PortC:  14-19
// (see C:\Arduino\hardware\cores\arduino\pins_arduino.c)
//        
//       
enum pins {
  pinHVSRHIZ    = 5,
  pinHVSRLE     = 6,
  pinLED        = 2,  // pre RevA HW: 13;
  pinNVCLK_SCLK = 17,
  pinNVCLK_IO   = 19,
  pinNVCLK_CE   = 3,
  pinBUT_1      = 14,
  pinBUT_2      = 15,
  pinBUT_3      = 16,
  pinHVPWM      = 9,
  pinBeep1      = 8,
  pinBeep2      = 11
};
#if 0
const int pinHVSRHIZ    = 5;
const int pinHVSRLE     = 6;
const int pinLED        = 2;  // pre RevA HW: 13;
const int pinNVCLK_SCLK = 17;
const int pinNVCLK_IO   = 19;
const int pinNVCLK_CE   = 3;
const int pinBUT_1      = 14;
const int pinBUT_2      = 15;
const int pinBUT_3      = 16;
#endif


#ifdef MAIN
//       1         2         3         4         5         6         7         8
//345678901234567890123456789012345678901234567890123456789012345678901234567890
PROGMEM char HelpCLI[] = {"\n\
: Commands Help\n\
: Entering command name alone will display current values.\n\
: \n\
: ala [0..23]:[0..59] |Set Alarm\n\
: bla [0..1]          |Blank Display\n\
: bri [0..15]         |Brightness: 0:off 15:bright\n\
: cfa [0..1]          |Cross-fade 0:off 1:on \n\
: chi [0..3]          |Chime 0:off 1:hourly 2:2x/hr 3:4x/hr\n\
: dcy [0..4]          |Display Cycle 0:off 1:60s 2:20s 3:30s 4:10s\n\
: dor [0..1]          |Display Order Date; 0 - 'mm dd yyyy', 1 - 'yyyy mm dd' \n\
: dow [1..7]          |Day of Week 1=Sunday \n\
: dse [0..1]          |Daylight Savings Time Enable \n\
: dst [...]           |Daylight Savings Time Set\n\
:                     |start-week-day-month, end-week-day-month\n\
: hel [c,b]           |Help on c)ommands or b)uttons\n\
: oft [0..23]         |Off Time \n\
: ont [0..23]         |On Time \n\
: rep                 |Report all option states\n\
: tfx [0..4]          |Transition Effects:0:None 1:Blink 2:Fade 3:Wipe-off 4:Wipe-slot\n\
: tim hhmmss.t [a,p]  |Set Time. For 12h end w/ a or p\n\
: ver                 |Show Version\n\
: wis [0..4]          |Wipe Style : 0:Right>Left 1:Left>Right 2:Toggle-dir 3:Random\n\
: wir [1..8]          |Wipe Rate : 1:Fastest 8:Slowest\n\
: ymd yyyy/mm/dd      |Set year, month, day \n\
: \n\
"};
#endif

//
// CLI Command names
//
enum CmdNums {
 cmdBLA,
 cmdBRI,
 cmdCFA,
 cmdCHI,
 cmdDCY,
 cmdDOR,
 cmdDOW,
 cmdDSE,
 cmdDST,
 cmdHEL,
 cmdOFT,
 cmdONT,
 cmdREP,
 cmdTFX,
 cmdTIM,
 cmdTST,
 cmdVER,
 cmdWIS,
 cmdWIR,
 cmdYMD,
 cmdALA,
 cmdMAX
};

typedef struct sCmdTab 
{
  char name[4];
  unsigned char num;
};

#ifdef MAIN
//
// map commands to command code
//
sCmdTab CmdTab[] = {
  { "ala", cmdALA},
  { "bri", cmdBRI},
  { "hel", cmdHEL},
  { "ver", cmdVER},
  { "tst", cmdTST},
  { "tim", cmdTIM},
  { "cfa", cmdCFA},
  { "chi", cmdCHI},
  { "ymd", cmdYMD},
  { "dor", cmdDOR},
  { "rep", cmdREP},
  { "ont", cmdONT},
  { "oft", cmdOFT},
  { "bla", cmdBLA},
  { "dow", cmdDOW},
  { "dse", cmdDSE},
  { "dst", cmdDST},
  { "dcy", cmdDCY},
  { "tfx", cmdTFX},
  { "wir", cmdWIR},
  { "wis", cmdWIS} 
};
#else
extern sCmdTab CmdTab[];
#endif

// interrupts enable/disable
Extern uint8_t _sreg;
inline void DisabIrups(void) { _sreg = SREG; cli(); }
inline void EnabIrups(void)  { SREG = _sreg; }

//
// Transition Effects
//
enum tfx { 
  tfxNone, 
  tfxBlink, 
  tfxFade, 
  tfxWipeViaOff, 
  tfxWipeViaSlotMachine, 
  tfxMAX 
};

Extern unsigned char TransFxCur, TransFxVal;

enum fxstyle 
{ 
  fxsWipeLR         = 0b00000000,
  fxsWipeRL         = 0b00000001,
  fxsWipeTog        = 0b00000010,
  fxsWipeRandom     = 0b00000011,

  fxsMAX 
};


Extern unsigned char FxWipeStyleCur, FxWipeStyleVal; 


#ifdef MAIN
PROGMEM unsigned char WipeTab[] = { // used by fxsWipeRandom
  0b00000010, // 0
  0b00000010, // 1
  0b01000010, // 2
  0b01001010, // 3
  0b01001110, // 4
  0b01001110, // 5
  0b01001110, // 6
  0b01001111, // 7
  0b11001111, // 8
  0b11001111, // 9
  0b11001111, // 10
  0b11001111, // 11
  0b11101111, // 12
  0b11101111, // 13
  0b11111111, // 14
  0b11111111, // 15
  0b11111111, // 16
  0b11111111, // 17
  0b11111111, // 18
  0b11111111, // 19
};
#else
Extern PROGMEM unsigned char WipeTab[];
#endif
#define WIPESTEPS 20


#define STARTUP_INTERVAL 3
#define DISPLAY_TIMEOUT_INTERVAL 90 


// TimeDigits[] keeps the time of day
Extern unsigned char TimeDigits[7]; // HH MM SS T 
#define TENTHS  TimeDigits[0]
#define SECONES TimeDigits[1]
#define SECTENS TimeDigits[2]
#define MINONES TimeDigits[3]
#define MINTENS TimeDigits[4]
#define HRONES  TimeDigits[5]
#define HRTENS  TimeDigits[6]

#define LTENTHS  LclTimeDigits[0]
#define LSECONES LclTimeDigits[1]
#define LSECTENS LclTimeDigits[2]
#define LMINONES LclTimeDigits[3]
#define LMINTENS LclTimeDigits[4]
#define LHRONES  LclTimeDigits[5]
#define LHRTENS  LclTimeDigits[6]


// Test Numbers
enum tstNumbers 
{
 tstHV             = 0,
 tstAllSame        = 1,
 tstJustOne        = 2,
 tstJustOneSeq     = 3,
 tstRamp           = 4,
 tstRandom         = 5,
 tstAllSameAnimate = 6,
 tstRandomAnimate  = 7,
 tstBeep           = 8,
 tstWipe           = 9,
 tstMAX
};



// options; used when setting from buttons
// order here determines order and option number
// when adjusting from buttons
enum options 
{
optAlarmHrs      = 1,
optAlarmMins       ,
opt12Hour          ,
optYear            ,
optMonth           ,
optDayOfMonth      ,
optDayOfWeek       ,
optBright          ,
optCrossFade       ,
optDateOrderMDY    ,
optTransFx         ,
optWipeStyle       ,
optWipeRate        ,
optDispCycle       ,
optChime           ,
optOnTime          ,
optOffTime         ,
optDSTEnable       ,
optDSTStrtDOWCount ,
optDSTStrtDOW      ,
optDSTStrtMo       ,
optDSTEndDOWCount  ,
optDSTEndDOW       ,
optDSTEndMo        ,
optMAX
};

#ifdef MAIN

PROGMEM char HelpButtons[] = {"\n\
: BUTTONS:\n\
: \n\
: Button Names: (from top to bottom or right to left): UP, DOWN, SET\n\
: While running normally, pressing any button briefly will cycle through date->time->off\n\
: To set time, press and hold any button until display changes then release. Then\n\
: use UP and DOWN to adjust the time, then press Set. To set options, repeat the same \n\
: procedure to enter set mode but do not set time. Instead, press SET again to step to option 01.\n\
: You may now press SET repeatedly to cycle through all options. Use UP/DOWN to adjust any option.\n\
: Exit set option mode by pressing and holding SET at any time. Release when normal operation\n\
: resumes.\n\
: \n\
: For more information options do a 'help cmds'. \n\
: The equivalent cli command are in ()'s \n\
: \n\
: 01 : Alarm Hours (ala)\n\
: 02 : Alarm Mins (ala)\n\
: 03 : 12/24 Hour Mode:0=12h 1=24h \n\
: 04 : Year (ymd)\n\
: 05 : Month (ymd)\n\
: 06 : Day of Month (ymd)\n\
: 07 : Day of Week (dow)\n\
: 08 : Brightness (bri)\n\
: 09 : Cross Fade (cfa)\n\
: 10 : Date Order (dom) \n\
: 11 : Transition Effect (tfx)\n\
: 12 : Wipe Style (wip)\n\
: 13 : Wipe Rate(wir)\n\
: 14 : Display Cycle (dcy)\n\
: 15 : Chime (chi)\n\
: 16 : On Time (ont)\n\
: 17 : Off Time (oft)\n\
: 18 : DST Enable (dse)\n\
: 19 : DST Start day of week count (dst)\n\
: 20 : DST Start day of week (dst)\n\
: 21 : DST Start month (dst)\n\
: 22 : DST End day of week count (dst)\n\
: 23 : DST End day of week (dst)\n\
: 24 : DST End month (dst)\n\
: \n\
"};
#else
extern PROGMEM char TestHelp[];
#endif



Extern unsigned char TestManageDiv DS(5);
Extern unsigned char TestManageCtr DS(TestManageDiv);



// The following enables me to use D(<printf syntax>) for debugging
Extern char chbuf[80]; // < careful not to overrun buffer.
#define D(...) { sprintf(chbuf, __VA_ARGS__); Serial.print(chbuf); Serial.print("\r\n"); }

#define _D(...) { sprintf(chbuf, __VA_ARGS__); Serial.print(chbuf); Serial.print("\r\n"); }

#define SIOPrintf(...) { sprintf(chbuf, __VA_ARGS__); Serial.print(chbuf); Serial.print("\r\n"); }

#define ASCII_BIAS 0x30

//
// Timekeeping
//
Extern unsigned int SecondsSinceReset DS(0);
Extern volatile bool HzFlag           DS(0); // ISR sets these. Programmed loop watches for them
Extern volatile bool TenHzFlag        DS(0);
Extern volatile bool OneHundredHzFlag DS(0);
Extern unsigned char SecDivCtr        DS(10);

#define TIMESET_INTERVAL_SECS_DEFAULT 20 
Extern unsigned char TimSetIntervalSecs DS(TIMESET_INTERVAL_SECS_DEFAULT);
Extern unsigned char TimSetIntervalSecsCtr DS(TIMESET_INTERVAL_SECS_DEFAULT);

Extern unsigned char TimeSinceLastCmdSecs DS(0);

Extern unsigned char HrsCtr; // 0..23
Extern unsigned char SecsCtr; // 0..59
Extern unsigned char MinsCtr; // 0..59
Extern unsigned char TenthsCtr; 

Extern unsigned char AlarmHrs, AlarmMins, AlarmSnoozeCtr, AlarmSnoozeVal;
Extern boolean AlarmTriggered DS(0);


Extern unsigned char Month, Day, Year, DayOfWeek;

#define DISPCYC_DWELL_SECS_DEFAULT 3
Extern unsigned char DispCycDwellCtr DS(DISPCYC_DWELL_SECS_DEFAULT);
Extern unsigned char DispCycDwellVal DS(DISPCYC_DWELL_SECS_DEFAULT);

Extern unsigned char DispCycIntvl; 

Extern unsigned char ChimeIntvl;

  //
// pure virtual declaration used by CNVMem and CNVClkMem
class CABSNVMem
{
public:

    virtual unsigned char Read(unsigned char) = 0;
    virtual unsigned int ReadWord(unsigned char) = 0;

    virtual void Write(unsigned char, unsigned char) = 0;
    virtual void WriteWord(unsigned char, unsigned int) = 0;

};


void Setup(void);
void loop(void);
inline void DisplayMux(void);
void TimeSetFromNVC(void);
void Inits();
void HVpwmManage(void);
void NVCInit(void);
void NVCWrb(unsigned char a, unsigned char d);
unsigned char NVCRdb(unsigned char a) ;
void NVCSetHMS(void) ;
void DoCmd(unsigned char, char *);
void ShowCmds(unsigned int CmdNum);
void TestButtonEventHandler(void);
void NormButtonEventHandler(void);
void SetTimeButtonEventHandler(void);
void SetOptionsButtonEventHandler(void);
void DecrementClock(void);
void TimeKeeping(unsigned char);
enum timekeeepingopts { tkoNorm, tkoIncByTenSecs };
void DispWriteHMS(void);
void TestHundredHzChores(void);
void TimeSet(char *ccp, boolean *cmdvalid) ;
enum OptNext { optnInit = 0, optnNext = 1 };
void OptNext(char);
void OptSaveOption(void);
void OptGet(void);
void OptAdjValBri(unsigned char Val);
void OptAdjVal(char dir);
unsigned char OptGetBrightVal();
void DisplayChange(unsigned char );
void report_free_mem(void);
void tolower(char *);
void IndicatePM(boolean On);
char * skiptonextarg(char *cp);
boolean NVMemIsInitialized(void); 
void NVMemInitialize(unsigned char configtype); 
void DoNVMemInit(unsigned char configtype);
void DoNVClkInit(void);
int RangeCheck(int curval, int incval, int min, int max, boolean *) ;
int XRangeCheck(int curval, int incval, int min, int max);
void DisplayOptVal(void);
void Blank(unsigned char bval);
void DispInfo(unsigned char dispmode);
unsigned char Twelve2TwentyFour();
void DispCycleInit(void);
void DispCycleReset(void);
bool DispIsCycling(void);
void DoTest(unsigned char, int, int, int);
void DSTGetSettings(void);
void NVMemManage(void);
void ChimeBusiness(void);
void UpdateMDY(void);
void StartAlarm(void);
void EnterTestMode(void);
void ShowVersionOnDisplay(void);

#endif // nixie_system_h

#if 0
// test inputs
//
ala 00:01
tim 00:00:50
bla 1

ala 01:01
oft 1 
ont 2
tim 00:59:52

#endif
