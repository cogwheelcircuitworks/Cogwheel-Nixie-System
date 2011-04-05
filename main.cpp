
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

// (bobc 20Aug2010)

// Beautification brought to you by astyle -A1

/*

*/

#include "WProgram.h"
#include "avr/wdt.h"
#include "TimerOne.h"
#include "MsTimer2.h"
#include "Spi.h"
#include <Print.h>

#define MAIN
#include "nixie_system.h"
#include "buildno.h"
#include "buttons.h"
#include "display.h"
#include "nvclk.h"
#include "eeprom.h"
#include "adc.h"
#include "sio.h"
#include "hv.h"
#include "led.h"
#include "dst.h"
#include "beep.h"

#undef int
#include <stdio.h>


#include <avr/pgmspace.h>


#undef D
// uncomment one or the other:
//#define D(...) // debugging off
#define D _D // debugging on


//
// main(void)
//
//
int main(void)
{
    boolean dotest = 0;

    Inits();           // one-time start-up initializations
    ShowCmds(cmdVER);  // boot-up banner on Serial
    //report_free_mem(); // important ram consumption info

    // start-up high-voltage
    HV.State = hvStRampUp;
    HV.SetPWM(HV.DutyCycleMaxValMinHV);

    DispCycleInit();


    // start-up scanning
    Disp.ScanEnable();

    // 
    // If a button is held down at power-up do configuration or tests 
    // 
    ButtonPowerUpState = Buttons.Eval(beaPowerUp);
    {
    boolean doreboot = 0;


    switch (ButtonPowerUpState)
    {
      case confNoChange:
        break;

      case confIN17x7:
        DoNVMemInit(confIN17x7);
        DoNVClkInit();
        doreboot++;
        break;

      case confTst:
        dotest++;
        break;

      default: // unimplemented power-up configs are met with same 'recycle now'response 
        doreboot++;
        break;
    }
    if (doreboot)
    {
      Led.SetPattern(".-.  R");
      SIO.TxConstSt(PSTR(": power cycle now"));
      SIO.TxSt(EOL);
      while(1) ;
    }

    }

    Disp.BlankMask(0b00000000); 

    TransFxCur = tfxNone;

    if (dotest)
    {
      EnterTestMode();
    }
    else
    {
      //
      // normal start
      //
      DoTest(tstAllSameAnimate,0,0,0); // Exercise all elements of all tubes
      DisplayChange(dispTests);
      Beep.SetPattern("! ! ! -"); // dit, dit, dit, dah !
      NVClk.GetHMS(TimeDigits);
    }


    //
    // main loop
    //
    while (OrderPrevails) // never exited under normal circumstances.. 
    {
        unsigned char c;

        while(!OneHundredHzFlag); // spin here to keep soft loop and isrs in sync
        OneHundredHzFlag = 0; // isr will set it 100x/sec.

        if (HzFlag) // 1x per second things
        {
            HzFlag = 0;

            SecsCtr =  (SECTENS*10) + SECONES;
            MinsCtr =  (MINTENS*10) + MINONES;
            HrsCtr = Twelve2TwentyFour();

            if ( !(AlarmHrs == 0 && AlarmMins == 0))// 00:00 means disabled
            {
                if (

                    AlarmHrs == HrsCtr &&
                    AlarmMins == MinsCtr &&
                    SecsCtr == 0 &&
                    !AlarmTriggered)
                {
                    StartAlarm();
                    AlarmTriggered = 1;
                    Disp.StateLocked = 0;
                    DisplayChange(dispHrsMinsSecs);
                }
            }

            // every minute
            if (SecsCtr == 0)
            {
                if (AlarmSnoozeCtr && AlarmSnoozeCtr++ &&
                        AlarmSnoozeCtr == AlarmSnoozeVal && AlarmTriggered)
                {
                    AlarmSnoozeCtr = 0;
                    StartAlarm();
                }

            }


            NVMemManage();

            if (SecondsSinceReset < 255)
                SecondsSinceReset++; // prevent wrap;

            if (TimeSinceLastCmdSecs < 255)
                TimeSinceLastCmdSecs++; // prevent wrap;



            // Every hour (xx:00:00.0)..
            if ( CurDispMode & dispIsNorm &&
                    SECTENS == 0 && SECONES == 0 &&
                    MINONES == 0 && MINTENS == 0)
            {
                // deal w/on and off times
                unsigned char ontime  = NVClk.Read(nvadOnTime);
                unsigned char offtime = NVClk.Read(nvadOffTime);
                if (ontime != offtime)
                {
                    if (HrsCtr == offtime)
                    {
                        DisplayChange(dispOff);
                        Disp.StateLocked = 1;
                    }
                    else if (HrsCtr == ontime)
                    {
                        Disp.StateLocked = 0;
                        DisplayChange(dispHrsMinsSecs);
                    }
                }

                UpdateMDY();

                // Check DST
                if ((NVClk.Read(nvadDSTOn) == 1) && HrsCtr == 2)
                {
                    // Start DST
                    if ( DST.StartNow(NVClk.GetMM(), NVClk.GetDD(), NVClk.GetDOW()))
                    {
                        NVClk.Write(nvadDSTFellBack,0);
                        NVClk.SetH(3);
                        SIO.TxConstSt(PSTR(":dst adj forward 1h")) ;
                        SIO.TxSt(EOL);
                        if (CurDispMode == dispHrsMinsSecs)
                            DisplayChange(dispHrsMinsSecs); // refreshes display
                    }
                    // End DST
                    else if (NVClk.Read(nvadDSTFellBack) != 1 &&
                             DST.EndNow(NVClk.GetMM(), NVClk.GetDD(), NVClk.GetDOW()))
                    {
                        NVClk.Write(nvadDSTFellBack,1);
                        NVClk.SetH(1);
                        SIO.TxConstSt(PSTR(":dst adj back 1h")) ;
                        SIO.TxSt(EOL);
                        if (CurDispMode == dispHrsMinsSecs)
                            DisplayChange(dispHrsMinsSecs); // refreshes display
                    }
                }
            }


            // done with startup, switch to dispHrsMinsSecs mode
            if (SecondsSinceReset == STARTUP_INTERVAL && !dotest)
            {

                HV.DutyCycleUserVal= NVClk.ReadWord(nvadHVUserVal);
                TransFxCur = TransFxVal;
                DisplayChange(dispHrsMinsSecs);
                Led.SetPattern(0);
                Led.BeepAlso = 0;
                TimeSinceLastCmdSecs = DISPLAY_TIMEOUT_INTERVAL+1; // prevent first time.
            }

            // if we've been left in an alternate mode, flip back to dispHrsMinsSecs
            if (CurDispMode != dispHrsMinsSecs && 
                TimeSinceLastCmdSecs == DISPLAY_TIMEOUT_INTERVAL && 
                !dotest)
                  DisplayChange(dispHrsMinsSecs);


            if (TimeSinceLastCmdSecs > 5 && !dotest)
            {
                if (TimSetIntervalSecsCtr)
                    TimSetIntervalSecsCtr--;

                if (!(TimSetIntervalSecsCtr)) // periodically set time from NVClk
                {
                    NVClk.GetHMS(TimeDigits);

                    if (CurDispMode & dispIsNorm)
                    {

                        if (NVClk.IsIn12HourMode())
                            IndicatePM(NVClk.IsPM());
                        else
                            IndicatePM(0);

                    }

                    TimSetIntervalSecsCtr = TimSetIntervalSecs;

                }

                // Cycle Display with other things besides time, periodically
                if (DispCycIntvl) 
                {
                    unsigned char mod,offset;
                    offset = 3;
                    switch(DispCycIntvl)
                    {
                        // dcy implementation:
                        //
                    case 4:
                        mod = 10;
                        break;  // every 10 secs
                    case 2:
                        mod = 20;
                        break;  // every 20 secs
                    case 3:
                        mod = 30;
                        break;  // every 30 secs
                    case 1:
                        mod = 60;
                        offset = 33;
                        break; // every 60 seconds
                    }


                    if (!((SecsCtr-offset)%mod))
                    {
                        DispCycDwellCtr = DispCycDwellVal-1; // triggers next block of code
                        DispCycleCur = 1;
                    }

                    if (DispCycDwellCtr) DispCycDwellCtr++;

                    if (DispCycDwellCtr == DispCycDwellVal)
                    {
                        if ((DispCycle[DispCycleCur]) == dispNone)
                            DispCycleReset();
                        else
                        {
                            DisplayChange(DispCycle[DispCycleCur]);
                            DispCycDwellCtr = 1;
                            DispCycleCur++;
                            if ((DispCycle[DispCycleCur]) == dispOff)  // skip over off  (it is there for the buttons)
                                DispCycleCur++;
                        }

                    }
                } // DispCycIntvl
            }


        } // HzFlag




        if (TenHzFlag)
        {
            TenHzFlag = 0;


            if (Buttons.AnyEvent && (CurDispMode & dispIsNorm))
                NormButtonEventHandler();

            else if ((Buttons.AnyEvent | Buttons.AnyPressedCtr) && (CurDispMode & dispIsSetHMS))
            {
                SetTimeButtonEventHandler();
                TimeSinceLastCmdSecs = 0;
            }
            else if ((Buttons.AnyEvent | Buttons.AnyPressedCtr) && (CurDispMode & dispIsSetOptions))
            {
                SetOptionsButtonEventHandler();
                TimeSinceLastCmdSecs = 0;
            }
            else if (Buttons.AnyEvent && (CurDispMode & dispIsTest))
            {
                TestButtonEventHandler();
            }


        }

        SIO.Check();
        SIO.IWork();


    } // while(OrderPrevails)

    return 0; // notreached
}

//
// void ChimeBusiness(void)
//
//   Called from main(), takes care of audible alerts and chimes
//
//
void ChimeBusiness(void)
{

  boolean dobeep = 0;

  if (TenthsCtr == 0)
    Beep.SyncWithSeconds = 1; // set

  if (AlarmTriggered)
    return;


    //
    // Chime
    //
    if ( !(CurDispMode == dispOff && Disp.StateLocked) &&
            (TenthsCtr == 0) && 
            (CurDispMode & dispIsNorm) && 
            ( SecsCtr == 55   || 
              SecsCtr == 10 || 
              SecsCtr == 25 || 
              SecsCtr == 40)
       )

    {

        switch(ChimeIntvl)
        {
        case 0: // not at all
            break;

        case 1: // on the hour
            if (MinsCtr == 59 && SecsCtr == 55)
                dobeep = 1;
            break;

        case 2: // half hour
            if ((MinsCtr == 59 || MinsCtr == 29) && SecsCtr == 55)
                dobeep = 1;
            break;

        case 3: // quarter hour
            if ((MinsCtr == 59 || MinsCtr == 29 || MinsCtr == 14 || MinsCtr == 44) && SecsCtr == 55)
                dobeep = 1;
            break;

        case 4: // every 15 secs. Ultra annoying.
            dobeep = 1;
            break;

        }
        if (dobeep) 
        {
            Beep.SyncWithSecondsEnabled = 1;
            Beep.SetPattern("! ! ! -");
        }
    }
}




