
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
#include "beep.h"
#include "led.h"


#undef D
//#define D(...) // debugging off
#define D _D // debugging on 

  //
 // void CBeep::HundredHzChores(void)
//
//   
//   Called (as you might imagine) 100x/second, we watch how beeping
//   is progressing; We are timing beep durations or pauses betw
//   beeps. Then we look at *patp on what to do next and schedule that
//
void CBeep::HundredHzChores(void)
{

  if (Led.BeepAlso)
    return;

  if (!StateDone && StateCtr && ++StateCtr >= StateMaxVal)
  {
    StateCtr = 0;
    if (IsOn())
      State = 0;

    StateDone = 1;
  } 
  else
  if (StateDone)
  {
    if (DoInterCharPauseNext)
    {
      DoInterCharPauseNext = 0;
      if (SyncWithSecondsEnabled)
        Off(BEEP_INTER_CHAR);
      else
        Off(BEEP_INTER_PAT);
      return;
    } 
    else
    if (DoInterPatPauseNext)
    {
      DoInterPatPauseNext = 0;
      Off(InterPatPause);
      return;
    } 
    else
    if (DoRepeatNext)
    {
      DoRepeatNext = 0;
      patp = Pattern;
      return;
    }
    else
    if (*patp)
    {
      if (SyncWithSecondsEnabled && !SyncWithSeconds) 
        return;

      switch (*patp)
      {

        case '_':
        case '-': // longish beep
        On(BEEP_ON_DASH);
        DoInterPatPauseNext = 1;
        InterPatPause = BEEP_INTER_PAT;
        break;

        case '.': // short beep
          On(BEEP_ON_DOT);
          DoInterPatPauseNext = 1;
          InterPatPause = BEEP_INTER_PAT;
          break;

        case '!':
          On(BEEP_ON_FLICKER); // even shorter beep
          DoInterPatPauseNext = 1;
          InterPatPause = BEEP_INTER_PAT;
          break;

        case '|': // shorter beep with shorter pause
          On(BEEP_ON_FLICKER);
          DoInterPatPauseNext = 1;
          InterPatPause = BEEP_INTER_PAT_SHORT;
          break;

        case ' ': // a pause
          InterPatPause = BEEP_INTER_PAT;
          DoInterCharPauseNext = 1;
          StateDone = 1;
          SyncWithSeconds = 0; 
          Val = 0;
          break;

        case 'R':  // valid only at end of string. Causes continuous repeat of pattern
          DoRepeatNext = 1;
          break;

      }

      if (*patp != ' ' && *patp != 'R')
        Val = 1;

      if (*patp != 'R')
        patp++;

    }
  }
}


  //
 // boolean CBeep::IsOn(void)
//
//   
//
boolean CBeep::IsOn(void)
{
    return  State;
}


  //
 // void CBeep::On(unsigned int period)
//
//   Starts active beeping
//
void CBeep::On(unsigned int period)
{
  StateMaxVal = period;
  StateCtr = 1;
  StateDone = 0;
  State = 1;
  Enable = 1;
}


  //
 // void CBeep::Off(unsigned int period)
//
//    Turns off beeping
//
//
void CBeep::Off(unsigned int period)
{
  StateMaxVal = period;

  StateCtr = 1;
  StateDone = 0;
  State = 0;
  Enable = 0;

}


  //
 // void CBeep::SetPattern(char *pat)
//
//     Beeping is controlled by strings. See
//     HundredHzChores() for pattern implementation
//
void CBeep::SetPattern(char *pat)
{
  //D("pat:%s",pat);

  if (pat && *pat && strlen(pat) < 15)
  {
    strcpy(Pattern,pat); 
  } else
  {
    *Pattern = 0;
  }
  patp = Pattern;

  Enable = 0;
   

  StateDone = 1;
  DoRepeatNext = 
  DoInterCharPauseNext = 
  DoInterPatPauseNext = 0;
  SyncWithSeconds = 0;
}

