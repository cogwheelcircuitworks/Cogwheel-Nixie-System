
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
#include "sio.h"

/*
 * Cut-and-pasted from www.arduino.cc playground section for determining heap and stack pointer locations.
 * http://www.arduino.cc/playground/Code/AvailableMemory
 *
 * Also taken from the Pololu thread from Paul at: http://forum.pololu.com/viewtopic.php?f=10&t=989&view=unread#p4218
 *
 * Reference figure of AVR memory areas .data, .bss, heap (all growing upwards), then stack growing downward:
 * http://www.nongnu.org/avr-libc/user-manual/malloc.html
 *
 */

extern unsigned int __data_start;
extern unsigned int __data_end;
extern unsigned int __bss_start;
extern unsigned int __bss_end;
extern unsigned int __heap_start;
//extern void *__malloc_heap_start; --> apparently already declared as char*
//extern void *__malloc_margin; --> apparently already declared as a size_t
extern void *__brkval;
// RAMEND and SP seem to be available without declaration here

int16_t ramSize=0;   // total amount of ram available for partitioning
int16_t dataSize=0;  // partition size for .data section
int16_t bssSize=0;   // partition size for .bss section
int16_t heapSize=0;  // partition size for current snapshot of the heap section
int16_t stackSize=0; // partition size for current snapshot of the stack section
int16_t freeMem1=0;  // available ram calculation #1
int16_t freeMem2=0;  // available ram calculation #2


/* This function places the current value of the heap and stack pointers in the
 * variables. You can call it from any place in your code and save the data for
 * outputting or displaying later. This allows you to check at different parts of
 * your program flow.
 * The stack pointer starts at the top of RAM and grows downwards. The heap pointer
 * starts just above the static variables etc. and grows upwards. SP should always
 * be larger than HP or you'll be in big trouble! The smaller the gap, the more
 * careful you need to be. Julian Gall 6-Feb-2009.
 */
uint8_t *heapptr, *stackptr;
uint16_t diff=0;
void check_mem() 
{
  stackptr = (uint8_t *)malloc(4);          // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);      // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);           // save value of stack pointer
}


/* Stack and heap memory collision detector from: http://forum.pololu.com/viewtopic.php?f=10&t=989&view=unread#p4218
 * (found this link and good discussion from: http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1213583720%3Bstart=all )
 * The idea is that you need to subtract your current stack pointer (conveniently given by the address of a local variable)
 * from a pointer to the top of the static variable memory (__bss_end). If malloc() is being used, the top of the heap
 * (__brkval) needs to be used instead. In a simple test, this function seemed to do the job, showing memory gradually
 * being used up until, with around 29 bytes free, the program started behaving erratically.
 */
//extern int __bss_end;
//extern void *__brkval;

int get_free_memory()
{
  int free_memory;
#if 0

  if((int)__brkval == 0)
     free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);

#endif
  return free_memory;
}



void report_free_mem(void)                     // run over and over again
{
#if 0
  Serial.print("\n\n--------------------------------------------");
  Serial.print("\n\nLOOP BEGIN: get_free_memory() reports [");
  Serial.print( get_free_memory() );
  Serial.print("] (bytes) which must be > 0 for no heap/stack collision");
  

  Serial.print("\n\nSP should always be larger than HP or you'll be in big trouble!");
#endif
  
  check_mem();

#if 0
  Serial.print("\nheapptr=[0x"); Serial.print( (int) heapptr, HEX); Serial.print("] (growing upward, "); Serial.print( (int) heapptr, DEC); Serial.print(" decimal)");
  
  Serial.print("\nstackptr=[0x"); Serial.print( (int) stackptr, HEX); Serial.print("] (growing downward, "); Serial.print( (int) stackptr, DEC); Serial.print(" decimal)");
  
  Serial.print("\ndifference should be positive: diff=stackptr-heapptr, diff=[0x");
  diff=stackptr-heapptr;
  Serial.print( (int) diff, HEX); Serial.print("] (which is ["); Serial.print( (int) diff, DEC); Serial.print("] (bytes decimal)");
  
  
  Serial.print("\n\nLOOP END: get_free_memory() reports [");
  Serial.print( get_free_memory() );
  Serial.print("] (bytes) which must be > 0 for no heap/stack collision");
  
  
  // ---------------- Print memory profile -----------------
  Serial.print("\n\n__data_start=[0x"); Serial.print( (int) &__data_start, HEX ); Serial.print("] which is ["); Serial.print( (int) &__data_start, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__data_end=[0x"); Serial.print((int) &__data_end, HEX ); Serial.print("] which is ["); Serial.print( (int) &__data_end, DEC); Serial.print("] bytes decimal");
  
  Serial.print("\n__bss_start=[0x"); Serial.print((int) & __bss_start, HEX ); Serial.print("] which is ["); Serial.print( (int) &__bss_start, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__bss_end=[0x"); Serial.print( (int) &__bss_end, HEX ); Serial.print("] which is ["); Serial.print( (int) &__bss_end, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__heap_start=[0x"); Serial.print( (int) &__heap_start, HEX ); Serial.print("] which is ["); Serial.print( (int) &__heap_start, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__malloc_heap_start=[0x"); Serial.print( (int) __malloc_heap_start, HEX ); Serial.print("] which is ["); Serial.print( (int) __malloc_heap_start, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__malloc_margin=[0x"); Serial.print( (int) &__malloc_margin, HEX ); Serial.print("] which is ["); Serial.print( (int) &__malloc_margin, DEC); Serial.print("] bytes decimal");

  Serial.print("\n__brkval=[0x"); Serial.print( (int) __brkval, HEX ); Serial.print("] which is ["); Serial.print( (int) __brkval, DEC); Serial.print("] bytes decimal");

  Serial.print("\nSP=[0x"); Serial.print( (int) SP, HEX ); Serial.print("] which is ["); Serial.print( (int) SP, DEC); Serial.print("] bytes decimal");

  Serial.print("\nRAMEND=[0x"); Serial.print( (int) RAMEND, HEX ); Serial.print("] which is ["); Serial.print( (int) RAMEND, DEC); Serial.print("] bytes decimal");

#endif
  // summaries:
  {
    char buf[80];
  ramSize   = (int) RAMEND       - (int) &__data_start;
  dataSize  = (int) &__data_end  - (int) &__data_start;
  bssSize   = (int) &__bss_end   - (int) &__bss_start;
  heapSize  = (int) __brkval     - (int) &__heap_start;
  stackSize = (int) RAMEND       - (int) SP;
  freeMem1  = (int) SP           - (int) __brkval;
  freeMem2  = ramSize - stackSize - heapSize - bssSize - dataSize;

  diff=stackptr-heapptr;

  sprintf(buf,": ramttl:%d, .data:%d, .bss:%d, free:%d", ramSize, dataSize, bssSize, freeMem1, diff);
  SIO.TxSt(buf);
  SIO.TxSt(EOL);
  }
 

}
