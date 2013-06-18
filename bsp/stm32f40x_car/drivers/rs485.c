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
#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>

#include "stm32f4xx.h"
#include "rs485.h"
#include "jt808.h"
#include "camera.h"

#include <finsh.h>


#define RS485_GPIO_TX			GPIO_Pin_2 	// PA2
#define RS485_GPIO_RX			GPIO_Pin_3  // PA3
#define RS485_GPIO_TxC			GPIOA
#define RS485_GPIO_RxD			GPIOA
#define RCC_APBPeriph_USART2 	RCC_APB1Periph_USART2


typedef __packed struct
{
	uint16_t	wr;
	uint8_t		body[UART2_RX_SIZE];
}LENGTH_BUF;




/*串口接收缓存区定义*/
uint8_t	uart2_rxbuf[UART2_RXBUF_SIZE];
uint16_t uart2_rxbuf_wr	= 0, uart2_rxbuf_rd = 0;
uint32_t last_rx_tick	= 0;

uint8_t	uart2_rx[UART2_RX_SIZE];
uint16_t uart2_rx_wr = 0;


//#define UART2_RX_SIZE 1024
//static uint8_t	uart2_rx[UART2_RX_SIZE];
//static uint16_t uart2_rx_wr = 0;


/*声明一个RS485设备*/
static struct rt_device dev_RS485;

/*RS485原始信息数据区定义*/
#define RS485_RAWINFO_SIZE 2048
static uint8_t					RS485_rawinfo[RS485_RAWINFO_SIZE];
static struct rt_messagequeue	mq_RS485;

/*
   RS485接收中断处理
 */
void USART2_IRQHandler( void )
{
	rt_interrupt_enter( );
	if( USART_GetITStatus( USART2, USART_IT_RXNE ) != RESET )
	{
		uart2_rxbuf[uart2_rxbuf_wr++]	= USART_ReceiveData( USART2 );
		uart2_rxbuf_wr					%= UART2_RXBUF_SIZE;
		USART_ClearITPendingBit( USART2, USART_IT_RXNE );
		last_rx_tick = rt_tick_get( );
	}
	rt_interrupt_leave( );
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
static void RS485_baud( int baud )
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate				= baud;
	USART_InitStructure.USART_WordLength			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits				= USART_StopBits_1;
	USART_InitStructure.USART_Parity				= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode					= USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( USART2, &USART_InitStructure );
}

FINSH_FUNCTION_EXPORT( RS485_baud, config gsp_baud );

/*初始化*/
static rt_err_t dev_RS485_init( rt_device_t dev )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;

/*usart2 管脚设置*/

	//RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );

	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin		= RS485_GPIO_TX | RS485_GPIO_RX;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	GPIO_PinAFConfig( GPIOA, GPIO_PinSource2, GPIO_AF_USART2 );
	GPIO_PinAFConfig( GPIOA, GPIO_PinSource3, GPIO_AF_USART2 );
	
	RS485_baud( 57600 );
	USART_Cmd( USART2, ENABLE );
	USART_ITConfig( USART2, USART_IT_RXNE, ENABLE );
	//USART_GetFlagStatus( USART2, USART_FLAG_TC );

/*NVIC 设置*/
	NVIC_InitStructure.NVIC_IRQChannel						= USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init( &NVIC_InitStructure );
