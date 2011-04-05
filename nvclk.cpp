
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

///
///
///  Driver for Dallas DS1302 Non-volatile clock
///
///  Interfaced via 3-wire bit-bang (programmed I/O)
///
///
///
#include "WProgram.h"
#include "nixie_system.h"
#include "Spi.h"
#include "display.h"
#include "nvclk.h"
#include "sio.h"
#include "hv.h"


#undef D
// uncomment one or the other:
#define D(...) // debugging off
//#define D _D // debugging on


static inline void delay(int x) { } // useful when looking on logic analyzer


//
// void NVCWrb(unsigned char a, unsigned char d)
//
//    Write byte d to location a in nvc memory
//
//
void CNVClk::Wrb(unsigned char a, unsigned char d)
{

    unsigned char nvclki;

    D("wrb");


    pinMode(pinNVCLK_IO  , OUTPUT);

    digitalWrite(pinNVCLK_CE,0);
    digitalWrite(pinNVCLK_SCLK,0);

    a &= 0b11111110; // lsb=0 means write

    // address
    digitalWrite(pinNVCLK_CE,1);

    for(nvclki=0; nvclki<8; nvclki++)
    {
        delay(5);
        digitalWrite(pinNVCLK_SCLK,0);
        delay(5);
        digitalWrite(pinNVCLK_IO,a & 0b00000001);
        a >>= 1;
        delay(5);
        digitalWrite(pinNVCLK_SCLK,1);
        delay(5);
    }

    delay(20);

    // data byte
    for(nvclki=0; nvclki<8; nvclki++)
    {


        digitalWrite(pinNVCLK_SCLK,0);
        delay(5);

        digitalWrite(pinNVCLK_IO,d&1);
        d >>= 1;
        delay(5);

        digitalWrite(pinNVCLK_SCLK,1);
        delay(5);

    }

    // eD of statement
    digitalWrite(pinNVCLK_SCLK,0);
    digitalWrite(pinNVCLK_CE,0);

    delay(20);


}

//
// NVCRdb()
//
//   Reads a byte from a NVC register
//
unsigned char CNVClk::Rdb(unsigned char a)
{

    unsigned char nvclki, nvclkd;
    D("rdb");



    digitalWrite(pinNVCLK_CE,0);
    digitalWrite(pinNVCLK_SCLK,0);

    // see dallas ds1302 data sheet

    a |= 0b00000001; // lsb asserted in addr byte means read..

    digitalWrite(pinNVCLK_SCLK,0);
    delay(1);
    digitalWrite(pinNVCLK_CE,1);

    // clock out address
    for(nvclki=0; nvclki<8; nvclki++)
    {
        delay(1);
        digitalWrite(pinNVCLK_SCLK,0);
        delay(1);
        digitalWrite(pinNVCLK_IO,a & 0b00000001);
        a >>= 1;
        delay(1);
        digitalWrite(pinNVCLK_SCLK,1);
        delay(2);
    }

    delay(5);

    // clock in the data
    pinMode(pinNVCLK_IO  , INPUT);

    nvclkd = 0;
    for(nvclki=0; nvclki<8; nvclki++)
    {
        digitalWrite(pinNVCLK_SCLK,1);
        delay(5);
        digitalWrite(pinNVCLK_SCLK,0);
        delay(5);
        nvclkd >>= 1;
        nvclkd |= ( digitalRead(pinNVCLK_IO) ? 0b10000000 : 0b00000000);
    }

    digitalWrite(pinNVCLK_CE,0);

    pinMode(pinNVCLK_IO  , OUTPUT);

    delay(10);


    return nvclkd;

}

//
// void NVCInit(void)
//
//    Initialize the NVC
//
void CNVClk::Init(void)
{
    D("init");
    // disable write-protect
    Wrb(NVC_ADR_CONTROL,0x00);
    // attach power to the NVC's cap or battery
    Wrb(NVC_ADR_CHARGER,0b10100000); // trickle charger off
    // restart count in case it wasn't running
    Wrb(NVC_ADR_SEC,(Rdb(NVC_ADR_SEC) & 0x7f));

    In12HourMode = (Rdb(NVC_ADR_HR) & NVC_12HR ? 1 : 0);
}


//
// CNVClk::Set12HourMode(boolean WantTwelveHourMode)
//
//       WantTwelveHourMode : 1 - switch to 12hr mode
//                          : 0 - switch to 24hr mode
//
void CNVClk::Set12HourMode(boolean WantTwelveHourMode)
{
    unsigned char  hrbyte;

    unsigned char hr;

    D("set12hmode");
    // switching from 24hr to 12hr mode
    if (WantTwelveHourMode)
    {
        Wrb(NVC_ADR_HR, (0b00010000 | 0b00000010) | NVC_12HR );

        hrbyte = Rdb(NVC_ADR_HR);
    }
    else
    {
        Wrb(NVC_ADR_HR, (0b00010000 | 0b00000010) );

        hrbyte = Rdb(NVC_ADR_HR);
    }

    Wrb(NVC_ADR_MIN,0);
    Wrb(NVC_ADR_SEC,0);

    In12HourMode = (Rdb(NVC_ADR_HR) & NVC_12HR ? 1 : 0);
}


