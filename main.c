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
 todo independed RGB bright for day and night mode.
 todo bright in night mode can be set to 0 and wake up by any key for a wile.
 todo if bright in night mode setted to 0 then RGB bright must set to 0 automatically?
 todo refactor project
 todo move ap.procCounter/AP_CYCLES to the local variable in apProc();
 todo simplify the RGB toggle process.
 
 KEY1 - PLUS BUTTON   - change effects
 KEY2 - MENU BUTTON 	- time setup
 KEY3 - MINUS BUTTON 	- on/off RBG backlight
 
 For the correct operation of the project, the following settings are required when compiling the Release target on Cosmic:

 1. Project Settings -> C Compiler -> Category: Optimization -> check the Disable for Debugging (-no) and the Speed Optimization;
 2. Project Settings -> Linker -> Category: Input -> Change Segment Ram to 0x0ff-0x37f, and add the section .fbss to the Ram segment.
 
 or you can use my Cosmic project.
 
 on Cosmic .hex file is a little smaller than on SDCC
*/

//#define STACK_START 0x03FF
//#define STACK_SIZE  512
//#define STACK_SENTINEL 0xAA

//void initStackSentinel(void) {
//    uint8_t* ptr = (uint8_t*)(STACK_START - STACK_SIZE);
//    while (ptr < (uint8_t*)STACK_START) {
//        *ptr++ = STACK_SENTINEL;
//    }
//}

int main(void) {
	
  DISABLE_INTERRUPTS();
	// switch to 16MHz (default is 2MHz)
  sfr_CLK.CKDIVR.byte = 0x00;
	//initStackSentinel();
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
