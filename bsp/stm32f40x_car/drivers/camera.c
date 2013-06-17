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
#include "sst25.h"

#define JT808_PACKAGE_MAX		512

typedef enum 
{
	CAM_NONE=0,
	CAM_IDLE=1,
	CAM_START,
	CAM_GET_PHOTO,
	CAM_RX_PHOTO,
	CAM_END,
	CAM_FALSE
}CAM_STATE;


typedef enum
{
	RX_IDLE=0,
	RX_SYNC1,
	RX_SYNC2,
	RX_HEAD,
	RX_DATA,
	RX_FCS,
	RX_0D,
	RX_0A,
}CAM_RX_STATE;	


typedef  struct
{
	CAM_STATE	State;				///处理过程状态，
	CAM_RX_STATE Rx_State;			///接收状态
	u8			Retry;				///重复照相次数
	Style_Cam_Requset_Para	Para;	///触发当前拍照相关信息
} Style_Cam_Para;


typedef __packed struct
{
	char 	Head[6];				///幻数部分，表示当前数据区域为某固定数据开始
	u32		Len;					///数据长度，包括数据头部分内容,数据头部分固定为64字节
	u8		State;					///表示图片状态标记，0xFF为初始化状态，bit0==0表示已经删除,bit1==0表示成功上传,bit2==0表示该数据为不存盘数据
	T_TIMES	Time;					///记录数据的时间，BCD码表示，YY-MM-DD-hh-mm-ss
	u32		Data_ID;				///数据ID,顺序递增方式记录
	u8		Media_Format;			///0：JPEG；1：TIF；2：MP3；3：WAV；4：WMV； 其他保留
	u8		Media_Style;			///数据类型:0：图像；1：音频；2：视频；
	u8		Channel_ID;				///数据通道ID
	u8		TiggerStyle;			///触发方式
	u8		position[28];			///位置信息
}TypeDF_PackageHead;

typedef __packed struct
{
	u32 	Address;				///地址
	u32 	Len;					///长度
	u32		Data_ID;				///数据ID
}TypeDF_PackageInfo;

typedef __packed struct
{
	u16		Number;					///图片数量
	TypeDF_PackageInfo	FirstPic;	///第一个图片
	TypeDF_PackageInfo	LastPic;	///最后一个图片
}TypeDF_PICPara;


typedef __packed struct
{
	u32 	Address;				///地址
	u32		Data_ID;				///数据ID
	u8		Delete;					///删除标记
	u8		Pack_Mark[16];			///包标记
}TypePicMultTransPara;


#define	DF_CamAddress_Start		0x1000			///图片数据存储开始位置
#define DF_CamAddress_End		0X20000			///图片数据存储结束位置
#define DF_CamSaveSect			0x400			///图片数据存储最小间隔

extern rt_device_t _console_device;
extern u16 Hex_To_Ascii(const u8* pSrc, u8* pDst, u16 nSrcLength);

const char  CAM_HEAD[]={"PIC_01"};

static Style_Cam_Para			Current_Cam_Para={CAM_NONE,0,0,0};		
static TypeDF_PICPara			DF_PicParameter;						///FLASH存储的图片信息

/* 消息队列控制块*/
struct rt_messagequeue mq_Cam;
/* 消息队列中用到的放置消息的内存池*/
static char msgpool_cam[256];

static char CamTestBuf[2000];

extern MsgList* list_jt808_tx;



/*********************************************************************************
*函数名称:u8 HEX_to_BCD(u8 HEX)
*功能描述:将1个字节大小的HEX码转换为BCD码；返回BCD码
*输 入:none
*输 出:none 
*返 回 值:none
*作 者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u8 HEX_to_BCD(u8 HEX)
{
 u8 BCD_code=0;
 BCD_code=HEX%10;
 BCD_code|=((HEX%100)/10)<<4;
 return BCD_code; 
}


/*********************************************************************************
*函数名称:u8 BCD_to_HEX(u8 BCD)
*功能描述:将1个字节大小的BCD码转换为HEX码；返回HEX码
*输 入:none
*输 出:none 
*返 回 值:none
*作 者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u8 BCD_to_HEX(u8 BCD)
{
 u8 HEX_code=0;
 HEX_code = BCD&0x0F;
 HEX_code += (BCD>>4)*10;
 return HEX_code;
}



/*********************************************************************************
*函数名称:bool leap_year(u16 year)
*功能描述:本函数计算年year是不是闰年,是闰年返回1,否则返回0;
			闰年为366天否则为365天,闰年的2月为29天.
*输	入:year	年
*输	出:none 
*返 回 值:,是闰年返回1,否则返回0;
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u8 leap_year(u16 year)
{
 u8 leap;
 if((year&0x0003)==0)
 	{
 	if(year%100==0)
 		{
		if(year%400==0)
			leap=1;
		else
			leap=0;
 		}
	else
		leap=1;
 	}
 else
 	leap=0;
 return leap;
}


/*********************************************************************************
*函数名称:u32 Timer_To_Day(T_TIMES *T)
*功能描述:计算该时间的总天数，开始时间为2000年1月1日
*输	入:none
*输	出:none 
*返 回 值:时间对应的秒
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u32 Timer_To_Day(T_TIMES *T)
{
 u32 long_day;
 u16 i,year,month,day;
 
 year=BCD_to_HEX(T->years);
 month=BCD_to_HEX(T->months);
 day=BCD_to_HEX(T->days);
 year+=2000;
 long_day=0;
 for(i=2000;i<year;i++)
 	{
 	long_day+=365;
	long_day+=leap_year(i);	
 	}
 
 switch(month)
 	{
	case 12 :
 		long_day+=30;
	case 11 :
 		long_day+=31;
	case 10 :
 		long_day+=30;
	case 9 :
 		long_day+=31;
	case 8 :
 		long_day+=31;
	case 7 :
 		long_day+=30;
	case 6 :
 		long_day+=31;
	case 5 :
 		long_day+=30;
	case 4 :
 		long_day+=31;
	case 3 :
		{
 		long_day+=28;
		long_day+=leap_year(year);	
		}	
	case 2 :
 		long_day+=31;
	case 1 :
 		long_day+=day-1;
	default :
		nop;
 	}
 return long_day;
}


/*********************************************************************************
*函数名称:u8 Get_Month_Day(u8 month,u8 leapyear)
*功能描述:获取该月的天数
*输	入:none
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u8 Get_Month_Day(u8 month,u8 leapyear)
{
  u8 day;
  switch(month)
 	{
	case 12 :
 		day=31;
		break;
	case 11 :
 		day=30;
		break;
	case 10 :
 		day=31;
		break;
	case 9 :
 		day=30;
		break;
	case 8 :
 		day=31;
		break;
	case 7 :
 		day=31;
		break;
	case 6 :
 		day=30;
		break;
	case 5 :
 		day=31;
		break;
	case 4 :
 		day=30;
		break;
	case 3 :
 		day=31;
		break;
	case 2 :
		{
 		day=28;
		day+=leapyear;	
		break;
		}	
	case 1 :
 		day=31;
		break;
	default :
		nop;
 	}
  return day;
}


/*********************************************************************************
*函数名称:u32 Times_To_LongInt(T_TIMES *T)
*功能描述:本函数将RTC_TIMES类型的时间转换为long int类型的数据，
		单位为秒，起始时间为2000年1月1日00:00:00
*输	入:none
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u32 Times_To_LongInt(T_TIMES *T)
{
 u32 timer_int,hour;
 u16 i,minute,second;
 hour=BCD_to_HEX(T->hours);
 minute=BCD_to_HEX(T->minutes);
 second=BCD_to_HEX(T->seconds);
 timer_int=Timer_To_Day(T);
 hour*=3600;
 minute*=60;
 timer_int=timer_int*86400+hour+minute+second;			///timer_int*24*3600=timer_int*86400
 return timer_int;
}



/*********************************************************************************
*函数名称:void LongInt_To_Times(u32 timer_int, T_TIMES *T)
*功能描述:将基准时间是2000年1月1日00:00:00的长整形数据转为T_TIMES类型
*输	入:none
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
**********************************************6***********************************/
void LongInt_To_Times(u32 timer_int, T_TIMES *T)
{
 u32 long_day1,long_day2;
 u16 i,day,leapyear;
 long_day2=0;
 long_day1=timer_int/86400;
 for(i=2000;i<2100;i++)
 	{
 	day=365+leap_year(i);
 	long_day2+=day;
	//如果当前年的总天数小于计算天数则得到了年份
	if(long_day2>long_day1)
		{
		long_day2-=day;
		leapyear=leap_year(i);
		break;
		}
 	}
 T->years=HEX_to_BCD(i-2000);
 for(i=1;i<=12;i++)
 	{
 	day=Get_Month_Day(i,leapyear);
 	long_day2+=day;
	//如果当前月的总天数小于计算天数则得到了月份
	if(long_day2>long_day1)
		{
		long_day2-=day;
		break;
		}
 	}
 T->months=HEX_to_BCD(i);
 day=long_day1-long_day2+1;
 T->days=HEX_to_BCD(day);
 
 long_day1=timer_int%86400;
 i=long_day1/3600;
 T->hours=HEX_to_BCD(i);

 i=long_day1%3600/60;
 T->minutes=HEX_to_BCD(i);
 
 i=long_day1%60;
 T->seconds=HEX_to_BCD(i);
}


