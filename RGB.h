#ifndef _RGB_H_
#define	_RGB_H_

#ifdef	__cplusplus
extern "C" {
#endif

void RGBinit ( void );

void RGBsetR ( uint16_t value );
void RGBsetG ( uint16_t value );
void RGBsetB ( uint16_t value );
void RGBgammaCalculate (uint16_t outMax);
uint16_t RGBgammaGet (uint8_t in);


#ifdef	__cplusplus
}
#endif

#endif	/* _RGB_H_ */