/*485收发转换口，5V2电源口初始化*/
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC, ENABLE );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = RS485_RXTX_PIN;
	GPIO_Init( RS485_RXTX_PORT, &GPIO_InitStructure );
	GPIO_ResetBits( RS485_RXTX_PORT, RS485_RXTX_PIN );
	
	GPIO_InitStructure.GPIO_Pin = POWER485CH1_PIN;
	GPIO_Init( POWER485CH1_PORT, &GPIO_InitStructure );
	GPIO_ResetBits( POWER485CH1_PORT, POWER485CH1_PIN );

	RX_485const;
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
static rt_err_t dev_RS485_open( rt_device_t dev, rt_uint16_t oflag )
{	
	RX_485const;
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
static rt_err_t dev_RS485_close( rt_device_t dev )
{
	
	return RT_EOK;
}

/***********************************************************
* Function:RS485_read
* Description:数据模式下读取数据
* Input:void* buff,要读取的数据结构体，改结构体为*LENGTH_BUF,
* Input:rt_size_t count,接收端可以接收的最大长度
* Output:buff
* Return:rt_size_t
* Others:
***********************************************************/
static rt_size_t dev_RS485_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{
	LENGTH_BUF *readBuf;
	readBuf->wr=0;
	readBuf=(LENGTH_BUF *)buff;

	for(readBuf->wr=0;readBuf->wr<count;readBuf->wr++)
		{
		if(uart2_rxbuf_wr != uart2_rxbuf_rd)
			{
			readBuf->body[readBuf->wr]=uart2_rxbuf[uart2_rxbuf_rd++];
			if(uart2_rxbuf_rd>=sizeof(uart2_rxbuf))
				uart2_rxbuf_rd=0;
			}
		else
			{
			break;
			}
		}
	return RT_EOK;
}

/***********************************************************
* Function:		RS485_write
* Description:	数据模式下发送数据，要对数据进行封装
* Input:		const void* buff	要发送的原始数据
       rt_size_t count	要发送数据的长度
* Output:
* Return:
* Others:
***********************************************************/
static rt_size_t dev_RS485_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
{
	rt_size_t	len = count;
	uint8_t		*p	= (uint8_t*)buff;
	TX_485const;
	while( len )
	{
		USART_SendData( USART2, *p++ );
		while( USART_GetFlagStatus( USART2, USART_FLAG_TC ) == RESET )
		{
		}
		len--;
	}
	RX_485const;
	return RT_EOK;
}


/***********************************************************
* Function:		RS485_control
* Description:	控制模块
* Input:		rt_uint8_t cmd	命令类型
    void *arg       参数,依据cmd的不同，传递的数据格式不同
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t dev_RS485_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
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
rt_size_t RS485_write( uint8_t *p, uint8_t len )
{
	return dev_RS485_write( &dev_RS485, 0, p, len );
}


bool RS485_read_char(u8 *c)
{
	if(uart2_rxbuf_wr != uart2_rxbuf_rd)
		{
		*c=uart2_rxbuf[uart2_rxbuf_rd++];
		if(uart2_rxbuf_rd>=sizeof(uart2_rxbuf))
			uart2_rxbuf_rd=0;
		return true;
		}
	return false;

}

FINSH_FUNCTION_EXPORT( RS485_write, write to RS485 );


/* write one character to serial, must not trigger interrupt */
static void usart2_putc( const char c )
{
	USART_SendData( USART2, c );
	while( !( USART2->SR & USART_FLAG_TXE ) )
	{
		;
	}
	USART2->DR = ( c & 0x1FF );
}


/*********************************************************************************
*函数名称:void BkpSram_init(void)
*功能描述:backup sram 初始化
*输	入:	none
*输	出:	none
*返 回 值:void
*作	者:白养民
*创建日期:2013-06-18
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
void BkpSram_init(void)
{
	u16 uwIndex,uwErrorIndex=0;
	
	/* Enable the PWR APB1 Clock Interface */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);
	
    /* Enable BKPSRAM Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
	
	/* Enable the Backup SRAM low power Regulator to retain it's content in VBAT mode */
	PWR_BackupRegulatorCmd(ENABLE);

	/* Wait until the Backup SRAM low power Regulator is ready */
	while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET)
	{
	}
	//rt_kprintf("\r\n BkpSram_init OK!");
}


