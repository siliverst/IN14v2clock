#include <stdint.h>
#include "keyevents.h"
#include "macro.h"
#include "keyboard.h"
#include "display.h"
#include "iface.h"
#include "ds3231.h"
#include "eeprom.h"
#include "rgb.h"
#include "effects.h"

/*
Key events (keyboard.c gives the debounced press and release events). Keys: S1 "+", S2 "M", S3 "-".
Time display:   "+" click - antipoisoning effect, hold 2s - time setup;
                "M" click - smooth change on/off, hold 2s - settings menu;
                "-" click - backlight on/off, hold 2s - backlight color setup.
Setup modes:    "+" / "-" - change the value (by the press, key repeat by the hold);
                "M" - time and color setup: next step by the press, after the last one save and exit;
                      menu: click (release) - save and next item, hold 1s - save and exit.
keyPress()  - the press of a single key, keyClick() - the release of a short press (< 1s),
keyHold()   - the hold actions and the key repeat, every 100ms.
A key bit is cleared in ki.bf to ignore its release after a hold action.
*/

static void keyPress(uint8_t k);
static void keyClick(uint8_t k);
static void keyHold(uint8_t k);
static void fxSelect(uint8_t kind, uint8_t addr, uint8_t max);


Keyevents ki;

void keyevents_proc ( void )
{
	uint8_t k, m;

	ki.keys_mirror = keyboard_get_key();
	if (ki.keys_mirror == 0xFF) {return;}

	k = (ki.keys_mirror & 0x7F) - 1;				// S1_DOWN -> S1 ...
	m = (uint8_t)(1 << k);
	if (ki.keys_mirror & 0x80){
		// release
		if (ki.bf & m){
			if ((ki.bf == m) && (ki.lp_counter[k] < 1*10)) {keyClick(k);}
			ki.bf &= (uint8_t)~m;
		}
	}else{
		// press
		ki.bf |= m;
		if (ki.bf == m) {keyPress(k);}
	}
}

void keyevents_counters ( void )
{
	uint8_t t;

	//The counters count the time the button is pressed, tick 1/10s.
	for(t=0; t<KEYS_COUNT; t++){
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
			iface_exit_setup();
		}
	}

	// the hold actions only when one key is pressed
	for(t=0; t<KEYS_COUNT; t++){
		if (ki.bf == (uint8_t)(1 << t)) {keyHold(t);}
	}
}

/*
"+" / "M" in the time display: the first click shows the number of the current effect, the clicks during it select the next one
*/
static void fxSelect(uint8_t kind, uint8_t addr, uint8_t max)
{
	if (fxHolding(kind)){
		EEPROM_set(addr, (EE(addr) < max) ? EE(addr) + 1 : 0);
	}
	fxDemo(kind);
}

// 0..max in a circle
static uint8_t wrapStep(uint8_t v, uint8_t up, uint8_t max)
{
	if (up) {return (v >= max) ? 0 : v + 1;}
	return v ? v - 1 : max;
}

static uint8_t bcdStep(uint8_t v, uint8_t up, uint8_t max)
{
	v = wrapStep(bcd_to_decimal(v), up, max);
	return (uint8_t)((v / 10) << 4) | (v % 10);
}

// "+" (up = 1) or "-" (up = 0) in the setup modes
static void valueStep(uint8_t up)
{
	uint8_t s = i.display_state;
	uint8_t v;

	switch(s){
		case SETUP_HOURS:
			i.hoursSetupValue = bcdStep(i.hoursSetupValue, up, 23);
			break;

		case SETUP_MINUTES:
			i.minutesSetupValue = bcdStep(i.minutesSetupValue, up, 59);
			break;

		case SETUP_SECONDS:
			i.secondsSetupValue = bcdStep(i.secondsSetupValue, up, 59);
			break;

		case SETUP_R:
		case SETUP_G:
		case SETUP_B:
			// the color is changed in e only, it is saved by "M" or reloaded from EEPROM by timeout
			EE(R_ADDR + s - SETUP_R) += up ? 1 : 255;
			RGBset(100);
			break;

		case SETUP_BRIGHT:
		case SETUP_NIGHT_BR:
		case SETUP_RGB_BR:
		case SETUP_RGB_NIGHT_BR:
			// no circle: digits 5..100 day, 0, 5..100 night, backlight 0..100
			v = EE(s);
			if (up) {if (v < BRIGHT_MAX) v++;}
			else {if (v) v--;}
			if ((s <= SETUP_NIGHT_BR) && (v < BRIGHT_MIN)){
				v = ((s == SETUP_NIGHT_BR) && !up) ? 0 : BRIGHT_MIN;
			}
			EE(s) = v;
			iface_menu_preview();
			break;

		default:
			// other settings menu items: 0..max in a circle
			if (s <= SETUP_MENU_LAST){
				EE(s) = wrapStep(EE(s), up, eeMax[s]);
			}
			break;
	}
	iface_flag05sReset();								// the edited part is visible while changing
	i.timeSetupCounter = 60*10;
}