//
// void DispWriteHMS(void)
//
//    Write current time to display buffer
//
void DispWriteHMS(void)
{
    unsigned char DispCtr;
    for (DispCtr = 0; DispCtr < Disp.NumDevs; DispCtr++)
    {
        Disp.Devs[DispCtr].NewVal = TimeDigits[DispCtr] & 0b00001111;
    }

}


//
// void DoTest(unsigned char TestNo, int Val1, int Val2, int Val3)
//
//   Executes Testno. Val1, Val2, and Val3 are optional parameters to the test
//
//
void DoTest(unsigned char TestNo, int Val1, int Val2, int Val3)
{

    unsigned char count;


    if (TestNo != tstJustOne)
    {
        Disp.ScanEnable();
        Disp.Blink();
    }
    Beep.SetPattern(0);

    for(count = 0; count < Disp.NumDevs; count++) Disp.Devs[count].NewVal = 0;

    switch(TestNo)
    {
// 0:HV 1:AllSame 2:JustOne 3:JustOneSeq 4:Ramp 5:Random 6:AllSameAnimate 7:RandomAnimate 8:Beep

    case tstHV:
        Disp.ScanOffTimer = 0;
        Disp.ScanDisable();
        Disp.Devs[0].NewVal = 0;
        Disp.ScanCtr = 255;
        Disp.Mux();
        HV.SetPWM(((unsigned int)(Val1&0x0FFF)));
        //SIOPrintf(":HVPWM: %d",HV.DutyCycleCurVal);
        break;

    case tstAllSame:
        for(count = 0; count < Disp.NumDevs; count++)
            Disp.Devs[count].NewVal = Val1;
        break;

    case tstJustOne:
    case tstJustOneSeq:
        Disp.ScanOffTimer = 0;
        Disp.ScanDisable();
        Disp.ScanCtr = Val1;
        Disp.Devs[Disp.ScanCtr].NewVal = Val2;
        --Disp.ScanCtr;
        Disp.Mux();
        TestManageCtr = TestManageDiv = Val3;
        break;


    case tstRandom:
        Disp.Devs[0].NewVal  = 0;
        Disp.Devs[1].NewVal = 9;
        Disp.Devs[2].NewVal = 1;
        Disp.Devs[3].NewVal = 8;
        Disp.Devs[4].NewVal = 2;
        Disp.Devs[5].NewVal = 7;
        Disp.Devs[6].NewVal = 3;
        Disp.Devs[7].NewVal = 6;
        break;


    case tstRamp:
        HV.State = hvStRampDown;
        break;


    case tstAllSameAnimate:
        TestManageCtr = TestManageDiv = Val1;
        break;

    case tstRandomAnimate:
        for(count = 0; count < Disp.NumDevs; count++)
            if (++Disp.Devs[count].NewVal >= Disp.NumDevs)
                break;

    case tstBeep:
        Beep.Val = Val1;
        Beep.SyncWithSecondsEnabled = 1;
        switch(Val2)
        {
        case 0:
            Beep.SetPattern("! R");
            break;
        case 1:
            Beep.SetPattern("|||||| R");
            break;
        case 2:
            Beep.SetPattern("- R");
            break;
        case 3:
            Beep.SetPattern("_ R");
            break;
        case 4:
            Beep.SetPattern("! ! ! - ");
            break;
        }

        break;

    case tstWipe:
        Disp.WipeRateVal = Val1;

        break;

    default:
        break;
    }

    Disp.CurTest = TestNo;
    Disp.CurTestVal = Val1;
}

//
// void TestHundredHzChores(void)
//
//    Called from on OneHundredHzFlag, performs various display dynamic test pattern functions
//    Mostly animations in certain modes
//
void TestHundredHzChores()
{
    unsigned char count;

    //
    // Skipping logic for doing things less than 1x per call
    //
    // Set TestManageDiv to zero and things happe 1x per call
    //
    // Set TestManageDiv to 1 things will happen every other call, etc..
    //
    if (TestManageDiv)
        if (!(--TestManageCtr))
            TestManageCtr = TestManageDiv;
        else
            return;

    switch(Disp.CurTest)
    {
    case tstRamp:
        if (HV.State == hvStAdjusted)
            HV.State = hvStRampDown;
        else if (HV.State == hvStOff)
            HV.State = hvStRampUp;
        break;

    case tstJustOne:
        if (TimeSinceLastCmdSecs >= 15)
            Disp.CurTest = tstJustOneSeq;
        break;

    case tstJustOneSeq:
#if 1
        if (Disp.ScanCtr == 0)
            for(count = 0; count < Disp.NumDevs; count++)
                if (++Disp.Devs[count].NewVal >= Disp.NumElems)
                    Disp.Devs[count].NewVal = 0;
#endif
        Disp.Mux();
        break;

    case tstAllSameAnimate:
        for(count = 0; count < Disp.NumDevs; count++)
            if (++Disp.Devs[count].NewVal >= Disp.NumElems)
                Disp.Devs[count].NewVal = 0;
        break;

    case tstRandomAnimate:
        for(count = 0; count < Disp.NumDevs; count++)
            if (++Disp.Devs[count].NewVal >= Disp.NumElems)
                break;

    default:
        break;

    }
}
// void TimeKeeping(opt)
//
// Called from TenHzNonMaskableChores(), performs actual abstract,
// hw-independent aspect of time keeping functions
//
// Generally, the processor keeps the time itself, but depends on
// regular resets from the NVClk
//
//    opt - tkoIncBySecs : increments by seconds. Used when setting
//    time from buttons.
//
//
void TimeKeeping12(unsigned char tkoption)
{
    TenthsCtr = TENTHS;

    if ( tkoption == tkoIncByTenSecs ||
            (tkoption == tkoNorm && (++TENTHS > 9 )))
    {
        TENTHS = 0;
        if ( tkoption == tkoIncByTenSecs ||
                (tkoption == tkoNorm && (++SECONES > 9 )))
        {
            SECONES = 0;


            if (++SECTENS > 5)
            {
                SECTENS = 0;
                if (++MINONES > 9)
                {
                    MINONES = 0;
                    if (++MINTENS > 5)
                    {
                        MINTENS = 0;

                        if (HRTENS < 1) // 01 02 03 04 05 06 07 08 09
                        {
                            if (++HRONES > 9)
                            {
                                HRONES = 0;
                                HRTENS = 1;
                            }
                        }
                        else // 10 11 -> 12  12->1
                        {
                            unsigned char OldHours = HRTENS*10 + HRONES;

                            if (++HRONES > 2)
                            {
                                HRTENS = 0;
                                HRONES = 1;
                            }
                            if (OldHours == 11 && HRTENS*10 + HRONES == 12)
                            {
                                if (NVClk.IsIn12HourMode())
                                {
                                    if (Disp.IsPM)
                                    {
                                        Disp.IsPM = 0;
                                        IndicatePM(0);
                                    }
                                    else
                                    {
                                        Disp.IsPM = 1;
                                        IndicatePM(1);
                                    }
                                }
                            }
                        }

                    }
                }
            }
        }
    }
}

//
// void TimeKeeping24(unsigned char tkoption)
//
//    Same role as TimeKeeping12(), but for 12 hour mode
//
//
void TimeKeeping24(unsigned char tkoption)
{
    TenthsCtr = TENTHS;

    if ( tkoption == tkoIncByTenSecs ||
            (tkoption == tkoNorm && (++TENTHS > 9 )))
    {
        TENTHS = 0;
        if ( tkoption == tkoIncByTenSecs ||
                (tkoption == tkoNorm && (++SECONES > 9 )))
        {
            SECONES = 0;
            if (++SECTENS > 5)
            {
                SECTENS = 0;
                if (++MINONES > 9)
                {
                    MINONES = 0;
                    if (++MINTENS > 5)
                    {
                        MINTENS = 0;

                        ++HRONES;
                        if ( (HRTENS <= 1 && HRONES > 9) || // handles 00 through 19
                                (  HRTENS == 2 && HRONES > 3 ) ) // handles 20 through 23
                        {
                            HRONES = 0;
                            if (++HRTENS > 2)
                                HRTENS = 0;
                        }

                    }
                }
            }
        }
    }
}
//
// void TimeKeeping(unsigned char tkoption)
//
//
//   Calls TimeKeeping12() or TimeKeeping24() as appropriate
//
//
void TimeKeeping(unsigned char tkoption)
{
    if (NVClk.IsIn12HourMode())
        TimeKeeping12(tkoption);
    else
        TimeKeeping24(tkoption);
}


//
// void TenHzNonMaskableChores(void)
//
//     Called 10x/second from OneKhzIrup();
//     Takes care of things that must get done 10/x per second no matter what.
//     Less critical things get done in the top while(OrderPrevails) loop
//     in main()
//
inline void TenHzNonMaskableChores(void)
{



    // HzFlag is the soft seconds indicator that other code watches for
    if (!(--SecDivCtr))
    {
        //D("TenHzNonMaskableChores, SecDivCtr:%d",SecDivCtr);
        SecDivCtr = 10;
        HzFlag = 1;
    }


    // restart a/d sampling on high voltage
    if (HVADC.DoneFlag)
    {
        HVADC.Restart();
        HVADC.DoneFlag = 0;
    }

    // Disp.ScanOffTimer allows us to blink the display

    if (Disp.ScanOffTimer)
    {
        if (Disp.ScanOffTimer++ == 3)
        {
            Disp.ScanOffTimer = 0;
            Disp.ScanEnable();
        }
    }

    if (CurDispMode != dispSetHrsMinsSecs)
        TimeKeeping(tkoNorm);

    switch(CurDispMode)
    {
    case dispOff:
        break;

    case dispHrsMinsSecs:
        DispWriteHMS();
        break;

    case dispSetHrsMinsSecs:
        DispWriteHMS();
        break;

    case dispSetOptions:
        break;

    default:
        break;
    }

    ChimeBusiness();

}


