#ifndef _KEYEVENTS_H_
#define _KEYEVENTS_H_

#ifdef	__cplusplus
extern "C" {
#endif

#define KEYS_COUNT				3
// bits of the pressed keys (bf)
#define S1               	0					// "+"
#define S2               	1					// "M"
#define S3               	2					// "-"
#define S1_M               _BV(S1)
#define S2_M               _BV(S2)
#define S3_M               _BV(S3)


typedef struct {
	uint8_t	keys_mirror;							// the last key event from the keyboard buffer
	uint8_t	bf;												// pressed keys, a key is cleared here when its release must be ignored
	uint8_t lp_counter[KEYS_COUNT];		// 100ms ticks of the key press (long press)
	uint8_t kr_counter[KEYS_COUNT];		// 100ms ticks of the key repeat
}Keyevents;



void keyevents_proc ( void );
void keyevents_counters ( void );

#ifdef	__cplusplus
}
#endif

#endif	/* _KEYEVENTS_H_ */