/*********************************************************************************
*函数名称:u8 BkpSram_write(u32 addr,u8 *data, u16 len)
*功能描述:backup sram 数据写入
*输	入:	addr	:写入的地址
		data	:写入的数据指针
		len		:写入的长度
*输	出:	none
*返 回 值:u8	:	0:表示操作失败，	1:表示操作成功
*作	者:白养民
*创建日期:2013-06-18
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u8 BkpSram_write(u32 addr,u8 *data, u16 len)
{
	u32 i;
	//addr &= 0xFFFC;
	//*(__IO uint32_t *) (BKPSRAM_BASE + addr) = data;
	for(i=0;i<len;i++)
		{
		if(addr<0x1000)
			{
			*(__IO uint8_t *) (BKPSRAM_BASE + addr) = *data++;
			}
		else
			{
			return 0;
			}
		++addr;
		}
	return 1;
}

/*********************************************************************************
*函数名称:u16 bkpSram_read(u32 addr,u8 *data, u16 len)
*功能描述:backup sram 数据读取
*输	入:	addr	:读取的地址
		data	:读取的数据指针
		len		:读取的长度
*输	出:	none
*返 回 值:u16	:表示实际读取的长度
*作	者:白养民
*创建日期:2013-06-18
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u16 bkpSram_read(u32 addr,u8 *data, u16 len)
{
	u32 i;
	//addr &= 0xFFFC;
	//data = *(__IO uint32_t *) (BKPSRAM_BASE + addr);
	for(i=0;i<len;i++)
		{
		if(addr<0x1000)
			{
			*data++ = *(__IO uint8_t *) (BKPSRAM_BASE + addr);
			}
		else
			{
			break;
			}
		++addr;
		}
	return i;
}

void bkpSram_wr(u32 addr,char *psrc)
{
	char pstr[128];
	memset(pstr,0,sizeof(pstr));
	memcpy(pstr,psrc,strlen(psrc));
	bkpSram_write(addr,pstr,strlen(pstr)+1);
}
FINSH_FUNCTION_EXPORT( bkpSram_wr, write from backup sram );


void bkpSram_rd(u32 addr)
{
	char pstr[128];
	bkpSram_read(addr,pstr,sizeof(pstr));
	rt_kprintf("\r\n str=%s\r\n",pstr);
}
FINSH_FUNCTION_EXPORT( bkpSram_rd, read from backup sram );


ALIGN( RT_ALIGN_SIZE )
static char thread_RS485_stack[1024];
struct rt_thread thread_RS485;


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void rt_thread_entry_RS485( void* parameter )
{
	rt_err_t	res;
	BkpSram_init();
	Cam_Device_init();
	while( 1 )
	{
		if(Camera_Process()==0)
		{
			//lcd_process();
		}
		rt_thread_delay( RT_TICK_PER_SECOND / 20 );
	}
}

/*RS485设备初始化*/
void RS485_init( void )
{
	//rt_sem_init( &sem_RS485, "sem_RS485", 0, 0 );
	rt_mq_init( &mq_RS485, "mq_RS485", &RS485_rawinfo[0], 128 - sizeof( void* ), RS485_RAWINFO_SIZE, RT_IPC_FLAG_FIFO );

	
	dev_RS485.type	= RT_Device_Class_Char;
	dev_RS485.init	= dev_RS485_init;
	dev_RS485.open	= dev_RS485_open;
	dev_RS485.close	= dev_RS485_close;
	dev_RS485.read	= dev_RS485_read;
	dev_RS485.write	= dev_RS485_write;
	dev_RS485.control = dev_RS485_control;

	rt_device_register( &dev_RS485, "RS485", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE );
	rt_device_init( &dev_RS485 );

	
	rt_thread_init( &thread_RS485,
						"RS485",
						rt_thread_entry_RS485,
						RT_NULL,
						&thread_RS485_stack[0],
						sizeof( thread_RS485_stack ), 9, 5 );
	rt_thread_startup( &thread_RS485 );
}

/*RS485开关*/
rt_err_t RS485_onoff( uint8_t openflag )
{
	return 0;
}

FINSH_FUNCTION_EXPORT( RS485_onoff, RS485_onoff([1 | 0] ) );
/************************************** The End Of File **************************************/
