#ifndef _GPS_H_
#define _GPS_H_


#define CTL_GPS_BAUD	0x30
#define CTL_GPS_OUTMODE	0x31	/*gps信息的输出模式*/


#define GPS_OUTMODE_TRIGGER	0x01	/*秒触发输出*/
#define GPS_OUTMODE_GPRMC	0x02
#define GPS_OUTMODE_GPGSV	0x04
#define GPS_OUTMODE_BD		0x08

#define GPS_OUTMODE_ALL		0xFFFFFFFF	/*全数据输出*/


/*
北斗更新的操作状态结果码，分为几个部分

0000 0000-0FFF FFFF  包序号


*/


#define BDUPG_RES_UART_OK		(0x10000000)	/*没有错误升级成功-升级成功*/
#define BDUPG_RES_UART_READY	(0x1000FFFF)	/*串口更新就绪*/


#define BDUPG_RES_USB_OK		(0x20000000)	/*没有错误升级成功-升级成功*/

#define BDUPG_RES_USB_FILE_ERR	(0x2000FFFC)	/*更新文件格式错误*/
#define BDUPG_RES_USB_NOFILE	(0x2000FFFD)	/*更新文件不存在*/
#define BDUPG_RES_USB_WAITUSB	(0x2000FFFD)	/*更新文件不存在*/
#define BDUPG_RES_USB_NOEXIST	(0x2000FFFE)	/*u盘不存在*/
#define BDUPG_RES_USB_READY		(0x2000FFFF)	/*u盘就绪*/


#define BDUPG_RES_USB_MODULE_H	(0x20010000)	/*模块型号高16bit*/
#define BDUPG_RES_USB_MODULE_L	(0x20020000)	/*模块型号低16bit*/
#define BDUPG_RES_USB_FILE_VER	(0x20030000)	/*软件版本*/


#define BDUPG_RES_THREAD	(0xFFFFFFFE)	/*创建升级线程失败-创建更新失败*/
#define BDUPG_RES_RAM		(0xFFFFFFFD)	/*分配RAM失败*/
#define BDUPG_RES_TIMEOUT	(0xFFFFFFFC)	/*超时失败*/








void thread_gps_upgrade_uart( void* parameter );
void thread_gps_upgrade_udisk( void* parameter );


#endif

