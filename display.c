#include "STM8S003F3.h"
#include "hc595.h"
#include "display.h"
#include "rgb.h"
#include "macro.h"
#include "eeprom.h"
#include <string.h>

const uint16_t scaleNixie[101] = {
    0,100,200,300,400,500,600,700,800,900,
  1000,1100,1200,1300,1400,1500,1600,1700,1800,1900,
  2000,2100,2200,2300,2400,2500,2600,2700,2800,2900,
  3000,3100,3200,3300,3400,3500,3600,3700,3800,3900,
  4000,4100,4200,4300,4400,4500,4600,4700,4800,4900,
  5000,5100,5200,5300,5400,5500,5600,5700,5800,5900,
  6000,6100,6200,6300,6400,6500,6600,6700,6800,6900,
  7000,7100,7200,7300,7400,7500,7600,7700,7800,7900,
  8000,8100,8200,8300,8400,8500,8600,8700,8800,8900,
  9000,9100,9200,9300,9400,9500,9600,9700,9800,9900,
  10000 };
	
static volatile	uint8_t zero_data[8] 							= {0,0,0,0,0,0,0,0};
static volatile uint8_t disp_data[8]							= {0,0,0,0,0,0,0,0};
			 volatile uint8_t flag10ms;
static 					uint16_t displayBright;

static uint8_t *displayNixieBuffPrepare(uint8_t *inbuff, uint8_t dmask);

void displayInit ( void )
{
	// configure TIM2 with 100Hz frequency, count to 10000;
  sfr_TIM2.CR1.ARPE = 1;								// use buffered period register to avoid glitches
  sfr_TIM2.PSCR.PSC = 4;								// set TIM2 prescaler fTim = fcpu/2^pre with pre in [0..15]

  
  sfr_TIM2.ARRH.byte = hibyte(10000);		// set reload period = (ARR+1)/fTim
  sfr_TIM2.ARRL.byte = lobyte(10000);
	
	sfr_TIM2.CCR1H.byte = hibyte(1000);		// set PWM channel 1 duty period
	sfr_TIM2.CCR1L.byte = lobyte(1000);
	sfr_TIM2.CCR2H.byte = hibyte(0);			// set PWM channel 2 duty period
	sfr_TIM2.CCR2L.byte = lobyte(0);
	
	sfr_TIM2.CCMR1.OC1PE 	= 1;						// buffer compare register to avoid glitches when changing duty cycle
	sfr_TIM2.CCER1.CC1E 	= 0;						// TIM2 channel 1 output disable
	sfr_TIM2.CCER1.CC2E 	= 0;						// TIM2 channel 2 output disable
	sfr_TIM2.IER.CC1IE 		= 1;						// TIM2 channel 1 compare interrupt enable
	sfr_TIM2.IER.CC2IE 		= 0;						// TIM2 channel 2 compare interrupt disable (used before for DOT dimming)
	sfr_TIM2.IER.UIE 			= 1;						// TIM2 update interrupt enable
	sfr_TIM2.CR1.CEN     	= 1;						// start TIM2
	
	hc595Init();
	displaySetBright(100);
	hc595ChainShiftOut(zero_data,sizeof(zero_data));
	RGBinit();
	//displayRGBset(e.rgbGlobalEn);
	displayRGBset(0);
}

