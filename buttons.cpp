
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

#include "buttons.h"
#include "led.h"
#include "beep.h"



#undef D
// for debugging:
#define D(...) // off
//#define D _D // on


//
// void CButtons::HundredHzChores(void)
//
//     Called 100x/sec to keep track of button state
//
void CButtons::HundredHzChores(void)
{

    // Keep track of how long a buttons been pressed with AnyPressedCtr
    // But when it reachas BUT_CTR_MAX we stop counting to prevent wrap
    if (BUT_ANY_PRESSED && AnyPressedCtr < BUT_CTR_MAX )
    {
        ++AnyPressedCtr;
        AnyReleasedCtr = 0;
    }


    // if they are holding a button down but we've been told
    // to ignore the next event, we do so.
    if (BUT_ANY_PRESSED && IgnoreUntilRelease)
    {
        D("Buttons.HundredHzChores(): IgnoreUntilRelease IgnoreNextEvent:%d", IgnoreNextEvent);
        IgnoreNextEvent = 1;
        return;
    }
    IgnoreUntilRelease = 0;

    // also keep track of how long a buttons been released
    if (BUT_NONE_PRESSED &&
            AnyReleasedCtr < BUT_CTR_MAX )
    {
        ++AnyReleasedCtr;
        AnyPressedCtr = 0;
    }

    // Any Pressed Long Time
    if (AnyPressedCtr == BUT_PRESSED_LONGTIME)
    {
        AnyPressedLongTimeEvent = 1;
        AnyEvent = 1;
    }

    // Any button event
    if (AnyPressedCtr == BUT_DEBOUNCE_PERIOD)
    {
        AnyPressedEvent = 1;
        AnyEvent = 1;
        if (KeyClickEnable)
        {
          Beep.SyncWithSecondsEnabled = 0;
          Beep.SetPattern("!");
        }
    }
    if (AnyReleasedCtr == BUT_DEBOUNCE_PERIOD)
    {
        AnyReleasedEvent = 1;
        AnyEvent = 1;
    }

    // Up button
    if (BUT_UP_PRESSED &&
            UpPressedCtr < BUT_CTR_MAX )
    {
        ++UpPressedCtr;

        UpReleasedCtr = 0;
    }
    if (!BUT_UP_PRESSED &&
            UpReleasedCtr < BUT_CTR_MAX )
    {
        ++UpReleasedCtr;
        UpPressedCtr = 0;
    }
    // Up button event
    if (UpPressedCtr == BUT_DEBOUNCE_PERIOD)
    {
        UpPressedEvent = 1;
        AnyEvent = 1;
    }
    if (UpReleasedCtr == BUT_DEBOUNCE_PERIOD)
    {
        UpReleasedEvent = 1;
        AnyEvent = 1;
    }

    // Down Button
    if (BUT_DWN_PRESSED &&
            DwnPressedCtr < BUT_CTR_MAX )
    {
        ++DwnPressedCtr;

        DwnReleasedCtr = 0;
    }
    if (!BUT_DWN_PRESSED &&
            DwnReleasedCtr < BUT_CTR_MAX )
    {
        ++DwnReleasedCtr;
        DwnPressedCtr = 0;
    }
    // Dwn button event
    if (DwnPressedCtr == BUT_DEBOUNCE_PERIOD)
    {
        DwnPressedEvent = 1;
        AnyEvent = 1;
    }
    if (DwnReleasedCtr == BUT_DEBOUNCE_PERIOD)
    {
        DwnReleasedEvent = 1;
        AnyEvent = 1;
    }

    // Select Button
    if (BUT_SEL_PRESSED &&
            SelPressedCtr < BUT_CTR_MAX )
    {
        ++SelPressedCtr;
        SelReleasedCtr = 0;
    }
    if (!BUT_SEL_PRESSED &&
            SelReleasedCtr < BUT_CTR_MAX )
    {
        ++SelReleasedCtr;
        SelPressedCtr = 0;
    }
    // Sel button event
    if (SelPressedCtr == BUT_DEBOUNCE_PERIOD)
    {
        D("SelPressedEvent");
        SelPressedEvent = 1;
        AnyEvent = 1;
    }
    if (SelReleasedCtr == BUT_DEBOUNCE_PERIOD)
    {
        D("SelReleasedEvent");
        SelReleasedEvent = 1;
        AnyEvent = 1;
    }

    // Sel Pressed Long Time
    if (SelPressedCtr == BUT_PRESSED_LONGTIME)
    {
        SelPressedLongTimeEvent = 1;
        AnyEvent = 1;
    }

    if (AnyEvent)
        TimeSinceLastCmdSecs = 0;


    if (AnyEvent & IgnoreNextEvent)
    {
        D("Buttons.HundredHzChores(): IgnoreNextEvent");
        Flush(bflEventsOnly);
        IgnoreNextEvent = 0;
    }

}

