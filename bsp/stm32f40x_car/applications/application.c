/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <stdio.h>

#include "stm32f4xx.h"
#include <board.h>
#include <rtthread.h>

/*
应该在此处初始化必要的设备和事件集
*/
void rt_init_thread_entry(void* parameter)
{
	mma8451_driver_init();
	printer_driver_init();  
  	usbh_init();
  	spi_sd_init();
	

#if 0	

  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
  
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC, ENABLE );
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE );
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART5, ENABLE );
  
  GPIO_InitStructure.GPIO_Mode	  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd	  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init( GPIOD, &GPIO_InitStructure );
  GPIO_ResetBits( GPIOD, GPIO_Pin_10 );


  while(1)
  {
  	GPIO_ResetBits( GPIOD, GPIO_Pin_10 ); 
    rt_thread_delay(RT_TICK_PER_SECOND*5);
	GPIO_SetBits( GPIOD, GPIO_Pin_10 ); 
    rt_thread_delay(RT_TICK_PER_SECOND*5);
  }
#endif

}



int rt_application_init()
{
    rt_thread_t tid;


    tid = rt_thread_create("init",
                            rt_init_thread_entry, RT_NULL,
                            2048, RT_THREAD_PRIORITY_MAX-2, 20);


    if (tid != RT_NULL)  rt_thread_startup(tid);
    return 0;
}

/*@}*/
