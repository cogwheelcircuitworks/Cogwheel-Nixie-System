
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

#ifndef DST_H
#define DST_H

/*

Daylight Savings Time 

 
favorite dst rules:
http://www.webexhibits.org/daylightsaving/g.html

South America
Brazil
"Legislation enacted in 2008 decrees that, henceforth, DST begins at midnight on the third Sunday of October, and reverts to Standard Time at midnight on the third Sunday of February. The exception is that, when Carnaval falls on the third Sunday of February, DST will be extended for one week."
 *
*/

struct dsttab
{
    unsigned char min;
    unsigned char max;
};



enum dstDayz { dstFirst, dstSecond, dstThird, dstFourth, dstLast, dstLast30, dstLast28 };

enum dstDowz { dstSunday = 1, dstMonday, dstTuesday, dstWednesday, dstThursday, dstFriday };

enum dstMos { dstJanuary = 1, dstFebruary, dstMarch, dstApril, dstMay,
              dstJune, dstJuly, dstAugust, dstSeptember, dstOctober, dstNovember, dstDecember
            };

#ifdef MAIN
struct dsttab DSTDayOfMonthRanges[] = 
{
  {1 , 7} , // first DOW range
  {8 , 14}, // second
  {15, 21}, // third
  {22, 28}, // fourth
  {25, 31}, // last in a 31 day month
  {24, 30}, // last in a 30
  {22, 28}  // last in a 28
};
#else
extern struct dsttab DSTDayOfMonthRanges[7];
#endif


class CDST
{
public:
    unsigned char StartDayOfWeekCount;   // 1st, 2nd, 3rd, 4th, etc.
    unsigned char StartDayOfWeek;        // 1=sun, 2=mon, etc..
    unsigned char StartMonth;            // Month

    unsigned char EndDayOfWeekCount;     // 1st, 2nd, 3rd, 4th, etc.
    unsigned char EndDayOfWeek;          // 1=sun, 2=mon, etc..
    unsigned char EndMonth;              // Month

    boolean StartNow(unsigned char curMonth, unsigned char curDayOfMonth, unsigned char curDayOfWeek);
    boolean EndNow(unsigned char curMonth  , unsigned char curDayOfMonth, unsigned char curDayOfWeek);
};


Extern CDST DST;




#endif
