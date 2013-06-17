/*
串口网关，类似一个软串口
finsh和rt_printf的重定向
*/
#include <rtthread.h>
#include <rtdevice.h>
#include "serial.h"
#include <stm32f4xx_usart.h>
#include <finsh.h>


#define UART1_GPIO_TX		GPIO_Pin_9
#define UART1_TX_PIN_SOURCE GPIO_PinSource9
#define UART1_GPIO_RX		GPIO_Pin_10
#define UART1_RX_PIN_SOURCE GPIO_PinSource10
#define UART1_GPIO			GPIOA
#define UART1_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define RCC_APBPeriph_UART1	RCC_APB2Periph_USART1

#define VUART_RXBUF_SIZE	1024
static uint8_t vuart_rxbuf[VUART_RXBUF_SIZE];

static struct rt_ringbuffer rb_vuart;
struct rt_device dev_vuart;


#define BD_SYNC_40	0
#define BD_SYNC_0D	1
#define BD_SYNC_0A	2
static uint8_t bd_packet_status=BD_SYNC_40; /*北斗升级报文接收状态*/


extern uint8_t flag_bd_upgrade_uart;
void USART1_IRQHandler(void)
{
	rt_uint8_t ch;
    rt_interrupt_enter();

    if( USART_GetITStatus( USART1, USART_IT_RXNE ) != RESET )
	{
		ch = USART_ReceiveData( USART1 );
		USART_ClearITPendingBit( USART1, USART_IT_RXNE );
		//rt_ringbuffer_putchar(&rb_vuart,ch);
		if(flag_bd_upgrade_uart==0)
		{
			rt_ringbuffer_putchar(&rb_vuart,ch);	
			//if (dev_vuart.rx_indicate != RT_NULL)  /*默认finsh使用*/
			{
				dev_vuart.rx_indicate(&dev_vuart,1);
			}
		}
#if 1		
		else	/*接收升级数据，由于串口波特率可变,需要同步到正确的数据头*/
		{
			switch(bd_packet_status)
			{
				case BD_SYNC_40:
					if(ch==0x40) 
					{
						rt_ringbuffer_init(&rb_vuart,vuart_rxbuf,VUART_RXBUF_SIZE);
						rt_ringbuffer_putchar(&rb_vuart,ch);
						bd_packet_status=BD_SYNC_0D;
					}
					break;
				case BD_SYNC_0D:
					if(ch==0x0D) bd_packet_status=BD_SYNC_0A;
					rt_ringbuffer_putchar(&rb_vuart,ch);
					break;
				case BD_SYNC_0A:
					if(ch==0x0A) bd_packet_status=BD_SYNC_40;
					else bd_packet_status=BD_SYNC_0D;
					rt_ringbuffer_putchar(&rb_vuart,ch);
					break;
			}

		}
#endif

	
    }	

    rt_interrupt_leave();

}

void uart1_baud(int buad)
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = buad;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
}
FINSH_FUNCTION_EXPORT( uart1_baud, uart1 baud );


/*初始化串口1*/
static rt_err_t dev_vuart_init( rt_device_t dev )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	USART_InitTypeDef	USART_InitStructure;

	RCC_AHB1PeriphClockCmd(UART1_GPIO_RCC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APBPeriph_UART1, ENABLE);

/*uart1 管脚设置*/
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX | UART1_GPIO_TX;
	GPIO_Init(UART1_GPIO, &GPIO_InitStructure);
    GPIO_PinAFConfig(UART1_GPIO, UART1_TX_PIN_SOURCE, GPIO_AF_USART1);
    GPIO_PinAFConfig(UART1_GPIO, UART1_RX_PIN_SOURCE, GPIO_AF_USART1);

/*NVIC 设置*/
	NVIC_InitStructure.NVIC_IRQChannel						= USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	uart1_baud(115200);

	USART_Cmd( USART1, ENABLE );
	USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );

	return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t dev_vuart_open( rt_device_t dev, rt_uint16_t oflag )
{
	return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t dev_vuart_close( rt_device_t dev )
{
	
	return RT_EOK;
}

/***********************************************************
* Function:gps_read
* Description:数据模式下读取数据
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_size_t dev_vuart_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{
	if(count>1)
		return rt_ringbuffer_get(&rb_vuart,buff,count);
	else
		return rt_ringbuffer_getchar(&rb_vuart,buff);

}

/*
static void uart1_putc( const char c )
{
	USART_SendData( USART1, c );
	while( !( USART1->SR & USART_FLAG_TXE ) )
	{
		;
	}
	USART1->DR = ( c & 0x1FF );
}
*/

/***********************************************************
* Function:		gps_write
* Description:	数据模式下发送数据，要对数据进行封装
* Input:		const void* buff	要发送的原始数据
       rt_size_t count	要发送数据的长度
       rt_off_t pos		使用的socket编号
* Output:
* Return:
* Others:
***********************************************************/

static rt_size_t dev_vuart_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
{
	rt_size_t	size = count;
	uint8_t		*ptr	= (uint8_t*)buff;

	if (dev_vuart.flag & RT_DEVICE_FLAG_STREAM)
		{
			while (size)
			{
				if (*ptr == '\n')
				{
					while (!(USART1->SR & USART_FLAG_TXE));
					USART1->DR = '\r';
				}

				while (!(USART1->SR & USART_FLAG_TXE));
				USART1->DR = (*ptr & 0x1FF);

				++ptr; --size;
			}
		}
		else
		{
			/* write data directly */
			while (size)
			{
				while (!(USART1->SR & USART_FLAG_TXE));
				USART1->DR = (*ptr & 0x1FF);

				++ptr; --size;
			}
		}
	return RT_EOK;
}

/***********************************************************
* Function:		gps_control
* Description:	控制模块
* Input:		rt_uint8_t cmd	命令类型
    void *arg       参数,依据cmd的不同，传递的数据格式不同
* Output:
* Return:
* Others:
***********************************************************/
#define RT_DEVICE_CTRL_BAUD	(RT_DEVICE_CTRL_SUSPEND+1)
static rt_err_t dev_vuart_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
	int i=*(int *)arg;

	RT_ASSERT(dev != RT_NULL);
	switch (cmd)
	{
	case RT_DEVICE_CTRL_SUSPEND:
		/* suspend device */
		dev_vuart.flag |= RT_DEVICE_FLAG_SUSPENDED;
		USART_Cmd(USART1, DISABLE);
		break;

	case RT_DEVICE_CTRL_RESUME:
		/* resume device */
		dev_vuart.flag &= ~RT_DEVICE_FLAG_SUSPENDED;
		USART_Cmd(USART1, ENABLE);
		break;
	case RT_DEVICE_CTRL_BAUD:
		uart1_baud(i);
		break;
	}

	return RT_EOK;

}


void rt_hw_usart_init()
{
/*初始化接收缓冲区*/
	rt_ringbuffer_init(&rb_vuart,vuart_rxbuf,VUART_RXBUF_SIZE);
	
	dev_vuart.type	= RT_Device_Class_Char;
	dev_vuart.init	= dev_vuart_init;
	dev_vuart.open	= dev_vuart_open;
	dev_vuart.close	= dev_vuart_close;
	dev_vuart.read	= dev_vuart_read;
	dev_vuart.write	= dev_vuart_write;
	dev_vuart.control = dev_vuart_control;
	dev_vuart.rx_indicate = RT_NULL;
	dev_vuart.tx_complete = RT_NULL;

	rt_device_register( &dev_vuart, "vuart", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM );
	rt_device_init( &dev_vuart );
}


