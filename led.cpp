
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
#include "led.h"
#include "beep.h"


boolean CLED::IsOn(void)
{
    return  State;
}

void CLED::SetState(boolean On)
{
  if (On)
  {
      State = 1;
      digitalWrite(Pin, LOW);
  }
  else
  {
      State = 0;
      digitalWrite(Pin, HIGH);
  }
}

void CLED::On(unsigned int period)
{
  StateMaxVal = period;
  StateCtr = 1;
  StateDone = 0;
  SetState(1);

  if (BeepAlso)
    Beep.Enable = 1;
}


void CLED::Off(unsigned int period)
{
  StateMaxVal = period;
  StateCtr = 1;
  StateDone = 0;
  SetState(0);

  if (BeepAlso)
    Beep.Enable = 0;
}

void CLED::HundredHzChores(void)
{
  // dont' mess with led state until current state countdown (StateCtr) hits max
  // we also don't bump StateCtr if it is zero. That means some other function
  // needs to start the chain reaction
  if (!StateDone && StateCtr && ++StateCtr >= StateMaxVal)
  {
    StateCtr = 0;
    if (IsOn())
      SetState(0);

    StateDone = 1;
  } 
  else
  if (StateDone)
  {
    if (DoInterCharPauseNext)
    {
      DoInterCharPauseNext = 0;
      Off(LED_INTER_CHAR);
      return;
    } 
    else
    if (DoInterPatPauseNext)
    {
      DoInterPatPauseNext = 0;
      Off(LED_INTER_PAT);
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
      if (*patp == '_') 
      {
        On(LED_ON_DASH);
        DoInterPatPauseNext = 1;
        patp++;
        return;
      }
      else
      if (*patp == '-') 
      {
        On(LED_ON_DASH);
        DoInterPatPauseNext = 1;
        patp++;
        return;
      }
      else
      if (*patp == '.')
      {
        On(LED_ON_DOT);
        DoInterPatPauseNext = 1;
        patp++;
        return;
      }
      else
      if (*patp == '!')
      {
        On(LED_ON_FLICKER);
        DoInterPatPauseNext = 1;
        patp++;
        return;
      } 
      else
      if (*patp == ' ')
      {
        DoInterCharPauseNext = 1;
        StateDone = 1;
        patp++;
        return;
      }
      else
      if (*patp == 'R') 
      {
        DoRepeatNext = 1;
        return;
      }
    }

#if 0
    if (*++patp)
      DoInterPatPauseNext = 1;
    else
      DoInterCharPauseNext = 1;
#endif

  }

}

void CLED::SetPattern(char *pat)
{
  strcpy(Pattern,pat);
  patp = pat;
  StateDone = 1;
}


void CLED::Blink(char count)
{
    static char pat[20], i;
    char actualcount;
    pat[0] = 0;

    if (count <= 0)
    {
      strcat(pat,"-");
      actualcount = abs(count);
    } 
    else
    {
      actualcount = count;
    }

    //D("actualcount:%d",actualcount);
    //D("");

    for(i = 0; i < actualcount; i++)
        strcat(pat,"!");

    strcat(pat,"  R");

    Led.SetPattern(pat);

}
