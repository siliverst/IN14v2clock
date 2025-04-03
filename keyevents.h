#ifndef _KEYEVENTS_H_
#define _KEYEVENTS_H_

#ifdef	__cplusplus
extern "C" {
#endif

#define KEYS_COUNT				3
// Bits of the current button state (bf)
#define KEY1               	0
#define KEY2               	1
#define KEY3               	2
#define KEY1_M               _BV(KEY1)
#define KEY2_M               _BV(KEY2)
#define KEY3_M               _BV(KEY3)


typedef struct {
	uint8_t	keys_mirror;
	uint8_t	bf;												//buttons flags
	uint8_t lp_counter[KEYS_COUNT];
	uint8_t kr_counter[KEYS_COUNT];		//key repeat counters
}Keyivents;



void keyevents_init ( void );
void keyevents_proc ( void );
void keyevents_counters ( void );

#ifdef	__cplusplus
}
#endif

#endif	/* _KEYEVENTS_H_ */