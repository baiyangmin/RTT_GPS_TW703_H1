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
#include "Menu_Include.h"
//#include "stm32f4xx.h"
//#include "gps.h"

static uint8_t menu_pos_bd=0;
static uint8_t fupgrading=0;	/*是否在升级状态，禁用按键判断*/
static rt_thread_t tid_upgrade=RT_NULL;	/*开始更新*/



/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void menu_set(void)
{
	lcd_fill( 0 );
	lcd_text12( 0, 3, "北斗", 4, LCD_MODE_SET );
	lcd_text12( 0, 17, "升级", 4, LCD_MODE_SET );
	if( menu_pos_bd == 0 )
	{
		lcd_text12( 35, 3, "1.串口升级", 10, LCD_MODE_INVERT );
		lcd_text12( 35, 19, "2.USB升级", 9, LCD_MODE_SET );
	}else
	{
		lcd_text12( 35, 3, "1.串口升级", 10, LCD_MODE_SET );
		lcd_text12( 35, 19, "2.USB升级", 9, LCD_MODE_INVERT );
	}
	lcd_update_all( );
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
static void show( void )
{
	menu_pos_bd=0;
	menu_set();
}

/*
提供回调函数用以显示信息
*/
#if 0
static void msg( unsigned int res )
{
	char buf[32];
	unsigned int len;
	lcd_fill( 0 );
	lcd_text12( 0, 3, "北斗", 4, LCD_MODE_SET );
	lcd_text12( 0, 17, "升级", 4, LCD_MODE_SET );
	switch(res)
	{
		case BDUPG_RES_UART_OK:
			lcd_text12( 35, 10,"串口更新完成",12, LCD_MODE_SET );
			fupgrading=0;
			tid_upgrade=RT_NULL;
			break;
		case BDUPG_RES_USB_OK:
			lcd_text12( 35, 10,"USB更新完成",11, LCD_MODE_SET );
			fupgrading=0;
			tid_upgrade=RT_NULL;
			break;			
		case BDUPG_RES_THREAD:
			lcd_text12( 35, 10,"创建更新失败",12, LCD_MODE_SET );
			fupgrading=0;
			tid_upgrade=RT_NULL;
			break;
		case BDUPG_RES_TIMEOUT:
			lcd_text12( 35, 10,"超时错误",8, LCD_MODE_SET );
			fupgrading=0;
			tid_upgrade=RT_NULL;
			break;
		case BDUPG_RES_RAM:
			lcd_text12( 35, 10,"分配RAM失败",11, LCD_MODE_SET );
			fupgrading=0;
			tid_upgrade=RT_NULL;
			break;
		case BDUPG_RES_UART_READY:
			lcd_text12( 35, 10,"串口开始更新",12, LCD_MODE_SET );
			break;
		case BDUPG_RES_USB_READY:
			lcd_text12( 35, 10,"查找u盘",7, LCD_MODE_SET );
			break;	
		case BDUPG_RES_USB_NOEXIST:
			lcd_text12( 35, 10,"U盘不存在",10, LCD_MODE_SET );
			fupgrading=0;
			tid_upgrade=RT_NULL;
			break;
		case BDUPG_RES_USB_NOFILE:
			lcd_text12( 35, 10,"文件不存在",10, LCD_MODE_SET );
			fupgrading=0;
			tid_upgrade=RT_NULL;
			break;
		case BDUPG_RES_USB_FILE_ERR:
			lcd_text12( 35, 10,"文件格式错误",12, LCD_MODE_SET );
			fupgrading=0;
			tid_upgrade=RT_NULL;
			break;
		case BDUPG_RES_USB_MODULE_H:
			break;
		case BDUPG_RES_USB_MODULE_L:
			break;
		case BDUPG_RES_USB_FILE_VER:
			break;
		default:
			if(res<0xffff)	/*默认不会发送这么多包数据*/
			{
				len=sprintf(buf,"发送第%d包",res);
				lcd_text12( 35, 10, buf, len, LCD_MODE_SET );
			}	
			
			break;
			
	}
	lcd_update_all( );
}
#endif

static void msg( void *p)
{
	//char buf[32];
	unsigned int len;
	char *pinfo;
	lcd_fill( 0 );
	lcd_text12( 0, 3, "北斗", 4, LCD_MODE_SET );
	lcd_text12( 0, 17, "升级", 4, LCD_MODE_SET );
	pinfo=(char *)p;
	len=strlen(pinfo);
	lcd_text12( 35, 10,pinfo+1,len-1, LCD_MODE_SET );
	if(*pinfo=='E')	/*出错或结束*/
	{
		fupgrading=0;
		tid_upgrade=RT_NULL;
	}

	lcd_update_all( );
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
static void keypress( unsigned int key )
{
	if(fupgrading)	return;
	switch( KeyValue )
	{
		case KeyValueMenu:
			pMenuItem=&Menu_5_other;
			pMenuItem->show();
			CounterBack=0;
			
			if(tid_upgrade) /*正在升级中*/
			{
				rt_thread_delete(tid_upgrade);
			}
			
			break;
		case KeyValueOk:
			if(BD_upgrad_contr==1)
				menu_set();
			if(menu_pos_bd==0)
			{
				tid_upgrade = rt_thread_create( "upgrade", thread_gps_upgrade_uart, (void*)msg, 1024, 5, 5 );
				if( tid_upgrade != RT_NULL )
				{
					msg("I等待串口升级");
					rt_thread_startup( tid_upgrade );
				}else
				{
					msg("E线程创建失败");
				}
			}else /*U盘升级*/
			{
				tid_upgrade = rt_thread_create( "upgrade", thread_gps_upgrade_udisk, (void*)msg, 1024, 5, 5 );
				if( tid_upgrade != RT_NULL )
				{
					msg("I等待U盘升级");
					rt_thread_startup( tid_upgrade );
				}else
				{
					msg("E线程创建失败");
				}
			}
			break;
		case KeyValueUP:
			menu_pos_bd=menu_pos_bd^0x01;
			menu_set();
			break;
		case KeyValueDown:
			menu_pos_bd=menu_pos_bd^0x01;
			menu_set();
			break;
	}
	KeyValue = 0;
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
static void timetick( unsigned int systick )
{

}

ALIGN( RT_ALIGN_SIZE )
MENUITEM Menu_5_3_bdupgrade =
{
	"北斗升级",
	8,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

/************************************** The End Of File **************************************/
