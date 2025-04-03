#include <stdint.h>
#include "keyevents.h"
#include "macro.h"
#include "keyboard.h"
#include "display.h"
#include "iface.h"
#include "ds3231.h"
#include "eeprom.h"

/*
longpress_keyX		-	in keyivents_counters
fastrelease_keyX 	-	in processing of button release
ivent_single_keyX - this is what the button does in longpress or fastrelease
*/

void event_single_key1(void);
void event_single_key2(void);
void event_single_key3(void);
void longpress_key1(void);
void longpress_key2(void);
void longpress_key3(void);
void fastReleaseKey1(void);
void fastReleaseKey2(void);
void fastReleaseKey3(void);

Keyivents ki;
extern Iface i;

void keyevents_init ( void )
{

}

void keyevents_proc ( void )
{
	ki.keys_mirror = keyboard_get_key();

	if(ki.keys_mirror != 0xFF){
		//---------------------------------------------------------------------------------------------------------------
		// Button 1 is pressed ( PLUS )
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == KEY1_DOWN)
		{
			bitmaskset(ki.bf,KEY1_M);
			if (ki.bf == KEY1_M){
				event_single_key1();
			}
		}
		//---------------------------------------------------------------------------------------------------------------
		// Button 2 is pressed ( MENU )
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == KEY2_DOWN)
		{
			bitmaskset(ki.bf,KEY2_M);
			if (ki.bf == KEY2_M){
				event_single_key2();
			}
		}
		//---------------------------------------------------------------------------------------------------------------
		// Button 3 is pressed ( MINUS )
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == KEY3_DOWN)
		{
			bitmaskset(ki.bf,KEY3_M);
			if (ki.bf == KEY3_M){
				event_single_key3();
			}
		}
		//---------------------------------------------------------------------------------------------------------------
		// Button 1 is released ( PLUS )
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == KEY1_UP)
		{
			if (bitchk(ki.bf,KEY1)){
				fastReleaseKey1();
				bitmaskclr(ki.bf,KEY1_M);
			}
		}
		//---------------------------------------------------------------------------------------------------------------
		// Button 2 is released ( MENU )
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == KEY2_UP)
		{
			if (bitchk(ki.bf,KEY2)){
				fastReleaseKey2();
				bitmaskclr(ki.bf,KEY2_M);
			}
		}
		//---------------------------------------------------------------------------------------------------------------
		// Button 3 is released ( MINUS )
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == KEY3_UP)
		{
			if (bitchk(ki.bf,KEY3)){
				fastReleaseKey3();
				bitmaskclr(ki.bf,KEY3_M);
			}
		}
	}
}

void keyevents_counters ( void )
{
	uint8_t t;
	
	//The counters count the time the button is pressed, tick 1/10s.
	for(t=0; t<sizeof(ki.lp_counter); t++){
		if (bitchk(ki.bf,t)){
			if (ki.lp_counter[t]<255) ki.lp_counter[t]++;
			if (ki.kr_counter[t]<255) ki.kr_counter[t]++;
		} else {
			ki.lp_counter[t]=0;
			ki.kr_counter[t]=0;
		}
	}
	if (i.timeSetupCounter){
		i.timeSetupCounter--;
		if (!i.timeSetupCounter){
			i.display_state = SETUP_NO;
		}
	}
	
	longpress_key1();
	longpress_key2();
	longpress_key3();
}

