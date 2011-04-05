
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
#include "eeprom.h"
#include "nixie_system.h"
#include "nvmem.h"
#include "hv.h"


#ifdef USING_ATMEGA_EEPROM 


  //
 // unsigned int CNVMem::ReadWord(unsigned char adr)
//
//    Read a 16 bit value from non-vol memory 
//
//
unsigned int CNVMem::ReadWord(unsigned char adr)
{
  return Read(adr) | Read(adr+1) << 8;
}

  //
 // unsigned char CNVMem::Read(unsigned char adr)
//
//   
//   Read a byte from non-vol memory
//
//
unsigned char CNVMem::Read(unsigned char adr)
{
  return EEPROM.read(adr);
}

  //
 // void CNVMem::WriteWord(unsigned char adr, unsigned int val)
//
//
//    Write a 16 bit value to non-vol memory
//
//    adr : address in non-vol memory
//    val : 16 bit value
//
//
void CNVMem::WriteWord(unsigned char adr, unsigned int val)
{
    Write(adr,val); Write(adr+1,val >> 8);
}

  //
 // void CNVMem::Write(unsigned char adr, unsigned char val)
//
//    Write a byte to non-vol memory
//
//   char adr
//   char val 
//
void CNVMem::Write(unsigned char adr, unsigned char val)
{
    EEPROM.write(adr,val);
}


#endif