static void keyPress(uint8_t k)
{
	uint8_t s = i.display_state;

	if (s == SETUP_NO) {return;}						// the time display acts by the click or the hold
	if (k != S2){
		valueStep(k == S1);
		return;
	}
	// "M" in the time and color setup: next step, the menu acts by the release
	switch (s){
		case SETUP_HOURS:
		case SETUP_MINUTES:
			i.display_state++;
			break;

		case SETUP_SECONDS:
			ds3231_write_time(&i.secondsSetupValue,&i.minutesSetupValue,&i.hoursSetupValue);
			i.seconds = i.secondsSetupValue;
			i.minutes = i.minutesSetupValue;
			i.hours = i.hoursSetupValue;
			iface_exit_setup();
			break;

		case SETUP_R:
		case SETUP_G:
			EEPROM_set(R_ADDR + s - SETUP_R, EE(R_ADDR + s - SETUP_R));
			i.display_state++;
			break;

		case SETUP_B:
			EEPROM_set(B_ADDR, e.b);
			iface_exit_setup();
			break;

		default:
			return;
	}
	i.timeSetupCounter = 60*10;
	bitmaskclr(ki.bf,S2_M);								// its release is not a click
}

static void keyClick(uint8_t k)
{
	if (i.display_state == SETUP_NO){
		if (k == S1) {fxSelect(FX_AP_DEMO, AP_TYPE_ADDR, AP_TYPE_MAX);}
		else if (k == S2) {fxSelect(FX_FLIP_DEMO, FLIP_ADDR, FLIP_MAX);}
		else {RGBtoggle();}
	}else if ((k == S2) && (i.display_state <= SETUP_MENU_LAST)){
		iface_menu_next();
	}
}

static void keyHold(uint8_t k)
{
	uint8_t s = i.display_state;
	uint8_t lp = ki.lp_counter[k];

	if (s == SETUP_NO){
		if (lp < 2*10) {return;}
		i.timeSetupCounter = 60*10;
		if (k == S1){
			i.display_state = SETUP_HOURS;
			i.hoursSetupValue = i.hours;
			i.minutesSetupValue = i.minutes;
			i.secondsSetupValue = i.seconds;
			iface_show(100, IFACE_RGB_KEEP);
		}else if (k == S2){
			i.display_state = SETUP_ZERO;
			iface_menu_preview();
		}else{
			i.display_state = SETUP_R;
			iface_show(100, 100);
		}
		bitmaskclr(ki.bf,(uint8_t)(1 << k));
	}else if (k == S2){
		// settings menu: hold "M" for 1s - save the item and exit
		if ((s <= SETUP_MENU_LAST) && (lp >= 1*10)){
			EEPROM_set(s, EE(s));
			iface_exit_setup();
			bitmaskclr(ki.bf,S2_M);
		}
	}else if ((s > SETUP_MENU_LAST) || (eeMax[s] > 9)){
		// key repeat for values, not for on/off items: 5 per second after 0.8s, 10 per second after 1.8s
		if (ki.kr_counter[k] >= ((lp >= 18) ? 1 : (lp >= 8) ? 2 : 255)){
			ki.kr_counter[k] = 0;
			valueStep(k == S1);
		}
	}
}
