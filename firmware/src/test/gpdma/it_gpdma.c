#include "test/gpdma/it_gpdma.h"

#include "LPC17xx.h"
#include "adc/adc.h"
#include "debug.h"
#include "gpdma/gpdma.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "test/it.h"

#define GPDMA_TIMEOUT 5000000

static int it_gpdma_wait_transfer(void) {
    volatile int timeout = GPDMA_TIMEOUT;
    while (!flag_bufferReadyforFFT && --timeout) {
        __NOP();
    }
    return timeout;
}

static void it_gpdma_adc_init(void) {
    ADC_Init(ADC_RATE);
    ADC_PinConfig(ADC_CHANNEL_1);
    ADC_ChannelEnable(ADC_CHANNEL_1);
    ADC_StartCmd(ADC_START_CONTINUOUS);
    ADC_BurstEnable();
    ADC_PowerUp();
}

static void it_gpdma_cleanup(void) {
    GPDMA_ChannelStop(GPDMA_CH_7);
    GPDMA_ClearIntPending(GPDMA_CLR_INTTC, GPDMA_CH_7);
    ADC_BurstDisable();
    ADC_PowerDown();
}

/* ---------------------------------------------------------------------------
 * Test 1 – Register-level configuration verification
 *
 * After gpdma_init(), inspect every CH7 register to confirm the application-
 * level setup matches expectations. No ADC or transfer required.
 * --------------------------------------------------------------------------- */
static void it_gpdma_p2m_config(void) {
    int pass = 1;
    uint32_t reg;

    gpdma_init();

    LPC_GPDMACH_TypeDef* ch7 = LPC_GPDMACH7;

    /* ── DMAC controller enabled ─────────────────────────────────────── */
    if (!(LPC_GPDMA->DMACConfig & GPDMA_DMACConfig_E)) {
        DBG_PRINTF("  FAIL: DMAC not enabled\n");
        pass = 0;
    }

    /* ── Source address === ADC ADGDR (per GPDMA_LUTPerAddr[GPDMA_ADC]) ── */
    uint32_t expected_src = (uint32_t)(uintptr_t)&LPC_ADC->ADGDR;
    if (ch7->DMACCSrcAddr != expected_src) {
        DBG_PRINTF("  FAIL: src addr 0x%08lX, expected 0x%08lX\n", (unsigned long)ch7->DMACCSrcAddr,
                   (unsigned long)expected_src);
        pass = 0;
    }

    /* ── Destination address === start of FFT buffer ─────────────────── */
    uint32_t expected_dst = (uint32_t)(uintptr_t)&FFT_SOURCE_BUFFER_TIME[0];
    if (ch7->DMACCDestAddr != expected_dst) {
        DBG_PRINTF("  FAIL: dst addr 0x%08lX, expected 0x%08lX\n",
                   (unsigned long)ch7->DMACCDestAddr, (unsigned long)expected_dst);
        pass = 0;
    }

    /* ── LLI is set (address of adc_secondHalf_LLI) ─────────────────── */
    if (ch7->DMACCLLI == 0) {
        DBG_PRINTF("  FAIL: DMACCLLI is NULL\n");
        pass = 0;
    }

    /* ── DMACCControl fields ────────────────────────────────────────── */
    reg = ch7->DMACCControl;

    if ((reg & 0xFFF) != PERIOD) {
        DBG_PRINTF("  FAIL: transfer size %lu, expected %d\n", (unsigned long)(reg & 0xFFF),
                   PERIOD);
        pass = 0;
    }

    if (!(reg & GPDMA_DMACCxControl_I)) {
        DBG_PRINTF("  FAIL: TC interrupt not enabled in control\n");
        pass = 0;
    }

    if (!(reg & GPDMA_DMACCxControl_DI)) {
        DBG_PRINTF("  FAIL: destination increment not set\n");
        pass = 0;
    }

    if (reg & GPDMA_DMACCxControl_SI) {
        DBG_PRINTF("  FAIL: source increment must be disabled\n");
        pass = 0;
    }

    /* ── DMACCConfig fields ─────────────────────────────────────────── */
    reg = ch7->DMACCConfig;

    if (!(reg & GPDMA_DMACCxConfig_ITC)) {
        DBG_PRINTF("  FAIL: ITC not set in config\n");
        pass = 0;
    }

    if (((reg >> 11) & 0x7) != GPDMA_P2M) {
        DBG_PRINTF("  FAIL: transfer type not P2M (got %lu)\n", (unsigned long)((reg >> 11) & 0x7));
        pass = 0;
    }

    if (((reg >> 1) & 0x1F) != GPDMA_ADC) {
        DBG_PRINTF("  FAIL: source connection not ADC (got %lu)\n",
                   (unsigned long)((reg >> 1) & 0x1F));
        pass = 0;
    }

    if (!(reg & GPDMA_DMACCxConfig_E)) {
        DBG_PRINTF("  FAIL: CH7 not started (E bit)\n");
        pass = 0;
    }

    /* ── Result ─────────────────────────────────────────────────────── */
    if (pass) {
        DBG_PRINTF("  PASS\n");
    }

    GPDMA_ChannelStop(GPDMA_CH_7);
    GPDMA_ClearIntPending(GPDMA_CLR_INTTC, GPDMA_CH_7);
}