//
// void OneKhzIrup(void)
//
//    Called off of Timer2 -generated interrupt
//
void OneKhzIrup(void)
{
    const char OneHundredHzDiv = 5;
    static unsigned char OneHundredHzCtr = OneHundredHzDiv;

    const char TenHzDiv = 50;
    static unsigned char TenHzCtr = TenHzDiv;

    static bool FiveHundredHzTog;


    // We could have gotten away w/a 500khz irup if it weren't
    // for the need to get a reasonable high pitch out of the
    // beeper TODO in future hardware: Add dedicated or at least louder
    // hw for noise-making
    if (Beep.Enable)
    {
        if (!Beep.Ctr--)
        {
            digitalWrite(pinBeep1,    LOW);
            digitalWrite(pinBeep2,    HIGH);
            Beep.Ctr = Beep.Val;
        }
        else
        {
            digitalWrite(pinBeep1,    HIGH);
            digitalWrite(pinBeep2,    LOW);
        }
    }

    if (!FiveHundredHzTog)
    {
        FiveHundredHzTog = 1;
        return;
    }
    FiveHundredHzTog = 0;

    // from here  down gets executed 500x/sec

    if (Disp.ScanEnabled)
        Disp.Mux();


    // TenHz Things
    if (!(--TenHzCtr))
    {
        TenHzNonMaskableChores();

        TenHzCtr = TenHzDiv;
        TenHzFlag = 1; // indication to do soft TenHz Chores
    }

    // 100Hz things
    if (!(--OneHundredHzCtr))
    {
        Led.HundredHzChores();
        Beep.HundredHzChores();
        Disp.HundredHzChores();
        HV.HundredHzChores();
        Buttons.HundredHzChores();

        if (CurDispMode == dispTests)
            TestHundredHzChores();

        OneHundredHzCtr = OneHundredHzDiv;
        // pretty important this is the last thing
        // to happen here so main() keeps synced
        OneHundredHzFlag = 1;
    }
}

//
// void DSTGetSettings(void)
//
// Obtain DST Settings from NVmem
//
void DSTGetSettings(void)
{
    /*
    DST.StartDayOfWeekCount = dstLast;
    DST.StartDayOfWeek      = dstSunday;   // sunday
    DST.StartMonth          = dstMarch;  // in march
    */

    DST.StartDayOfWeekCount = NVClk.Read(nvadDSTStartDayOfWeekCount);
    DST.StartDayOfWeek      = NVClk.Read(nvadDSTStartDayOfWeek     );
    DST.StartMonth          = NVClk.Read(nvadDSTStartMonth         );

    DST.EndDayOfWeekCount   = NVClk.Read(nvadDSTEndDayOfWeekCount  );
    DST.EndDayOfWeek        = NVClk.Read(nvadDSTEndDayOfWeek       );
    DST.EndMonth            = NVClk.Read(nvadDSTEndMonth           );

}



//
// void IndicatePM(boolean On)
//
//
//   On : 1 indicate somehow that it is PM
//      : 0 AM
//
//
// This just turns the LED on if PM is set *if* the
// ledMode is set to ledAMPM
//
// This enables us to override the function of the LED for
// other things.
//
//
void IndicatePM(boolean On)
{
    if (Led.Mode == ledAMPM)
    {
        if (On)
            digitalWrite(pinLED, LOW);
        else
            digitalWrite(pinLED, HIGH);
    }
}


//
// void DoCmd(unsigned int hash, char *cparg)
//
//    Execute an operational command
//
//     unsigned int hash    : command id
//     unsigned char *cparg : parameter
//
void DoCmd(unsigned char cmdid, char *cparg)
{
    char *cp,copt1,copt2,copt3;
    int iopt1,iopt2,iopt3,iopt4, iopt5, iopt6;
    boolean paramvalid = 0;
    boolean showcmd = 0;
    boolean cmdvalid = 0;
    unsigned char invalids = 0;


    cmdvalid = paramvalid = 1; // optimistic

    if (DispIsCycling())
    {
        DispCycleReset();
        DisplayChange(dispHrsMinsSecs);
    }

    if (*cparg == '\r' || *cparg == '\n')
        *cparg = 0;

    switch(cmdid)
    {

    case cmdVER:
        showcmd++; // will trigger call to ShowCmds()
        break;

    case cmdREP:
        for(iopt1 = 0; iopt1 < cmdMAX; iopt1++)
        {
            if (iopt1 != cmdVER &&
                iopt1 != cmdHEL &&
                iopt1 != cmdTST &&
                iopt1 != cmdREP)

                ShowCmds(iopt1);
        }

        break;

    case cmdTIM:
        if (CurDispMode != dispHrsMinsSecs)
            DisplayChange(dispHrsMinsSecs);  // then change to HMS
        if (*cparg)
            TimeSet(cparg,&paramvalid);
        else
            showcmd++; // will trigger call to ShowCmds()
        break;

    }


    if (*cparg)
    {
        switch(cmdid)
        {
        case cmdBRI:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,15,&paramvalid);

            if (paramvalid)
            {
                OptAdjValBri(copt1);
                Disp.CurOpt =  optBright;
                OptSaveOption();
            }
            break;

        case cmdTST:
            copt1 = atoi(cparg);
            iopt1 = atoi((cparg = skiptonextarg(cparg)));
            iopt2 = atoi((cparg = skiptonextarg(cparg)));
            iopt3 = atoi((cparg = skiptonextarg(cparg)));
            copt1 = RangeCheck(copt1,0,0,15,&paramvalid);

            DisplayChange(dispTests);
            DoTest(copt1,iopt1,iopt2,iopt3);

            break;


        case cmdCFA:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,1,&paramvalid);
            if (paramvalid)
            {
                Disp.CrossFadeVal = copt1;
                Disp.CrossFadeRestore();
                NVClk.Write(nvadCrossFade,Disp.CrossFadeVal);
            }
            break;

        case cmdYMD:
            if (strlen(cparg) == 11)
            {
                // YYYY.MM.DD
                NVClk.SetYMD(cparg) ;
                UpdateMDY();
            }
            else
                paramvalid = 0;
            break;

        case cmdDOR:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,1,&paramvalid);
            if (paramvalid)
            {
                NVClk.Write(nvadDateOrdMDY,copt1);
                DispCycleInit();
            }
            break;

        case cmdONT:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,23,&paramvalid);
            if (paramvalid)
                NVClk.Write(nvadOnTime,copt1);
            break;

        case cmdOFT:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,23,&paramvalid);
            if (paramvalid)
                NVClk.Write(nvadOffTime,copt1);
            break;


        case cmdBLA:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,1,&paramvalid);
            if (paramvalid)
            {
                if (copt1 == 1)
                {
                    DisplayChange(dispOff);
                    Disp.StateLocked = 1;
                }
                else
                {
                    Disp.StateLocked = 0;
                    DisplayChange(dispHrsMinsSecs);
                }
            }
            break;

        case cmdDOW:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,1,7,&paramvalid);
            if (paramvalid)
            {
                NVClk.SetDOW(copt1);
                UpdateMDY();
            }
            break;


        case cmdDSE:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,1,&paramvalid);
            if (paramvalid)
            {
                NVClk.Write(nvadDSTOn, copt1);
                NVClk.Write(nvadDSTFellBack, 0);
            }
            break;



        case cmdDST:
            // dst startdowcount startdow startmonth enddowcount enddow endmonth
        {

            iopt1 = atoi(cparg);
            iopt2 = atoi((cparg = skiptonextarg(cparg)));
            iopt3 = atoi((cparg = skiptonextarg(cparg)));
            iopt4 = atoi((cparg = skiptonextarg(cparg)));
            iopt5 = atoi((cparg = skiptonextarg(cparg)));
            iopt6 = atoi((cparg = skiptonextarg(cparg)));

            iopt1 = RangeCheck(iopt1,0,1,5,&paramvalid);
            if (!paramvalid) invalids++;
            iopt2 = RangeCheck(iopt2,0,1,7,&paramvalid);
            if (!paramvalid) invalids++;
            iopt3 = RangeCheck(iopt3,0,1,12,&paramvalid);
            if (!paramvalid) invalids++;
            iopt4 = RangeCheck(iopt4,0,1,5,&paramvalid);
            if (!paramvalid) invalids++;
            iopt5 = RangeCheck(iopt5,0,1,7,&paramvalid);
            if (!paramvalid) invalids++;
            iopt6 = RangeCheck(iopt6,0,1,12,&paramvalid);
            if (!paramvalid) invalids++;

            if (!invalids)
            {

                NVClk.Write(nvadDSTStartDayOfWeekCount, iopt1);
                NVClk.Write(nvadDSTStartDayOfWeek     , iopt2);
                NVClk.Write(nvadDSTStartMonth         , iopt3);

                NVClk.Write(nvadDSTEndDayOfWeekCount  , iopt4);
                NVClk.Write(nvadDSTEndDayOfWeek       , iopt5);
                NVClk.Write(nvadDSTEndMonth           , iopt6);

                DSTGetSettings();

            }
            else
                paramvalid = 0;

        }
        break;

        case cmdHEL:
            if (*cparg == 'c')
            {
                SIO.TxConstSt(HelpCLI);
            }
            else if (*cparg == 'b')
            {
                SIO.TxConstSt(HelpButtons);
            }
            else
                paramvalid = 0;
            break;

        case cmdTFX:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,tfxMAX-1,&paramvalid);
            if (paramvalid)
            {
                NVClk.Write(nvadTransFx,copt1);
                TransFxCur = TransFxVal = copt1;
                
                copt1 = CurDispMode;
                DisplayChange(dispHrsMinsSecs);
                DisplayChange(copt1);
            }
            break;

        case cmdWIR:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,WIPE_RATE_MIN, WIPE_RATE_MAX, &paramvalid);
            if (paramvalid)
            {
                NVClk.Write(nvadWipeRate,copt1);
                Disp.WipeRateVal = copt1;

                // demonstrate it
                copt1 = CurDispMode;
                DisplayChange(dispHrsMinsSecs);
                DisplayChange(copt1);
            }
            break;

        case cmdWIS:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,fxsMAX-1,&paramvalid);
            if (paramvalid)
            {
                NVClk.Write(nvadWipeStyle,copt1);
                FxWipeStyleVal = copt1;
                FxWipeStyleCur = FxWipeStyleVal;

                // demonstrate it
                copt1 = CurDispMode;
                DisplayChange(dispHrsMinsSecs);
                DisplayChange(copt1);
            }
            break;


        case cmdDCY:
            copt1 = atoi(cparg);
            copt1 = RangeCheck(copt1,0,0,4,&paramvalid);
            if (paramvalid)
            {
                NVClk.Write(nvadDisplayCycle,copt1);
                DispCycIntvl = copt1;
            }
            break;

        case cmdCHI:
            iopt1 = atoi(cparg);
            iopt1 = RangeCheck(iopt1,0,0,4,&paramvalid);
            if (paramvalid)
            {
                NVClk.Write(nvadChime,iopt1);
                ChimeIntvl = iopt1;
            }
            break;

        case cmdALA:
            // HH:MM
            //   ^
            // 01234
            cparg[2] = 0;
            iopt1 = atoi(cparg);
            iopt1 = RangeCheck(iopt1,0,0,23,&paramvalid);
            if (!paramvalid) invalids++;

            // HH:MM
            //    ^
            // 01234
            iopt2 = atoi(&cparg[3]);
            iopt2 = RangeCheck(iopt2,0,0,59,&paramvalid);
            if (!paramvalid) invalids++;

            if (!invalids)
            {
                NVClk.Write(nvadAlarmHrs,iopt1);
                AlarmHrs = iopt1;
                NVClk.Write(nvadAlarmMins,iopt2);
                AlarmMins = iopt2;
            }
            else
                paramvalid = 0;

            break;


        } // switch
    }
    else
        showcmd++;

    if (showcmd)
    {
        // did something
        SIO.TxSt(EOL);

        if (cmdid != cmdREP)
            ShowCmds(cmdid);
    }
    else if (!paramvalid || !cmdvalid)   // did nothing above
    {
        SIO.TxSt(EOL);
        SIO.TxConstSt(PSTR(":? Type 'help c' for Help"));
        SIO.TxSt(EOL);
    }

    TimeSinceLastCmdSecs = 0;

}