/*********************************************************************************
*函数名称:uint16_t data_to_buf( uint8_t * pdest, uint32_t data, uint8_t width )
*功能描述:将不同类型的数据存入buf中，数据在buf中为大端模式
*输	入:	pdest: 	存放数据的buffer
		data:	存放数据的原始数据
		width:	存放的原始数据占用的buf字节数
*输	出:	 
*返 回 值:存入的字节数
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
uint16_t data_to_buf( uint8_t * pdest, uint32_t data, uint8_t width )
{
	u8 *buf;
	u8 i;
	buf=pdest;

	switch( width )
	{
		case 1:
			*buf++ = data & 0xff;
			break;
		case 2:
			*buf++ = data >> 8;
			*buf++ = data & 0xff;
			break;
		case 4:
			*buf++ = data >> 24;
			*buf++ = data >> 16;
			*buf++ = data >> 8;
			*buf++ = data & 0xff;
			break;
	}
	return width;
}



/*********************************************************************************
*函数名称:uint16_t buf_to_data( uint8_t * psrc, uint8_t width )
*功能描述:将不同类型的数据从buf中取出来，数据在buf中为大端模式
*输	入:	psrc: 	存放数据的buffer
		width:	存放的原始数据占用的buf字节数
*输	出:	 none
*返 回 值:uint32_t 返回存储的数据
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
uint32_t buf_to_data( uint8_t * psrc, uint8_t width )
{
	u8 i;
	u8 *buf;
	u32 outData=0;
	buf=psrc;
	for(i=0;i<width;i++)
		{
		outData<<=8;
		outData+=*psrc++;
		}
	return outData;
}

/*********************************************************************************
*函数名称:u32 Cam_Flash_AddrCheck(u32 pro_Address)
*功能描述:检查当前的位置是否合法，不合法就修改为合法数据并返回
*输	入:none
*输	出:none 
*返 回 值:正确的地址值
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static u32 Cam_Flash_AddrCheck(u32 pro_Address)
{
	while(pro_Address>=DF_CamAddress_End)
		{
		pro_Address=pro_Address+DF_CamAddress_Start-DF_CamAddress_End;
		}
	if(pro_Address<DF_CamAddress_Start)
		{
		pro_Address=DF_CamAddress_Start;
		}
	return pro_Address;
}



/*********************************************************************************
*函数名称:u8 Cam_Flash_FirstPicProc(u32 temp_wr_addr)
*功能描述:修改第1个图片数据位置参数，因为要对flash进行erase，当erase的是第1个图片时就需要将DF_PicParameter中
		第一个图片的信息更新
*输	入:当前要擦写的位置
*输	出:none 
*返 回 值:1表示正常返回，0表示有错误发生
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static u8 Cam_Flash_FirstPicProc(u32 temp_wr_addr)
{
	u8 i;
	u8 ret;
	u32 TempAddress,u32TempData;
	TypeDF_PackageHead TempPackageHead;

	if((Cam_Flash_AddrCheck(temp_wr_addr)==(DF_PicParameter.FirstPic.Address&0xFFFFF000))&&(DF_PicParameter.Number))
		{
		TempAddress=(DF_PicParameter.FirstPic.Address+DF_PicParameter.FirstPic.Len+DF_CamSaveSect-1)/DF_CamSaveSect*DF_CamSaveSect;
		for(i=0;i<8;i++)
			{
			sst25_read(Cam_Flash_AddrCheck(TempAddress),(u8 *)&TempPackageHead,sizeof(TempPackageHead));
			if(strncmp(TempPackageHead.Head,CAM_HEAD,strlen(CAM_HEAD))==0)
				{
				if((TempAddress & 0xFFFFF000)!= (DF_PicParameter.FirstPic.Address & 0xFFFFF000))
					{
					DF_PicParameter.FirstPic.Address=Cam_Flash_AddrCheck(TempAddress);
					DF_PicParameter.FirstPic.Len=TempPackageHead.Len;
					DF_PicParameter.FirstPic.Data_ID=TempPackageHead.Data_ID;
					DF_PicParameter.Number--;
					ret = 1;
					break;
					}
				if(TempPackageHead.Len==0)
					{
					ret =  0;
					break;
					}
				TempAddress+=(TempPackageHead.Len+DF_CamSaveSect-1)/DF_CamSaveSect*DF_CamSaveSect;
				}
			else
				{
				ret =  0;
				break;
				}
			}
		ret =  0;
		}
	else
		{
		ret =  1;
		}
	sst25_erase_4k(temp_wr_addr);
	return ret;
}


/*********************************************************************************
*函数名称:u16 Cam_Flash_InitPara(void)
*功能描述:初始化Pic参数，包括读取FLASH中的图片信息，获取到图片的数量及开始位置，结束位置等，
		这些读取到得数据都存储在 DF_PicParameter 中。
*输	入:none
*输	出:none 
*返 回 值:有效的图片数量
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static u16 Cam_Flash_InitPara(void)
{
 u32 TempAddress,u32TempData;
 TypeDF_PackageHead TempPackageHead;
 TypeDF_PackageInfo TempPackageInfo;
 
 
 memset(&DF_PicParameter,0,sizeof(DF_PicParameter));
 DF_PicParameter.FirstPic.Address=DF_CamAddress_Start;
 DF_PicParameter.LastPic.Address=DF_CamAddress_Start;
 for(TempAddress=DF_CamAddress_Start;TempAddress<DF_CamAddress_End;)
 	{
 	sst25_read(TempAddress,(u8 *)&TempPackageHead,sizeof(TempPackageHead));
	if(strncmp(TempPackageHead.Head,CAM_HEAD,strlen(CAM_HEAD))==0)
		{
		DF_PicParameter.Number++;
		TempPackageInfo.Address=TempAddress;
		TempPackageInfo.Data_ID=TempPackageHead.Data_ID;
		TempPackageInfo.Len=TempPackageHead.Len;
		if(DF_PicParameter.Number==1)
			{
			DF_PicParameter.FirstPic=TempPackageInfo;
			DF_PicParameter.LastPic =TempPackageInfo;
			}
		else
			{
			if(TempPackageInfo.Data_ID>DF_PicParameter.LastPic.Data_ID)
				{
				DF_PicParameter.LastPic =TempPackageInfo;
				}
			else if(TempPackageInfo.Data_ID<DF_PicParameter.FirstPic.Data_ID)
				{
				DF_PicParameter.FirstPic=TempPackageInfo;
				}
			}
		TempAddress+=(TempPackageInfo.Len+DF_CamSaveSect-1)/DF_CamSaveSect*DF_CamSaveSect;
		}
	else
		{
		TempAddress+=DF_CamSaveSect;
		}
 	}
 return DF_PicParameter.Number;
}



/*********************************************************************************
*函数名称:rt_err_t Cam_Flash_WrPic(u8 *pData,u16 len, TypeDF_PackageHead *pHead)
*功能描述:向FLASH中写入图片数据，如果传递的数据的pHead->len为非0值表示为最后一包数据
*输	入:	pData:写入的数据指针，指向数据buf；
		len:数据长度，注意，该长度必须小于4096
		pHead:数据包头信息，该包头信息需要包含时间Timer，该值每次都必须传递，并且同样的包该值不能变化，
				最后一包数据需将len设置为数据长度len的长度包括包头部分，包头部分为固定64字节，其它包
				len为0.
*输	出:none 
*返 回 值:re_err_t
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static rt_err_t Cam_Flash_WrPic(u8 *pData,u16 len, TypeDF_PackageHead *pHead)
{
	u16 i;
	u32 temp_wr_addr;
	u32 temp_Len;
	static u8	WriteFuncUserBack=0;
	static u32	WriteAddress=0;
	static u32	WriteAddressStart=0;
	static T_TIMES	LastTime={0,0,0,0,0,0};
	
	u8 strBuf[256];
	
	
	if(memcmp(&LastTime,&pHead->Time,sizeof(LastTime))!=0)
		{
		LastTime=pHead->Time;
		WriteAddressStart=(DF_PicParameter.LastPic.Address+DF_PicParameter.LastPic.Len+DF_CamSaveSect-1)/DF_CamSaveSect*DF_CamSaveSect;
		WriteAddressStart=Cam_Flash_AddrCheck(WriteAddressStart);
		WriteAddress=WriteAddressStart+64;
		if((WriteAddressStart&0xFFF)==0)
			{
			Cam_Flash_FirstPicProc(WriteAddressStart);
			}
		else
			{
			///判断该区域是否数据异常，异常需要使用falsh写函数sst25_write_back
			sst25_read(WriteAddressStart,strBuf,sizeof(strBuf));
			WriteFuncUserBack=0;
			for(i=0;i<sizeof(strBuf);i++)
				{
				if(strBuf[i]!=0xFF)
					{
					if(i<64)		///如果包头部分也错误，清空包头部分内容为0xFF
						{
						memset(strBuf,0xFF,sizeof(strBuf));
						sst25_write_back(WriteAddressStart,strBuf,sizeof(strBuf));
						}
					
					WriteFuncUserBack=1;
					break;
					}
				}
			}
		}
	
	if(WriteFuncUserBack)
		{
		if((WriteAddress&0xFFFFF000)!=((WriteAddress+len)&0xFFFFF000))		///跨越两个扇区的处理
			{
			temp_wr_addr=(WriteAddress+len)&0xFFFFF000;
			temp_Len=temp_wr_addr-WriteAddress;
			sst25_write_back(Cam_Flash_AddrCheck(WriteAddress),pData,temp_Len);
			Cam_Flash_FirstPicProc(Cam_Flash_AddrCheck(temp_wr_addr));
			sst25_write_through(Cam_Flash_AddrCheck(temp_wr_addr),pData+temp_Len,len-temp_Len);
			WriteFuncUserBack=0;
			}
		else
			{
			sst25_write_back(Cam_Flash_AddrCheck(WriteAddress),pData,len);
			}
		}
	else
		{
		if((WriteAddress&0xFFFFF000)!=((WriteAddress+len)&0xFFFFF000))		///跨越两个扇区的处理
			{
			temp_wr_addr=(WriteAddress+len)&0xFFFFF000;
			temp_Len=temp_wr_addr-WriteAddress;
			sst25_write_through(Cam_Flash_AddrCheck(WriteAddress),pData,temp_Len);
			Cam_Flash_FirstPicProc(Cam_Flash_AddrCheck(temp_wr_addr));
			sst25_write_through(Cam_Flash_AddrCheck(temp_wr_addr),pData+temp_Len,len-temp_Len);
			}
		else
			{
			sst25_write_through(Cam_Flash_AddrCheck(WriteAddress),pData,len);
			}
		}
	WriteAddress+=len;
 	if(pHead->Len)
 		{
 		WriteAddress=WriteAddressStart;
 		DF_PicParameter.Number++;
 		DF_PicParameter.LastPic.Data_ID++;
		DF_PicParameter.LastPic.Address=Cam_Flash_AddrCheck(WriteAddressStart);
		DF_PicParameter.LastPic.Len=pHead->Len;
		if(DF_PicParameter.Number==1)
			{
			DF_PicParameter.FirstPic=DF_PicParameter.LastPic;
			}
		
		memcpy(pHead->Head,CAM_HEAD,strlen(CAM_HEAD));
		pHead->Data_ID=DF_PicParameter.LastPic.Data_ID;
		pHead->Media_Style=0;
		pHead->Media_Format=0;
		sst25_write_through(Cam_Flash_AddrCheck(WriteAddressStart),(u8 *)pHead,sizeof(TypeDF_PackageHead));
 		}
	return RT_EOK;
}



/*********************************************************************************
*函数名称:u32 Cam_Flash_FindPicID(u32 id,TypeDF_PackageHead *p_head)
*功能描述:从FLASH中查找指定ID的图片，并将图片包头新新存储在p_head中
*输	入:	id:多媒体ID号
		p_head:输出参数，输出为图片多媒体数据包头信息(类型为TypeDF_PackageHead)
*输	出:p_head 
*返 回 值:u32 0xFFFFFFFF表示没有找到图片，其它表示找到了图片，返回值为图片在flash中的地址
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u32 Cam_Flash_FindPicID(u32 id,TypeDF_PackageHead *p_head)
{
	u32 i;
	u32 TempAddress;
	
	static TypeDF_PackageInfo lastPackInfo={0xFFFFFFFF,0,0xFFFFFFFF};
	static TypeDF_PackageHead TempPackageHead;
	
	if(DF_PicParameter.Number==0)
		return 0xFFFFFFFF;
	if((id<DF_PicParameter.FirstPic.Data_ID)||(id>DF_PicParameter.LastPic.Data_ID))
		return 0xFFFFFFFF;
	if(id!=lastPackInfo.Data_ID)
		{
		lastPackInfo.Data_ID=0xFFFFFFFF;
		TempAddress=DF_PicParameter.FirstPic.Address;
		for(i=0;i<DF_PicParameter.Number;i++)
			{
			TempAddress=Cam_Flash_AddrCheck(TempAddress);
			sst25_read(TempAddress,(u8 *)&TempPackageHead,sizeof(TypeDF_PackageHead));
			if(strncmp(TempPackageHead.Head,CAM_HEAD,strlen(CAM_HEAD))==0)
				{
				///查看该图片是否被删除
				if(TempPackageHead.State & BIT(0))
					{
					if(TempPackageHead.Data_ID==id)
						{
						lastPackInfo.Data_ID=id;
						lastPackInfo.Address=TempAddress;
						lastPackInfo.Len=TempPackageHead.Len;
						memcpy((void *)p_head,(void *)&TempPackageHead,sizeof(TypeDF_PackageHead));
						return lastPackInfo.Address;
						}
					}
				TempAddress+=(TempPackageHead.Len+DF_CamSaveSect-1)/DF_CamSaveSect*DF_CamSaveSect;
				}
			else
				{
				return 0xFFFFFFFF;
				}
			}
		}
	else
		{
		memcpy((void *)p_head,(void *)&TempPackageHead,sizeof(TypeDF_PackageHead));
		return lastPackInfo.Address;
		}
	return 0xFFFFFFFF;
}




/*********************************************************************************
*函数名称:rt_err_t Cam_Flash_RdPic(void *pData,u16 *len, u32 id,u8 offset )
*功能描述:从FLASH中读取图片数据
*输	入:	pData:写入的数据指针，指向数据buf；
		len:返回的数据长度指针注意，该长度最大为512
		id:多媒体ID号
		offset:多媒体数据偏移量，从0开始，0表示读取多媒体图片包头信息，包头信息长度固定为64字节，采用
				结构体TypeDF_PackageHead格式存储
*输	出:none 
*返 回 值:re_err_t
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
rt_err_t Cam_Flash_RdPic_20130605(void *pData,u16 *len, u32 id,u8 offset )
{
	u16 i;
	u32 TempAddress;
	u32 temp_Len;
	TypeDF_PackageHead TempPackageHead;
	static TypeDF_PackageInfo lastPackInfo={0,0,0};

	*len=0;
	
	if(DF_PicParameter.Number==0)
		return RT_ERROR;
	if((id<DF_PicParameter.FirstPic.Data_ID)||(id>DF_PicParameter.LastPic.Data_ID))
		return RT_ERROR;
	if(id!=lastPackInfo.Data_ID)
		{
		lastPackInfo.Data_ID=0xFFFFFFFF;
		TempAddress=DF_PicParameter.FirstPic.Address;
		for(i=0;i<DF_PicParameter.Number;i++)
			{
			TempAddress=Cam_Flash_AddrCheck(TempAddress);
			sst25_read(TempAddress,(u8 *)&TempPackageHead,sizeof(TempPackageHead));
			if(strncmp(TempPackageHead.Head,CAM_HEAD,strlen(CAM_HEAD))==0)
				{
				if(TempPackageHead.Data_ID==id)
					{
					lastPackInfo.Data_ID=id;
					lastPackInfo.Address=TempAddress;
					lastPackInfo.Len=TempPackageHead.Len;
					break;
					}
				TempAddress+=(TempPackageHead.Len+DF_CamSaveSect-1)/DF_CamSaveSect*DF_CamSaveSect;
				}
			else
				{
				return RT_ERROR;
				}
			}
		}

	if(lastPackInfo.Data_ID == 0xFFFFFFFF)
		{
		return RT_ERROR;
		}
	if(offset>(lastPackInfo.Len-1)/512)
		{
		return RT_ENOMEM;
		}
	if(offset==(lastPackInfo.Len-1)/512)
		{
		temp_Len=lastPackInfo.Len-(offset*512);
		}
	else
		{
		temp_Len=512;
		}
	sst25_read(lastPackInfo.Address+offset*512,(u8 *)pData,temp_Len);
	*len=temp_Len;
	return RT_EOK;
}


/*********************************************************************************
*函数名称:rt_err_t Cam_Flash_RdPic(void *pData,u16 *len, u32 id,u8 offset )
*功能描述:从FLASH中读取图片数据
*输	入:	pData:写入的数据指针，指向数据buf；
		len:返回的数据长度指针注意，该长度最大为512
		id:多媒体ID号
		offset:多媒体数据偏移量，从0开始，0表示读取多媒体图片包头信息，包头信息长度固定为64字节，采用
				结构体TypeDF_PackageHead格式存储
*输	出:none 
*返 回 值:re_err_t
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
rt_err_t Cam_Flash_RdPic(void *pData,u16 *len, u32 id,u8 offset )
{
	u16 i;
	u32 TempAddress;
	u32 temp_Len;
	TypeDF_PackageHead TempPackageHead;
	
	*len=0;
	
	TempAddress=Cam_Flash_FindPicID(id,&TempPackageHead);
	if(TempAddress==0xFFFFFFFF)
		{
		return RT_ERROR;
		}
	if(TempPackageHead.Data_ID == 0xFFFFFFFF)
		{
		return RT_ERROR;
		}
	if(offset>(TempPackageHead.Len-1)/512)
		{
		return RT_ENOMEM;
		}
	if(offset==(TempPackageHead.Len-1)/512)
		{
		temp_Len=TempPackageHead.Len-(offset*512);
		}
	else
		{
		temp_Len=512;
		}
	sst25_read(TempAddress+offset*512,(u8 *)pData,temp_Len);
	*len=temp_Len;
	return RT_EOK;
}



/*********************************************************************************
*函数名称:rt_err_t Cam_Flash_DelPic(u32 id)
*功能描述:从FLASH中删除图片，实际上并没有删除，而是将删除标记清0，这样下次就再也不查询该图片
*输	入:	id:多媒体ID号
*输	出:none 
*返 回 值:re_err_t
*作	者:白养民
*创建日期:2013-06-13
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
rt_err_t Cam_Flash_DelPic(u32 id )
{
	u32 TempAddress;
	TypeDF_PackageHead TempPackageHead;
	
	TempAddress=Cam_Flash_FindPicID(id,&TempPackageHead);
	if(TempAddress==0xFFFFFFFF)
		{
		return RT_ERROR;
		}
	TempPackageHead.State &= ~(BIT(0));
	sst25_write_through(TempAddress,(u8 *)&TempPackageHead,sizeof(TypeDF_PackageHead));
	return RT_EOK;
}

/*********************************************************************************
*函数名称:rt_err_t Cam_Flash_TransOkSet(u32 id)
*功能描述:将该ID的图片标记为发送成功
*输	入:	id:多媒体ID号
*输	出:none 
*返 回 值:re_err_t
*作	者:白养民
*创建日期:2013-06-13
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
rt_err_t Cam_Flash_TransOkSet(u32 id )
{
	u32 TempAddress;
	TypeDF_PackageHead TempPackageHead;
	
	TempAddress=Cam_Flash_FindPicID(id,&TempPackageHead);
	if(TempAddress==0xFFFFFFFF)
		{
		return RT_ERROR;
		}
	TempPackageHead.State &= ~(BIT(1));
	sst25_write_through(TempAddress,(u8 *)&TempPackageHead,sizeof(TypeDF_PackageHead));
	return RT_EOK;
}


/*********************************************************************************
*函数名称:u16 Cam_Flash_SearchPic(T_TIMES *start_time,T_TIMES *end_time,Style_Cam_Requset_Para *para,u8 *psrc)
*功能描述:从FLASH中查找指定时间段的图片索引
*输	入:	start_time	开始时间，
		end_time		结束时间，
		para			查找图片的属性
		para			存储查找到得图片的位置，
*输	出: u16 类型，表示查找到得图片数量 
*返 回 值:re_err_t
*作	者:白养民
*创建日期:2013-06-5
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u16 Cam_Flash_SearchPic(T_TIMES *start_time,T_TIMES *end_time,TypeDF_PackageHead *para,u8 *pdest)
{
	u16 i;
	u32 TempAddress;
	u32 temp_u32;
	u32 temp_Len;
	TypeDF_PackageHead TempPackageHead;
	u16 ret_num=0;
	
	if(DF_PicParameter.Number==0)
		return 0;

	TempAddress=DF_PicParameter.FirstPic.Address;
	for(i=0;i<DF_PicParameter.Number;i++)
		{
		TempAddress=Cam_Flash_AddrCheck(TempAddress);
		sst25_read(TempAddress,(u8 *)&TempPackageHead,sizeof(TempPackageHead));
		if(strncmp(TempPackageHead.Head,CAM_HEAD,strlen(CAM_HEAD))==0)
			{
			///查看该图片是否被删除
			if(TempPackageHead.State & BIT(0))
				{
				///比较多媒体类型，多媒体通道，多媒体触发源
				if((TempPackageHead.Media_Style==para->Media_Style)&&((TempPackageHead.Channel_ID==para->Channel_ID)||(para->Channel_ID==0))&&((TempPackageHead.TiggerStyle==para->TiggerStyle)||(para->TiggerStyle==0xFF)))
					{
					temp_u32=Times_To_LongInt(&TempPackageHead.Time);
					///比较时间是否在范围
					if((temp_u32>Times_To_LongInt(start_time))&&(temp_u32<=Times_To_LongInt(end_time)))
						{
						///找到了数据
						data_to_buf(pdest,TempAddress,4);
						//memcpy(pdest,&TempAddress,4);
						pdest+=4;
						ret_num++;
						}
					}
				}
			TempAddress+=(TempPackageHead.Len+DF_CamSaveSect-1)/DF_CamSaveSect*DF_CamSaveSect;
			}
		else
			{
			return ret_num;
			}
		}
	
	return ret_num;
}

/*********************************************************************************
*函数名称:static rt_err_t Cam_Device_open( void )
*功能描述:打开模块供电
*输	入:none
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static rt_err_t Cam_Device_open( void )
{
	Power_485CH1_ON;
	return RT_EOK;
}


/*********************************************************************************
*函数名称:static rt_err_t Cam_Devic_close( void )
*功能描述:关闭模块供电
*输	入:none
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static rt_err_t Cam_Devic_close( void )
{
	Power_485CH1_OFF;
	return RT_EOK;
}




/*********************************************************************************
*函数名称:void Cam_Device_init( void )
*功能描述:初始化CAM模块相关接口
*输	入:none
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
void Cam_Device_init( void )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	///开电
	Power_485CH1_ON;

	///初始化消息队列
	rt_mq_init( &mq_Cam, "mq_cam", &msgpool_cam[0], sizeof(Style_Cam_Requset_Para), sizeof(msgpool_cam), RT_IPC_FLAG_FIFO );

	///初始化flash参数
	Cam_Flash_InitPara();

	///初始化照相状态参数
	Current_Cam_Para.Retry=0;
	Current_Cam_Para.State=CAM_IDLE;
}

/*********************************************************************************
*函数名称:static void Cam_Start_Cmd(u16 Cam_ID)
*功能描述:向camera模块发送开始拍照指令
*输	入:Cam_ID	需要拍照的camera设备ID
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static void Cam_Start_Cmd(u16 Cam_ID)
{
 	u8 Take_photo[10]= { 0x40, 0x40, 0x61, 0x81, 0x02, 0X00, 0X00, 0X02, 0X0D, 0X0A };	 //----  报警拍照命令
 	Take_photo[4]=(u8)Cam_ID;
 	Take_photo[5]=(u8)(Cam_ID>>8);
	RS485_write(Take_photo,10);
	uart2_rxbuf_rd=uart2_rxbuf_wr;
	rt_kprintf("\r\n发送拍照命令");
}

/*********************************************************************************
*函数名称:static void Cam_Read_Cmd(u16 Cam_ID)
*功能描述:向camera模块发送读取照片数据指令
*输	入:Cam_ID	需要拍照的camera设备ID
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static void Cam_Read_Cmd(u16 Cam_ID)
{
	uint8_t Fectch_photo[10] = { 0x40, 0x40, 0x62, 0x81, 0x02, 0X00, 0XFF, 0XFF, 0X0D, 0X0A };  //----- 报警取图命令
	Fectch_photo[4]=(u8)Cam_ID;
 	Fectch_photo[5]=(u8)(Cam_ID>>8);
	RS485_write(Fectch_photo,10);
	uart2_rxbuf_rd=uart2_rxbuf_wr;
	rt_kprintf("\r\n读取照片命令");
}


/*********************************************************************************
*函数名称:rt_err_t take_pic_request( Style_Cam_Requset_Para *para)
*功能描述:请求拍照指令
*输	入:para拍照参数
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
rt_err_t take_pic_request( Style_Cam_Requset_Para *para)
{
 return rt_mq_send(&mq_Cam,(void *) para,sizeof(Style_Cam_Requset_Para));
}


void takepic(u16 id)
{
 Style_Cam_Requset_Para tempPara;
 memset(&tempPara,0,sizeof(tempPara));
 tempPara.Channel_ID=id;
 tempPara.PhoteSpace=0;
 tempPara.PhotoTotal=1;
 tempPara.SavePhoto=1;
 tempPara.TiggerStyle=Cam_TRIGGER_PLANTFORM;
 take_pic_request(&tempPara);
 rt_kprintf("\r\n请求拍照ID=%d",id);
}
FINSH_FUNCTION_EXPORT( takepic, takepic);

void readpic(u16 id)
{
 u16 i;
 u16 len;
 u8 *pbuf;
 Style_Cam_Requset_Para tempPara;
 memset(&tempPara,0,sizeof(tempPara));
 tempPara.Channel_ID=id;
 tempPara.PhoteSpace=0;
 tempPara.PhotoTotal=1;
 tempPara.SavePhoto=1;
 tempPara.TiggerStyle=0;
 pbuf=rt_malloc(1024);
 rt_kprintf("\r\n拍照数据:\r\n");
 for(i=0;i<100;i++)
 	{
 	if(RT_EOK==Cam_Flash_RdPic(pbuf,&len,id,i))
 		{
		Hex_To_Ascii(pbuf,CamTestBuf,len);
		rt_kprintf(CamTestBuf);
 		}
	else
		{
		break;
		}
 	}
 rt_free(pbuf);
}
FINSH_FUNCTION_EXPORT( readpic, readpic);


void getpicpara(void)
{
 
 rt_kprintf("\r\n pic Number=%d \r\n Data_ID_1=%d \r\n Address_1=%d \r\n Len_1   =%d \r\n Data_ID_2=%d \r\n Address_2=%d \r\n Len_2   =%d",
 			DF_PicParameter.Number,
 			DF_PicParameter.FirstPic.Data_ID,
 			DF_PicParameter.FirstPic.Address,
 			DF_PicParameter.FirstPic.Len,
 			DF_PicParameter.LastPic.Data_ID,
 			DF_PicParameter.LastPic.Address,
 			DF_PicParameter.LastPic.Len);
}
FINSH_FUNCTION_EXPORT( getpicpara, getpicpara);




static void delay_us( const uint32_t usec )
{
	__IO uint32_t	count	= 0;
	const uint32_t	utime	= ( 168 * usec / 7 );
	do
	{
		if( ++count > utime )
		{
			return;
		}
	}
	while( 1 );
}


static bool Cam_Check_Para(Style_Cam_Requset_Para *para)
{
 if((para->Channel_ID<=15)||(para->Channel_ID==0xFFFF))
 	{
 	if(para->PhotoNum < para->PhotoTotal)
		return  true;
 	}
 return false;
}


/*********************************************************************************
*函数名称:bool Camera_RX_Data(u16 *RxLen)
*功能描述:拍照接收数据处理
*输	入:RxLen:表示接收到得数据长度指针
*输	出:RxLen:表示接收到得数据长度指针 
*返 回 值:0表示接收进行中,1表示接收成功
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static u8 Camera_RX_Data(u16 *RxLen)
{
	u8 ch;
	static u16 page_size=0;
	static u16 cam_rx_head_wr=0;
	
	/*串口是否收到数据*/
	while( RS485_read_char(&ch) )
	{
		switch(Current_Cam_Para.Rx_State)
		{
			case RX_DATA: /*保存信息格式: 位置(2B) 大小(2B) FLAG_FF 数据 0D 0A*/
				uart2_rx[uart2_rx_wr++] = ch;
				uart2_rx_wr 			%= UART2_RX_SIZE;
				if(uart2_rx_wr==page_size) Current_Cam_Para.Rx_State=RX_FCS;
				break;	
			case RX_IDLE:
				if(ch==0x40) Current_Cam_Para.Rx_State=RX_SYNC1;
				break;
			case RX_SYNC1:
				if(ch==0x40) Current_Cam_Para.Rx_State=RX_SYNC2;
				else Current_Cam_Para.Rx_State=RX_IDLE;
				break;
			case RX_SYNC2:
				if(ch==0x63)
				{
					cam_rx_head_wr=0;
					Current_Cam_Para.Rx_State=RX_HEAD;
				}	
				else 
					Current_Cam_Para.Rx_State=RX_IDLE;
				break;
			case RX_HEAD:
				uart2_rx[cam_rx_head_wr++]=ch;
				if(cam_rx_head_wr==5)
				{
					uart2_rx_wr=0;
					page_size=(uart2_rx[3]<<8)|uart2_rx[2];
					Current_Cam_Para.Rx_State=RX_DATA;
				}
				break;
			case RX_FCS:
				Current_Cam_Para.Rx_State=RX_0D;
				break;		
			case RX_0D:
				if(ch==0x0d) Current_Cam_Para.Rx_State=RX_0A;
				else Current_Cam_Para.Rx_State=RX_IDLE;
				break;	
			case RX_0A:
				Current_Cam_Para.Rx_State=RX_IDLE;
				if(ch==0x0a) return 1;
				break;	
		}
	}
	return 0;
}




