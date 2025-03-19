#include "iwdg.h"

void iwdg_init(uint8_t period) {

  // start IDWG (must be the first value written to this register, see UM)
  sfr_IWDG.KR.byte  = (uint8_t) 0xCC;
  // 0x06 -> 0,004s x period
  iwdg_set_period(0x06, period);
    
} // iwdg_init


/**
  \fn void iwdg_set_period(uint8_t prescaler, uint8_t reload)
   
  \brief change IWDG timeout period
  
  \param[in]  prescaler   IWDG clock prescaler (freq=64kHz/2^(PR+2))
  \param[in]  reload      IWDG reload value
   
  set independent watchdog (IWDG) timeout period
*/
void iwdg_set_period(uint8_t prescaler, uint8_t reload) {

  // unlock write access to prescaler and reload registers
  sfr_IWDG.KR.byte  = (uint8_t) 0x55;
  
  // set clock prescaler (freq=64kHz/2^(PR+2))
  sfr_IWDG.PR.byte  = prescaler;
  
  // set reload counter
  sfr_IWDG.RLR.byte = reload;
  
  // re-lock IWDG registers and start new period
  sfr_IWDG.KR.byte  = (uint8_t) 0xAA;
    
} // iwdg_set_period

