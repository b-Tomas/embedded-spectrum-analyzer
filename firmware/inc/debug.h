#pragma once

/**
 * Debug-only printf wrapper.
 * Forwards to printf in debug builds and compiles to nothing when NDEBUG is defined
 * (Release). Semihosted printf blocks the target when no debugger is attached, so it must not run
 * in a Release build.
 *
 * NDEBUG is defined automatically by the compiler in Release builds.
 */

#ifdef NDEBUG
#define DBG_PRINTF(...) ((void)0)
#else
#include <stdio.h>
#define DBG_PRINTF(...) printf(__VA_ARGS__)
#endif