void event_single_key1(void)
{
	switch(i.display_state){
		case SETUP_NO:
			apStart();
			break;
		case SETUP_HOURS:
			iface_flag05sReset();
			++i.hoursSetupValue;
			if ((i.hoursSetupValue&0x0F) > 0x09){i.hoursSetupValue +=0x06;}
			if (i.hoursSetupValue > 0x23) 			{i.hoursSetupValue = 0;}
			break;
		
		case SETUP_MINUTES:
			iface_flag05sReset();
			++i.minutesSetupValue;
			if ((i.minutesSetupValue&0x0F) > 0x09){i.minutesSetupValue +=0x06;}
			if (i.minutesSetupValue > 0x59) 			{i.minutesSetupValue = 0;}
			break;
			
		case SETUP_SECONDS:
			iface_flag05sReset();
			++i.secondsSetupValue;
			if ((i.secondsSetupValue&0x0F) > 0x09){i.secondsSetupValue +=0x06;}
			if (i.secondsSetupValue > 0x59) 			{i.secondsSetupValue = 0;}
			break;
			
		case SETUP_R:
			++i.setupValue;
			displayRset(i.setupValue);
			break;
		
		case SETUP_G:
			++i.setupValue;
			displayGset(i.setupValue);
			break;
		
		case SETUP_B:
			++i.setupValue;
			displayBset(i.setupValue);
			break;
						
		case  SETUP_NIGHT_BR_EN:
		case	SETUP_NIGHT_RGB_EN:
		case  SETUP_ANTIPOISONING_AT_NIGHT_ONLY:
			if (i.setupValue){
				i.setupValue = 0;
			}else{
				i.setupValue = 1;
			}
			break;
		
		case SETUP_BRIGHT:
		case SETUP_NIGHT_BR:
			if(++i.setupValue > 100) {
				i.setupValue = 100;
			}
			displaySetBright(i.setupValue);
			break;
		
		case SETUP_NIGHT_BR_START_H:
		case SETUP_NIGHT_BR_END_H:
			iface_flag05sReset();
			if (++i.setupValue > 23)				{i.setupValue=0;}
			break;
		
		case SETUP_NIGHT_BR_START_M:
		case SETUP_NIGHT_BR_END_M:
			iface_flag05sReset();
			if (++i.setupValue > 59) 				{i.setupValue = 0;}
			break;
		
		default:
			break;
	}
	if (i.display_state!=SETUP_NO){
		i.timeSetupCounter = 60*10;
	}
}


void event_single_key2(void)
{

}

void event_single_key3(void)
{
	switch(i.display_state){
		case SETUP_NO:
		
			break;
		case SETUP_HOURS:
			iface_flag05sReset();
			--i.hoursSetupValue;
			if ((i.hoursSetupValue&0x0f) == 0x0f)	{i.hoursSetupValue -= 0x06;}
			if (i.hoursSetupValue > 0x23) 				{i.hoursSetupValue = 0x23;}
			break;
		
		case SETUP_MINUTES:
			iface_flag05sReset();
			--i.minutesSetupValue;
			if ((i.minutesSetupValue&0x0f) == 0x0f)	{i.minutesSetupValue -=0x06;}
			if (i.minutesSetupValue > 0x59) 				{i.minutesSetupValue = 0x59;}
			break;
			
		case SETUP_SECONDS:
			iface_flag05sReset();
			--i.secondsSetupValue;
			if ((i.secondsSetupValue&0x0f) == 0x0f)	{i.secondsSetupValue -=0x06;}
			if (i.secondsSetupValue > 0x59) 				{i.secondsSetupValue = 0x59;}
			break;
		
		case SETUP_R:
			--i.setupValue;
			displayRset(i.setupValue);
			break;
		
		case SETUP_G:
			--i.setupValue;
			displayGset(i.setupValue);
			break;
		
		case SETUP_B:
			--i.setupValue;
			displayBset(i.setupValue);
			break;
		
		case  SETUP_NIGHT_BR_EN:
		case	SETUP_NIGHT_RGB_EN:
		case  SETUP_ANTIPOISONING_AT_NIGHT_ONLY:
			if (i.setupValue){
				i.setupValue = 0;
			}else{
				i.setupValue = 1;
			}
			break;
			
		case SETUP_BRIGHT:
		case SETUP_NIGHT_BR:
			if (--i.setupValue < 5){
				i.setupValue = 5;
			}
			displaySetBright(i.setupValue);
			break;
		
		case SETUP_NIGHT_BR_START_H:
		case SETUP_NIGHT_BR_END_H:
			iface_flag05sReset();
			if (--i.setupValue > 23){
				i.setupValue = 23;
			}
			break;
		
		case SETUP_NIGHT_BR_START_M:
		case SETUP_NIGHT_BR_END_M:
			iface_flag05sReset();
			if (--i.setupValue > 59){
				i.setupValue = 59;
			}
			break;
			
		default:
			break;
	}
	if (i.display_state!=SETUP_NO){
		i.timeSetupCounter = 60*10;
	}
}

