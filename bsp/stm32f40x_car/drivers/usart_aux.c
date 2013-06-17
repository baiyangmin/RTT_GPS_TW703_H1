/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// 文件名
 * Author:			// 作者
 * Date:			// 日期
 * Description:		// 模块描述
 * Version:			// 版本信息
 * Function List:	// 主要函数及其功能
 *     1. -------
 * History:			// 历史修改记录
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/


/*同外设通信，当前支持CAN,2xUART，
   采用
   <0x7e><chn><type><info><0x7e>

   不使用线程，定时检查接收的数据即可
 */
#include <rtthread.h>

static struct timer tmr_aux;

#define UART3_RXBUF_SIZE 128
static uint8_t rxbuf[UART3_RXBUF_SIZE];
static uint8_t rxbuf_wr=0;	

struct rt_ringbuffer rb_uart3_rx;


static rt_tick_t last_tick=0;


/**串口3中断服务程序**/
void UART3_IRQHandler( void )
{
	rt_interrupt_enter( );
	if( USART_GetITStatus( USART3, USART_IT_RXNE ) != RESET )
	{
		rt_ringbuffer_putchar(rb_uart3_rx,USART_ReceiveData( UART4 ));
		USART_ClearITPendingBit( UART4, USART_IT_RXNE );
		last_tick = rt_tick_get( );
	}

   if (USART_GetITStatus(UART4, USART_IT_TC) != RESET)
   {
   		USART_ClearITPendingBit(UART4, USART_IT_TC);
   }

	rt_interrupt_leave( );
}

ALIGN( RT_ALIGN_SIZE )
static char thread_aux_stack [256];
struct rt_thread thread_aux;

/*接收数据处理，如何判断结束,超时还是0x7e*/
static void rt_thread_entry_aux( void * parameter )
{
	uint8_t ch;
	static uint8_t rx[256];
	static uint8_t bytestuff=0;
	uint16_t rx_wr=0;
	while(1)
	{
		rt_thread_delay(RT_TICK_PER_SECOND/20);
		if(rt_tick_get()-last_tick<10) continue;
		while(rt_ringbuffer_getchar(rb_uart3_rx,&ch)==1)
		{
			if(ch==0x7e)
			{
				if(rx_wr) break;
				else continue;
			}
			if(ch==0x7d) 
			{
				bytestuff=1;
				continue;
			}
			if(bytestuff)
			{
				ch+=0x7c;
			}
			rx[rx_wr++]=ch;
		}
		
		
	}

}


/*外设接口初始化*/
void usart_aux_init( void )
{
	USART_InitTypeDef	USART_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;

/*初始化串口3*/
	RCC_APB2PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE );
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	GPIO_PinAFConfig( GPIOB, GPIO_PinSource10, GPIO_AF_USART3 );
	GPIO_PinAFConfig( GPIOB, GPIO_PinSource11, GPIO_AF_USART3 );

	NVIC_InitStructure.NVIC_IRQChannel						= USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	USART_InitStructure.USART_BaudRate				= 115200; 
	USART_InitStructure.USART_WordLength			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits				= USART_StopBits_1;
	USART_InitStructure.USART_Parity				= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode					= USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( USART3, &USART_InitStructure );

	/* Enable USART */
	USART_Cmd( USART3, ENABLE );
	USART_ITConfig( USART3, USART_IT_RXNE, ENABLE );

	rt_thread_init( &thread_aux,
	                "uart_aux",
	                rt_thread_entry_aux,
	                RT_NULL,
	                &thread_aux_stack [0],
	                sizeof( thread_aux_stack ), 16, 5 );
	rt_thread_startup( &thread_aux );

}

/************************************** The End Of File **************************************/
