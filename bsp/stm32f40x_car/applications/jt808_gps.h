#ifndef _JT808_GPS_H_
#define _JT808_GPS_H_

#include <rtthread.h>
#include "stm32f4xx.h"


/*基本位置信息,因为字节对齐的方式，还是使用数组方便*/
typedef __packed struct _gps_baseinfo
{
	uint32_t alarm;
	uint32_t status;
	uint32_t latitude; /*纬度*/
	uint32_t longitude;/*精度*/
	uint16_t altitude;
	uint16_t speed;
	uint16_t direction;
	uint8_t datetime[6];
}GPS_BASEINFO;


enum BDGPS_MODE 
{
	MODE_GET=0,	/*查询*/
	MODE_BD=1,
	MODE_GPS,
	MODE_BDGPS,
};


typedef  struct  _gps_status
{
   enum BDGPS_MODE   Position_Moule_Status;  /* 1: BD   2:  GPS   3: BD+GPS    定位模块的状态*/
   uint8_t  Antenna_Flag;//显示提示开路 
   uint8_t  Raw_Output;   //  原始数据输出  
}GPS_STATUS;

extern GPS_STATUS	gps_status;
extern GPS_BASEINFO gps_baseinfo;



void gps_rx( uint8_t * pinfo, uint16_t length );




#endif
