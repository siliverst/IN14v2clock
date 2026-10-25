#include "STM8S003F3.h"
#include "hc595.h"
#include "display.h"
#include "rgb.h"
#include "macro.h"
#include "eeprom.h"
#include <string.h>

/*
Digits PWM: the 595 chain is reloaded up to 3 times per TIM2 period (10000 ticks, 100/200/400Hz, see displaySetPwm()):
by update with the "on" frame, by CC1 (at t1) and CC3 (at t2 >= t1) with the frames of the digits still on.
A digit has its on time: full bright digits - all the period, dim digits - dimLevel, others - bright0.
t1, t2 are the two on times of the normal and dim digits, CC3 is used only for the dim digits (flip fade effect).
Two banks of frames: main fills the back bank and switches "front" (one byte write, atomic),
so the ISR never shifts out a half updated frame.
*/
#define FRAME_SIZE		HC595_CHAIN
#define PERIOD				10000
static uint8_t	frames[2][3*FRAME_SIZE];					// [bank][on frame, frame after t1, frame after t2]
static volatile uint8_t front;										// bank used by ISR
static uint8_t	digits[NIXIE_COUNT];							// last shown digits, NIXIE_OFF - blank
static uint8_t	fullMask;													// digits at full brightness
static uint8_t	dimMask;													// digits with dimLevel brightness
static uint16_t	dimLevel;													// on time of the dim digits, timer ticks
static uint8_t	bright0;													// current digits brightness, %
static uint8_t	pwmDiv = 1;												// TIM2 periods in 10ms
volatile uint8_t flag10ms;

static void displayRender(void);

void displayInit ( void )
{
	// the colon neons (PD4, PD5) are not used by this firmware, the pins stay inputs

	// configure TIM2: 100Hz at start (1MHz, 10000 ticks), then displaySetPwm()
  sfr_TIM2.CR1.ARPE = 1;								// use buffered period register to avoid glitches
  sfr_TIM2.PSCR.PSC = 4;								// set TIM2 prescaler fTim = fcpu/2^pre with pre in [0..15], see displaySetPwm()
  sfr_TIM2.ARRH.byte = hibyte(10000);		// set reload period = (ARR+1)/fTim
  sfr_TIM2.ARRL.byte = lobyte(10000);
	sfr_TIM2.CCMR1.OC1PE 	= 1;						// buffer compare registers to avoid glitches when changing duty cycle:
	sfr_TIM2.CCMR3.OC3PE 	= 1;						// main writes them just after update, a new value below the counter is missed
	sfr_TIM2.IER.UIE 			= 1;						// TIM2 update interrupt enable
	sfr_TIM2.CR1.CEN     	= 1;						// start TIM2
	
	hc595Init();
	hc595ChainShiftOut(frames[0]);	// all off
	memset(digits, NIXIE_OFF, sizeof(digits));
	displaySetBright(100);
	
	RGBinit();
}

/*
set bits of the digits from mask into the frame.
Bit of the chain: g = position*10 + digit, byte frame[7 - g/8], bit g%8 (see README, wiring):
the frame is shifted out from frame[0] bit 7, g = 0 (U14 Q0) is the last bit, g = 60..63 are U21 Q4..Q7 (not connected).
*/
static void displayFill(uint8_t *frame, uint8_t mask)
{
	uint8_t pos, g;
	
	memset(frame, 0, FRAME_SIZE);
	for (pos = 0, g = 0; pos < NIXIE_COUNT; pos++, g += 10, mask >>= 1){
		if ((mask & 1) && (digits[pos] < 10)){
			frame[FRAME_SIZE - 1 - ((g + digits[pos]) >> 3)] |= (uint8_t)(1 << ((g + digits[pos]) & 7));
		}
	}
}