/*********************************************************************************
*函数名称:void Camera_Process(void)
*功能描述:进行照相相关处理(包括有:拍照，存储照片，发送拍照结束指令给808)
*输	入:none
*输	出:none 
*返 回 值:none
*作	者:白养民
*创建日期:2013-06-3
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
u8 Camera_Process(void)
{
 u16 RxLen;
 static u16 cam_photo_size;
 static u8 cam_page_num=0;
 u8 cam_last_page=0;
 static uint32_t tick;
 static TypeDF_PackageHead pack_head;
 
 switch(Current_Cam_Para.State)
	{
	case CAM_IDLE:
		{
			if(RT_EOK == rt_mq_recv(&mq_Cam,(void *)&Current_Cam_Para.Para,sizeof(Style_Cam_Requset_Para),RT_WAITING_NO))
				{
				Current_Cam_Para.Retry=0;
				Current_Cam_Para.State=CAM_START;
				rt_kprintf("\r\n收到拍照消息");
				}
			else
				{
				return 0;
				}
		}
	case CAM_START:
		{
			if(Current_Cam_Para.Retry>=3)
				{
				Current_Cam_Para.State=CAM_FALSE;
				break;
				}
			Current_Cam_Para.Retry++;
			memset(&pack_head,0,sizeof(pack_head));
			cam_page_num=0;
			cam_photo_size=0;
			tick=rt_tick_get();
			Cam_Start_Cmd(Current_Cam_Para.Para.Channel_ID);
			Current_Cam_Para.State=CAM_RX_PHOTO;
			Current_Cam_Para.Rx_State=RX_IDLE;
			break;
		}
	case CAM_GET_PHOTO:
		{
			tick=rt_tick_get();
			Cam_Read_Cmd(Current_Cam_Para.Para.Channel_ID);
			Current_Cam_Para.State=CAM_RX_PHOTO;
			Current_Cam_Para.Rx_State=RX_IDLE;
			break;
		}
	case CAM_RX_PHOTO:
		{
			if(1==Camera_RX_Data(&RxLen))
			{
				rt_kprintf("\r\n接收到拍照数据!");
				tick=rt_tick_get();
				///收到数据,存储,判断是否图片结束
				if(uart2_rx_wr>512)		///数据大于512,非法
				{
					rt_kprintf("\r\nCAM%d invalided\r\n",Current_Cam_Para.Para.Channel_ID);
					Current_Cam_Para.State=CAM_START;
					break;
				}
				if(uart2_rx_wr==512)
				{
					if((uart2_rx[510]==0xff)&&(uart2_rx[511]==0xD9))
					{
						cam_last_page=1;
					}
				}
				else
				{
					cam_last_page=1;
				}
				
				cam_page_num++;
				cam_photo_size+=uart2_rx_wr;
				///第一包数据，需要填写参数，然后存储才正确
				if(cam_page_num==1)
				{
					pack_head.Channel_ID=Current_Cam_Para.Para.Channel_ID;
					pack_head.TiggerStyle=Current_Cam_Para.Para.TiggerStyle;
					pack_head.Media_Format=0;
					pack_head.Media_Style=0;
					
					pack_head.Time.years=0x13;
					pack_head.Time.months=0x06;
					pack_head.Time.days=tick>>24;
					pack_head.Time.hours=tick>>16;
					pack_head.Time.minutes=tick>>8;
					pack_head.Time.seconds=tick;
					pack_head.State=0xFF;
					if(Current_Cam_Para.Para.SavePhoto)
						{
						pack_head.State &= ~(BIT(2));
						}
				}
				///最后一包数据，需要将长度写入。
				if(cam_last_page)
				{
					pack_head.Len=cam_photo_size+64;
					Current_Cam_Para.State=CAM_END;
				}
				else
				{
					Current_Cam_Para.State=CAM_GET_PHOTO;
				}
				///保存数据
				Cam_Flash_WrPic(uart2_rx,uart2_rx_wr,&pack_head);
				Hex_To_Ascii(uart2_rx,CamTestBuf,uart2_rx_wr);
				rt_kprintf("\r\n拍照数据:\r\n");
				rt_kprintf("%s\r\n",CamTestBuf);
				rt_kprintf("\r\n%d>CAM%d photo %d bytes",rt_tick_get()*10,Current_Cam_Para.Para.Channel_ID,cam_photo_size);
				uart2_rx_wr=0;
			}
			else if(rt_tick_get()-tick>RT_TICK_PER_SECOND*2)	///判读是否超时
			{
				Current_Cam_Para.State=CAM_START;
			}
			break;
		}
	case CAM_END:
		{
			rt_kprintf("\r\n拍照成功!");
			getpicpara();
			
			Current_Cam_Para.State=CAM_IDLE;
			break;
		}
	case CAM_FALSE:
		{
			rt_kprintf("\r\n拍照失败!");
			Current_Cam_Para.State=CAM_IDLE;
			break;
		}
	}
 return 1;
}


/*********************************************************************************
*函数名称:rt_err_t Cam_add_tx_pic_timeout( JT808_TX_MSG_NODEDATA * nodedata )
*功能描述:发送图片相关数据的超时处理函数
*输 入:	nodedata	:正在处理的发送链表
*输 出: none
*返 回 值:rt_err_t
*作 者:白养民
*创建日期:2013-06-16
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static rt_err_t Cam_add_tx_pic_timeout( JT808_TX_MSG_NODEDATA * nodedata )
{
 rt_kprintf("\r\n Cam_tx_timeout");
}


/*********************************************************************************
*函数名称:u16 Cam_add_tx_pic_getdata( JT808_TX_MSG_NODEDATA * nodedata )
*功能描述:在jt808中处理照片数据传输ACK_ok函数，该函数是 JT808_TX_MSG_NODEDATA 的 cb_tx_response 回调函数
*输 入:	nodedata	:正在处理的发送链表
*输 出: none
*返 回 值:rt_err_t
*作 者:白养民
*创建日期:2013-06-16
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static rt_err_t Cam_add_tx_pic_response( JT808_TX_MSG_NODEDATA * nodedata ,uint8_t linkno, uint8_t *pmsg )
{
	JT808_TX_MSG_NODEDATA	* iterdata=nodedata;
	TypePicMultTransPara	* p_para;
	uint16_t				i,pack_num;
	uint16_t				body_len; /*消息体长度*/
	uint8_t					* msg;
	uint32_t				tempu32data;

	body_len	= ( ( *( pmsg + 2 ) << 8 ) | ( *( pmsg + 3 ) ) ) & 0x3FF;
	msg			= pmsg + 12;

	tempu32data=buf_to_data(msg,4);
	msg+=4;
	p_para = (TypePicMultTransPara*)(iterdata->user_para);
	if(tempu32data==p_para->Data_ID)
		{
		memset(p_para->Pack_Mark,0,sizeof(p_para->Pack_Mark));
		if(body_len>=7)
			{
			pack_num=*msg++;
			for(i=0;i<pack_num;i++)
				{
				tempu32data=buf_to_data(msg,2);
				if(tempu32data)
					tempu32data--;
				msg+=2;
				p_para->Pack_Mark[tempu32data/8] |= BIT(tempu32data%8);
				}
			iterdata->state=GET_DATA;
			rt_kprintf("\r\n Cam_add_tx_pic_response\r\n lost_pack=%d",pack_num);
			}
		else
			{
			iterdata->state=ACK_OK;
			if(p_para->Delete)
				{
				//Cam_Flash_DelPic(p_para->Data_ID);
				}
			rt_kprintf("\r\n Cam_add_tx_pic_response_ok!");
			}
		}
}


