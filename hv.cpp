
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

#include <WProgram.h>
#include "nixie_system.h"
#include "hv.h"
#include "adc.h"
#include "TimerOne.h"
#include "sio.h"

Extern CADC HVADC;


#undef D
// uncomment one or the other:
#define D(...) // debugging off
//#define D _D // debugging on

//
// HVpwmManage(void)
//
//     Called 100x/sec, monitors high voltage voltage
//
void CHV::HundredHzChores(void)
{

#if 0
    static unsigned char howofthen;
    if (!(++howofthen%100) )
    {
      D("HVpwmManage() HVADC.Val:%d HV.MaxV: %d State:%d", HVADC.Val,HV.MaxV, State);
      howofthen = 0;
    }
#endif

    // on startup: State is off
    // We switch to rampup and start increasing voltage (dutycycle)
    // we stop when we have reached

    if (HVADC.Val >= HV.MaxV)
    {
        if (State != hvStShutdown)
        {
            D("> MaxOpV\r");
            SIO.TxSt(":HV Safety Shutdown");
            SIO.TxSt(EOL);
        }
        State = hvStShutdown;
        Timer1.disablePwm(HVPin);
        digitalWrite(HVPin, HIGH);
    }

    switch (State)
    {
    case hvStOff:
        State = hvStRampUp;
        DutyCycleCurVal = DutyCycleMaxValMinHV;
        Timer1.pwm(HVPin, DutyCycleCurVal, Period);
        D("off\r");
        break;

    case hvStRampUp:
        D("Up Curv: %d, Userv: %d", DutyCycleCurVal, DutyCycleUserVal);
        // Ramp Up actually means decreasing the dutycycle register value
        if (DutyCycleCurVal > DutyCycleUserVal)
        {
            DutyCycleCurVal -= DutyCycleInc;
        }
        else
        {
            D("DutyCycleMinValMaxHV Reached\r");
            State = hvStAdjusted;
        }

        Timer1.pwm(HVPin, DutyCycleCurVal, Period);
        D("up Curval: %d ADCVal: %d\r",DutyCycleCurVal, HVADC.Val);
        break;

    case hvStRampDown:
        D("Down Curv: %d, Userv: %d", DutyCycleCurVal, DutyCycleUserVal);
        if (DutyCycleCurVal < DutyCycleMaxValMinHV)
        {
            DutyCycleCurVal += DutyCycleInc;
        }
        else
        {
            D("DutyCycleMaxValMinHV Reached\r");
            State = hvStOff;
        }
        Timer1.pwm(HVPin, DutyCycleCurVal, Period);
        break;

    case hvStShutdown:
        break;

    case hvStAdjusted:
        break;

    default:
        break;
    }


}

void CHV::Shutdown(void)
{
  Timer1.disablePwm(HVPin);
  digitalWrite(HVPin, HIGH);
}

void CHV::SetPWM(unsigned int Val)
{
  D("SetPWM(%d)",Val);
  if (Val >= DutyCycleMaxValMinHV)
    Val = DutyCycleMaxValMinHV;
  else 
  if (Val <= DutyCycleMinValMaxHV)
    Val = DutyCycleMinValMaxHV;

  DutyCycleCurVal = Val;
  Timer1.pwm(HV.HVPin, DutyCycleCurVal, HV.Period);
 
}
