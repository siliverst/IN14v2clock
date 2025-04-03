#include "STM8S003F3.h"
#include "display.h"
#include "ds3231.h"
#include "macro.h"
#include "service_timing.h"
#include "keyboard.h"
#include "iface.h"
#include "eeprom.h"
#include "iwdg.h"

/* 
 KEY1 - PLUS BUTTON   - change effects
 KEY2 - MENU BUTTON 	- time setup
 KEY3 - MINUS BUTTON 	- on/off RBG backlight
*/

int main(void) {
	
  DISABLE_INTERRUPTS();
	// switch to 16MHz (default is 2MHz)
  sfr_CLK.CKDIVR.byte = 0x00;
	
	EEPROM_dataInit();
	displayInit();
	ds3231Init();
	keyboard_init();
	iface_init();
	service_timing_init();
	iwdg_init(0xff);	//1.02s period
	iwdg_service();
  ENABLE_INTERRUPTS(); 

	while (1) {
		service_timing_10ms_proc();
		iface_proc(); 
		iwdg_service();
	}
}
