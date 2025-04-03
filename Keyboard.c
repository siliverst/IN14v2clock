#include "STM8S003F3.h"
#include "keyboard.h"
#include "macro.h"

#define KEY1_PIN				sfr_PORTA.IDR.IDR3
#define KEY2_PIN				sfr_PORTA.IDR.IDR2
#define KEY3_PIN				sfr_PORTA.IDR.IDR1

Keyboard kbd;
static void insert_button(uint8_t b);


void keyboard_init(void)
{
	// configure KEY1_PIN as IA input
	sfr_PORTA.DDR.DDR3 = 0;     // input(=0) or output(=1)
	sfr_PORTA.CR1.C13  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTA.CR2.C23  = 0;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTA.ODR.ODR3 = 1;
	// configure KEY2_PIN as IA input
	sfr_PORTA.DDR.DDR2 = 0;     // input(=0) or output(=1)
	sfr_PORTA.CR1.C12  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTA.CR2.C22  = 0;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTA.ODR.ODR2 = 1;
	// configure KEY3_PIN as IA input
	sfr_PORTA.DDR.DDR1 = 0;     // input(=0) or output(=1)
	sfr_PORTA.CR1.C11  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTA.CR2.C21  = 0;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTA.ODR.ODR1 = 1;
	
	kbd.KEY1_reg = 0xFE;
	kbd.KEY2_reg = 0xFE;
	kbd.KEY3_reg = 0xFE;
	kbd.HeadPtr = 0;
	kbd.TailPtr = 0;
	kbd.Refresh = 0;
}

void keyboard_refresh( void )
{
	kbd.KEY1_reg <<= 1;
	if(!KEY1_PIN) kbd.KEY1_reg |= 0x01;
	if(kbd.KEY1_reg == 0x7F) {insert_button(KEY1_DOWN);}
	if(kbd.KEY1_reg == 0x80) {insert_button(KEY1_UP);}
	
	kbd.KEY2_reg <<= 1;
	if(!KEY2_PIN) kbd.KEY2_reg |= 0x01;
	if(kbd.KEY2_reg == 0x7F) {insert_button(KEY2_DOWN);}
	if(kbd.KEY2_reg == 0x80) {insert_button(KEY2_UP);}
	
	kbd.KEY3_reg <<= 1;
	if(!KEY3_PIN) kbd.KEY3_reg |= 0x01;
	if(kbd.KEY3_reg == 0x7F) {insert_button(KEY3_DOWN);}
	if(kbd.KEY3_reg == 0x80) {insert_button(KEY3_UP);}
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