/*********************************************************************************
*函数名称:u16 Cam_add_tx_pic_getdata( JT808_TX_MSG_NODEDATA * nodedata )
*功能描述:在jt808_tx_proc当状态为GET_DATA时获取照片数据，该函数是 JT808_TX_MSG_NODEDATA 的 get_data 回调函数
*输 入:	nodedata	:正在处理的发送链表
*输 出: none
*返 回 值:rt_err_t
*作 者:白养民
*创建日期:2013-06-16
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
static u16 Cam_add_tx_pic_getdata( JT808_TX_MSG_NODEDATA * nodedata )
{
	JT808_TX_MSG_NODEDATA	* iterdata=nodedata;
	TypePicMultTransPara	* p_para = (TypePicMultTransPara *)nodedata->user_para;
	TypeDF_PackageHead 		TempPackageHead;
	uint16_t				i,wrlen,pack_num;
	uint16_t				body_len; /*消息体长度*/
	uint8_t					* msg;
	uint32_t				tempu32data;

	//释放原来的资源
	rt_free(iterdata->pmsg);
	iterdata->pmsg=RT_NULL;
	iterdata->msg_len = 0;
	//查找当前图片是否存在
	tempu32data=Cam_Flash_FindPicID(p_para->Data_ID,&TempPackageHead);
	if(tempu32data == 0xFFFFFFFF)
		{
		return 0xFFFF;
		}
	for(i=0;i<iterdata->packet_num;i++)
		{
		if(p_para->Pack_Mark[i/8] & BIT(i%8))
			{
			if(i+1 < iterdata->packet_num)
				{
				body_len = JT808_PACKAGE_MAX;
				}
			else
				{
				body_len = iterdata->size - JT808_PACKAGE_MAX * i;
				}
			iterdata->pmsg = rt_malloc(body_len);
			iterdata->msg_len = body_len;
			if( RT_NULL == iterdata->pmsg )
				{
				rt_kprintf("\r\n Cam_add_tx_pic_getdata rt_malloc error!");
				return 0;
				}
			if(i==0)
				{
				sst25_read(tempu32data,(u8 *)&TempPackageHead,sizeof(TypeDF_PackageHead));
				iterdata->pmsg[0];
				wrlen=0;
				wrlen+=data_to_buf(iterdata->pmsg+wrlen,TempPackageHead.Data_ID,4);			///多媒体ID
				wrlen+=data_to_buf(iterdata->pmsg+wrlen,TempPackageHead.Media_Style,1);		///多媒体类型
				wrlen+=data_to_buf(iterdata->pmsg+wrlen,TempPackageHead.Media_Format,1);	///多媒体格式编码
				wrlen+=data_to_buf(iterdata->pmsg+wrlen,TempPackageHead.TiggerStyle,1);		///事件项编码 
				wrlen+=data_to_buf(iterdata->pmsg+wrlen,TempPackageHead.Channel_ID,1);		///通道 ID
				memcpy(iterdata->pmsg+wrlen,TempPackageHead.position,28);					///位置信息汇报
				wrlen+=28;
				sst25_read(tempu32data+64,iterdata->pmsg+wrlen,body_len-wrlen);
				}
			else
				{
				tempu32data = tempu32data + JT808_PACKAGE_MAX * i + 64 - 36;
				sst25_read(tempu32data,iterdata->pmsg,body_len);
				}
			p_para->Pack_Mark[i/8] &= ~(BIT(i%8));
			iterdata->packet_no=i+1;
			rt_kprintf("\r\n cam_get_data ok\r\n PAGE=%d",iterdata->packet_no);
			return iterdata->packet_no;
			}
		}
	rt_kprintf("\r\n cam_get_data_false!");
	return 0;
}


