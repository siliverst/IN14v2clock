#ifndef _IWDG_H_
#define _IWDG_H_

// includes
#include <stdint.h>
#include "STM8S003F3.h"

/// start IWDG watchdog and set period*4ms
void iwdg_init(uint8_t period);

/// set IWDG timeout period
void iwdg_set_period(uint8_t prescaler, uint8_t reload);


INLINE void iwdg_service(void) {

  // trigger service of IWDG
  sfr_IWDG.KR.byte = (uint8_t) 0xAA;
    
} // iwdg_service

#endif  // _IWDG_H_