//
// boolean CNVClk::IsIn12HourMode(void)
//
//    Returns : 0 - is in 24h mode
//              1 - is in 12h mode
//
boolean CNVClk::IsIn12HourMode(void)
{
    return(In12HourMode);
}

//
// void CNVClk::Sync(void)
//
//   Spin on the NVClk until the seconds digit changes
//   Guarantees we are as close in sync as possible w/NVClk.
//
void CNVClk::Sync(void)
{
#ifdef NVCLK_SYNC_TIGHT

    unsigned char osecs, nsecs, t;
    unsigned int timelimit = 0x7FFF;
#   define SYNC_DELAY_CONST 0x004;
    unsigned int delay = SYNC_DELAY_CONST;

    osecs = SecsCtr; 

    do
    {
        t = Rdb(NVC_ADR_SEC) ;
        nsecs = ((t >> 4)*10) + (t & 0x0f);

        if (!timelimit-- )
        {
            printf(":NVClk: hw err");
            break;
        }

        while(!delay--);

        delay = SYNC_DELAY_CONST;

    } while(osecs == nsecs);

    D("Sync %d %d",osecs,nsecs);
    D("CNVClk::Sync(): timelim: %d", timelimit);
    SecDivCtr = 1;
#endif

}

//
// void CNVClk::GetHMS(unsigned char t[] )
//
//  Gets HHMMSS from clock. Loads bcd digits
//  it into t[] TENTHS going into t[0] 
//  HRTENS going into t[6]
//
//
void CNVClk::GetHMS(unsigned char t[] )
{

    unsigned char mask, c;

    D("GetHMS");

    Sync();

    if (IsIn12HourMode())
        mask = 0b00011111;
    else
        mask = 0b00111111;

    c = Rdb(NVC_ADR_HR);
    NVC_HRTENS = (c & mask) >> 4;
    // XXX Attempt to overcome the 2->3 Bug
    if (NVC_HRTENS > 2) NVC_HRTENS = 2;
    NVC_HRONES = (c & 0x0f);

    c = Rdb(NVC_ADR_MIN);
    NVC_MINTENS = c >> 4;
    NVC_MINONES = c & 0x0f;

    c = Rdb(NVC_ADR_SEC);
    NVC_SECTENS = c >> 4;
    NVC_SECONES = c & 0x0f;

    if (c & 0b10000000) // if it's halted...
        Wrb(NVC_ADR_SEC, c & 0b01111111);  // ..restart it

    NVC_TENTHS = 0; // hardware doesn't maintain this resolution.

    In12HourMode = (Rdb(NVC_ADR_HR) & NVC_12HR ? 1 : 0);


}

//
// void NVCSetHMS(void) {
//
//   Set the Non-vol clock's time
//
//   Need to call Set12HourMode() and SetPM() first
//
void CNVClk::SetHMS(unsigned char t[])
{
    unsigned char pmbits = Rdb(NVC_ADR_HR) & (NVC_12HR | NVC_PM);

    D("SetHMS");

    Wrb(NVC_ADR_HR,( (NVC_HRTENS << 4) | NVC_HRONES ) | pmbits ) ;


    Wrb(NVC_ADR_MIN,(NVC_MINTENS << 4) | NVC_MINONES) ;
    Wrb(NVC_ADR_SEC, ( (NVC_SECTENS << 4) | NVC_SECONES & 0b01111111))   ;


}

//
// void CNVClk::SetH(unsigned char h)
//
//
//
//
//       unsigned char h
//
//    Returns :
//
void CNVClk::SetH(int h)
{
    unsigned char pmbits = Rdb(NVC_ADR_HR) & (NVC_12HR | NVC_PM);

    D("SetH");

    Wrb(NVC_ADR_HR , ((h/10)<<4) | ((h%10)&0x0F) | pmbits );

}

//
// void CNVClk::SetYMD(unsigned char t[])
//
//
//   Set Year, Month and Day
//
//       unsigned char t[] : format "yy/mm/dd"
//
void CNVClk::SetYMD(const char t[])
{
    D("SetYMD");
    // yyyy/mm/dd
    // 0123456789
    Wrb(NVC_ADR_YEAR , (t[2]-0x30)<<4 | (t[3]-0x30)&0x0F );
    Wrb(NVC_ADR_MONTH, (t[5]-0x30)<<4 | (t[6]-0x30)&0x0F );
    Wrb(NVC_ADR_DATE,  (t[8]-0x30)<<4 | (t[9]-0x30)&0x0F );

}

