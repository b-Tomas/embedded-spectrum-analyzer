#include "display/gfx.h"
#include "system/system.h"
#include "noise_sampling_mode.h"
#include "gpdma/gpdma.h"
#include "lpc17xx_dac.h"
#include "system/eq_mode.h"
#include "system/system.h"
#include "noise_sampling_mode.h"

static void init() {
    init_bars();
    for(volatile int i=0;i<N_BARS;i++)
    nsm_buffer[i]=0; //clear the nsm buffer
    noiseDone = 0;
    displayTick = 0;
    avg_sample = 0;
}
static void deInit() {
    // No-op
}

static void tick() {
    
    
    
    if (!flag_bufferReadyforFFT)
    return;
    flag_bufferReadyforFFT = RESET;

    dsp_FFT();
    applyFilter();

    if (SYSTEM.flag_readyToDisplay && (displayTick%NOISE_TICKS == 0)) {
        SYSTEM.flag_readyToDisplay = RESET;
        if(!noiseDone){
        uint8_t bars[N_BARS];
        //bars has the magnitude of fft
        dsp_computeMagnitudeBars(bars);
        //acumulation fase
        for(volatile int i=0;i<N_BARS;i++)
        nsm_buffer[i] += bars[i];
        //view acumulation
        update_bars(nsm_buffer);
        }
        else{
        //sum of the indx of the sum indexes
        for(volatile int i=0;i<N_BARS;i++)    
        avg_sample += nsm_buffer[i];
        avg_sample/=(N_BARS*NOISE_SAMPLES); //it works ??
        for(volatile int i=0;i<N_BARS;i++)
        nsm_buffer[i] = (i > avg_sample) ? 0 : Q15_ONE;
        }
        if(displayTick == NOISE_SAMPLES)
        noiseDone = SET;
        displayTick++;
        
    }
    
    // No-op
}


static void handleKey(char c) {
    // No-op
}

static SystemMode_T noiseSamplingModeCfg = {init, deInit, tick, handleKey};

void noiseSamplingMode_registerHooks() {
    system_registerMode(noiseSamplingMode, &noiseSamplingModeCfg);
}
