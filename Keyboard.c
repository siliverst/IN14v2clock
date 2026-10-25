#include "STM8S003F3.h"
#include "keyboard.h"
#include "macro.h"

// keys on the board front from the left: UP (KEY3), DOWN (KEY2), SET (KEY1), the firmware uses them as "-", "M", "+"
#define S1_PIN				sfr_PORTA.IDR.IDR3			// "+" (SET, KEY1, the right one)
#define S2_PIN				sfr_PORTA.IDR.IDR2			// "M" (DOWN, KEY2, the middle one)
#define S3_PIN				sfr_PORTA.IDR.IDR1			// "-" (UP, KEY3, the left one)

Keyboard kbd;
static void insert_button(uint8_t b);


void keyboard_init(void)
{
	// PA1..PA3 inputs with pull-up (also an external 10k), no interrupt
	sfr_PORTA.DDR.byte &= (uint8_t)~0x0E;		// input(=0) or output(=1)
	sfr_PORTA.CR1.byte |= 0x0E;							// input: 0=float, 1=pull-up
	sfr_PORTA.CR2.byte &= (uint8_t)~0x0E;		// input: 0=no exint, 1=exint

	kbd.S1_reg = 0xFE;
	kbd.S2_reg = 0xFE;
	kbd.S3_reg = 0xFE;
	kbd.HeadPtr = 0;
	kbd.TailPtr = 0;
}

// a key event is given after 7 equal samples (20ms period): press - 0x7F, release - 0x80
static void key_refresh(uint8_t *reg, uint8_t pressed, uint8_t key)
{
	*reg <<= 1;
	if (pressed) *reg |= 0x01;
	if (*reg == 0x7F) {insert_button(key);}
	if (*reg == 0x80) {insert_button(key | 0x80);}
}

void keyboard_refresh( void )
{
	key_refresh(&kbd.S1_reg, !S1_PIN, S1_DOWN);
	key_refresh(&kbd.S2_reg, !S2_PIN, S2_DOWN);
	key_refresh(&kbd.S3_reg, !S3_PIN, S3_DOWN);
}


uint8_t keyboard_get_key(void)
{
	uint8_t result;
	if(kbd.HeadPtr == kbd.TailPtr) return 255;
	result = kbd.Buffer[kbd.TailPtr];
	if(++kbd.TailPtr == 8) kbd.TailPtr = 0;
	return(result);
}

static void insert_button(uint8_t b )
{
	kbd.Buffer[kbd.HeadPtr] = b;
	if(++kbd.HeadPtr == 8) kbd.HeadPtr = 0;
}
