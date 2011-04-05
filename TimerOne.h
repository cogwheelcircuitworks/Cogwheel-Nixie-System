/*
 *  Interrupt and PWM utilities for 16 bit Timer1 on ATmega168 by Jesse Tane
 *  Copyright (c) 2008 labs.ideo.com
 *
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the Creative Commons Attribution (by) License as published
 *  by Creative Commons (http://creativecommons.org); either version 3.0 of
 *  the license, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful, but
 *  without any warranty; without even the implied warranty of merchantability
 *  or fitness for a particular purpose. See the specified license for more detail.
 */
 
#define RESOLUTION 65536    // Timer1 is 16 bit

class TimerOne
{
  public:
  
    // properties
    unsigned int pwmPeriod;
    unsigned char clockSelectBits;

    // methods
    void initialize(long microseconds=1000000);
    void start();
    void stop();
    void pwm(char pin, int duty, long microseconds=-1);
    void disablePwm(char pin);
    void attachInterrupt(void (*isr)(), long microseconds=-1);
    void detachInterrupt();
    void setPeriod(long microseconds);
    void setPwmDuty(char pin, int duty);
    void (*isrCallback)();
};

extern TimerOne Timer1;
