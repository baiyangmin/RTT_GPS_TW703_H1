#ifndef __RTC_H__
#define __RTC_H__

//#include <time.h>
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"


void RTC_TimeShow(void);
void rt_hw_rtc_init(void);
void set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second);
void set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t date);
#include <time.h>

/*time_t time(time_t* t);
 */
#endif
