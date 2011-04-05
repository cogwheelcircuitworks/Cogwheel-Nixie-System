
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

#ifndef nvclk_h
#define nvclk_h


// toggle for debugging:
#define ND(...)
//#define ND D

//
// Non-vol clock chip Dallas DS1302
//

# define NVC_ADR_SEC     0x80
# define NVC_ADR_MIN     0x82

# define NVC_ADR_HR      0x84
# define NVC_12HR        0b10000000
# define NVC_PM          0b00100000

# define NVC_ADR_DATE    0x86
# define NVC_ADR_MONTH   0x88
# define NVC_ADR_DAY     0x8a
# define NVC_ADR_YEAR    0x8c
# define NVC_ADR_CONTROL 0x8e
# define NVC_ADR_CHARGER 0x90

# define NVCLK_SYNC_TIGHT


#define NVC_RAM_OFFSET  0b11000000

#define NVC_TENTHS  t[0]
#define NVC_SECONES t[1]
#define NVC_SECTENS t[2]
#define NVC_MINONES t[3]
#define NVC_MINTENS t[4]
#define NVC_HRONES  t[5]
#define NVC_HRTENS  t[6]


class CNVClk : public CABSNVMem
{
public:
boolean IsIn12HourMode(void);
boolean In12HourMode;
boolean IsPM(void);

boolean NeedsUpdating;

void Init(void);

void          Wrb(unsigned char a, unsigned char d);
unsigned char Rdb(unsigned char         a);

void Set12HourMode(boolean );
void GetHMS(unsigned char t[] );
void SetHMS(unsigned char t[]);
void SetH(int );
unsigned char GetS(void);

unsigned char GetYY(void);
unsigned char GetMM(void);
unsigned char GetDD(void);
unsigned char GetDOW(void);

void SetYY(unsigned char);
void SetMM(unsigned char);
void SetDD(unsigned char);
void SetDOW(unsigned char);

void SetYMD(const char t[]);
void SetPM(boolean );
void Sync(void);

unsigned char Read(unsigned char);
unsigned int ReadWord(unsigned char);

void Write(unsigned char, unsigned char);
void WriteWord(unsigned char, unsigned int);


};





// turn this on if you are going to use the NVClocks's RAM
#define NVC_IMPLEMENT_SCRATCH_RAM_FUNCTIONS

Extern CNVClk NVClk;

#endif // nvclk_h