void ShowCmdName(unsigned char CmdNum)
{
    unsigned int i;
    for (i = 0; i < cmdMAX; i++)
        if (CmdTab[i].num == CmdNum)
            break;

    if (i != cmdMAX)
        SIO.TxSt(":");
    SIO.TxSt(CmdTab[i].name);
    SIO.TxSt(" ");
}

//
// void ShowCmds(unsigned int CmdNum)
//
//   Display Output of Commands for
//   Command Line Interpreter.
//   Normally called from DoCmd() when no options
//   have been entered
//
void ShowCmds(unsigned int CmdNum)
{

    unsigned char tmp;

    if (CmdNum != cmdVER && CmdNum != cmdHEL)
        ShowCmdName(CmdNum);

    switch (CmdNum)
    {
    case cmdVER:
        SIO.TxSt(EOL);
        SIO.TxConstSt(WHATNOTICE);
        SIO.TxSt(EOL);
        SIO.TxConstSt(PSTR(": Ver:"));
        SIO.TxConstSt(PSTR(VERSION));
        SIO.TxConstSt(PSTR(", Build:"));
        SIO.TxConstSt(PSTR(BUILDNUMBER));
        SIO.TxConstSt(PSTR(", "));
        SIO.TxConstSt(PSTR(BUILDDATE));
        SIO.TxSt(EOL);
        SIO.TxConstSt(COPYRIGHTNOTICE1);
        SIO.TxSt(EOL);
        break;


    case cmdTIM:
        sprintf(chbuf,"%d%d:%d%d:%d%d.%d",
                HRTENS,HRONES,MINTENS,MINONES,SECTENS,SECONES,TENTHS);
        if (NVClk.IsIn12HourMode())
        {
            if (NVClk.IsPM())
                strcat(chbuf," p");
            else
                strcat(chbuf," a");
        }
        SIO.TxSt(chbuf);
        break;

    case cmdBRI:
        sprintf(chbuf,"%d",OptGetBrightVal());
        SIO.TxSt(chbuf);
        break;

    case cmdCFA:
        sprintf(chbuf,"%d",OptGetBrightVal());
        if (NVClk.Read(nvadCrossFade) == 1)
            SIO.TxSt("1");
        else
            SIO.TxSt("0");

        break;

    case cmdYMD:
        sprintf(chbuf,"%d",OptGetBrightVal());
        sprintf(chbuf,"20%2.2d.%2.2d.%2.2d",NVClk.GetYY(),NVClk.GetMM(), NVClk.GetDD());
        SIO.TxSt(chbuf);

        break;

    case cmdDOR:
        if (NVClk.Read(nvadDateOrdMDY) == 1)
            SIO.TxSt("1");
        else
            SIO.TxSt("0");

        break;

    case cmdONT:
        sprintf(chbuf,"%d",NVClk.Read(nvadOnTime));
        SIO.TxSt(chbuf);
        break;

    case cmdOFT:
        sprintf(chbuf,"%d",NVClk.Read(nvadOffTime));
        SIO.TxSt(chbuf);
        break;

    case cmdBLA:
        sprintf(chbuf,"%d",Disp.Blanked);
        SIO.TxSt(chbuf);
        break;

    case cmdDOW:
        sprintf(chbuf,"%d",NVClk.GetDOW());
        SIO.TxSt(chbuf);
        break;

    case cmdDSE:
        if (NVClk.Read(nvadDSTOn) == 1)
            SIO.TxSt("1");
        else
            SIO.TxSt("0");
        break;

    case cmdCHI:
        sprintf(chbuf,"%d",ChimeIntvl);
        SIO.TxSt(chbuf);
        break;

    case cmdALA:
        sprintf(chbuf,"%2.2d:%2.2d",AlarmHrs,AlarmMins);
        SIO.TxSt(chbuf);
        break;

    case cmdDST:
        sprintf(chbuf, "%d %d %d %d %d %d",
                NVClk.Read(nvadDSTStartDayOfWeekCount),
                NVClk.Read(nvadDSTStartDayOfWeek     ),
                NVClk.Read(nvadDSTStartMonth         ),

                NVClk.Read(nvadDSTEndDayOfWeekCount  ),
                NVClk.Read(nvadDSTEndDayOfWeek       ),
                NVClk.Read(nvadDSTEndMonth           )
               );
        SIO.TxSt(chbuf);
        break;

    case cmdTFX:
        sprintf(chbuf,"%d",NVClk.Read(nvadTransFx));
        SIO.TxSt(chbuf);
        break;

    case cmdWIS:
        sprintf(chbuf,"%d",FxWipeStyleVal);
        SIO.TxSt(chbuf);
        break;

    case cmdWIR:
        sprintf(chbuf,"%d",Disp.WipeRateVal);
        SIO.TxSt(chbuf);
        break;

    case cmdDCY:
        sprintf(chbuf,"%d",NVClk.Read(nvadDisplayCycle));
        SIO.TxSt(chbuf);
        break;

    case cmdHEL:
        SIO.TxSt(":help [cmds,buttons]");
        break;
    }
    SIO.TxSt(EOL);

}

//
// void DisplayChange(unsigned char NewMode)
//
//
//   Takes care of switching the display over from one mode to
//   NewMode. See dispModes in display.h for list of modes
//
//
void DisplayChange(unsigned char NewMode)
{
    static boolean PrevFadeState;
    unsigned char c,i;
    unsigned char TransFxTmp;


    if (Disp.StateLocked)
        return;

    HV.SetPWM(HV.DutyCycleUserVal);

    Disp.ScanEnable();

    // no fancy transition when not in normal mode
    if (!(NewMode & dispIsNorm) ||
            !(CurDispMode & dispIsNorm) )
        TransFxTmp = tfxBlink;
    else
        TransFxTmp = TransFxCur;



    // Entry Transition Effects
    if (CurDispMode != dispOff)
    {
        switch(TransFxTmp)
        {
        case tfxNone:
            break;
        case tfxBlink:
            Disp.Blink();
            break;
        case tfxFade:
            Disp.FadeDown();
            break;
        case tfxWipeViaOff:
            Disp.WipeToWhat(wipeOff);
            break;
        case tfxWipeViaSlotMachine:
            Disp.WipeToWhat(wipeAltVal);
            break;
        }
    }


    if (NewMode != dispHrsMinsSecs)
    {
        Led.SetState(0);
        Disp.CrossFadeClear();
    }

    CurDispMode = NewMode;

    Disp.Modified = 0;

    // Change to NewMode
    switch (NewMode)
    {
    case dispOff:
        Disp.Blanked = 1;
        HV.Shutdown();
        break;

    case dispHrsMinsSecs:
        Disp.BlankMask(0);
        //NVClk.GetHMS(TimeDigits);
        if (NVClk.IsIn12HourMode())
        {
            Led.Mode = ledAMPM;
            IndicatePM(NVClk.IsPM());
        }
        break;

    case dispYear:
    case dispMD:
    case dispDM:
        DispInfo(NewMode);
        break;


    case dispSetHrsMinsSecs:
        Disp.BlankMask(0);
        if (NVClk.IsIn12HourMode())
        {
            // while we are setting, we keep the AM/PM info here
            Disp.IsPM = NVClk.IsPM();

            Led.Mode = ledAMPM;
            IndicatePM(Disp.IsPM);
        }

        break;

    case dispSetOptions:
        Disp.BlankMask(0b00011000);
        OptNext(optnInit);
        break;

    case dispTests:
        Disp.CrossFadeClear();
        break;

    default:
        break;
    }
    // Transition Effects completion
    switch(TransFxTmp)
    {
    case tfxNone:
        break;
    case tfxBlink:
        while(!Disp.ScanEnabled);
        break;
    case tfxFade:
        Disp.FadeUp();
        break;
    case tfxWipeViaOff:
    case tfxWipeViaSlotMachine:
        Disp.WipeToOn();
        break;
    }

    if (CurDispMode == dispHrsMinsSecs)
        Disp.CrossFadeRestore();

    Buttons.Flush(bflEventsOnly);
    Buttons.IgnoreUntilRelease = 1;
}