void longpress_key1(void)
{
	if (ki.bf == KEY1_M){
		if (i.display_state == SETUP_NO){
			if (ki.lp_counter[KEY1] >= 2*10){
				bitmaskclr(ki.bf,KEY1_M);
			}
		}else if (i.display_state != SETUP_NIGHT_BR_EN \
					&& i.display_state != SETUP_NIGHT_RGB_EN && i.display_state != SETUP_ANTIPOISONING_AT_NIGHT_ONLY && i.display_state != SETUP_NO){
			if (ki.lp_counter[KEY1] >= 18){
				if(ki.kr_counter[KEY1] >= 1){
					ki.kr_counter[KEY1] = 0;
					event_single_key1 ();
				}
			}else if (ki.lp_counter[KEY1] >= 8){
				if(ki.kr_counter[KEY1] >= 2){
					ki.kr_counter[KEY1] = 0;
					event_single_key1 ();
				}
			}
		}
	}
}

void longpress_key2(void)
{
	if (ki.bf == KEY2_M){
		if (ki.lp_counter[KEY2] >= 2*10){
			if (i.display_state == SETUP_NO){
				i.display_state = SETUP_BRIGHT;
				i.timeSetupCounter = 60*10;
				displayRGBset(0);
				i.setupValue = e.bright;
				displaySetBright(i.setupValue);
			}else{
				i.display_state = SETUP_NO;
			}
			bitmaskclr(ki.bf,KEY2_M);
		}
	}
}

void longpress_key3(void)
{
if (ki.bf == KEY3_M){
		if (i.display_state == SETUP_NO){
			if (ki.lp_counter[KEY3] >= 2*10){
				displaySetBright(100);
				i.display_state = SETUP_R;
				i.setupValue = EEPROM_readByte(R_ADDR);
				displayRset(EEPROM_readByte(R_ADDR));
				displayGset(EEPROM_readByte(G_ADDR));
				displayBset(EEPROM_readByte(B_ADDR));
				i.timeSetupCounter = 60*10;
				bitmaskclr(ki.bf,KEY3_M);
			}
		}else if (i.display_state != SETUP_NIGHT_BR_EN \
					&& i.display_state != SETUP_NIGHT_RGB_EN && i.display_state != SETUP_ANTIPOISONING_AT_NIGHT_ONLY && i.display_state != SETUP_NO){
			if (ki.lp_counter[KEY3] >= 18){
				if(ki.kr_counter[KEY3] >= 1){
					ki.kr_counter[KEY3] = 0;
					event_single_key3 ();
				}
			}else if (ki.lp_counter[KEY3] >= 8){
				if(ki.kr_counter[KEY3] >= 2){
					ki.kr_counter[KEY3] = 0;
					event_single_key3 ();
				}
			}
		}
	}
}

void fastReleaseKey1(void)
{
	if (ki.bf == KEY1_M){
		if (ki.lp_counter[KEY1] < 1*10){
			if (i.display_state == SETUP_NO){
				event_single_key1();
			}
		}
	}
}

