
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

#ifndef display_h
#define display_h





typedef struct DevStruct
{

  unsigned char NewVal;
  unsigned char CurVal;
  unsigned char PreVal; // for fading
  unsigned char AltVal; // for effects
  unsigned char CrossFadeCtr;
  unsigned char CrossFadeVal;
  boolean UseAltVal; // for effects
  boolean CrossFadeEnabled;
  boolean Blanked;
  boolean FxBlanked;
};

#define HW_NUMCATHODES 8

class CDisplay
{
  public:

  void Init();
  void Blink(void);
  void Mux(void);
  void ScanEnable(void);
  void ScanDisable(void);
  void HundredHzChores(void);

  void CrossFadeClear(void);
  void CrossFadeRestore(void);

  void FadeUp(void);
  void FadeDown(void);

  void BlankMask(unsigned char);
  void FxBlankMask(unsigned char);
  void FxAltMask(unsigned char);

  void BlankEnable(void);
  void BlankDisable(void);
  void BlankInhibit(void);
  void BlankRestore(void);

  DevStruct Devs[HW_NUMCATHODES];

  unsigned int ScanPos; 
  unsigned char ScanCtr;
  unsigned char NumDevs;
  unsigned char NumElems;
  volatile boolean ScanEnabled;
  boolean CrossFadeEnabled;
  boolean CrossFadeVal; 
  boolean StateLocked;
  boolean Blanked;
  boolean BlankedPrevState;
  unsigned char ScanOffTimer;

  unsigned char WipeIndex, WipeIndexMaxVal;

  unsigned char WipeRateVal, WipeRateCount;

#define WIPE_RATE_MIN 1
#define WIPE_RATE_MEDIUM 4
#define WIPE_RATE_MAX 8

  volatile boolean WipeRunning;
  unsigned char WipingToWhat;
  unsigned char WipingFromWhat;

  void WipeToWhat(unsigned char);
  void WipeToOn(void);
  void WipeChore(void);
  void WipeToCommunalActions(void);
  void WipeStyleChore(void);


 boolean Modified; 

 unsigned char IsPM;  // used only while setting, otherwise follow NVClk

 unsigned char CurTest;  
 int CurTestVal;  
 
 unsigned char CurOpt;
 int CurOptVal; 
};

enum WipingToWhat { wipeOff, wipeOn, wipeAltVal};

Extern CDisplay Disp;



#endif

//
//
