
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
#include "dst.h"

#define LASTDOW 5

#undef D
// uncomment one or the other:
#define D(...) // debugging off
//#define D _D // debugging on


//
// boolean
// CDST::StartNow(unsigned char curMonth, unsigned char curDayOfMonth, unsigned char curDayOfWeek)
//
// When handed curMonth, curDayOfMonth, and curDayOfWeek, and given that you have preset
// StartDayOfWeekCount, StartDayOfWeek, and StartMonth, will tell you if you should
// start DST.
//
// Note that it is assumed curMonth, curDayOfMonth, and curDayOfWeek start from 1
//
//
boolean CDST::StartNow(unsigned char curMonth, unsigned char curDayOfMonth, unsigned char curDayOfWeek)
{

    if (StartDayOfWeekCount == dstLast)
    {
        if (curMonth == dstSeptember ||
                curMonth == dstApril ||
                curMonth == dstJune ||
                curMonth == dstNovember)
            StartDayOfWeekCount = dstLast30;
        else if ( curMonth == dstFebruary )
            StartDayOfWeekCount = dstLast28;
    }

    D("curmo:%d strtmo%d",curMonth, StartMonth);
    D("cdow:%d,dow:%d",curDayOfWeek, StartDayOfWeek);
    D("cdom:%d min:%d max:%d", curDayOfMonth, DSTDayOfMonthRanges[StartDayOfWeekCount-1].min,DSTDayOfMonthRanges[StartDayOfWeekCount-1].max);
    // on to business...
    if (curMonth == StartMonth &&
            curDayOfWeek == StartDayOfWeek &&
            curDayOfMonth >= DSTDayOfMonthRanges[StartDayOfWeekCount-1].min &&
            curDayOfMonth <= DSTDayOfMonthRanges[StartDayOfWeekCount-1].max
       )
    {
        D("yes");
        return 1;
    }
    else
    {
        D("no");
        return 0;
    }
}

//
// boolean CDST::EndNow(unsigned char curMonth, unsigned char curDayOfMonth, unsigned char curDayOfWeek)
//
//
//   See definition for StartNow()
//
boolean CDST::EndNow(unsigned char curMonth, unsigned char curDayOfMonth, unsigned char curDayOfWeek)
{
    if(EndDayOfWeekCount == dstLast)
    {
        if (curMonth == dstSeptember ||
                curMonth == dstApril ||
                curMonth == dstJune ||
                curMonth == dstNovember)
            EndDayOfWeekCount = dstLast30;
        else if ( curMonth == dstFebruary )
            EndDayOfWeekCount = dstLast28;
    }


    D("sm%d cm%d",curMonth, EndMonth);
    D("cdow%d,dow%d",curDayOfWeek, EndDayOfWeek);
    D("cdom:%d min%d max%d", curDayOfMonth, DSTDayOfMonthRanges[EndDayOfWeekCount].min,DSTDayOfMonthRanges[EndDayOfWeekCount].max);
    if (curMonth == EndMonth &&
            curDayOfWeek == StartDayOfWeek &&
            curDayOfMonth >= DSTDayOfMonthRanges[EndDayOfWeekCount-1].min &&
            curDayOfMonth <= DSTDayOfMonthRanges[EndDayOfWeekCount-1].max
       )
        return 1;
    else
        return 0;

}
