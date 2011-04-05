
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

#ifndef LED_H
#define LED_H

enum ledmodes {ledAMPM, ledNorm};

#define LED_INTER_PAT 15
#define LED_INTER_CHAR 100
#define LED_ON_DASH 60
#define LED_ON_DOT 12
#define LED_ON_FLICKER 1

class CLED
{
  public:

  void SetState(boolean On);
  boolean IsOn(void);
  void HundredHzChores(void);
  void On(unsigned int);
  void Off(unsigned int);
  void Blink(char);
  boolean StateDone;
  boolean State;
  unsigned char Mode;
  unsigned int Pin;
  unsigned int StateCtr;
  unsigned int StateMaxVal;
  void SetPattern(char *);
  char Pattern[16];
  char *patp;
  boolean DoInterPatPauseNext;
  boolean DoInterCharPauseNext;
  boolean DoRepeatNext;
  boolean BeepAlso;
};

Extern CLED Led;
 
#endif
