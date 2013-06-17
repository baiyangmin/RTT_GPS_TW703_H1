#ifndef _CAMERAPRO_H_
#define _CAMERAPRO_H_


//#include "uffs_types.h"


#define nop 
#ifndef BIT
#define BIT(i) ((unsigned long)(1<<i))
#endif


typedef __packed struct
{
 u8 years;
 u8 months;
 u8 days;
 u8 hours;
 u8 minutes;
 u8 seconds;
} T_TIMES;



typedef enum
{
	Cam_TRIGGER_PLANTFORM=0,	///平台下发
	Cam_TRIGGER_TIMER,			///定时动作
	Cam_TRIGGER_ROBBERY,		///抢劫报警
	Cam_TRIGGER_HIT,			///碰撞
	Cam_TRIGGER_OPENDOR,		///开门
	Cam_TRIGGER_CLOSEDOR,		///关门
	Cam_TRIGGER_LOWSPEED,		///低速超过20分钟
	Cam_TRIGGER_SPACE,			///定距离拍照
	Cam_TRIGGER_OTHER,			///其他
}Cam_Trigger;


typedef  __packed struct _Style_Cam_Requset_Para
{
	Cam_Trigger	TiggerStyle;		///触发拍照的信号源
	u16			Channel_ID;			///照相通道的ID号
	u16			PhotoTotal;			///需要拍照的总数量
	u16			PhotoNum;			///当前拍照成功的数量
	u16			PhoteSpace;			///拍照间隔，单位为妙
	u8			SendPhoto;			///拍照结束后是否发送，1表示发送，0表示不发送
	u8			SavePhoto;			///拍照结束后是否保存，1表示保存，0表示不保存
	void ( *cb_cam_response )( uint32_t pic_id, uint8_t *pmsg );
}Style_Cam_Requset_Para;




extern rt_err_t Cam_Flash_RdPic(void *pData,u16 *len, u32 id,u8 offset );
extern rt_err_t take_pic_request( Style_Cam_Requset_Para *para);
extern void 	Cam_Device_init( void );
extern u8 		Camera_Process(void);
#endif