void fastReleaseKey2(void)
{
	if (ki.bf == KEY2_M){
		if (ki.lp_counter[KEY2] < 1*10){
			switch (i.display_state){
				case SETUP_NO:
					i.display_state = SETUP_HOURS;
					i.timeSetupCounter = 60*10;
					i.hoursSetupValue = i.hours;
					i.minutesSetupValue = i.minutes;
					i.secondsSetupValue = i.seconds;
					displaySetBright(100);
					break;
				case SETUP_HOURS:
					i.display_state = SETUP_MINUTES;
					break;
					
				case SETUP_MINUTES:
					i.display_state = SETUP_SECONDS;
					break;
					
				case SETUP_SECONDS:
					ds3231_write_time(&i.secondsSetupValue,&i.minutesSetupValue,&i.hoursSetupValue);
					i.display_state = SETUP_NO;
					break;		
					
				case SETUP_R:
					i.display_state++;
					EEPROM_writeByte(R_ADDR,i.setupValue);
					i.setupValue = EEPROM_readByte(G_ADDR);
					break;
				
				case SETUP_G:
					i.display_state++;
					EEPROM_writeByte(G_ADDR,i.setupValue);
					i.setupValue = EEPROM_readByte(B_ADDR);
					break;
				
				case SETUP_B:
					i.display_state = SETUP_NO;
					EEPROM_writeByte(B_ADDR,i.setupValue);
					break;
				
				case SETUP_BRIGHT:
					i.display_state++;
					e.bright = i.setupValue;
					EEPROM_writeByte(BRIGHT_ADDR,e.bright);
					i.setupValue = e.nBright;
					displaySetBright(i.setupValue);
					break;
				
				case SETUP_NIGHT_BR:
					e.nBright = i.setupValue;
					EEPROM_writeByte(NIGHT_BR_ADDR,e.nBright);
					i.display_state++;
					i.setupValue = e.nBrightEn;
					displaySetBright(100);
					break;
				
				case SETUP_NIGHT_BR_EN:
					e.nBrightEn = i.setupValue;
					EEPROM_writeByte(NIGHT_BR_EN_ADDR,e.nBrightEn);
					i.display_state++;
					i.setupValue = e.nBrightStartH;
					break;
				
				case SETUP_NIGHT_BR_START_H:
					e.nBrightStartH = i.setupValue;
					EEPROM_writeByte(NIGHT_BR_START_H_ADDR,e.nBrightStartH);
					i.display_state = SETUP_NIGHT_BR_START_M;
					i.setupValue = e.nBrightStartM;
					break;
				
				case SETUP_NIGHT_BR_START_M:
					e.nBrightStartM = i.setupValue;
					EEPROM_writeByte(NIGHT_BR_START_M_ADDR,e.nBrightStartM);
					i.display_state = SETUP_NIGHT_BR_END_H;
					i.setupValue = e.nBrightEndH;
					break;
				
				case SETUP_NIGHT_BR_END_H:
					e.nBrightEndH = i.setupValue;
					EEPROM_writeByte(NIGHT_BR_STOP_H_ADDR,e.nBrightEndH);
					i.display_state = SETUP_NIGHT_BR_END_M;
					i.setupValue = e.nBrightEndM;
					break;
				
				case SETUP_NIGHT_BR_END_M:
					e.nBrightEndM = i.setupValue;
					EEPROM_writeByte(NIGHT_BR_STOP_M_ADDR,e.nBrightEndM);
					i.display_state = SETUP_NIGHT_RGB_EN;
					i.setupValue = e.rgbAtNightEn;
					break;
				
				case SETUP_NIGHT_RGB_EN:
					e.rgbAtNightEn = i.setupValue;
					EEPROM_writeByte(NIGHT_RGB_EN_ADDR,e.rgbAtNightEn);
					i.display_state++;
					i.setupValue = e.antipoisoningAtNihgtOnly;
					break;
				
				case SETUP_ANTIPOISONING_AT_NIGHT_ONLY:
					e.antipoisoningAtNihgtOnly = i.setupValue;
					EEPROM_writeByte(ANTIPOISONING_AT_NIGHT_ONLY_ADDR,e.antipoisoningAtNihgtOnly);
					i.display_state = SETUP_NO;
					break;
		
				default:
					i.display_state = SETUP_NO;
					break;
			}
		}
	}
}

void fastReleaseKey3(void)
{
	if (ki.bf == KEY3_M){
		if (ki.lp_counter[KEY3] < 1*10){
			if (i.display_state == SETUP_NO){
				RGBtoggle();
			}
		}
	}
}
