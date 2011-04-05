
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


//
// Buttons
//
#ifndef buttons_h
#define buttons_h

#include "WProgram.h"
#include "nixie_system.h"


#define BUT_UP (digitalRead(pinBUT_1))
#define BUT_UP_PRESSED (BUT_UP == 0)

#define BUT_DWN (digitalRead(pinBUT_2))
#define BUT_DWN_PRESSED (BUT_DWN == 0)

#define BUT_SEL (digitalRead(pinBUT_3))
#define BUT_SEL_PRESSED (BUT_SEL == 0)

#define BUT_ANY_PRESSED  ( !BUT_UP |  !BUT_DWN |  !BUT_SEL)
#define BUT_NONE_PRESSED ( !BUT_SEL_PRESSED && !BUT_DWN_PRESSED && !BUT_UP_PRESSED)

#define BUT_DEBOUNCE_PERIOD 3 // x .01 sec
#define BUT_CTR_MAX 1000 


#define BUT_PRESSED_LONGTIME_LO 115 // x .01 sec (HI is in case we miss)
#define BUT_PRESSED_LONGTIME 125 // x .01 sec

#define BUT_PRESSED_REALLY_LONGTIME 300 // x .01 sec


// Button global
Extern char ButtonPowerUpState;

enum ButtonEvalArg { beaPowerUp, beaNormalOps};
enum ButtonFlushMode { bflAll, bflEventsOnly };

class CButtons
{
  public:
    void Init(void);
    char Eval(char arg);
    void HundredHzChores(void);
    void Flush(unsigned char);
    boolean SelPressed(void);
    boolean UpPressed(void);
    boolean DwnPressed(void);
    

    unsigned int pinBUT_1;
    unsigned int pinBUT_2;
    unsigned int pinBUT_3;

    unsigned int  AnyPressedCtr;
    unsigned int  UpPressedCtr;
    unsigned int  DwnPressedCtr;
    unsigned int  SelPressedCtr;


    unsigned int AnyReleasedCtr;
    unsigned int UpReleasedCtr;
    unsigned int DwnReleasedCtr;
    unsigned int SelReleasedCtr;


    boolean AnyEvent;

    boolean AnyPressedEvent;
    boolean AnyReleasedEvent;
    boolean AnyPressedLongTimeEvent;
    boolean SelPressedLongTimeEvent;

    boolean UpPressedEvent;
    boolean DwnPressedEvent;
    boolean SelPressedEvent;

    boolean UpReleasedEvent;
    boolean DwnReleasedEvent;
    boolean SelReleasedEvent;

    boolean IgnoreUntilRelease;

    boolean IgnoreNextEvent;

    boolean KeyClickEnable;


};


Extern CButtons Buttons;

#endif