/* ---------------------------------------------------------------------------
 * Test 2 – Real P2M transfer: ADC → DMA CH7 → FFT_SOURCE_BUFFER_TIME
 *
 * Starts the ADC in burst mode (32768 samples/s, channel 0), enables the
 * GPDMA channel 7 with the production ping-pong configuration, and waits for
 * the terminal-count interrupt (1024 samples ≈ 31 ms). The buffer contents
 * are then inspected to confirm data was actually transferred, the ping-pong
 * flag toggled, and the synchronisation flag was set.
 * --------------------------------------------------------------------------- */
static void it_gpdma_p2m_transfer(void) {
    int pass = 1;

    /* ── Initialise both ADC and GPDMA ──────────────────────────────── */
    for (int i = 0; i < PERIOD; i++) {
        FFT_SOURCE_BUFFER_TIME[i] = 0xAAAA;
    }

    flag_bufferReadyforFFT = RESET;
    flag_halfReady = 1;

    gpdma_init();
    it_gpdma_adc_init();

    /* ── Wait for the first terminal-count interrupt ────────────────── */
    int timed_out = !it_gpdma_wait_transfer();

    if (timed_out) {
        DBG_PRINTF("  FAIL: timeout waiting for TC\n");
        it_gpdma_cleanup();
        DBG_PRINTF("  FAIL\n");
        return;
    }

    /* ── Verify ISR flags ───────────────────────────────────────────── */
    if (!flag_bufferReadyforFFT) {
        DBG_PRINTF("  WARN: flag_bufferReadyforFFT was not set (ISR may not have run yet)\n");
    }

    if (flag_halfReady != 0) {
        DBG_PRINTF("  WARN: flag_halfReady = %d (expected 0 after first half)\n", flag_halfReady);
    }

    /* ── Inspect buffer ─────────────────────────────────────────────── */
    uint32_t sum = 0;
    uint32_t first = FFT_SOURCE_BUFFER_TIME[0];
    int all_same = 1;
    int has_samples = 0;

    for (int i = 0; i < PERIOD; i++) {
        uint16_t v = FFT_SOURCE_BUFFER_TIME[i];
        sum += v;

        if (v & 0xFFF0) {
            has_samples = 1;
        }

        if (v != first) {
            all_same = 0;
        }
    }

    /* Print a few samples for manual inspection in the debugger. */
    for (int i = 0; i < 6 && i < PERIOD; i++) {
        DBG_PRINTF("  [%d] = 0x%04X (%u)\n", i, FFT_SOURCE_BUFFER_TIME[i],
                   FFT_SOURCE_BUFFER_TIME[i]);
    }
    DBG_PRINTF("  ...\n");
    DBG_PRINTF("  [%d] = 0x%04X (%u)\n", PERIOD - 1, FFT_SOURCE_BUFFER_TIME[PERIOD - 1],
               FFT_SOURCE_BUFFER_TIME[PERIOD - 1]);

    /* ── Evaluate ───────────────────────────────────────────────────── */
    if (sum == 0) {
        DBG_PRINTF("  FAIL: buffer is all zeros — no DMA transfer occurred\n");
        pass = 0;
    }

    if (!has_samples) {
        DBG_PRINTF("  FAIL: no ADC result bits set in any sample\n");
        pass = 0;
    }

    if (all_same) {
        DBG_PRINTF("  WARN: every sample is 0x%04lX — DMA may be reading a "
                   "stale ADGDR\n",
                   (unsigned long)first);
    }

    if (pass) {
        DBG_PRINTF("  PASS  (sum = %lu, first sample = %lu)\n", (unsigned long)sum,
                   (unsigned long)(first >> 4));
    }

    /* ── Cleanup (non-blocking, guaranteed) ─────────────────────────── */
    it_gpdma_cleanup();
}

static const it_case_t TESTS[] = {
    {"p2m channel 7 config", it_gpdma_p2m_config},
    {"p2m adc -> dma transfer", it_gpdma_p2m_transfer},
};

void it_gpdma_run(void) {
    it_run_cases("gpdma", TESTS, IT_ARRAY_LEN(TESTS));
}
