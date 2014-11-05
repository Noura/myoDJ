//
//  delay.h
//  BeatWave
//
//  Created by Noura Howell on 11/3/14.
//
//

#ifndef __BeatWave__delay__
#define __BeatWave__delay__

#include <stdio.h>

// initialization
void Delay_Init(float delay_samples, float dfb, float dfw, float dmix);

// setters
void Delay_set_fb(float val);
void Delay_set_fw(float val);
void Delay_set_mix(float val);
void Delay_set_delay(float n_delay);

// getters
float Delay_get_fb();
float Delay_get_fw();
float Delay_get_mix();

// process
float Delay_task(float xin);

#endif /* defined(__BeatWave__delay__) */