//
// void ButtonEventHandler(void)
//
//    Called when a button is pressed or released
//
//    If there was also some other low-level command navigation like ir remote, etc
//    this would be the place to put it
//
void TestButtonEventHandler(void)
{
    unsigned char count;

    // D("TestButtonEventHandler(): IUR:%d",Buttons.IgnoreUntilRelease);

    // Pressed long time means get out of this Mode
    if (Buttons.AnyPressedCtr > BUT_PRESSED_LONGTIME && Buttons.SelPressed())
    {
        // Pressing and holding any button for a long time will toggle you in and out of set mode

        DisplayChange(dispHrsMinsSecs);

        Buttons.IgnoreUntilRelease = 1;
    }

    if (Buttons.SelReleasedEvent) // Button SEL
    {
        // Sel pressed. Change test #
        if (++Disp.CurTest >= tstMAX)
            Disp.CurTest = 0;

        DoTest(Disp.CurTest,0,0,0);

    }

    if (Buttons.UpReleasedEvent) // Button UP
    {
        switch (Disp.CurTest)
        {
        case tstHV:
            HV.DutyCycleCurVal -= 5;
            HV.SetPWM(HV.DutyCycleCurVal);
            break;

        case  tstRandom:
        case tstAllSame:
            for(count = 0; count < Disp.NumDevs; count++) // cycle through digits
                if (++Disp.Devs[count].NewVal >= Disp.NumElems)
                    Disp.Devs[count].NewVal = 0;
            break;

        case  tstJustOne:
            Disp.Mux(); // move to next display device
            break;

        case  tstAllSameAnimate:
        case  tstRandomAnimate:
        case tstJustOneSeq:
            TestManageDiv++; // changes rate animation rate
            break;

        }

    }

    if (Buttons.DwnReleasedEvent) // Button Down
    {
        switch (Disp.CurTest)
        {
        case tstHV:
            HV.DutyCycleCurVal += 5;
            HV.SetPWM(HV.DutyCycleCurVal);
            break;

        case  tstRandom:
        case tstAllSame:
        case  tstJustOne:
            for(count = 0; count < Disp.NumDevs; count++) // cycle through digits
            {
                if (Disp.Devs[count].NewVal)
                    --Disp.Devs[count].NewVal;
                else
                    Disp.Devs[count].NewVal = Disp.NumElems - 1;

            }
            // tstJustOne we are in charge of muxing.
            if (Disp.CurTest  == tstJustOne)
            {
                --Disp.ScanCtr;
                Disp.Mux();
            }

            break;


        case  tstAllSameAnimate:
        case  tstRandomAnimate:
        case tstJustOneSeq:
            if (TestManageDiv)
                --TestManageDiv;
            break;

        }
    }

    //if (Buttons.AnyEvent) { }

    Buttons.Flush(bflEventsOnly);

}

//
// void ButtonEventHandler(void)
//
//    Called when a button is pressed or released
//
//    If there was also some other low-level command navigation like ir remote, etc
//    this would be the place to put it
//
void NormButtonEventHandler(void)
{
    unsigned char count;

    if (Buttons.AnyReleasedEvent)
    {
        Disp.StateLocked = 0;

        //
        // If alarm is triggered...
        // A press shuts it off
        if (AlarmTriggered)
        {
            Beep.SetPattern(0);
            SIO.TxConstSt(PSTR(": Alarm Snooze")); SIO.TxSt(EOL);
            AlarmSnoozeCtr = 1;
            Buttons.IgnoreNextEvent = 1;
            return;
        }
        //
        // Pressing and releasing any button briefly will cycle through a top set of options
        //
        DispCycleCur++;
        if (DispCycle[DispCycleCur] == 0)
            DispCycleCur = 0;


        DisplayChange(DispCycle[DispCycleCur]);
        //
        // If display commanded off then we keep it off
        //
        if (DispCycle[DispCycleCur] == dispOff)
            Disp.StateLocked = 1;



    }
    if (Buttons.AnyPressedLongTimeEvent)
    {
        if (AlarmTriggered)
        {
            AlarmTriggered = 0;
            SIO.TxConstSt(PSTR(": Alarm Disabled")); SIO.TxSt(EOL);
            Beep.SetPattern("--!-");
            AlarmSnoozeCtr = 0;
            Buttons.Flush(bflEventsOnly);
            return;
        }
        // D("NormButtonEventHandler(): You press me long time");
        // Pressing and holding any button for a long time will transfer you into the next mode
        DisplayChange(dispSetHrsMinsSecs); // then change to off

    }
    Buttons.Flush(bflEventsOnly);


}


//
// void ButtonEventHandler(void)
//
//    Called when a button is pressed or released
//
//    If there was also some other low-level command navigation like ir remote, etc
//    this would be the place to put it
//
void SetOptionsButtonEventHandler(void)
{
    unsigned char count;

    unsigned char speedramp;



    // You get to set mode by pressing a long time.
    if (Buttons.AnyPressedCtr > BUT_PRESSED_LONGTIME && Buttons.SelPressed())
    {
        if (Disp.Modified)
            OptSaveOption();

        DisplayChange(dispHrsMinsSecs);
        Buttons.IgnoreUntilRelease = 1;
    }


    if (Buttons.DwnPressedEvent || (Buttons.DwnPressed() && Buttons.AnyPressedCtr > BUT_PRESSED_LONGTIME))
    {
        OptAdjVal(-1);
        Disp.Modified = 1; // indicates we have changed the time
    }
    if (Buttons.UpPressedEvent || (Buttons.UpPressed() && Buttons.AnyPressedCtr > BUT_PRESSED_LONGTIME))
    {
        OptAdjVal(1);
        Disp.Modified = 1; // indicates we have changed the time
    }
    if (Buttons.SelReleasedEvent)
    {
        // if option has been modified and they press SEL
        // We must write out the option
        if (Disp.Modified)
            OptSaveOption();

        OptNext(optnNext);
    }

    Buttons.Flush(bflEventsOnly);

}

  //
 // void EnterTestMode()
//
//   We get here through normal operation
//   or at power-up
//
void EnterTestMode()
{
  Buttons.KeyClickEnable = 0;
  Led.SetPattern("");
  DisplayChange(dispTests);
  DoTest(0,0,0,0); // start w/first test
  Buttons.IgnoreUntilRelease = 1;
}

//
// void ButtonEventHandler(void)
//
//    Called when a button is pressed or released
//
//    If there was also some other low-level command navigation like ir remote, etc
//    this would be the place to put it
//
void SetTimeButtonEventHandler(void)
{
    unsigned char count;

    unsigned char speedramp;

    // speedramp is a multiplier to accelerate rate of change for set time operations
    if (Buttons.AnyPressedCtr)
    {
        speedramp = 0;
        if (Buttons.AnyPressedCtr > 150)
            speedramp = 3;
        //if (Buttons.AnyPressedCtr > 350)
        //   speedramp = 10;
        if (Buttons.AnyPressedCtr > 550)
            speedramp = 100;

    }


    // You get to set mode by pressing a long time.
    // But you get to tests by pressing for for a really long time
    if ( Buttons.AnyPressedCtr > BUT_PRESSED_REALLY_LONGTIME && Buttons.SelPressed())
    {
      EnterTestMode();
    }

    if (Buttons.AnyPressedCtr > BUT_PRESSED_LONGTIME_LO &&
            Buttons.AnyPressedCtr < BUT_PRESSED_LONGTIME &&
            Buttons.SelPressed())
    {
        DisplayChange(dispHrsMinsSecs);
        Buttons.IgnoreUntilRelease = 1;
    }

    // implementation of speedramp count down
    if (Buttons.DwnPressedCtr > 150)
    {
        Disp.Modified = 1; // indicates we have changed the time
        for(count = 0; count < speedramp; count++)
            DecrementClock();

    }
    // implementation of speedramp count up
    if (Buttons.UpPressedCtr > 150)
    {

        for(count = 0; count < speedramp; count++)
            TimeKeeping(tkoIncByTenSecs);

        Disp.Modified = 1; // indicates we have changed the time
    }


    if (Buttons.DwnPressedEvent)
    {
        DecrementClock();
        Disp.Modified = 1; // indicates we have changed the time
    }
    if (Buttons.UpPressedEvent)
    {
        Disp.Modified = 1; // indicates we have changed the time
        TimeKeeping(tkoIncByTenSecs);

    }
    if (Buttons.SelReleasedEvent)
    {
        // if time has been modified and they press SEL, exit to dispHrsMinsSecs
        if (Disp.Modified)
        {
            if (NVClk.IsIn12HourMode())
            {
                NVClk.SetPM(Disp.IsPM);
            }

            NVClk.SetHMS(TimeDigits);
            DisplayChange(dispHrsMinsSecs);
            Buttons.IgnoreUntilRelease = 1;
        }
        else
        {
            DisplayChange(dispSetOptions);
            Buttons.IgnoreUntilRelease = 1;
        }


    }

    Buttons.Flush(bflEventsOnly);

}



#undef ASCII_VOLTAGE_BARGRAPH
#if defined(ASCII_VOLTAGE_BARGRAPH)
//
// void ASCIIVoltageBarGraph(void)
//
//    Shows continuous bar-graph of voltage coming from HV Generator
//
//
void ASCIIVoltageBarGraph(void)
{

    // ASCII bargraph of ADC input
    {
        unsigned int v, i;
        v = HVADC.ADC0Val;

        for(i = 0; i < 64; i++)
        {
            if (i > (v >> 3) )
                Serial.print(" ");
            else
                Serial.print("*");
        }
        Serial.print("\r");
    }
}
#endif


//
// void DecrementClock(void)
//
//   Decrements time. Used when setting time from buttons
//
//
void DecrementClock12(void)
{
    const unsigned char underflow = 255;


    TENTHS = SECONES = 0;
    if (--SECTENS == underflow)  // xx:xx:0x -> xx:xx:5x
    {
        SECTENS = 5;
        if (--MINONES == underflow) // xx:x0:xx -> xx:x9:xx
        {
            MINONES = 9;
            if (--MINTENS == underflow)  // xx:0x:xx -> 5x:xx:xx
            {
                MINTENS = 5;

                unsigned char OldHour = HRTENS*10 + HRONES;

                --HRONES;

                // 01 -> 12
                if (HRTENS == 0)
                {
                    // 09 08 07 06 05 04 03 02 01 -> 12
                    if (HRONES < 1)
                    {
                        HRTENS = 1;
                        HRONES = 2;
                    }
                }
                else // 12 11 10 -> 09
                {
                    if (HRONES == underflow)
                    {
                        HRTENS = 0;
                        HRONES = 9;
                    }
                }

                // 11p 09p 08p 07p 06p 05p 04p 03p 02p 01p 12p -> 11a
                // 10a 09a 07a 06a 05a 04a 03a 02a 01a 012a  -> 11p
                if ( (( ((HRTENS*10 + HRONES) == 11 && OldHour == 12)) ))
                {
                    if (Disp.IsPM)
                    {
                        Disp.IsPM = 0;
                        IndicatePM(0);
                    }
                    else
                    {
                        Disp.IsPM = 1;
                        IndicatePM(1);
                    }
                }

            }
        }
    }
}

