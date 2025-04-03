#include "STM8S003F3.h"
#include "hc595.h"

// HC595 pin definitions
#define CLOCK_PIN   						sfr_PORTD.ODR.ODR3   // Pin 20 (SRCLK HC595 (11))
#define LATCH_PIN   						sfr_PORTC.ODR.ODR5   // Pin 15 (RCLK 	HC595 (12))
#define	OE_PIN      						sfr_PORTD.ODR.ODR2   // Pin 19 (nOE 	HC595 (13))
#define DATA_PIN    						sfr_PORTC.ODR.ODR7   // Pin 17 (SER 	HC595 (14) of the first 74HC595)

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

// with in14*6 board need to shift 60 bit (7 bytes and 4 bits)
void hc595ChainShiftOut (uint8_t *data, uint8_t length) 
{
	uint8_t	r,b,t;
	
	for (r = 0; r < length-1; r++){
		t = *data++;
		for (b = 0; b < 8; b++) {
			// Write the MSB (bit 7) to the DATA pin
			if (t & 0x80) {
					DATA_PIN = 1;
			} else {
					DATA_PIN = 0;
			}
			// Pulse the SRCLK (Shift Clock)
			CLOCK_PIN = 1;
			t <<= 1;	// Shift to the next bit
			CLOCK_PIN = 0;
		}
	}
	
	//last 4 bit
	t = *data++;
	for (b = 4; b < 8; b++) {
		if (t & 0x80) {
				DATA_PIN = 1;
		} else {
				DATA_PIN = 0;
		}
		CLOCK_PIN = 1;
		t <<= 1;
		CLOCK_PIN = 0;
	}
	
	LATCH_PIN = 1;
	LATCH_PIN = 0;
}