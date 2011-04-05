
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
 
#include "WProgram.h"

#include "nixie_system.h"
#include "adc.h"

Extern CADC HVADC;

  //
 // void ADCInit(void)
//
//     Called once at startup to initialize the hardware
//
//    
void CADC::Init(void)
{
  ADMUX =  1<<REFS0 |  // adc ref = vcc, mux channel 0

           0<<MUX3  |  // chan 6 pin18 on '328 TQFP package
           1<<MUX2  |
           1<<MUX1  |
           0<<MUX0  ;


  ADCSRA = 0;

  ADCSRA =  1<<ADEN  | // enable

            1<<ADPS2 | // clock = sysclk/128
            1<<ADPS1 |
            1<<ADPS0 |

            1<<ADIE ;  // interrupt enable


  ADCSRA |= 1<<ADSC;   // start first conversion


}
  //
 // inline void ADCRestart(void)
//
//    
//  Calling this starts the next conversion
//
//
void CADC::Restart(void)
{
  ADCSRA |= 1<<ADSC;           // restart conversion
}


  //
 // ISR(ADC_vect)          
//
//    ADC Interrupt Service Routine 
//
ISR(ADC_vect)          
{

  HVADC.Val =  ADCL | ADCH << 8; // get sample

  HVADC.DoneFlag = 1;

}
