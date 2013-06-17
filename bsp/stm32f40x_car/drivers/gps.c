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
#include "gps.h"
#include "jt808.h"

#include <finsh.h>

#define GPS_PWR_PORT	GPIOD
#define GPS_PWR_PIN		GPIO_Pin_10

#define GPS_GPIO_TX			GPIO_Pin_12 // PC12
#define GPS_GPIO_RX			GPIO_Pin_2  // PD2
#define GPS_GPIO_TxC		GPIOC
#define GPS_GPIO_RxD		GPIOD
#define RCC_APBPeriph_UART5 RCC_APB1Periph_UART5

/*声明一个gps设备*/
static struct rt_device dev_gps;
static uint32_t			gps_out_mode = GPS_OUTMODE_ALL;

/*串口接收缓存区定义*/
#define UART5_RX_SIZE 256

typedef __packed struct
{
	uint16_t	wr;
	uint8_t		body[UART5_RX_SIZE];
}LENGTH_BUF;

static LENGTH_BUF uart5_rxbuf;

//static uint8_t	uart5_rxbuf[UART5_RX_SIZE];	/*预留前两个字节，保存长度*/
//static uint16_t uart5_rxbuf_wr = 2;

/*gps原始信息数据区定义*/
#define GPS_RAWINFO_SIZE 2048
static uint8_t					gps_rawinfo[GPS_RAWINFO_SIZE];
static struct rt_messagequeue	mq_gps;

uint8_t							flag_bd_upgrade_uart = 0;

extern struct rt_device			dev_vuart;

static rt_uint8_t				*ptr_mem_packet = RT_NULL;

//*****************************************
//CRC16 高字节表
//*****************************************

