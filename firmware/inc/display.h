#if !defined(DISPLAY_H)
#define DISPLAY_H

/*@biref setup for the Display
 */
void DISPLAY_setup(void);

/* @brief Display config when mode is in real time.
 */
void DISPLAY_setRelTimeMode(void);

/*@brief Display config when mode is in equalizer mode.
 */
void DISPLAY_setEqualizerMode(void);
#endif // DISPLAY_H