#pragma once

#include <stdint.h>
#include "dsp.h"
#include "lpc_types.h"

#define NOISE_SAMPLES 10
#define NOISE_TICKS 10

extern uint8_t nsm_buffer[N_BARS];
extern FlagStatus noiseDone;
extern  uint8_t displayTick;
extern  uint16_t avg_sample;