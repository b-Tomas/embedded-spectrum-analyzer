#include <stdio.h>
#include <stdint.h>
#include <math.h>
#define LOG2_PERIOD  3
#define PERIOD       (1 << LOG2_PERIOD)

static int16_t tw_cos[PERIOD];
static int16_t tw_sin[PERIOD];

void init_twiddle(void) {
    for (int k = 0; k < PERIOD; k++) {
        double angle = 2.0 * M_PI * k / PERIOD;
        tw_cos[k] = (int16_t)lround( cos(angle) * 32767.0);
        tw_sin[k] = (int16_t)lround( sin(angle) * 32767.0);
    }
}

void printArray(int16_t* ArrayT) {
    printf("[");
    for (int i = 0; i < PERIOD; i++) {
        printf("%d", ArrayT[i]);
        if (i < PERIOD - 1) printf(",");
    }
    printf("]\n");
}
void printArrayNormalized(int16_t* ArrayT) {
    printf("[");
    for (int i = 0; i < PERIOD; i++) {
        printf("%f", ArrayT[i]/32767.0);
        if (i < PERIOD - 1) printf(",");
    }
    printf("]\n");
}

int main(void){
init_twiddle();
printArray(tw_cos);
printArrayNormalized(tw_cos);

 return 0;   
}