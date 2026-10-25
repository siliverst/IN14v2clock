#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#ifdef	__cplusplus
extern "C" {
#endif

// key events: S1 "+", S2 "M", S3 "-", the release is the press | 0x80
#define S1_DOWN           0x01
#define S2_DOWN           0x02
#define S3_DOWN           0x03

#define S1_UP             0x81
#define S2_UP             0x82
#define S3_UP             0x83

typedef struct
{
	uint8_t S1_reg;
	uint8_t S2_reg;
	uint8_t S3_reg;
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
