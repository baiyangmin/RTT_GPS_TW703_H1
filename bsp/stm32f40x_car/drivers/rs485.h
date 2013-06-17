#ifndef _H_RS485
#define _H_RS485

#include "camera.h"

#ifndef false
#define false	0
#endif
#ifndef true
#define true	1
#endif

typedef u8 bool;

#define 	RS485_RXTX_PORT			GPIOC
#define 	RS485_RXTX_PIN			GPIO_Pin_4

#define 	POWER485CH1_PORT		GPIOB
#define 	POWER485CH1_PIN			GPIO_Pin_8


#define  RX_485const         GPIO_ResetBits(RS485_RXTX_PORT,RS485_RXTX_PIN)
#define  TX_485const         GPIO_SetBits(RS485_RXTX_PORT,RS485_RXTX_PIN)  
#define  Power_485CH1_ON     GPIO_SetBits(POWER485CH1_PORT,POWER485CH1_PIN)  // 第一路485的电	       上电工作
#define  Power_485CH1_OFF    GPIO_ResetBits(POWER485CH1_PORT,POWER485CH1_PIN)

/*串口接收缓存区定义*/
#define UART2_RXBUF_SIZE 1024
extern uint8_t	uart2_rxbuf[UART2_RXBUF_SIZE];
extern uint16_t uart2_rxbuf_wr, uart2_rxbuf_rd;
extern uint32_t last_rx_tick;

#define UART2_RX_SIZE 1024
extern uint8_t	uart2_rx[UART2_RX_SIZE];
extern uint16_t uart2_rx_wr;



extern rt_size_t RS485_write( uint8_t *p, uint8_t len );
extern bool RS485_read_char(u8 *c);
extern void RS485_init( void );

#endif
