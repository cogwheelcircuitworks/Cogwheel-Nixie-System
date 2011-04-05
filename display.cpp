
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

#include "WProgram.h"
#include "nixie_system.h"
#include "nvclk.h"
#include "Spi.h"
#include "display.h"
#include "hv.h"
#include "sio.h"


#undef D
// uncomment one or the other:
#define D(...) // debugging off
//#define D _D // debugging on


void CDisplay::CrossFadeClear(void)
{
  unsigned char devno;
  CrossFadeEnabled = 0;

  D("CrossFadeClear()");

  for(devno = 0; devno < NumDevs; devno++)
  {
    Devs[devno].CrossFadeCtr = 0;
    Devs[devno].PreVal = Devs[devno].CurVal; //
  }
  CrossFadeEnabled = 0;


}

void CDisplay::CrossFadeRestore(void)
{
   D("CrossFadeRestore()");
   CrossFadeClear();
   CrossFadeEnabled = CrossFadeVal;
}

//
// void CDisplay::Init(void)
//
//    Called to reset Display
//
void CDisplay::Init(void)
{
  unsigned char devno;

  NumElems = 10; // TODO: These should be config options
  NumDevs  = 7;

  WipeRateVal = 4; // norm: 2

  BlankedPrevState = Blanked = 0;

  CrossFadeVal = 0;
  if (NVClk.Read(nvadCrossFade) == 1)
    CrossFadeVal = 1;

  CrossFadeEnabled = CrossFadeVal;
  Blanked = 0;

  for(devno = 0; devno < NumDevs; devno++)
  {
    Devs[devno].CrossFadeEnabled = 1;
    Devs[devno].Blanked = 0;
    Devs[devno].CrossFadeVal = 12;  // x100 msec
    Devs[devno].CurVal = Devs[devno].PreVal = 0;
    Devs[devno].UseAltVal = 0;
  }

  Devs[0].CrossFadeVal = 5;  // TENTH Sec gets shorter x100 msec

  StateLocked = 0;


}


//
// void CDisplay::HundredHzChores(void)
//
//
//  Called 100x/second
//
void CDisplay::HundredHzChores(void)
{
  unsigned char count;

  if (WipeRunning) WipeChore();

  for(count = 0; count < Disp.NumDevs; count++)
    if (++Disp.Devs[count].AltVal >= Disp.NumElems)
          Disp.Devs[count].AltVal = 0;

}

//
// void CDisplay::Mux(void)
//
//    Typically called from FiveHundredHzIrup(), this is where each device get's lit
//    in succession.
//
//    It can also be called by from the testing routines to light one at a time.
//
void CDisplay::Mux(void)
{
  unsigned char CathHi, CathLo;
  unsigned char Val;
  DevStruct *dp;

  //
  // Writing to the HV Shift Registers (HVSR) interfaced on SPI..
  // There are 3 8 bit muxes so we clock 24 bits to them at a time.
  //
  // First Byte: Anode Drivers, one tube each Bit 7 MS Tube, Bit 0 LS Tube.
  //
  // Second Byte: Cathode Drivers 15..8
  //
  // Third Byte: Cathode Drivers 7..0

  digitalWrite(pinHVSRHIZ, LOW); // first things first; turn off all outputs

  if (++ScanCtr >= NumDevs)
    ScanCtr = 0; // determines which tube we are lighting

  dp = &Devs[ScanCtr];

  if (CrossFadeEnabled && dp->CrossFadeEnabled)
  {
    // display element changed; need to cross-fade
    if (dp->CurVal != dp->NewVal)
    {
      dp->PreVal = dp->CurVal;
      dp->CrossFadeCtr = dp->CrossFadeVal;
    } 

  }

  if (dp->UseAltVal)
  {
    Val = dp->CurVal = dp->AltVal;
  }
  else
  {
    Val = dp->CurVal = dp->NewVal;
  }


  // chunk out the 8 anodes
  if (!Blanked && ((!dp->Blanked  && !dp->FxBlanked) || dp->UseAltVal) )
      ScanPos = 1<<ScanCtr; // Anode, one on at a time.
  else
    ScanPos = 0;

  Spi.transfer(ScanPos);


  CathHi = CathLo = 0b11111111;
  // chunk out the 16 cathodes
  if (Val >= 8)
    CathLo &= ~(1<<Val-8);
  else
    CathHi &= ~(1<<Val);




  // if we are actively cross fading, then we light up a second set of elements
  // Current limitation: Only one element can be lit a time.
  // TODO: Fix this to support segmented tubes.
  if (CrossFadeEnabled && dp->CrossFadeCtr)
  {
    Val = dp->PreVal;
    if (Val >= 8)
      CathLo &= ~(1<<Val-8);
    else
      CathHi &= ~(1<<Val);

    dp->CrossFadeCtr--;
  }

  Spi.transfer(CathLo);
  Spi.transfer(CathHi);

  // transfer the shift register to the output latch
  digitalWrite(pinHVSRLE, HIGH);
  digitalWrite(pinHVSRLE, LOW);

  digitalWrite(pinHVSRHIZ, HIGH); // turn outputs back on

}




//
// void DisplayBlink(void)
//
//    Schedules the display to be turned rapidly off, then on
//    Designating a change in mode, or some such
//
void CDisplay::Blink(void)
{
  //D("Blink");
  if (Disp.BlankedPrevState) return;
  digitalWrite(pinHVSRHIZ, LOW);  // float HV outputs
  ScanEnabled = 0;
  ScanOffTimer = 1;
}


void CDisplay::ScanDisable(void)
{
  //D("ScanDisable %d",ScanOffTimer);
  if (ScanOffTimer) return;
  digitalWrite(pinHVSRHIZ, LOW);
  ScanEnabled = 0;
}

