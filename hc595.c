#include "STM8S003F3.h"
#include "hc595.h"

// HC595 pin definitions
#define CLOCK_PIN   						sfr_PORTD.ODR.ODR3   // Pin 20 (SRCLK)
#define LATCH_PIN   						sfr_PORTC.ODR.ODR5   // Pin 15 (RCLK)
#define	OE_PIN      						sfr_PORTD.ODR.ODR2   // Pin 19 (OE of U14 only)
#define DATA_PIN    						sfr_PORTC.ODR.ODR7   // Pin 17 (PC7, DATA connected to DS of the first 74HC595)

void hc595Init (void) 
{
	// Initialize GPIOs for the pins used with the 74HC595
	
	// configure CLOCK_PIN as output
	sfr_PORTD.DDR.DDR3 = 1;     // input(=0) or output(=1)
	sfr_PORTD.CR1.C13  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTD.CR2.C23  = 1;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
  
  // configure LATCH_PIN as output
	sfr_PORTC.DDR.DDR5 = 1;     // input(=0) or output(=1)
	sfr_PORTC.CR1.C15  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTC.CR2.C25  = 1;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
		
	// configure OE_PIN as output
	sfr_PORTD.DDR.DDR2 = 1;     // input(=0) or output(=1)
	sfr_PORTD.CR1.C12  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTD.CR2.C22  = 1;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope	
		
	// configure DATA_PIN as output
	sfr_PORTC.DDR.DDR7 = 1;     // input(=0) or output(=1)
	sfr_PORTC.CR1.C17  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTC.CR2.C27  = 1;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope	
	
	// enable output for the first 595 in chain, all other 595 OE inputs is on ground.
	OE_PIN = 0;
	
}
/*
Shift out HC595_CHAIN bytes (MSB first) and latch them. Called from the TIM2 ISRs, the time matters:
the shortest PWM pulse of a digit is about the time of one shift out. In C it was ~14 cycles per bit (~65us
for the chain), in assembler ~4.5 cycles per bit (~19us): bccm copies the carry (the bit) to DATA at once.
DATA is set 1 cycle before the CLOCK rise, CLOCK is high 2 cycles (125ns), the 74HC595 needs ~20ns.
The pointer is passed by a global, so the code does not depend on the calling convention of the compiler.
Ports: PC_ODR 0x500A (DATA bit 7, LATCH bit 5), PD_ODR 0x500F (CLOCK bit 3).
The chain length is in the code (ldw y,#8), HC595_CHAIN is checked below.
*/
#if HC595_CHAIN != 8
#error "the chain length is in the assembler code below"
#endif
uint8_t *hc595Src;

#define BIT_COSMIC		_asm("bccm 20490,#7\n bset 20495,#3\n sll a\n bres 20495,#3\n")

void hc595ChainShiftOut (uint8_t *data)
{
	hc595Src = data;
#if defined(__CSMC__)
	_asm("ldw x,_hc595Src\n ldw y,#8\nL_hc595_byte:\n ld a,(x)\n incw x\n sll a\n");
	BIT_COSMIC; BIT_COSMIC; BIT_COSMIC; BIT_COSMIC; BIT_COSMIC; BIT_COSMIC; BIT_COSMIC; BIT_COSMIC;
	_asm(" decw y\n jrne L_hc595_byte\n bset 20490,#5\n bres 20490,#5\n");
#else
	__asm
		ldw x, _hc595Src
		ldw y, #8
	00001$:
		ld a, (x)
		incw x
		sll a
		bccm 0x500a, #7
		bset 0x500f, #3
		sll a
		bres 0x500f, #3
		bccm 0x500a, #7
		bset 0x500f, #3
		sll a
		bres 0x500f, #3
		bccm 0x500a, #7
		bset 0x500f, #3
		sll a
		bres 0x500f, #3
		bccm 0x500a, #7
		bset 0x500f, #3
		sll a
		bres 0x500f, #3
		bccm 0x500a, #7
		bset 0x500f, #3
		sll a
		bres 0x500f, #3
		bccm 0x500a, #7
		bset 0x500f, #3
		sll a
		bres 0x500f, #3
		bccm 0x500a, #7
		bset 0x500f, #3
		sll a
		bres 0x500f, #3
		bccm 0x500a, #7
		bset 0x500f, #3
		sll a
		bres 0x500f, #3
		decw y
		jrne 00001$
		bset 0x500a, #5
		bres 0x500a, #5
	__endasm;
#endif
}