//inbuff -> {seconds, tens of seconds, minutes, tens of minutes, hours, tens of hours}
//dmask  -> bitmask of digits that have been prepared
static uint8_t *displayNixieBuffPrepare(uint8_t *inbuff, uint8_t dmask)
{
	static uint8_t data[8];
	
	memset(data,0,sizeof(data));
	
	if (bitchk(dmask,0)){
		// seconds
		switch(inbuff[0]){
			case 0:
				bitset(data[7],4);
				break;
			case 1:
				bitset(data[7],5);
				break;
			case 2:
				bitset(data[7],6);
				break;
			case 3:
				bitset(data[7],7);
				break;
			case 4:
				bitset(data[6],0);
				break;
			case 5:
				bitset(data[6],1);
				break;
			case 6:
				bitset(data[6],2);
				break;
			case 7:
				bitset(data[6],3);
				break;
			case 8:
				bitset(data[6],4);
				break;
			case 9:
				bitset(data[6],5);
				break;
			default:
				break;
		}
	}
	if (bitchk(dmask,1)){
		// tens of seconds
		switch(inbuff[1]){
			case 0:
				bitset(data[6],6);
				break;
			case 1:
				bitset(data[6],7);
				break;
			case 2:
				bitset(data[5],0);
				break;
			case 3:
				bitset(data[5],1);
				break;
			case 4:
				bitset(data[5],2);
				break;
			case 5:
				bitset(data[5],3);
				break;
			case 6:
				bitset(data[5],4);
				break;
			case 7:
				bitset(data[5],5);
				break;
			case 8:
				bitset(data[5],6);
				break;
			case 9:
				bitset(data[5],7);
				break;
			default:
			break;
		}
	}
	if (bitchk(dmask,2)){
		// minutes 
		switch(inbuff[2]){
			case 0:
				bitset(data[4],0);
				break;
			case 1:
				bitset(data[4],1);
				break;
			case 2:
				bitset(data[4],2);
				break;
			case 3:
				bitset(data[4],3);
				break;
			case 4:
				bitset(data[4],4);
				break;
			case 5:
				bitset(data[4],5);
				break;
			case 6:
				bitset(data[4],6);
				break;
			case 7:
				bitset(data[4],7);
				break;
			case 8:
				bitset(data[3],0);
				break;
			case 9:
				bitset(data[3],1);
				break;
			default:
				break;
		}
	}
	if (bitchk(dmask,3)){
		// tens of minutes
		switch(inbuff[3]){
			case 0:
				bitset(data[3],2);
				break;
			case 1:
				bitset(data[3],3);
				break;
			case 2:
				bitset(data[3],4);
				break;
			case 3:
				bitset(data[3],5);
				break;
			case 4:
				bitset(data[3],6);
				break;
			case 5:
				bitset(data[3],7);
				break;
			case 6:
				bitset(data[2],0);
				break;
			case 7:
				bitset(data[2],1);
				break;
			case 8:
				bitset(data[2],2);
				break;
			case 9:
				bitset(data[2],3);
				break;
			default:
			break;
		}
	}
	if (bitchk(dmask,4)){
		// hours
		switch(inbuff[4]){
			case 0:
				bitset(data[2],4);
				break;
			case 1:
				bitset(data[2],5);
				break;
			case 2:
				bitset(data[2],6);
				break;
			case 3:
				bitset(data[2],7);
				break;
			case 4:
				bitset(data[1],0);
				break;
			case 5:
				bitset(data[1],1);
				break;
			case 6:
				bitset(data[1],2);
				break;
			case 7:
				bitset(data[1],3);
				break;
			case 8:
				bitset(data[1],4);
				break;
			case 9:
				bitset(data[1],5);
				break;
			default:
			break;
		}
	}
	if (bitchk(dmask,5)){
		// tens of hours
		switch(inbuff[5]){
			case 0:
				bitset(data[1],6);
				break;
			case 1:
				bitset(data[1],7);
				break;
			case 2:
				bitset(data[0],0);
				break;
			case 3:
				bitset(data[0],1);
				break;
			case 4:
				bitset(data[0],2);
				break;
			case 5:
				bitset(data[0],3);
				break;
			case 6:
				bitset(data[0],4);
				break;
			case 7:
				bitset(data[0],5);
				break;
			case 8:
				bitset(data[0],6);
				break;
			case 9:
				bitset(data[0],7);
				break;
			default:
			break;
		}
	}
	return data;
}

/*uint8_t bright -> 0-100%
*/
void displaySetBright(uint8_t bright)
{
	static uint16_t brightLast = 0;
	
	if (bright == 0){
		memset(disp_data,0,sizeof(disp_data));
	}else{
		if (bright > 100) {bright = 100;}
		displayBright = scaleNixie[bright];
		if (brightLast != displayBright){
			brightLast = displayBright;
			if (brightLast < NIXIE_MIN_BRIGHT){
				brightLast = NIXIE_MIN_BRIGHT;
			}
			sfr_TIM2.CCR1H.byte = hibyte(brightLast);		// set PWM channel 1 duty period
			sfr_TIM2.CCR1L.byte = lobyte(brightLast);
			
			RGBgammaCalculate(displayBright);
		}
	}
}

/*input: 
uint8_t data[6] -> {seconds, tens of seconds, minutes, tens of minutes, hours, tens of hours}
*/
void displayNixie(uint8_t *data, uint8_t full_bright_bitmask)
{
	memcpy(disp_data,displayNixieBuffPrepare(data,bin(00111111)),sizeof(disp_data));
	memcpy(zero_data,displayNixieBuffPrepare(data,full_bright_bitmask),sizeof(zero_data));
}

// this interrupt lights up the digits
ISR_HANDLER (TIM2_UPD_ISR, _TIM2_OVR_UIF_VECTOR_)
{
	hc595ChainShiftOut(disp_data,sizeof(disp_data));
	flag10ms = 1;
	sfr_TIM2.SR1.UIF = 0;
} // TIM2_UPD_ISR

// this interrupt turns off the digits
ISR_HANDLER (TIM2_CAP_ISR, _TIM2_CAPCOM_CC1IF_VECTOR_)
{
	hc595ChainShiftOut(zero_data,sizeof(zero_data));
	sfr_TIM2.SR1.CC1IF = 0;
} // TIM2_CAP_ISR

// state 0 or 1 (off or on)
void displayRGBset (uint8_t state)
{
	if (state){
		RGBsetR(RGBgammaGet(EEPROM_readByte(R_ADDR)));
		RGBsetG(RGBgammaGet(EEPROM_readByte(G_ADDR)));
		RGBsetB(RGBgammaGet(EEPROM_readByte(B_ADDR)));
	}else{
		RGBsetR(0);
		RGBsetG(0);
		RGBsetB(0);
	}
}
// value is from 0 to 255
void displayRset(uint8_t value)
{
	RGBsetR(RGBgammaGet(value));
}

void displayGset(uint8_t value)
{
	RGBsetG(RGBgammaGet(value));
}

void displayBset(uint8_t value)
{
	RGBsetB(RGBgammaGet(value));
}