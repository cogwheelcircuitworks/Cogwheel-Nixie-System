
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


// astyle -A1 --indent=spaces=2

#include <WProgram.h>
#include "nixie_system.h"
#include "stdarg.h"
#include "sio.h"


#undef D

// uncomment one or the other:
#define D(...) // debugging off
//#define D _D // debugging on


void CSIO::TxSt(char *cp)
{
  Serial.write(cp);
}


void CSIO::TxConstSt(const char *cp)  
{

  unsigned char c;
  while((c = pgm_read_byte(cp)))
	{       

    if (c == '\n')
      Serial.write('\r');

		Serial.write(c);


    cp++;
	} 
} 


boolean CSIO::Incoming(void)
{
  return(Serial.available());
}

void CSIO::Init(void)
{
  RxBufI = 0;
  Serial.begin(57600);
}


void CSIO::Check(void)
{
  char c;

  if (Serial.available())
  {

    c = RxChBuf[RxBufI] = Serial.read();

    Serial.write(c);

    // first, take care of case where we've overruns...
    if (RxBufI == RXBUFSIZ)
    {
      RxOverruns++;
      Sendup = 1;
    }
    else
    {
      if (c == '\n' || c == '\r')
      {
        Serial.write('\n');
        Sendup = 1;
      }

      RxBufI++;

    }

  }
}

//
// void CSIO::RxSt()
//
//     Have a look at what the UART dragged in.
//     Serial Receive String
//
void CSIO::RxSt()
{
  char *cp;
  char *cparg;
  char *rxchbuf;
  char cmd[4];
  unsigned char i;
  unsigned char shift, loop;

  unsigned int hash;

  rxchbuf = RxChBuf;

  cp = rxchbuf;
  tolower(cp);
  cparg = SkipToNextArg(cp);

  i = 0;
  cmd[i] = cp[i++];
  cmd[i] = cp[i++];
  cmd[i] = cp[i++];
  cmd[i] = 0;

  for (i=0;i<cmdMAX;i++)
    if (!strcmp(cmd,CmdTab[i].name))
      break;

  if (i != cmdMAX) // if found assign it.
    i = CmdTab[i].num; 

  DoCmd(i,cparg); 

  Sendup = 0;
}

//
// SIOIWork()
//
//  If characters have been received, then process them
//
void CSIO::IWork(void)
{

  if (Sendup)
  {
    unsigned char i;
    RxSt();
    for (i = 0; i < RXBUFSIZ; i++) RxChBuf[i] = 0;
    RxBufI = 0;
  }
}

//
// char * CSIO::SkipToNextArg(const char *cp)
//
//   Scans *cp, past whitespace, returns pointer to
//   next non whitespace char
//
//
char * CSIO::SkipToNextArg(char *cp)
{
  if (*cp && *cp != ' ' && *cp != '\t')  \
    while(*cp && *cp != ' ' && *cp != '\t') cp++;
  \
  while( *cp == ' ' || *cp == '\t') cp++;

  return cp;
}