static unsigned char CRC16TabH[256] =
{
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

//*****************************************
//CRC16 低字节表
//*****************************************
static unsigned char CRC16TabL[256] =
{
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,
	0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
	0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
	0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
	0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
	0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
	0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
	0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
	0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,
	0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
	0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
	0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
	0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
	0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,
	0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
	0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
	0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
	0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
	0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,
	0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
	0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
	0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
	0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
	0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,
	0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,
	0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};
//*************************************************
//  计算CRC校验和
// unsigned char* src –  待校验数据
// int startpoint –  校验数据偏移
// int len –  校验数据长度
//************************************************
unsigned short  CalcCRC16( unsigned char*  src, int startpoint, int len )
{
	unsigned short	res		= 0;
	unsigned char	crc_h	= 0;
	unsigned char	crc_l	= 0;

	unsigned char	tmp;
	int				i;

	for( i = startpoint; i < ( len + startpoint ); i++ )
	{
		tmp		= crc_h ^ src[i];
		crc_h	= crc_l ^ CRC16TabH[tmp];
		crc_l	= CRC16TabL[tmp];
		//if(len>1000) rt_kprintf("\r\nsrc=%02x tmp=%02x crc_h=%02x crc_l=%02x",src[i],tmp,crc_h,crc_l);
	}
	//res = ( (unsigned short)crc_h ) << 8 + ( unsigned short)crc_l;
	res = ( crc_h << 8 ) | crc_l;

	return res;
}

/*
   gps接收中断处理，收到\n认为收到一包
   收到一包调用处理函数
 */

void UART5_IRQHandler( void )
{
	static uint8_t	last_ch = 0;
	uint8_t			ch;
	rt_interrupt_enter( );
	if( USART_GetITStatus( UART5, USART_IT_RXNE ) != RESET )
	{
		ch = USART_ReceiveData( UART5 );

		uart5_rxbuf.body[uart5_rxbuf.wr++]	= ch;
		uart5_rxbuf.wr						&= ( UART5_RX_SIZE - 1 );
		uart5_rxbuf.body[uart5_rxbuf.wr]	= 0;

		if( ( ch == 0x0a ) && ( last_ch == 0x0d ) ) /*遇到0d 0a 表明结束*/
		{
			if( uart5_rxbuf.wr < 124 )
			{
				rt_mq_send( &mq_gps, (void*)&uart5_rxbuf, uart5_rxbuf.wr + 2 );
			}
			uart5_rxbuf.wr = 0;
		}
		last_ch = ch;
		USART_ClearITPendingBit( UART5, USART_IT_RXNE );
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
void gps_baud( int baud )
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate				= baud;
	USART_InitStructure.USART_WordLength			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits				= USART_StopBits_1;
	USART_InitStructure.USART_Parity				= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode					= USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( UART5, &USART_InitStructure );
}

FINSH_FUNCTION_EXPORT( gps_baud, config gsp_baud );

/*初始化*/
static rt_err_t dev_gps_init( rt_device_t dev )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC, ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART5, ENABLE );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GPS_PWR_PIN;
	GPIO_Init( GPS_PWR_PORT, &GPIO_InitStructure );
	GPIO_ResetBits( GPS_PWR_PORT, GPS_PWR_PIN );

/*uart5 管脚设置*/

	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_12;
	GPIO_Init( GPIOC, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init( GPIOD, &GPIO_InitStructure );

	GPIO_PinAFConfig( GPIOC, GPIO_PinSource12, GPIO_AF_UART5 );
	GPIO_PinAFConfig( GPIOD, GPIO_PinSource2, GPIO_AF_UART5 );

/*NVIC 设置*/
	NVIC_InitStructure.NVIC_IRQChannel						= UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	gps_baud( 9600 );
	USART_Cmd( UART5, ENABLE );
	USART_ITConfig( UART5, USART_IT_RXNE, ENABLE );

	GPIO_SetBits( GPS_PWR_PORT, GPS_PWR_PIN );

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
static rt_err_t dev_gps_open( rt_device_t dev, rt_uint16_t oflag )
{
	GPIO_SetBits( GPS_PWR_PORT, GPS_PWR_PIN );
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
static rt_err_t dev_gps_close( rt_device_t dev )
{
	GPIO_ResetBits( GPS_PWR_PORT, GPS_PWR_PIN );
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
static rt_size_t dev_gps_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{
	return RT_EOK;
}

/* write one character to serial, must not trigger interrupt */
static void uart5_putc( const char c )
{
	USART_SendData( UART5, c );
	while( !( UART5->SR & USART_FLAG_TXE ) )
	{
		;
	}
	UART5->DR = ( c & 0x1FF );
}

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

static rt_size_t dev_gps_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
{
	rt_size_t	len = count;
	uint8_t		*p	= (uint8_t*)buff;

	while( len )
	{
		USART_SendData( UART5, *p++ );
		while( USART_GetFlagStatus( UART5, USART_FLAG_TC ) == RESET )
		{
		}
		len--;
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
static rt_err_t dev_gps_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
	int i = *(int*)arg;
	switch( cmd )
	{
		case CTL_GPS_OUTMODE:
			break;
		case CTL_GPS_BAUD:
			gps_baud( i );
	}
	return RT_EOK;
}

ALIGN( RT_ALIGN_SIZE )
static char thread_gps_stack[1024];
struct rt_thread thread_gps;


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void rt_thread_entry_gps( void* parameter )
{
	rt_err_t	res;
	LENGTH_BUF	buf;
	while( 1 )
	{
		res = rt_mq_recv( &mq_gps, (void*)&buf, 124, RT_TICK_PER_SECOND / 20 ); //等待100ms,实际上就是变长的延时,最长100ms
		if( res == RT_EOK )                                                     //收到一包数据
		{
			if( flag_bd_upgrade_uart == 0 )
			{
				gps_rx( buf.body, buf.wr );
			}else
			{
				if( buf.body[0] == 0x40 )
				{
					rt_device_write( &dev_vuart, 0, buf.body, buf.wr );
				}
			}
		}
		rt_thread_delay( RT_TICK_PER_SECOND / 20 );
	}
}

/*gps设备初始化*/
void gps_init( void )
{
	rt_mq_init( &mq_gps, "mq_gps", &gps_rawinfo[0], 128 - sizeof( void* ), GPS_RAWINFO_SIZE, RT_IPC_FLAG_FIFO );

	rt_thread_init( &thread_gps,
	                "gps",
	                rt_thread_entry_gps,
	                RT_NULL,
	                &thread_gps_stack[0],
	                sizeof( thread_gps_stack ), 11, 5 );
	rt_thread_startup( &thread_gps );

	dev_gps.type	= RT_Device_Class_Char;
	dev_gps.init	= dev_gps_init;
	dev_gps.open	= dev_gps_open;
	dev_gps.close	= dev_gps_close;
	dev_gps.read	= dev_gps_read;
	dev_gps.write	= dev_gps_write;
	dev_gps.control = dev_gps_control;

	rt_device_register( &dev_gps, "gps", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE );
	rt_device_init( &dev_gps );
}

/*gps开关*/
rt_err_t gps_onoff( uint8_t openflag )
{
	if( openflag == 0 )
	{
		GPIO_ResetBits( GPIOD, GPIO_Pin_10 );
	} else
	{
		GPIO_SetBits( GPIOD, GPIO_Pin_10 );
	}
	return 0;
}

FINSH_FUNCTION_EXPORT( gps_onoff, gps_onoff([1 | 0] ) );

/*线程退出的cleanup函数*/
static void cleanup( struct rt_thread *tid )
{
	if( ptr_mem_packet != RT_NULL )
	{
		rt_free( ptr_mem_packet );
	}
}

/*升级时的调试输出,当前已无法打印输出,串口被占用*/
static void msg_uart_usb( int res )
{
	//rt_kprintf("bd>%d\r\n",res);
}

/*更新北斗线程，接收调试串口来的数据，透传到gps串口中*/

void thread_gps_upgrade_uart( void* parameter )
{
#define BD_SYNC_40	0
#define BD_SYNC_0D	1
#define BD_SYNC_0A	2

/*定义一个函数指针，用作结果处理	*/

	void			( *msg )( void *p );
	unsigned int	resultcode;

	rt_uint8_t		buf[256];
	rt_uint8_t		info[32];
	rt_uint8_t		*p;
	rt_uint16_t		count = 0;
	rt_uint16_t		i;
	rt_size_t		len;
	rt_uint32_t		baud				= 9600;
	rt_uint16_t		packetnum			= 0;
	rt_uint8_t		bd_packet_status	= BD_SYNC_40;   /*北斗升级报文接收状态*/
	rt_uint8_t		last_char			= 0x0;

	rt_tick_t		last_sendtick = 0;                  /*北斗更新时记录收到应答的时刻*/

	msg = parameter;

	ptr_mem_packet = rt_malloc( 1200 );
	if( ptr_mem_packet == RT_NULL )
	{
		resultcode = BDUPG_RES_RAM;
		msg( "E内存不足" );
		return;
	}
	flag_bd_upgrade_uart = 1;

	dev_vuart.flag &= ~RT_DEVICE_FLAG_STREAM;
	rt_device_control( &dev_vuart, 0x03, &baud );
	p = ptr_mem_packet;

	while( 1 )
	{
		if( ( last_sendtick > 0 ) && ( rt_tick_get( ) - last_sendtick > RT_TICK_PER_SECOND * 12 ) )
		{
			/*升级程序发送数据，收到应答，再次发送数据。超时10s*/
			resultcode = BDUPG_RES_TIMEOUT;
			msg( "E超时错误" );
			goto end_upgrade_uart_memfree;
		}
		while( ( len = rt_device_read( &dev_vuart, 0, buf, 256 ) ) > 0 )
		{
			for( i = 0; i < len; i++ )
			{
				switch( bd_packet_status )
				{
					case BD_SYNC_40:
						if( buf[i] == 0x40 )
						{
							*p++				= 0x40;
							bd_packet_status	= BD_SYNC_0A;
							count				= 1;
						}
						break;
					case BD_SYNC_0A:
						if( ( buf[i] == 0x0a ) && ( last_char == 0x0d ) )
						{
							*p = 0x0a;
							count++;
							dev_gps_write( &dev_gps, 0, ptr_mem_packet, count );
							packetnum++;                                            /*显示传递的包数*/
							sprintf( info, "I发送第%d包", packetnum );
							msg( info );
							last_sendtick = rt_tick_get( );
							if( memcmp( ptr_mem_packet, "\x40\x41\xc0", 3 ) == 0 )  /*修改波特率*/
							{
								baud = ( *( ptr_mem_packet + 4 ) << 24 ) | ( *( ptr_mem_packet + 5 ) << 16 ) | ( *( ptr_mem_packet + 6 ) << 8 ) | *( ptr_mem_packet + 7 );
								gps_baud( baud );
								uart1_baud( baud );
							}
							if( memcmp( ptr_mem_packet, "\x40\x34\xc0", 3 ) == 0 )  /*模块软件复位*/
							{
								resultcode = 0;
								msg( "E更新完成" );                                 /*通知lcd显示完成*/
								goto end_upgrade_uart_memfree;
							}
							p					= ptr_mem_packet;
							bd_packet_status	= BD_SYNC_40;
						}else
						{
							*p++ = buf[i];
							count++;
						}
						break;
				}
				last_char = buf[i];
			}
		}
		rt_thread_delay( RT_TICK_PER_SECOND / 50 );
	}

end_upgrade_uart_memfree:
	rt_free( ptr_mem_packet );
	ptr_mem_packet = RT_NULL;
//end_upgrade_uart:
	baud = 115200;
	uart1_baud( baud );
	flag_bd_upgrade_uart = 0;
}

/*更新北斗线程，接收调试串口来的数据，透传到gps串口中*/
void thread_gps_upgrade_udisk( void* parameter )
{
#define READ_PACKET_SIZE 1012

	void		( *msg )( void *p );
	int			fd = -1, size, count = 0;
	rt_uint8_t	*pdata;             /*数据*/

	rt_uint8_t	buf[32];
	rt_uint8_t	ch_h, ch_l;
	rt_err_t	res;
	LENGTH_BUF	uart_buf;

	rt_uint32_t file_datalen;       /*升级文件长度*/
	rt_uint8_t	file_matchcode[2];  /*文件匹配码*/

	rt_uint16_t packet_num;

	rt_uint16_t crc;

	msg = parameter;

	ptr_mem_packet = rt_malloc( READ_PACKET_SIZE + 20 );
	if( ptr_mem_packet == RT_NULL )
	{
		msg( "E内存不足" );
		return;
	}
/*查找U盘*/
	while( 1 )
	{
		if( rt_device_find( "udisk" ) == RT_NULL ) /*没有找到*/
		{
			count++;
			if( count <= 10 )
			{
				msg( "I等待U盘插入" );
			}else
			{
				msg( "EU盘不存在" ); /*指示U盘不存在*/
				goto end_upgrade_usb_0;
			}
			rt_thread_delay( RT_TICK_PER_SECOND );
		}else
		{
			msg( "I查找升级文件" );
			break;
		}
	}

/*查找指定文件BEIDOU.IMG*/
	fd = open( "/udisk/BEIDOU.IMG", O_RDONLY, 0 );
	if( fd >= 0 )
	{
		msg( "I分析文件" );
	}else
	{
		msg( "E升级文件不存在" );
		goto end_upgrade_usb_0;
	}

	size	= read( fd, ptr_mem_packet, 16 );
	pdata	= ptr_mem_packet;
	if( ( *pdata != 0x54 ) || ( *( pdata + 1 ) != 0x44 ) )
	{
		msg( "E文件头错误" );
		goto end_upgrade_usb_1;
	}

	ch_h	= ( *( pdata + 9 ) & 0xf0 ) >> 4;
	ch_l	= ( *( pdata + 9 ) & 0xf );
	sprintf( buf, "I版本:%d.%d.%d", ch_h, ch_l, *( pdata + 10 ) );
	msg( buf );
/*升级数据长度*/
	file_datalen	= 0;
	file_datalen	= ( *( pdata + 11 ) ) << 24;
	file_datalen	|= ( *( pdata + 12 ) ) << 16;
	file_datalen	|= ( *( pdata + 13 ) ) << 8;
	file_datalen	|= *( pdata + 14 );
	rt_kprintf( "file_datalen=%x", file_datalen );
/*文件匹配码在尾部*/
	count	= 0;
	ch_h	= 0;

	do
	{
		res = read( fd, ptr_mem_packet, READ_PACKET_SIZE );
		if( res )
		{
			count = res;
		}
	}
	while( res > 0 );
	//rt_kprintf("res=%02x\r\n",res);
	//if(res==0) res=READ_PACKET_SIZE;

	if( ( ptr_mem_packet[count - 1] != 0x54 ) || ( ptr_mem_packet[count - 2] != 0x44 ) )
	{
		msg( "E文件尾错误" );
		goto end_upgrade_usb_1;
	}
	file_matchcode[0]	= ptr_mem_packet[count - 6];
	file_matchcode[1]	= ptr_mem_packet[count - 5];
	rt_kprintf( "file datalen=%x matchcode=%02x%02x", file_datalen, file_matchcode[0], file_matchcode[1] );
	close( fd );

	msg( "I配置端口" );

	fd = open( "/udisk/BEIDOU.IMG", O_RDONLY, 0 );
	if( fd < 0 )
	{
		msg( "E升级文件不存在" );
		goto end_upgrade_usb_0;
	}

/*开始升级*/
/*停止gps线程*/
	rt_thread_suspend( &thread_gps );

/*进入升级状态*/
	memcpy( buf, "\x40\x30\xC0\x00\x03\x00\x01\x34\x21\x0D\x0A", 11 );
	dev_gps_write( &dev_gps, 0, buf, 11 );
	rt_thread_delay( RT_TICK_PER_SECOND );
/*版本查询*/
	count = 0;
	dev_gps_write( &dev_gps, 0, "\x40\x10\xC0\x00\x10\x00\x01\xC2\x84\x0D\x0A", 11 );
	while( 1 )
	{
		res = rt_mq_recv( &mq_gps, (void*)&uart_buf, 124, RT_TICK_PER_SECOND * 5 );
		if( res == RT_EOK )                                       //收到一包数据
		{
			rt_kprintf( "\r\n版本查询\r\n" );
			for( ch_h = 0; ch_h < uart_buf.wr; ch_h++ )
			{
				rt_kprintf( "%02x ", uart_buf.body[ch_h] );
			}
			rt_kprintf( "\r\n" );
			if( ( uart_buf.wr == 15 ) && ( uart_buf.body[4] == 0x02 ) ) /*进入升级状态*/
			{
				ch_h	= ( uart_buf.body[7] & 0xf0 ) >> 4;
				ch_l	= ( uart_buf.body[7] & 0xf );
				sprintf( buf, "I版本:%d.%d.%d", ch_h, ch_l, uart_buf.body[8] );
				msg( buf );
				break;
			}
		}else /*超时*/
		{
			msg( "E进入升级错误" );
			goto end_upgrade_usb_2;
		}
	}

/*改波特率*/
	dev_gps_write( &dev_gps, 0, "\x40\x41\xC0\x00\x00\x00\xE1\x00\x00\x04\x07\xFC\x0D\x0A", 14 );
	gps_baud( 57600 );
	rt_thread_delay( RT_TICK_PER_SECOND );
/*连接查询*/
	dev_gps_write( &dev_gps, 0, "\x40\x15\xC0\x00\x01\x00\x01\x92\xD4\x0D\x0A", 11 );
	while( 1 )
	{
		res = rt_mq_recv( &mq_gps, (void*)&uart_buf, 124, RT_TICK_PER_SECOND * 5 );
		if( res == RT_EOK )                   //收到一包数据
		{
			if( ( uart_buf.wr == 11 ) && ( uart_buf.body[4] == 0x15 ) )
			{
				break;
			}
		}else /*超时*/
		{
			msg( "E修改波特率错误" );
			goto end_upgrade_usb_2;
		}
	}

/*开始烧写,烧写指令*/
	memcpy( buf, "\x40\xF2\xC0\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x0D\x0A", 18 );
	buf[6]	= file_datalen >> 24;
	buf[7]	= file_datalen >> 16;
	buf[8]	= file_datalen >> 8;
	buf[9]	= file_datalen & 0xff;
	buf[10] = file_matchcode[0];
	buf[11] = file_matchcode[1];

	crc		= CalcCRC16( buf, 1, 13 );
	buf[14] = ( crc & 0xff00 ) >> 8;
	buf[15] = crc & 0xff;

	dev_gps_write( &dev_gps, 0, buf, 18 );

	ch_l = 1;
	while( ch_l )
	{
		res = rt_mq_recv( &mq_gps, (void*)&uart_buf, 124, RT_TICK_PER_SECOND * 10 );
		if( res == RT_EOK )                   //收到一包数据
		{
			for( ch_h = 0; ch_h < uart_buf.wr; ch_h++ )
			{
				rt_kprintf( "%02x ", uart_buf.body[ch_h] );
			}
			rt_kprintf( "\r\n" );

			if( ( uart_buf.wr == 11 ) && ( uart_buf.body[4] == 0xf2 ) )
			{
				ch_l = 0;
			}
		}else /*超时*/
		{
			msg( "E开始升级错误" );
			goto end_upgrade_usb_2;
		}
	}

/*发送数据包*/
	count		= 0;            /*计算发送字节数*/
	packet_num	= 0;
	read( fd, buf, 15 );        /*跳过文件头*/
	rt_kprintf( "fd=%02x\r\n", fd );
	while( 1 )
	{
		rt_thread_delay( RT_TICK_PER_SECOND / 2 );
		memcpy( ptr_mem_packet, "\x40\xf2\x00\x00\x03\x01", 6 );
		res = read( fd, ptr_mem_packet + 6, READ_PACKET_SIZE );
		if( res < 0 )
		{
			msg( "E读取文件出错" );
			goto end_upgrade_usb_2;
		}

		count += res;
		if( res == READ_PACKET_SIZE )           /*判断是否为最后一包*/
		{
			if( count == file_datalen )         /*长度正为1012整数倍*/
			{
				ptr_mem_packet[2]	= 0x80 | ( packet_num >> 8 );
				ptr_mem_packet[3]	= packet_num & 0xff;
				res					= res - 6;
			}else
			{
				if( count == READ_PACKET_SIZE ) /*第一包*/
				{
					ptr_mem_packet[2]	= 0x40 | ( packet_num >> 8 );
					ptr_mem_packet[3]	= packet_num & 0xff;
				}else
				{
					ptr_mem_packet[2]	= packet_num >> 8;
					ptr_mem_packet[3]	= packet_num & 0xff;
				}
			}
		}else
		{
			ptr_mem_packet[2]	= 0x80 | ( packet_num >> 8 );
			ptr_mem_packet[3]	= packet_num & 0xff;
			/*去掉文件尾部的数据*/
			res = res - 6;
		}
		packet_num++;
		ptr_mem_packet[res + 6]		= ( res + 2 ) >> 8;
		ptr_mem_packet[res + 7]		= ( res + 2 ) & 0xff;
		crc							= CalcCRC16( ptr_mem_packet, 1, res + 7 );
		ptr_mem_packet[res + 8]		= ( crc & 0xff00 ) >> 8;
		ptr_mem_packet[res + 9]		= crc & 0xff;
		ptr_mem_packet[res + 10]	= 0x0d;
		ptr_mem_packet[res + 11]	= 0x0a;

		rt_kprintf( "\r\nres=%d,%02x%02x,%04x\r\n", res, ptr_mem_packet[2], ptr_mem_packet[3], crc );
		for( ch_h = 0; ch_h < 16; ch_h++ )
		{
			rt_kprintf( "%02x ", ptr_mem_packet[ch_h] );
		}
		rt_kprintf( "tx tick=%x\r\n", rt_tick_get( ) );
		dev_gps_write( &dev_gps, 0, ptr_mem_packet, res + 12 );

		ch_l = 1;
		while( ch_l )
		{
			res = rt_mq_recv( &mq_gps, (void*)&uart_buf, 124, RT_TICK_PER_SECOND * 12 );
			if( res == RT_EOK ) //收到一包数据
			{
				for( ch_h = 0; ch_h < uart_buf.wr; ch_h++ )
				{
					rt_kprintf( "%02x ", uart_buf.body[ch_h] );
				}
				rt_kprintf( "rx tick=%x\r\n", rt_tick_get( ) );
				if( uart_buf.wr == 11 )
				{
					if( uart_buf.body[4] == 0x02 )
					{
						msg( "E升级完成" );
						dev_gps_write( &dev_gps, 0, "\x40\x34\xC0\x00\x34\x00\x01\x84\x6B\x0D\x0A", 11 );
						goto end_upgrade_usb_2;
					}
					sprintf( buf, "I发送第%d包", packet_num );
					msg( buf );
					ch_l = 0;
				}
			}else /*超时*/
			{
				msg( "E升级错误" );
				goto end_upgrade_usb_2;
			}
		}
	}

end_upgrade_usb_2:
	rt_thread_resume( &thread_gps );
end_upgrade_usb_1:
	if( fd >= 0 )
	{
		close( fd );
	}
end_upgrade_usb_0:
	rt_free( ptr_mem_packet );
	ptr_mem_packet = RT_NULL;
}

/*gps升级*/
rt_err_t gps_upgrade( char *src )
{
	rt_thread_t tid;
	int			buad = 9600;
	rt_kprintf( "\nNow upgrade from %s\n", src );
	if( strncmp( src, "COM", 3 ) == 0 ) /*串口升级*/
	{
		tid = rt_thread_create( "upgrade", thread_gps_upgrade_uart, (void*)msg_uart_usb, 2048, 5, 5 );
		if( tid != RT_NULL )
		{
			rt_thread_startup( tid );
		}else
		{
			rt_kprintf( "\n Upgrade from uart fail\n" );
		}
	}else
	{
		tid = rt_thread_create( "upgrade", thread_gps_upgrade_udisk, (void*)msg_uart_usb, 512, 7, 5 );
		if( tid != RT_NULL )
		{
			rt_thread_startup( tid );
		}else
		{
			rt_kprintf( "\n Upgrade from uart fail\n" );
		}
	}
}

FINSH_FUNCTION_EXPORT( gps_upgrade, upgrade bd_gps );


/*
   查询设置bdgps模式
   0:查询

   const char BD_MODE[]={"$CCSIR,1,0*49\r\n"};
   const char GPS_MODE[]={"$CCSIR,2,0*4A\r\n"};
   const char GPSBD_MODE[]={"$CCSIR,3,0*4B\r\n"};
 */
rt_size_t gps_mode( uint8_t mode )
{
	switch( mode )
	{
		case 0:
			break;
		case 1: /*BD*/
			dev_gps_write( &dev_gps, 0, "$CCSIR,1,0*49\r\n", 15 );
			break;
		case 2: /*GPS*/
			dev_gps_write( &dev_gps, 0, "$CCSIR,2,0*4A\r\n", 15 );
			break;
		case 3: /*BDGPS*/
			dev_gps_write( &dev_gps, 0, "$CCSIR,3,0*4B\r\n", 15 );
			break;
	}
}

FINSH_FUNCTION_EXPORT( gps_mode, change mode );

/************************************** The End Of File **************************************/

