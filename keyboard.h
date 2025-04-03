#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#ifdef	__cplusplus
extern "C" {
#endif

#define KEY1_DOWN           0x01
#define KEY2_DOWN           0x02
#define KEY3_DOWN           0x03

#define KEY1_UP             0x81
#define KEY2_UP             0x82
#define KEY3_UP             0x83

typedef struct
{
	uint8_t KEY1_reg;
	uint8_t KEY2_reg;
	uint8_t KEY3_reg;
	uint8_t Refresh;
	uint8_t HeadPtr,TailPtr;
	uint8_t Buffer[8];
}Keyboard; 

void keyboard_init(void);
void keyboard_refresh(void);
unsigned char keyboard_get_key(void);


#ifdef	__cplusplus
}
#endif

#endif	/* _KEYBOARD_H_ */
