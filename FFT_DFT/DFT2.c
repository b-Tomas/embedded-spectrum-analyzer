#include <stdio.h>
#include <math.h>
#include <stdint.h>
#define AXIS_WIDTH 200
#define SCALAR 200
const int period = 100;
uint32_t memory1[period];
int32_t memory2[period];
int32_t memory3[period];
uint32_t* ArrayT = memory1;
int32_t* ArrayFR = memory2;
int32_t* ArrayFI = memory3;
// Ancho del eje: esto es para poder ver los numeros negativos de forma mas comoda

int sum(double* source) {
    double acum = 0.0;
    for (int i = 0; i < period; i++)
        acum += source[i];
    return (int)round(acum);
}

//Da un solo valor de la DFT
int RealDFTV(uint32_t* source, int k) {
    double temp[period];
    double omega = 2.0 * M_PI * k / period;
    for (int i = 0; i < period; i++)
        temp[i] = source[i] * cos(omega * i);
    return sum(temp);
}

int ImDFTV(uint32_t* source, int k) {
    double temp[period];
    double omega = 2.0 * M_PI * k / period;
    for (int i = 0; i < period; i++)
        temp[i] = source[i] * -sin(omega * i);
    return sum(temp);
}

void DFT(uint32_t* sourceT, int32_t* sourceR, int32_t* sourceI) {
    for (int k = 0; k < period; k++) {
        sourceR[k] = RealDFTV(sourceT, k);
        sourceI[k] = ImDFTV(sourceT, k);
    }
}



void printWave(int32_t* source) {
    for (int i = 0; i < period; i++) {
        int val = source[i];
        char line[2 * AXIS_WIDTH + 4];
        int pos = 0;

        if (val >= 0) {
            // Parte izquierda: espacios hasta el eje
            for (int j = 0; j < AXIS_WIDTH; j++)
                line[pos++] = ' ';
            // Eje
            line[pos++] = '|';
            // Parte derecha: guiones
            int bars = val;
            if (bars > AXIS_WIDTH) bars = AXIS_WIDTH;
            for (int j = 0; j < bars; j++)
                line[pos++] = '-';
            line[pos++] = '*';
        } else {
            // Valor negativo: la barra va a la izquierda del eje
            int bars = -val;
            if (bars > AXIS_WIDTH) bars = AXIS_WIDTH;
            int spaces = AXIS_WIDTH - bars;
            for (int j = 0; j < spaces; j++)
                line[pos++] = ' ';
            line[pos++] = '*';
            for (int j = 0; j < bars; j++)
                line[pos++] = '-';
            // Eje
            line[pos++] = '|';
        }

        line[pos] = '\0';
        puts(line);
        fflush(stdout);
    }
}

void setFunc_Seno(void) {
    for (int i = 0; i < period; i++)
        ArrayT[i] = (uint32_t)(2048 + SCALAR * sin(2.0 * M_PI * i / period));
}
void setFunc_Cuadrada(void) {
    for (int i = 0; i < period; i++)
        ArrayT[i] = (i < period / 2) ? 4000 : 96;
}
void setFunc_Triangular(void) {
    for (int i = 0; i < period; i++) {
        double t = (double)i / period;
        double val = 1.0 - 4.0 * fabs(t - 0.5);
        ArrayT[i] = (uint32_t)(2048 + SCALAR * val);
    }
}
void setFunc_Sawtooth(void) {
    for (int i = 0; i < period; i++) {
        double t = (double)i / period;  // t in [0, 1)
        double val = 2.0 * t - 1.0;    // val in [-1, 1)
        ArrayT[i] = (uint32_t)(2048 + SCALAR * val);
    }
}


// Para visualizar ArrayT centrado en 2048
void printWaveU32_centered(uint32_t* source) {
    int32_t centered[period];
    for (int i = 0; i < period; i++)
        centered[i] = (int32_t)source[i] - 2048;
    printWave(centered);
}

void setFunc(void) {
    for (int i = 0; i < period/2; i++)
       
        ArrayT[i] =4000;
}

int main(void) {
    setFunc_Seno();

    printf("--- Señal temporal (centrada en 2048) ---\n");
    printWaveU32_centered(ArrayT);

    printf("|----- DFT -----|\n");
    fflush(stdout);

    DFT(ArrayT, ArrayFR, ArrayFI);

    printf("Parte Real\n");
    printWave(ArrayFR);
        printf("Parte Imag\n");
    printWave(ArrayFI);
     

    return 0;
}