/*********************************************************************************
*函数名称:rt_err_t Cam_add_tx_pic(u32 mdeia_id ,u8 media_delete)
*功能描述:添加一个多媒体图片到发送列表中
*输 入:	mdeia_id	:照片id
		media_delete:照片传送结束后是否删除标记，非0表示删除
*输 出: none
*返 回 值:rt_err_t
*作 者:白养民
*创建日期:2013-06-16
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
rt_err_t Cam_add_tx_pic(u32 mdeia_id ,u8 media_delete)
{
	u16 i;
	u32 TempAddress;
	JT808_TX_MSG_NODEDATA	* pnodedata;
	TypePicMultTransPara	* p_para;
	TypeDF_PackageHead TempPackageHead;

	
	TempAddress=Cam_Flash_FindPicID(mdeia_id,&TempPackageHead);
	if(TempAddress==0xFFFFFFFF)
		{
		return RT_ERROR;
		}
	p_para=rt_malloc( sizeof( TypePicMultTransPara ) );
	if( p_para == NULL )
		{
		return RT_ERROR;
		}
	memset(p_para,0,sizeof(TypePicMultTransPara));
	
	pnodedata = rt_malloc( sizeof( JT808_TX_MSG_NODEDATA ) );
	if( pnodedata == NULL )
		{
		rt_free(p_para);
		return RT_ERROR;
		}
	memset(pnodedata,0,sizeof(JT808_TX_MSG_NODEDATA));
	
	pnodedata->linkno			= 1;			
	pnodedata->multipacket		= 1;		//表示为多包发送
	pnodedata->type				= TERMINAL_CMD;
	pnodedata->state			= GET_DATA;
	pnodedata->retry			= 0;
	//pnodedata->max_retry		= 3;
	pnodedata->tick				= rt_tick_get();
	pnodedata->cb_tx_timeout	= Cam_add_tx_pic_timeout;
	pnodedata->cb_tx_response	= Cam_add_tx_pic_response;
	pnodedata->head_id			= 0x801;
	pnodedata->head_sn			= 0x00;

	pnodedata->size				= TempPackageHead.Len-64+36;
	pnodedata->packet_num		= (pnodedata->size/JT808_PACKAGE_MAX);
	if(pnodedata->size%JT808_PACKAGE_MAX)
		pnodedata->packet_num++;
	pnodedata->packet_no		= 0;

	///填充用户区数据
	
	p_para->Address	= TempAddress;
	p_para->Data_ID	= mdeia_id;
	p_para->Delete	= media_delete;
	for(i=0;i<pnodedata->packet_num;i++)
		{
		p_para->Pack_Mark[i/8] |= BIT(i%8);
		}
	pnodedata->user_para		= (void *)p_para;
	pnodedata->get_data			= Cam_add_tx_pic_getdata;
	msglist_append( list_jt808_tx, pnodedata );
	rt_kprintf("\r\n Cam_add_tx_pic_ok!!!");
	return RT_EOK;
}

FINSH_FUNCTION_EXPORT( Cam_add_tx_pic, Cam_add_tx_pic);


/*********************************************************************************
*函数名称:void Cam_jt808_0x800(TypeDF_PackageHead *phead)
*功能描述:多媒体事件信息上传_发送照片多媒体信息
*输	入:	phead	:照片信息信息
*输	出:	none
*返 回 值:rt_err_t
*作	者:白养民
*创建日期:2013-06-16
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
rt_err_t Cam_jt808_0x800(TypeDF_PackageHead *phead)
{
 u8 ptempbuf[32];
 u16 datalen=0;
 datalen+=data_to_buf(ptempbuf+datalen,phead->Data_ID,4);
 datalen+=data_to_buf(ptempbuf+datalen,phead->Media_Style,1);
 datalen+=data_to_buf(ptempbuf+datalen,phead->Media_Format,1);
 datalen+=data_to_buf(ptempbuf+datalen,phead->TiggerStyle,1);
 datalen+=data_to_buf(ptempbuf+datalen,phead->Channel_ID,1);
 return jt808_add_tx_data(1,TERMINAL_ACK,0x800,ptempbuf,datalen);
}

/*********************************************************************************
*函数名称:rt_err_t Cam_jt808_0x8802(uint16_t fram_num,uint8_t *pmsg,u16 msg_len)
*功能描述:存储多媒体数据检索 
*输	入:	fram_num:应答流水号
		pmsg	:808消息体数据
		msg_len	:808消息体长度
*输	出:none
*返 回 值:rt_err_t
*作	者:白养民
*创建日期:2013-06-16
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
rt_err_t Cam_jt808_0x8802(uint16_t fram_num,uint8_t *pmsg,u16 msg_len)
{
 u8 *ptempbuf;
 u8 *pdestbuf;
 u16 datalen = 0;
 u16 i,mediatotal;
 TypeDF_PackageHead TempPackageHead;
 u32 TempAddress;
 rt_err_t ret;
 if(pmsg[0])
 	{
 	return RT_ERROR;
 	}
 if(DF_PicParameter.Number==0)
 	{
 	return RT_ERROR;
 	}
 
 ptempbuf = rt_malloc(DF_PicParameter.Number*4);
 if( ptempbuf == NULL )
	{
	return RT_ERROR;
	}
 
 memset(&TempPackageHead,0,sizeof(Style_Cam_Requset_Para));
 TempPackageHead.Media_Format	= 0;
 TempPackageHead.Media_Style	= 0;
 TempPackageHead.Channel_ID		= pmsg[1];
 TempPackageHead.TiggerStyle	= pmsg[2];
 ///查找符合条件的图片，并将图片地址存入ptempbuf中
 mediatotal=Cam_Flash_SearchPic((T_TIMES *)(pmsg+3),(T_TIMES *)(pmsg+9),&TempPackageHead,ptempbuf);
 
 if(mediatotal > (JT808_PACKAGE_MAX-4)/35)
 	{
 	mediatotal = (JT808_PACKAGE_MAX-4)/35;
 	}
 
 pdestbuf = rt_malloc(mediatotal*35+4);
 if( pdestbuf == NULL )
	{
	rt_free(ptempbuf);
	return RT_ERROR;
	}
 
 datalen+=data_to_buf(pdestbuf+datalen,fram_num,2);
 datalen+=data_to_buf(pdestbuf+datalen,mediatotal,2);
 for(i=0;i<mediatotal;i++)
 	{
 	TempAddress=buf_to_data(ptempbuf,4);
	ptempbuf+=4;
	sst25_read(TempAddress,(u8 *)&TempPackageHead,sizeof(TempPackageHead));
	datalen+=data_to_buf(pdestbuf+datalen,TempPackageHead.Data_ID,4);
	datalen+=data_to_buf(pdestbuf+datalen,TempPackageHead.Media_Style,1);
	datalen+=data_to_buf(pdestbuf+datalen,TempPackageHead.Channel_ID ,1);
	datalen+=data_to_buf(pdestbuf+datalen,TempPackageHead.TiggerStyle,1);
	memcpy(pdestbuf+datalen,TempPackageHead.position,28);					///位置信息汇报
	datalen+=28;
 	}
 ret = jt808_add_tx_data(1,TERMINAL_ACK,0x802,pdestbuf,datalen);
 
 rt_free(ptempbuf);
 rt_free(pdestbuf);
 return ret;
}


/*********************************************************************************
*函数名称:rt_err_t Cam_jt808_0x8803(uint8_t *pmsg,u16 msg_len)
*功能描述:存储多媒体数据上传
*输	入:	pmsg	:808消息体数据
		msg_len	:808消息体长度
*输	出:none
*返 回 值:rt_err_t
*作	者:白养民
*创建日期:2013-06-16
*---------------------------------------------------------------------------------
*修 改 人:
*修改日期:
*修改描述:
*********************************************************************************/
rt_err_t Cam_jt808_0x8803(uint8_t *pmsg,u16 msg_len)
{
 u8 media_delete = 0;
 u8 *ptempbuf;
 u16 datalen = 0;
 u16 i,mediatotal;
 TypeDF_PackageHead TempPackageHead;
 u32 TempAddress;
 rt_err_t ret;
 if(pmsg[0])
	{
	return RT_ERROR;
	}
 if(DF_PicParameter.Number==0)
	{
	return RT_ERROR;
	}
 
 ptempbuf = rt_malloc(DF_PicParameter.Number*4);
 if( ptempbuf == NULL )
	{
	return RT_ERROR;
	}
 
 memset(&TempPackageHead,0,sizeof(Style_Cam_Requset_Para));
 TempPackageHead.Media_Format	= 0;
 TempPackageHead.Media_Style	= 0;
 TempPackageHead.Channel_ID 	= pmsg[1];
 TempPackageHead.TiggerStyle	= pmsg[2];
 media_delete					= pmsg[15];
 ///查找符合条件的图片，并将图片地址存入ptempbuf中
 mediatotal=Cam_Flash_SearchPic((T_TIMES *)(pmsg+3),(T_TIMES *)(pmsg+9),&TempPackageHead,ptempbuf);

 for(i=0;i<mediatotal;i++)
	{
	TempAddress=buf_to_data(ptempbuf,4);
	ptempbuf+=4;
	sst25_read(TempAddress,(u8 *)&TempPackageHead,sizeof(TempPackageHead));
	Cam_add_tx_pic(TempPackageHead.Data_ID ,media_delete);
	}
 rt_free(ptempbuf);
 return RT_EOK;
}
/************************************** The End Of File **************************************/