//
// void DecrementClock24(void)
//
//
//   Make time go backwards in 24 h mode
//   Used to set time from the buttons
//
//
void DecrementClock24(void)
{
    const unsigned char underflow = 255;


    TENTHS = SECONES = 0;
    if (--SECTENS == underflow)  // xx:xx:0x -> xx:xx:5x
    {
        SECTENS = 5;
        if (--MINONES == underflow) // xx:x0:xx -> xx:x9:xx
        {
            MINONES = 9;
            if (--MINTENS == underflow)  // xx:0x:xx -> 5x:xx:xx
            {
                MINTENS = 5;

                --HRONES;
                if (HRONES == underflow) // x0:xx:xx -> x?:xx:xx
                {
                    // 23,22,21,20,19,18,17,16,15,14,13,12,11,10,09,08,07,06,05,04,03,02,01,00,23
                    //           ^--^                          ^--^                          ^--^
                    --HRTENS;
                    if (HRTENS == 1 || /* 20:xx:xx -> 19:xx:xx  */
                            HRTENS == 0)   /* 10:xx:xx -> 09:xx:xx  */
                        HRONES = 9;

                    if (HRTENS == underflow)   /* 00:xx:xx -> 23:xx:xx  */
                    {
                        HRONES = 3;
                        HRTENS = 2;
                    }
                }
            }
        }
    }
}

//
// void DecrementClock(void)
//
//
// Call either DecrementClock12() or DecrementClock24 as appropriate
//
void DecrementClock(void)
{
    if (NVClk.IsIn12HourMode())
        DecrementClock12();
    else
        DecrementClock24();
}
//
//
// void TimeSet(char *ccp) {
//
// Sets the currently running time. Note: Does not set the RTC's time.
//
void TimeSet(char *ccp,boolean *pcmdvalid)
{
    char *cp;
    unsigned char c;
    unsigned char buf[2];
    unsigned char LclTimeDigits[7];
    boolean cmdinvalid = 0;

    // OK:
    // "0"
    // "00"
    // "00:0"
    // "00:00:00.0"
    // "00:00:00"

    cp = ccp;


    buf[1] = 0;
    buf[0] = *cp;
    LHRTENS = atoi((const char *)buf);

    cp++;
    buf[0] = *cp;
    LHRONES = atoi((const char *)buf);

    c = (LHRTENS*10) + LHRONES;
    RangeCheck(c,0,0,23,pcmdvalid);
    cmdinvalid |= !*pcmdvalid;

    cp++;
    if (*cp == ':' || *cp == '.')
        cp++; // past colon if there is one

    buf[0] = *cp;
    LMINTENS = atoi((const char *)buf);

    cp++;
    buf[0] = *cp;
    LMINONES = atoi((const char *)buf);

    c = (LMINTENS*10) + LMINONES;
    RangeCheck(c,0,0,59,pcmdvalid);
    cmdinvalid |= !*pcmdvalid;


    cp++;
    if (*cp == ':' || *cp == '.')
        cp++; // past colon if there is one

    buf[0] = *cp;
    LSECTENS = atoi((const char *)buf);


    cp++;
    buf[0] = *cp;
    LSECONES = atoi((const char *)buf);
    c = (LSECTENS*10) + LSECONES;
    RangeCheck(c,0,0,59,pcmdvalid);
    cmdinvalid |= !*pcmdvalid;

    cp++;
    if (*cp == ':' || *cp == '.') cp++; // past dot

    if (*cp)
    {
        buf[0] = *cp;
        LTENTHS = RangeCheck(atoi((const char *)buf),0,0,9,pcmdvalid);
        cmdinvalid |= !*pcmdvalid;
    }
    else
        LTENTHS = 0;

    cp++;
    cp = skiptonextarg(cp);


    // 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23

    if (!cmdinvalid)
    {


        if (*cp == 'a')
        {
            NVClk.Set12HourMode(1);
            NVClk.SetPM(0);
            Led.Mode = ledAMPM;
        }
        else if (*cp == 'p')
        {
            NVClk.Set12HourMode(1);
            NVClk.SetPM(1);
            Led.Mode = ledAMPM;
        }
        else
        {
            Led.Mode = ledNorm;
            Led.SetState(0);
            NVClk.Set12HourMode(0);
        }


        for(c = 0; c < 8; c++)
            TimeDigits[c] = LclTimeDigits[c];

        NVClk.SetHMS(TimeDigits);

        if (NVClk.IsIn12HourMode())
            IndicatePM(NVClk.IsPM());
        else
            IndicatePM(0);

    }
    else
        *pcmdvalid = 0;

    // unstop
    Disp.Blanked = 0;
    Disp.ScanEnabled = 1;


}


//
// void tolower(char *cp)
//
//    Converts any upper case ascii to lower case
//
void tolower(char *cp)
{
    char *ocp = cp;
    do
    {
        if (*cp >= 'A' && *cp <= 'Z')
            *cp |= 0b00100000;

    }
    while(*cp++);
}



//
// skiptonextarg(char *cp)
//
//
//   Scans *cp, past whitespace, returns pointer to
//   next non whitespace char
//
char * skiptonextarg(char *cp)
{
    if (*cp && *cp != ' ' && *cp != '\t')
        while(*cp && *cp != ' ' && *cp != '\t')
            cp++;

    while( *cp == ' ' || *cp == '\t')
        cp++;

    return cp;
}


//
// unsigned char OptGetBrightVal()
//
//
//   Returns current brightness setting as a number 0-15
//
//
unsigned char OptGetBrightVal()
{
    // OptGetBrightVal();

    unsigned int step = (HV.DutyCycleMaxValMinHV - HV.DutyCycleMinValMaxHV) / 16;
    unsigned int i;
    unsigned char Val;

    for (i = HV.DutyCycleMinValMaxHV, Val = 15; i < HV.DutyCycleCurVal; Val--, i += step);


    return Val;
}


//
// void OptAdjValBri(unsigned char Val)
//
//   Val - 15 : brightest
//          0 : off
//
void OptAdjValBri(unsigned char Val)
{
    unsigned int step = (HV.DutyCycleMaxValMinHV - HV.DutyCycleMinValMaxHV) / 16;
    unsigned int i,j;



    // brightness is a number 0-15 no matter what the values of MaxValMinHV and MinValMaxHV are

    for (i = HV.DutyCycleMinValMaxHV, j = 15;
            j != Val;
            j--, i += step);

    if (CurDispMode & dispIsSetOptions)
    {
        Disp.Devs[0].NewVal = j % 10;
        Disp.Devs[1].NewVal = j / 10;
    }

    HV.DutyCycleUserVal = HV.DutyCycleCurVal = i;
    HV.SetPWM(HV.DutyCycleCurVal);
}

//
// void OptSaveOption(void)
//
//    Saves option # listed in Disp.CurOpt  to NV Mem
//
//
void OptSaveOption(void)
{
    boolean DoAGetSettings = 0;
    boolean DoAnUpdateMDY = 0;

    switch(Disp.CurOpt)
    {
        //  case opt12Hour: is taken care of in TimeSet()

    case optBright:
        NVClk.WriteWord(nvadHVUserVal,HV.DutyCycleCurVal);
        break;

    case optCrossFade:
        Disp.CrossFadeVal = Disp.CurOptVal;
        Disp.CrossFadeRestore();
        NVClk.Write(nvadCrossFade,Disp.CrossFadeVal);
        break;

    case optYear:
        NVClk.SetYY(Disp.CurOptVal);
        DoAnUpdateMDY++;
        break;

    case optMonth:
        NVClk.SetMM(Disp.CurOptVal);
        DoAnUpdateMDY++;
        break;

    case optDayOfMonth:
        NVClk.SetDD(Disp.CurOptVal);
        DoAnUpdateMDY++;
        break;

    case optDayOfWeek:
        NVClk.SetDOW(Disp.CurOptVal);
        DoAnUpdateMDY++;
        break;

    case optDateOrderMDY:
        NVClk.Write(nvadDateOrdMDY,Disp.CurOptVal);
        DispCycleInit();
        break;

    case optOnTime:
        NVClk.Write(nvadOnTime,Disp.CurOptVal);
        break;

    case optOffTime:
        NVClk.Write(nvadOnTime,Disp.CurOptVal);
        break;

    case optDSTEnable:
        NVClk.Write(nvadDSTOn,Disp.CurOptVal);
        break;

    case optDSTStrtDOWCount:
        NVClk.Write(nvadDSTStartDayOfWeekCount,Disp.CurOptVal);
        DoAGetSettings++;
        break;

    case optDSTStrtDOW:
        NVClk.Write(nvadDSTStartDayOfWeek,Disp.CurOptVal);
        DoAGetSettings++;
        break;

    case optDSTStrtMo:
        NVClk.Write(nvadDSTStartMonth,Disp.CurOptVal);
        DoAGetSettings++;
        break;

    case optDSTEndDOWCount:
        NVClk.Write(nvadDSTEndDayOfWeekCount,Disp.CurOptVal);
        DoAGetSettings++;
        break;

    case optDSTEndDOW:
        NVClk.Write(nvadDSTEndDayOfWeek,Disp.CurOptVal);
        DoAGetSettings++;
        break;

    case optDSTEndMo:
        NVClk.Write(nvadDSTEndMonth,Disp.CurOptVal);
        DoAGetSettings++;
        break;

    case optTransFx:
        NVClk.Write(nvadTransFx,Disp.CurOptVal);
        TransFxCur = TransFxVal = Disp.CurOptVal;
        break;

    case optWipeStyle:
        NVClk.Write(nvadWipeStyle,Disp.CurOptVal);
        FxWipeStyleVal = Disp.CurOptVal;
        FxWipeStyleCur = FxWipeStyleVal;
        break;

    case optWipeRate:
        NVClk.Write(nvadWipeRate,Disp.CurOptVal);
        Disp.WipeRateVal = Disp.CurOptVal;
        break;


    case optDispCycle:
        NVClk.Write(nvadDisplayCycle,Disp.CurOptVal);
        DispCycIntvl = Disp.CurOptVal;
        break;

    case optChime:
        NVClk.Write(nvadChime,Disp.CurOptVal);
        ChimeIntvl = Disp.CurOptVal;
        break;

    case optAlarmHrs:
        NVClk.Write(nvadAlarmHrs,Disp.CurOptVal);
        AlarmHrs = Disp.CurOptVal;
        break;

    case optAlarmMins:
        NVClk.Write(nvadAlarmMins,Disp.CurOptVal);
        AlarmMins = Disp.CurOptVal;
        break;

    default:
        break;
    }

    if (DoAGetSettings)
        DSTGetSettings();

    if (DoAnUpdateMDY)
        UpdateMDY();

}

