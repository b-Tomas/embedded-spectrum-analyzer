/**
 * On-target integration test harness.
 *
 * it_run_all() exercises each subsystem's tests on real hardware. Tests are grouped by subsystem
 * mirroring the source tree.
 */

#pragma once

#include <stdint.h>

/* An integration test. It's name (for logging purposes) and the routine that runs it. */
typedef struct {
    const char* name;
    void (*run)(void);
} it_case_t;

#define IT_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

/**
 * @brief Run every subsystem's integration tests once.
 */
void it_run_all(void);

/**
 * @brief Run a table of test cases, logging progress over semihosting.
 *
 * @param group human-readable group name for the log
 * @param cases table of test cases
 * @param n     number of entries in @p cases
 */
void it_run_cases(const char* group, const it_case_t* cases, uint32_t n);

/** Busy-wait some time. Hardware-agnostic. */
void it_delay(void);
