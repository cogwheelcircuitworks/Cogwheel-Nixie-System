
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

#ifndef BEEP_H
#define BEEP_H


#define BEEP_INTER_PAT   15
#define BEEP_INTER_PAT_SHORT 5
#define BEEP_INTER_CHAR  45 
#define BEEP_ON_DASH     60
#define BEEP_ON_DOT      6 
#define BEEP_ON_FLICKER  1
#define BEEP_DIV_DEFAULT 1 // controls pitch

class CBeep
{
  public:

  unsigned int Pin1;
  unsigned int Pin2;
  unsigned char Ctr;
  unsigned char Val; 
  boolean Enable;
  void HundredHzChores(void);
  unsigned int StateCtr;
  unsigned int StateMaxVal;
  boolean StateDone;
  boolean State;

  void SetPattern(char *);
  char Pattern[16];
  char *patp;

  boolean DoInterPatPauseNext;
  boolean DoInterCharPauseNext;
  boolean DoRepeatNext;

  unsigned char InterPatPause;

  boolean SyncWithSecondsEnabled;
  boolean SyncWithSeconds;

  boolean PatStart;
  
  void On(unsigned int);
  void Off(unsigned int);

  boolean IsOn();
};

Extern CBeep Beep;
  
// morse code BEEP blinking support (for errors) 

#if 0

struct BEEPMorseCodes {
	char inchar;
	char ostr[5];
};

#ifdef MAIN
struct BEEPMorseCodes BEEPMorseCodes[] = {
	{'A',       ".-   "},
	{'B',       "-... "},
	{'C',       "-.-. "},
	{'D',       "-..  "},
	{'E',       ".    "},
	{'F',       "..-. "},
	{'G',       "--.  "},
	{'H',       ".... "},
	{'I',       "..   "},
	{'J',       ".--- "},
	{'K',       "-.-  "},
	{'L',       ".-.. "},
	{'M',       "--   "},
	{'N',       "-.   "},
	{'O',       "---  "},
	{'P',       ".--. "},
	{'Q',       "--.- "},
	{'R',       ".-.  "},
	{'S',       "...  "},
	{'T',       "-    "},
	{'U',       "..-  "},
	{'V',       "...- "},
	{'W',       ".--  "},
	{'X',       "-..- "},
	{'Y',       "-.-- "},
	{'Z',       "--.. "},
	{'1',       ".----"},
	{'2',       "..---"},
	{'3',       "...--"},
	{'4',       "....-"},
	{'5',       "....."},
	{'6',       "-...."},
	{'7',       "--..."},
	{'8',       "---.."},
	{'9',       "----."},
	{'0',       "-----"},
	{'\0',      "....."}
};

#else
Extern struct BEEPMorseCodes BEEPMorseCodes[37];
#endif


#endif

#endif 