//
// void OptAdjVal(char dir)
//
//
//   Adjusts the next option value
//
//       dir : -1 decrease by 1
//           : 1 advance by 1
//
void OptAdjVal(char dir)
{

    boolean cmdvalid;

    switch(Disp.CurOpt)
    {
    case opt12Hour:
        if (NVClk.IsIn12HourMode() & dir == -1)
        {
            TimeSet("00:00:00.0",&cmdvalid);
            Disp.CurOptVal = 0;
        }
        else if (!NVClk.IsIn12HourMode() & dir == 1)
        {
            TimeSet("12:00:00.0 a",&cmdvalid);
            Disp.CurOptVal = 1;
        }
        break;

    case optBright:
    {
        unsigned int step = (HV.DutyCycleMaxValMinHV - HV.DutyCycleMinValMaxHV) / 16;
        unsigned int i,j;
        if (dir == -1)
        {
            if (--Disp.CurOptVal >= 0)
                OptAdjValBri(Disp.CurOptVal);
            else
                Disp.CurOptVal = 0; // irony is they won't be able to see it on display
        }
        else if (dir == 1)
        {

            if (++Disp.CurOptVal <= 15)
                OptAdjValBri(Disp.CurOptVal);
            else
                Disp.CurOptVal = 15;
        }
    }
    break;

    case optYear:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,10,99,&cmdvalid);
        break;

    case optMonth:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,1,12,&cmdvalid);
        break;

    case optDayOfMonth:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,1,31,&cmdvalid);
        break;

    case optDayOfWeek:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,1,7,&cmdvalid);
        break;

    case optCrossFade:
        if (dir == -1 && Disp.CurOptVal != 0)
            Disp.CurOptVal--;
        else if (dir ==  1 && Disp.CurOptVal < 1)
            Disp.CurOptVal++;
        break;

    case optDateOrderMDY:
        if (dir == -1 && Disp.CurOptVal != 0)
            Disp.CurOptVal--;
        else if (dir ==  1 && Disp.CurOptVal < 1)
            Disp.CurOptVal++;
        break;


    case optOnTime:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,0,23,&cmdvalid);
        break;

    case optOffTime:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,0,23,&cmdvalid);
        break;


    case optDSTEnable:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,0,1,&cmdvalid);
        break;

    case optDSTStrtDOWCount:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,1,7,&cmdvalid);
        break;

    case optDSTStrtDOW:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,1,7,&cmdvalid);
        break;

    case optDSTStrtMo:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,1,12,&cmdvalid);
        break;

    case optDSTEndDOWCount:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,1,7,&cmdvalid);
        break;

    case optDSTEndDOW:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,1,7,&cmdvalid);
        break;

    case optDSTEndMo:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,1,12,&cmdvalid);
        break;

    case optTransFx:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,0,tfxMAX-1,&cmdvalid);
        break;

    case optWipeStyle:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,0,fxsMAX-1,&cmdvalid);
        break;

    case optWipeRate:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,WIPE_RATE_MIN, WIPE_RATE_MAX, &cmdvalid);
        break;

    case optDispCycle:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,0,4,&cmdvalid);
        break;

    case optChime:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,0,4,&cmdvalid);
        break;

    case optAlarmHrs:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,0,23,&cmdvalid);
        break;

    case optAlarmMins:
        Disp.CurOptVal = RangeCheck(Disp.CurOptVal,dir,0,59,&cmdvalid);
        break;


    }
    DisplayOptVal();

}


//
// void OptNext(char dir)
//
//    Display next option
//
//    dir : 0 - resets the first option
//        : 1 - advances to the next option
//
//    When the last option is reached, the display is switched to
//    dispSetHrsMinsSecs mode
//
//
void OptNext(char dir)
{
    if (dir == 0)
        Disp.CurOpt = 1;
    else if (++Disp.CurOpt >= optMAX)
    {
        DisplayChange(dispSetHrsMinsSecs);
        return;
    }

    OptGet();
}

//
// void OptGet()
//
// Sets Displayed Option to Disp.CurOpt
//
//
void OptGet(void)
{
    unsigned char c,i,val;

    for(i = 0; i < Disp.NumDevs; i++) // cycle through digits
        Disp.Devs[i].NewVal = 0;


    Disp.Devs[5].NewVal = Disp.CurOpt % 10;
    Disp.Devs[6].NewVal = Disp.CurOpt / 10;


    Disp.BlankMask(0b00011000);

    switch(Disp.CurOpt)
    {
    case opt12Hour:
        if (NVClk.IsIn12HourMode())
            Disp.CurOptVal = 1;
        else
            Disp.CurOptVal = 0;
        break;

    case optYear:
        Disp.BlankMask(0b10010000);
        Disp.Devs[3].NewVal = 2;
        Disp.CurOptVal = NVClk.GetYY();
        break;

    case optMonth:
        Disp.CurOptVal = NVClk.GetMM();
        break;

    case optDayOfMonth:
        Disp.CurOptVal = NVClk.GetDD();
        break;

    case optDayOfWeek:
        Disp.CurOptVal = NVClk.GetDOW();
        break;

    case optBright:
        Disp.CurOptVal = OptGetBrightVal();
        break;

    case optCrossFade:
        Disp.CurOptVal = NVClk.Read(nvadCrossFade);
        break;

    case optDateOrderMDY:
        Disp.CurOptVal = NVClk.Read(nvadDateOrdMDY);
        break;

    case optOnTime:
        Disp.CurOptVal = NVClk.Read(nvadOnTime);
        break;

    case optOffTime:
        Disp.CurOptVal = NVClk.Read(nvadOffTime);
        break;

    case optDSTEnable:
        Disp.CurOptVal = NVClk.Read(nvadDSTOn);
        break;

    case optDSTStrtDOWCount:
        Disp.CurOptVal = NVClk.Read(nvadDSTStartDayOfWeekCount);
        break;

    case optDSTStrtDOW:
        Disp.CurOptVal = NVClk.Read(nvadDSTStartDayOfWeek);
        break;

    case optDSTStrtMo:
        Disp.CurOptVal = NVClk.Read(nvadDSTStartMonth);
        break;

    case optDSTEndDOWCount:
        Disp.CurOptVal = NVClk.Read(nvadDSTEndDayOfWeekCount);
        break;

    case optDSTEndDOW:
        Disp.CurOptVal = NVClk.Read(nvadDSTEndDayOfWeek);
        break;

    case optDSTEndMo:
        Disp.CurOptVal = NVClk.Read(nvadDSTEndMonth);
        break;

    case optTransFx:
        Disp.CurOptVal = NVClk.Read(nvadTransFx);
        break;

    case optWipeStyle:
        Disp.CurOptVal = NVClk.Read(nvadWipeStyle);
        break;

    case optWipeRate:
        Disp.CurOptVal = NVClk.Read(nvadWipeRate);
        break;

    case optDispCycle:
        Disp.CurOptVal = NVClk.Read(nvadDisplayCycle);
        break;

    case optChime:
        Disp.CurOptVal = NVClk.Read(nvadChime);
        break;

    case optAlarmHrs:
        Disp.CurOptVal = NVClk.Read(nvadAlarmHrs);
        break;

    case optAlarmMins:
        Disp.CurOptVal = NVClk.Read(nvadAlarmMins);
        break;

    }

    DisplayOptVal();
}


//
// void DispInfo(unsigned char dispmode)
//
//
//   Display certain static info such as year, month, day
//
//
void DispInfo(unsigned char dispmode)
{
    unsigned char c,i,val;

    switch(dispmode)
    {

        // 65 43 21 0

    case dispYear:
        Disp.Devs[4].NewVal = 2;
        Disp.Devs[3].NewVal = 0;

        val = Year;
        Disp.Devs[2].NewVal = val / 10;
        Disp.Devs[1].NewVal = val % 10;

        Disp.BlankMask(0b11100001);
        break;

    case dispMD:
        val = Month;
        Disp.Devs[6].NewVal = val / 10;
        Disp.Devs[5].NewVal = val % 10;


        val = Day;
        Disp.Devs[2].NewVal = val / 10;
        Disp.Devs[1].NewVal = val % 10;

        Disp.BlankMask(0b10011001);

        break;

    }
}

//
// void CNVMem::Initialize(unsigned char configtype)
//
//    Initialize non-vol memory with a particular
//    configuration type. See enum configtype in nixie_system.h
//
//
void NVMemInitialize(unsigned char configtype)
{

    NVClk.WriteWord(nvadVerNumber,atoi(BUILDNUMBER));
    NVClk.Write(nvadConfigType,configtype);

    NVClk.Write(nvadCrossFade             , 1); // enable fading by default
    NVClk.Write(nvadDateOrdMDY            , 1); // date order
    NVClk.Write(nvadOnTime                , 0);
    NVClk.Write(nvadOffTime               , 0);
    NVClk.Write(nvadTransFx               , tfxWipeViaSlotMachine); // tfxWipeViaOff); 
    NVClk.Write(nvadWipeStyle             , fxsWipeTog); 
    NVClk.Write(nvadWipeRate              , WIPE_RATE_MEDIUM); 
    NVClk.Write(nvadDisplayCycle          , 1);
    NVClk.Write(nvadChime                 , 0);
    NVClk.Write(nvadAlarmHrs              , 0);
    NVClk.Write(nvadAlarmMins             , 0);

    NVClk.Write(nvadDSTOn                 , 1);
    NVClk.Write(nvadDSTFellBack           , 0);

    NVClk.Write(nvadDSTStartDayOfWeekCount, dstSecond+1);
    NVClk.Write(nvadDSTStartDayOfWeek     , dstSunday);
    NVClk.Write(nvadDSTStartMonth         , dstMarch);

    NVClk.Write(nvadDSTEndDayOfWeekCount  , dstFirst+1);
    NVClk.Write(nvadDSTEndDayOfWeek       , dstSunday);
    NVClk.Write(nvadDSTEndMonth           , dstNovember);


    switch(configtype)
    {
    case confIN17x7:
        SIO.TxConstSt(PSTR(": Config Type: IN17x7 DST USA"));
        NVClk.WriteWord(nvadHVDutyCycleMinV   , 816); // pwm setting for highest voltage
        NVClk.WriteWord(nvadHVDutyCycleMaxV   , 990); // pwm setting for lowest voltage
        NVClk.WriteWord(nvadHVUserVal         , 875);



        break;

    default:
        SIO.TxConstSt(PSTR(": Unsupported configuration type:"));
        SIOPrintf("%d",configtype);
        break;
    }

    // TODO: configtype should initialize configuration dependedent on
    // what our setup is; IN17x7, IN18x6, B7971x4, etc.

    NVMemManage(); // updates checksum

}

//
// void NVMemManage(void)
//
//
//   Called periodically to tend to anything related to NVMem
//
void NVMemManage(void)
{
    int nvad,sum;
    sum = 0;

    if (!NVClk.NeedsUpdating)
        return;


    for (nvad = 0; nvad != nvadMAX; nvad++)
    {
        if (nvad == nvadCheckSum ||
                nvad == nvadCheckSumHiByte)
            continue;

        sum += NVClk.Read(nvad);
    }
    NVClk.WriteWord(nvadCheckSum,sum);

    NVClk.NeedsUpdating = 0;
}