//
// void CButtons::Init(void)
//
//    Called once at start-up to initialize button hardware
//
void CButtons::Init(void)
{

    pinMode(pinBUT_1 ,       INPUT);
    digitalWrite(pinBUT_1,   HIGH);     // writing a 1 to an I/O set to input enables pull-up

    pinMode(pinBUT_2 ,       INPUT);
    digitalWrite(pinBUT_2,   HIGH);

    pinMode(pinBUT_3 ,       INPUT);
    digitalWrite(pinBUT_3,   HIGH);

    KeyClickEnable = 1;

}

//
// void CButtons::Flush(void)
//
//    Clears all events and resets counters
//
void CButtons::Flush(unsigned char how)
{


    if (how == bflAll)
    {
        D("Flush(bflAll)");
        //AnyPressedCtr =
        UpPressedCtr =
            DwnPressedCtr =
                SelPressedCtr = 0;

        AnyReleasedCtr =
            UpReleasedCtr =
                DwnReleasedCtr =
                    SelReleasedCtr =  255;
        IgnoreUntilRelease = 0;
        IgnoreNextEvent = 0;
    }

    AnyEvent =
        AnyReleasedEvent =
            AnyPressedEvent =
                AnyPressedLongTimeEvent =
                    UpPressedEvent =
                        DwnPressedEvent =
                            SelPressedEvent =
                                SelPressedLongTimeEvent =
                                    UpReleasedEvent =
                                        DwnReleasedEvent =
                                            IgnoreNextEvent =
                                                    SelReleasedEvent = 0;


}
boolean CButtons::SelPressed(void)
{
    return BUT_SEL_PRESSED ? 1 : 0;
}

boolean CButtons::UpPressed(void)
{
    return BUT_UP_PRESSED ? 1 : 0;
}

boolean CButtons::DwnPressed(void)
{
    return BUT_DWN_PRESSED ? 1 : 0;
}





  //
 // char CButtons::Eval(char arg)
//
// Used at power-up, this blocking function is entered if a button has been pressed
// on entry into the function. Until the SEL button is pressed, it increments a counter
// each time the UP button is pressed, and decrements it  each time the DOWN counter
// is pressed. The LED is blinked correspondingly.
//
// When SEL is finally pressed, the routine returns with the count
//
char CButtons::Eval(char arg)
{
    unsigned char repeatcount;

    char statecount = confNoChange; // should == 0

    if (BUT_NONE_PRESSED)
        return 0;

    ShowVersionOnDisplay();

    Led.Blink(statecount);

    while (BUT_ANY_PRESSED) delay(10); // wait for initial let-go

    //
    // things are simpler when we are powering up because we can just
    // hang out here until it the we are done.
    //
    //if (arg == beaPowerUp) 
    {
        // wait for them to lift up

        while(OrderPrevails)
        {
            while (BUT_ANY_PRESSED)
            {
                delay(100);
                D("statecount:%d",statecount);

                if (BUT_UP_PRESSED)
                {
                    if (statecount < CONF_MAX) statecount++;
                    Led.Blink(statecount);
                }
                else if (BUT_DWN_PRESSED)
                {
                    if (statecount > CONF_MIN) statecount--;
                    Led.Blink(statecount);
                }
                else if (BUT_SEL_PRESSED)
                {
                    return statecount;
                }

            }
        }

    }
}


