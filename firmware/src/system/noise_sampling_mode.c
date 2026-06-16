#include "display/gfx.h"
#include "system/system.h"

static void init() {
    init_bars();
}

static void deInit() {
    // No-op
}

static void tick() {
    // No-op
}

static void handleKey(char c) {
    // No-op
}

static SystemMode_T noiseSamplingModeCfg = {init, deInit, tick, handleKey};

void noiseSamplingMode_registerHooks() {
    system_registerMode(noiseSamplingMode, &noiseSamplingModeCfg);
}