//
// boolean CNVMemIsInitialized(void)
//
//    Returns : 1 - yes,  0 - no
//
boolean NVMemIsInitialized(void)
{
    int nvad,sum;

    if (NVClk.ReadWord(nvadVerNumber) != atoi(BUILDNUMBER))
        return 0;

    sum = 0;
    for (nvad = 0; nvad != nvadMAX; nvad++)
    {
        if (nvad == nvadCheckSum ||
                nvad == nvadCheckSumHiByte)
            continue;

        sum += NVClk.Read(nvad);
    }

    if (sum != NVClk.ReadWord(nvadCheckSum))
        return 0;

    return 1;
}

//
// void DoNVMemInit(unsigned char configtype)
//
//
//   Just displays some descriptive text then calls NVMemInitialize();
//
//
void DoNVMemInit(unsigned char configtype)
{
    SIO.TxConstSt(PSTR(": Config Mem Init"));
    SIO.TxSt(EOL);
    NVMemInitialize(configtype);
}

//
// void DoNVClkInit(void)
//
//   Initializes non-vol clock to some reasonable value
//   in the not-to-distant past
//
void DoNVClkInit(void)
{
    boolean cmdvalid;
    TimeSet("00:00:00.0",&cmdvalid);
    NVClk.SetYMD("2010/09/19");
    NVClk.SetDOW(1/*sunday*/);
}

//
// int RangeCheck(int curval, int incval, int min, int max, boolean *pinrange)
//
//   Makes sure curval+incval is between min and max. If it isn't, it gets set
//   to min or max depending on which direction out of range it was
//
//   pinrange is set to indicate if curval+incval was in range
//
int RangeCheck(int curval, int incval, int min, int max, boolean *pinrange)
{
    *pinrange = 1;
    if (curval + incval < min)
    {
        *pinrange = 0;
        return min;
    }
    else if (curval + incval > max)
    {
        *pinrange = 0;
        return max;
    }
    else
    {
        return curval + incval;
    }
}

//
// void DisplayOptVal(void)
//
//    sends the value of CurOptVal
//    to the display.. 0..99
//
void DisplayOptVal(void)
{
    Disp.Devs[0].NewVal = Disp.CurOptVal % 10;
    Disp.Devs[1].NewVal = Disp.CurOptVal / 10;
}


//
// void DispCycleInit(void)
//
//
//   This sets what order the display gets cycled
//   under normal conditions
//
//   typically:
//
//   time -> year -> month -> day -> off ->back to beginning
//
//
void DispCycleInit(void)
{
    DispCycle[0] = dispHrsMinsSecs;

    // display yyyy -> month -> day or
    //         month -> day -> yyyy depending on optDateOrderMDY
    if (NVClk.Read(nvadDateOrdMDY))
    {
        DispCycle[1] = dispMD;
        DispCycle[2] = dispYear;
    }
    else
    {
        DispCycle[1] = dispYear;
        DispCycle[2] = dispMD;
    }

    DispCycle[3] = dispOff;
    DispCycle[4] = dispNone;

    DispCycleCur = 0;
    DispCycDwellCtr = 0;

}

//
// void DispCycleReset(void)
//
//  Restore Display to Normal timekeeping
//
//
void DispCycleReset(void)
{
    DispCycleCur = 0;
    DispCycDwellCtr = 0;
    DisplayChange(DispCycle[DispCycleCur]); // should be hhmmss.s
}

//
// bool DispIsCycling(void)
//
//
//   Returns 1 if DispIsCycling, otherwise 0
//
bool DispIsCycling(void)
{
    return DispCycleCur == 0 ? 0 : 1;
}

//
// void UpdateMDY(void)
//
//    Get local copies of Month, Day, Year, and DayOfWeek from
//    the Non-vol clock
//
void UpdateMDY(void)
{
    Month = NVClk.GetMM();
    Day = NVClk.GetDD();
    Year = NVClk.GetYY();
    DayOfWeek = NVClk.GetDOW();
}


//
// unsigned char Twelve2TwentyFour()
//
//   If we are in AM/PM mode, convert the 12 hour time
//   to 24 hour time
//
unsigned char Twelve2TwentyFour()
{

    /*
        12  1  2  3  4  5  6  7  8  9 10 11 12 01 02 03 04 05 06 07 08 09 10 11
        AM AM AM AM AM AM AM AM AM AM AM AM PM PM PM PM PM PM PM PM PM PM PM PM
         |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
         v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v
        00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23
    */

    unsigned char hour = HRONES + HRTENS*10;
    if (NVClk.IsIn12HourMode())
    {
        if (hour == 12 && !NVClk.IsPM()) // 12AM -> 00
            hour = 0;
        else if (NVClk.IsPM() && hour >= 1)
            hour += 12;
    }
    return hour;

}



//
// void Inits()
//
//   One-time startup initializations. Called from main()
//
void Inits()
{
    init();

    SIO.Init();
    NVClk.Init();

    if (!NVMemIsInitialized())
        DoNVMemInit(confIN17x7);

    Timer1.initialize();                   // setup Timer1 for PWM
    HV.State                   = hvStOff;
    HV.HVPin                   = pinHVPWM;
    HV.Period                  = 0xc0;     // basic duty cycle (doesn't get messed with)
    HV.DutyCycleInc            = 3;        // how many to step when ramping


    HV.DutyCycleMinValMaxHV    = NVClk.ReadWord(nvadHVDutyCycleMinV);
    HV.DutyCycleMaxValMinHV    = NVClk.ReadWord(nvadHVDutyCycleMaxV);


    HV.DutyCycleCurVal         = HV.DutyCycleMaxValMinHV;
    HV.DutyCycleUserValDefault = ((HV.DutyCycleMaxValMinHV-((HV.DutyCycleMaxValMinHV-HV.DutyCycleMinValMaxHV)/2))); // mid-range
    HV.MaxV                    = 485;  // 400                                          =  = ~200V

    HV.DutyCycleUserVal        = HV.DutyCycleMinValMaxHV;


    HV.DutyCycleUserVal        = HV.DutyCycleUserValDefault;

    MsTimer2::set(1, OneKhzIrup); // Timer2 is our perodic interrupt
    MsTimer2::start();

    HVADC.Init();

    // program i/o pins

    // High-Voltage
    pinMode(pinHVSRHIZ ,     OUTPUT);
    pinMode(pinHVSRLE ,      OUTPUT);
    digitalWrite(pinHVSRHIZ, LOW);
    digitalWrite(pinHVSRLE , LOW);

    // NVClk/Ram : Clock (bit-banged serial)
    pinMode(pinNVCLK_SCLK,   OUTPUT);
    pinMode(pinNVCLK_CE ,    OUTPUT);
    pinMode(pinNVCLK_IO ,    OUTPUT);

    // SPI (display high voltage shift registers)
    Spi.mode((0<<CPOL));


    // Buttons
    Buttons.pinBUT_1 = 14;
    Buttons.pinBUT_2 = 15;
    Buttons.pinBUT_3 = 16;
    Buttons.Init();
    Buttons.Flush(bflAll);

    Beep.Pin1 = pinBeep1; // Beeper
    Beep.Pin2 = pinBeep2;
    Beep.Val = Beep.Ctr = BEEP_DIV_DEFAULT;
    Beep.SyncWithSecondsEnabled = 1;
    pinMode(Beep.Pin1,    OUTPUT);
    pinMode(Beep.Pin2,    OUTPUT);


    Disp.Init(); // Display

    TransFxVal = TransFxCur = NVClk.Read(nvadTransFx); // Transition Effects
    DispCycIntvl = NVClk.Read(nvadDisplayCycle);

    ChimeIntvl = NVClk.Read(nvadChime);

    FxWipeStyleVal = NVClk.Read(nvadWipeStyle); 
    FxWipeStyleCur = FxWipeStyleVal;


    AlarmSnoozeVal = 5; // Mins
    AlarmSnoozeCtr = 0;
    AlarmHrs = NVClk.Read(nvadAlarmHrs);
    AlarmMins = NVClk.Read(nvadAlarmMins);


    UpdateMDY();

    // LED
    pinMode(pinLED     , OUTPUT);
    digitalWrite(pinLED, LOW);
    Led.Pin  = pinLED;
    Led.Mode = ledNorm;
    Led.BeepAlso = 1;
    Led.SetState(1);


    // Obtain DST Settings from NVmem
    DSTGetSettings();

}

//
// void StartAlarm(void)
//
//
//
void StartAlarm(void)
{
    SIO.TxConstSt(PSTR(":Alarm"));
    SIO.TxSt(EOL);
    Beep.SyncWithSecondsEnabled = 1;
    Beep.SetPattern("|||||| R");
}

  //
 // void ShowVersionOnDisplay(void);
//
//  Show software version and build number on display
//
void ShowVersionOnDisplay(void)
{
  char buf[16], i;
  strcpy(buf,VERSION);     // "1.0"
  strcat(buf," ");         // "1.0 "
  strcat(buf,BUILDNUMBER); // "1.0 2464"

  
 //      01234567
 for (i = 0;i < 16; i++) 
   buf[i] = buf[i] - 0x30; // ascii to binary

 
 // buf: 1.0 2464
 //      ^
 //      01234567
 Disp.Devs[6].NewVal  = buf[0];

 // buf: 1.0 2464
 //        ^
 //      01234567
 Disp.Devs[5].NewVal  = buf[2];


 // buf: 1.0 2464
 //          ^
 //      01234567
 Disp.Devs[3].NewVal = buf[4];
 Disp.Devs[2].NewVal = buf[5];
 Disp.Devs[1].NewVal = buf[6];
 Disp.Devs[0].NewVal = buf[7];
 Disp.BlankMask(0b00010000);


}

//
//   if (!(i)) { D("7");}

//
// PWM control from tty; Used to find optimum values
// for a given load / input voltage . Watch on scope
// and measure HV output w/voltmeter
//
#if 0

if (Serial.available())
{
    c = Serial.read();

    switch(c)
    {
    case 'd':
        HV.DutyCycleCurVal -= 0x1;
        break;

    case 'e':
        HV.DutyCycleCurVal += 0x1;
        break;

    case 'j':
        HV.Period--;
        break;

    case 'k':
        HV.Period++;
        break;

    default:
        break;

    }
    D("p%x d%x\r",HV.Period, HV.DutyCycleCurVal);
    Timer1.pwm(HV.Pin, HV.DutyCycleCurVal , HV.Period);
    // XXX
}

#endif

//    if (!(i)) { D("9"); i = 250;}
