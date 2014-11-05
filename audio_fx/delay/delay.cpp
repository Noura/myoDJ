//
//  delay.cpp
//  BeatWave
//
//  Created by Noura Howell on 11/3/14.
//
//  copied / slightly adapted from http://www.dsprelated.com/showcode/58.php

#include "delay.h"
#include "math.h"

#define DELAY_MAX_BUF_SIZE 64000

/*****************************************************************************
 *       Fractional delay line implementation in C:
 *
 *                    ---------[d_mix]--------------------------
 *                    |                                         |
 *                    |                                         |
 *                    |x1                                       v
 *     xin ------>[+]----->[z^-M]--[interp.]----[d_fw]-------->[+]-----> yout
 *                 ^                         |
 *                 |                         |
 *                 |----------[d_fb]<--------|
 *******************************************************************************/

float d_buffer[DELAY_MAX_BUF_SIZE];

struct fract_delay {
    float d_mix; /* blend */
    short d_samples; /* delay duration in number of samples */
    float d_fb; /* feedback volume */ //d_fb and d_mix should probably sum to .5
    float d_fw; /* delay tap mix volume */
    float n_fract; /* fractional part of the delay */
    float *rdPtr; /* read pointer */
    float *wrtPtr; /* write pointer */
};

static struct fract_delay _delay;

void Delay_Init(float delay_samples, float dfb, float dfw, float dmix) {
    Delay_set_delay(delay_samples);
    Delay_set_fb(dfb);
    Delay_set_fw(dfw);
    Delay_set_mix(dmix);
    _delay.wrtPtr = &d_buffer[DELAY_MAX_BUF_SIZE - 1];
}

void Delay_set_fb(float val) {
    _delay.d_fb = val;
}

void Delay_set_fw(float val) {
    _delay.d_fw = val;
}

void Delay_set_mix(float val) {
    _delay.d_mix = val;
}

void Delay_set_delay(float n_delay) {
    _delay.d_samples = (short)floor(n_delay);
    _delay.n_fract = (n_delay - _delay.d_samples);
}

float Delay_get_fb() {
    return _delay.d_fb;
}

float Delay_get_fw() {
    return _delay.d_fw;
}

float Delay_get_mix() {
    return _delay.d_mix;
}

float Delay_task(float xin) {
    float yout;
    float *y0;
    float *y1;
    float x1;
    float x_est;
    
    // calculate read pointer position
    _delay.rdPtr = _delay.wrtPtr - (short)_delay.d_samples;
    if (_delay.rdPtr < d_buffer) {
        _delay.rdPtr += DELAY_MAX_BUF_SIZE-1;
    }
    
    // linear interpolation to estimate previous audo value at the fractional
    // delay point in time (example: a 2.6 sample delay should use a weighted avg
    // of 2 samples back and 3 samples back)
    y0 = _delay.rdPtr-1;
    y1 = _delay.rdPtr;
    if (y0 < d_buffer) {
        y0 += DELAY_MAX_BUF_SIZE-1;
    }
    x_est = (*(y0) - *(y1))*_delay.n_fract + *(y1);
    
    // next value to store in buffer
    x1 = xin + x_est * _delay.d_fb;
    // store value in buffer
    *(_delay.wrtPtr) = x1;
    
    // output value
    yout = x1*_delay.d_mix + x_est*_delay.d_fw;
    
    // increment write pointer
    _delay.wrtPtr++;
    if ((_delay.wrtPtr-&d_buffer[0]) > DELAY_MAX_BUF_SIZE-1) {
        _delay.wrtPtr = &d_buffer[0];
    }
    
    return yout;
}