void CDisplay::ScanEnable(void)
{
  //D("ScanDisable %d",ScanOffTimer);
  if (ScanOffTimer) return;
  digitalWrite(pinHVSRHIZ, HIGH);
  ScanEnabled = 1;

  Blanked = BlankedPrevState;
}

void CDisplay::FadeDown(void)
{
  if (Disp.Blanked == 1) return;
  HV.State = hvStRampDown;
  while(HV.State != hvStOff)
      if (SIO.Incoming())
        break;
}


void CDisplay::FadeUp(void)
{
    if (Disp.Blanked == 1) return;
    HV.State = hvStOff; 
    HV.HundredHzChores(); // call once to make it happen

    HV.State = hvStRampUp;
    while(HV.State != hvStAdjusted)
      if (SIO.Incoming())
        break;

}



  //
 // void CDisplay::BlankMask(unsigned char bval)
//
//    Selectively disable displays based on bval
//
void CDisplay::BlankMask(unsigned char bval)
{
    unsigned char i;
    if (Disp.Blanked == 1) return;
    for (i = 0; i < 8; i++)
    {
        if (bval & 1<<i)
            Disp.Devs[i].Blanked = 1;
        else
            Disp.Devs[i].Blanked = 0;
    }
}

  //
 // void CDisplay::FxAltMask(unsigned char mask);
//
//   Sets any device N indicated by bit N in mask
//   to use Alternate Value. Used for transition effects
//
void CDisplay::FxAltMask(unsigned char mask)
{
    unsigned char i;
    if (Disp.Blanked == 1) return;
    for (i = 0; i < 8; i++)
    {
        if (mask & 1<<i)
        {
            Disp.Devs[i].UseAltVal = 1;

        }
        else
        {
            Disp.Devs[i].UseAltVal = 0;

            //Disp.Devs[i].FxBlanked = 0; 
            //Disp.Devs[i].Blanked = 0; 
        }
    }
}

  //
 // void CDisplay::FxBlankMask(unsigned char mask)
//
//    There is a second blanking mask for special
//    effects (wipe)
//
void CDisplay::FxBlankMask(unsigned char mask)
{
    unsigned char i;
    if (Disp.Blanked == 1) return;
    for (i = 0; i < 8; i++)
    {
        if (mask & 1<<i)
            Disp.Devs[i].FxBlanked = 1;
        else
            Disp.Devs[i].FxBlanked = 0;
    }
}

  //
 // void CDisplay::WipeToOn(void)
//
//   Sequentially turn on displays for pleasing
//   transition effect
//
//
void CDisplay::WipeToOn(void)
{
  if (Disp.Blanked == 1) return;
  WipeToCommunalActions();
  WipingFromWhat = WipingToWhat;
  WipingToWhat = wipeOn;
  while(WipeRunning)
    if (SIO.Incoming())
      break;

  WipeStyleChore(); // keeps it alternating
}

  //
 // void CDisplay::WipeToWhat(what)
//
//    Sequentially turn off displays for pleasing
//    transition effect.
//
void CDisplay::WipeToWhat(unsigned char what)
{
  if (Disp.Blanked == 1) 
    return;

  WipeToCommunalActions();
  WipingToWhat = what;
  while(WipeRunning)
    if (SIO.Incoming())
      break;

}

  //
 // void CDisplay::WipeStyleChore(void)
//
//    If FxWipeStyleVal == fxsWipeTog, we flip the direction here
//
void CDisplay::WipeStyleChore(void)
{
  unsigned char speed;

  // choose direction, left->right, right->left, or toggle
  if ((FxWipeStyleVal) == fxsWipeTog)
  {
    FxWipeStyleCur = ((FxWipeStyleCur == fxsWipeRL) ? fxsWipeLR : fxsWipeRL);
  }
  else 
  {
    FxWipeStyleCur = FxWipeStyleVal;
  }

  if (FxWipeStyleCur == fxsWipeRandom)
    WipeIndexMaxVal = WIPESTEPS-1;
  else
    WipeIndexMaxVal = 7;

  
}

  //
 // void CDisplay::WipeToCommunalActions(void)
//
//   
//   Code common to multiple functions
//
void CDisplay::WipeToCommunalActions(void)
{
  WipeRateCount = WipeRateVal;
  WipeIndex = 1;
  WipeRunning = 1;

  WipeStyleChore();

}

  //
 // void CDisplay::WipeChore(void)
//
//   
//   WipeChore() is called from the 100x/sec
//   interrupt to implement the wipe effect
//
//
void CDisplay::WipeChore(void)
{
  unsigned char v,mask;

  if (Disp.Blanked == 1) return;
  //
  // controls rate of wipe
  if (--WipeRateCount)
    return;

  // form mask
  for (v = 0, mask = 0; v < WipeIndex; v++)
  {
    if (FxWipeStyleCur == fxsWipeRL)
      mask |= (0b10000000 >> v);
    else
    if (FxWipeStyleCur == fxsWipeLR)
      mask |= (0b00000001 << v);
    else
    if (FxWipeStyleCur == fxsWipeRandom)
      mask = pgm_read_byte(&(WipeTab[WipeIndex-1])); // really doesn't need to be inside this for loop
  }

  D("%d %x",WipeIndex,mask);

  switch(WipingToWhat)
  {
    case wipeOn:
      mask = ~mask;
      if (WipingFromWhat == wipeOff)
        FxBlankMask(mask);
      else
        FxAltMask(mask);
      break;

    case wipeOff:
      FxBlankMask(mask);
      break;

    case wipeAltVal:
      FxAltMask(mask);
      break;
  }

  WipeRateCount = WipeRateVal;

  if (WipeIndex++ > WipeIndexMaxVal)
    WipeRunning = 0;



}