unsigned char CNVClk::GetYY(void)
{
    D("GetYY");
    return  ((Rdb(NVC_ADR_YEAR)>>4) * 10)  + (Rdb(NVC_ADR_YEAR)&0x0F);
}
unsigned char CNVClk::GetMM(void)
{
    D("GetMM");
    return  ((Rdb(NVC_ADR_MONTH)>>4) * 10)  + (Rdb(NVC_ADR_MONTH)&0x0F);

}
unsigned char CNVClk::GetDD(void)
{
    D("GetDD");
    return  ((Rdb(NVC_ADR_DATE)>>4) * 10)  + (Rdb(NVC_ADR_DATE)&0x0F);
}

unsigned char CNVClk::GetS(void)
{
    D("GetS");
    return ((Rdb(NVC_ADR_SEC)>>4) * 10)  + (Rdb(NVC_ADR_SEC)&0x0F);
}

unsigned char CNVClk::GetDOW(void)
{
    D("GetDOW");
    return  (Rdb(NVC_ADR_DAY));
}

void CNVClk::SetYY(unsigned char c)
{
    D("SetYY");
    Wrb(NVC_ADR_YEAR , ((c/10)<<4 | (c%10) & 0x0F));
}
void CNVClk::SetMM(unsigned char c)
{
    D("SetMM");
    Wrb(NVC_ADR_MONTH, ((c/10)<<4 | (c%10) & 0x0F));
}
void CNVClk::SetDD(unsigned char c)
{
    D("SetDD");
    Wrb(NVC_ADR_DATE,  ((c/10)<<4 | (c%10) & 0x0F));
}

void CNVClk::SetDOW(unsigned char c)
{
    D("SetDOW");
    Wrb(NVC_ADR_DAY, c );
}


//
// boolean CNVClk::IsPM(void)
//
//     If the NV Clock is in 12h mode, this will tell you inf
//     it is AM or PM
//
//  returns: 1 - PM, 0 - AM
//
//
boolean CNVClk::IsPM(void)
{

    D("SetIsPM");
    return(Rdb(NVC_ADR_HR) & NVC_PM ? 1 : 0);
}


//
// void CNVClk::SetPM(boolean setpm)
//
//    1 sets pm, 0 sets am
//
//    Clock is thrown into 12h mode as a side-effect
//
//
void CNVClk::SetPM(boolean setpm)
{
    D("SetPM");
    if (setpm)
    {
        Wrb(NVC_ADR_HR, Rdb(NVC_ADR_HR) | NVC_PM | NVC_12HR);
    }
    else
    {
        Wrb(NVC_ADR_HR,Rdb((NVC_ADR_HR) & ~NVC_PM) | NVC_12HR );
    }
}


#ifdef NVC_IMPLEMENT_SCRATCH_RAM_FUNCTIONS

//
// -------------------------- CNVClkMem ---------------------------------
//



//
// unsigned int CNVClk::ReadWord(unsigned char adr)
//
//    Read a 16 bit value from non-vol memory
//
//
unsigned int CNVClk::ReadWord(unsigned char adr)
{
    D("ReadWord");
    return Read(adr | NVC_RAM_OFFSET) | Read(adr+1 | NVC_RAM_OFFSET) << 8;
}

//
// unsigned char CNVClk::Read(unsigned char adr)
//
//
//   Read a byte from non-vol memory
//
//
unsigned char CNVClk::Read(unsigned char adr)
{
    D("Read(%d)",adr);
    unsigned char c = Rdb((adr << 1) | NVC_RAM_OFFSET);
    return c;
}

//
// void CNVClk::WriteWord(unsigned char adr, unsigned int val)
//
//
//    Write a 16 bit value to non-vol memory
//
//    adr : address in non-vol memory
//    val : 16 bit value
//
//
void CNVClk::WriteWord(unsigned char adr, unsigned int val)
{
    D("WriteWord");
    NeedsUpdating = 1;
    Write(adr | NVC_RAM_OFFSET,val);
    Write((adr+1) | NVC_RAM_OFFSET,val >> 8);
}

//
// void CNVClk::Write(unsigned char adr, unsigned char val)
//
//    Write a byte to non-vol memory
//
//   char adr
//   char val
//
void CNVClk::Write(unsigned char adr, unsigned char val)
{
    D("Write");
    NeedsUpdating = 1;
    Wrb(adr<<1 | NVC_RAM_OFFSET,val);
}



#endif // NVC_IMPLEMENT_SCRATCH_RAM_FUNCTIONS