/*
frames and compare values by the on times of the digits.
A compare at 0 or at the period end is not needed (nothing changes), it is disabled.
*/
static void displayRender(void)
{
	uint8_t *f = frames[front ^ 1];
	uint8_t d, b, mOn = 0, mT1 = 0, mT2 = 0;
	uint16_t l, t1, t2;
	
	t1 = t2 = (uint16_t)bright0 * 100;
	if (dimMask){
		if (dimLevel < t1) {t1 = dimLevel;}
		else {t2 = dimLevel;}
	}
	for (d = 0, b = 1; d < NIXIE_COUNT; d++, b <<= 1){
		l = (fullMask & b) ? PERIOD + 1 : (dimMask & b) ? dimLevel : (uint16_t)bright0 * 100;
		if (l) {mOn |= b;}
		if (l > t1) {mT1 |= b;}
		if (l > t2) {mT2 |= b;}
	}
	displayFill(f, mOn);
	displayFill(f + FRAME_SIZE, mT1);
	displayFill(f + 2*FRAME_SIZE, mT2);
	sfr_TIM2.CCR1H.byte = hibyte(t1);
	sfr_TIM2.CCR1L.byte = lobyte(t1);
	sfr_TIM2.CCR3H.byte = hibyte(t2);
	sfr_TIM2.CCR3L.byte = lobyte(t2);
	d = (t1 != 0) && (t1 < PERIOD);					// new CC1IE
	b = (t2 != t1) && (t2 < PERIOD);					// new CC3IE
	// a compare flag is set also when its interrupt is disabled: clear the stale one before enabling,
	// else the frame is shifted out at once (byte write: rc_w0 flags, the others are not touched)
	l = 0;
	if (d && !sfr_TIM2.IER.CC1IE) {l |= 0x02;}
	if (b && !sfr_TIM2.IER.CC3IE) {l |= 0x08;}
	if (l) {sfr_TIM2.SR1.byte = (uint8_t)~l;}
	sfr_TIM2.IER.CC1IE = d;
	sfr_TIM2.IER.CC3IE = b;
	front ^= 1;
}

/*
digits PWM: 0 - 100Hz, 1 - 200Hz, 2 - 400Hz. Only the timer clock is changed (1, 2, 4MHz), the period is
10000 ticks always, so all the compare values stay the same. The prescaler is buffered: applied at the update.
*/
void displaySetPwm(uint8_t f)
{
	sfr_TIM2.PSCR.PSC = 4 - f;
	pwmDiv = (uint8_t)(1 << f);
}

// bright -> 0-100%
void displaySetBright(uint8_t bright)
{
	bright0 = bright;
	displayRender();
}

uint8_t displayGetBright(void)
{
	return bright0;
}

// digits from mask are shown with the on time level, timer ticks (0..10000), mask 0 - off
void displayDim(uint8_t mask, uint16_t level)
{
	dimMask = mask;
	dimLevel = level;
	displayRender();
}

/*
data[6] -> {seconds, tens of seconds, minutes, tens of minutes, hours, tens of hours}, NIXIE_OFF - blank
full_bright_bitmask -> digits shown at 100% (also at the brightness 0)
*/
void displayNixie(uint8_t *data, uint8_t full_bright_bitmask)
{
	memcpy(digits, data, sizeof(digits));
	fullMask = full_bright_bitmask;
	displayRender();
}

/*
An EEPROM write stalls the CPU (and the ISRs) for up to 6ms, the 595 chain keeps its state meanwhile,
so the stall lengthens the current PWM phase. Wait for the start of the longer phase:
the "off" phase at low brightness, the "on" phase at high brightness (else digits flash or dim).
*/
void displayWaitLongPhase(void)
{
	uint8_t t = (bright0 < 50) ? sfr_TIM2.CCR1H.byte : 0;	// phase start, 256 ticks units
	
	if (sfr_TIM2.CR1.CEN){																// timer is not running yet at start
		while ((uint8_t)(sfr_TIM2.CNTRH.byte - t - 1) > 1);	// 256..767 ticks after the phase start (ISR is done)
	}
}

ISR_HANDLER (TIM2_UPD_ISR, _TIM2_OVR_UIF_VECTOR_)
{
	static uint8_t pwmCount;
	
	hc595ChainShiftOut(frames[front]);
	if (++pwmCount >= pwmDiv){
		pwmCount = 0;
		flag10ms = 1;
	}
	sfr_TIM2.SR1.UIF = 0;
  return;
} // TIM2_UPD_ISR


ISR_HANDLER (TIM2_CAP_ISR, _TIM2_CAPCOM_CC1IF_VECTOR_)
{
	//for digits (the flag is set also when the interrupt is disabled)
	if(sfr_TIM2.SR1.CC1IF){
		if (sfr_TIM2.IER.CC1IE){
			hc595ChainShiftOut(frames[front] + FRAME_SIZE);
		}
		sfr_TIM2.SR1.CC1IF = 0;
	}
	if(sfr_TIM2.SR1.CC3IF){
		if (sfr_TIM2.IER.CC3IE){
			hc595ChainShiftOut(frames[front] + 2*FRAME_SIZE);
		}
		sfr_TIM2.SR1.CC3IF = 0;
	}
  return;
} // TIM2_CAP_ISR
