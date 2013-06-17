#ifndef LCDDis_H_
#define LCDDis_H_

#include "sed1520.h"
#include <stdio.h>


#define KEY_MENU_PORT	GPIOC
#define KEY_MENU_PIN	GPIO_Pin_8

#define KEY_OK_PORT		GPIOA
#define KEY_OK_PIN		GPIO_Pin_8

#define KEY_UP_PORT		GPIOC
#define KEY_UP_PIN		GPIO_Pin_9

#define KEY_DOWN_PORT	GPIOD
#define KEY_DOWN_PIN	GPIO_Pin_3



extern void KeyCheckFun(void);
extern void Init_lcdkey(void);


#endif
