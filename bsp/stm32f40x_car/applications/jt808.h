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
#ifndef _H_JT808_H_
#define _H_JT808_H_

#include <stm32f4xx.h>
#include <rtthread.h>

#include "msglist.h"
#include "m66.h"

#define   MsgQ_Timeout 3


/*
   存储区域分配,采用绝对地址,以4K(0x1000)为一个扇区
 */

#define ADDR_PARAM 0x000000000

//------- 文本信息 --------
typedef struct _TEXT_INFO
{
	uint8_t TEXT_FLAG;          //  文本标志
	uint8_t TEXT_SD_FLAG;       // 发送标志位
	uint8_t TEXT_Content[100];  // 文本内容
}TEXT_INFO;

//----- 信息 ----
typedef struct _MSG_TEXT
{
	uint8_t TEXT_mOld;          //  最新的一条信息  写为1代表是最新的一条信息
	uint8_t TEXT_TYPE;          //  信息类型   1-8  中第几条
	uint8_t TEXT_LEN;           //  信息长度
	uint8_t TEXT_STR[150];      //  信息内容
}MSG_TEXT;

//-----  提问 ------
typedef struct _CENTER_ASK
{
	uint8_t		ASK_SdFlag;     //  标志位           发给 TTS  1  ；   TTS 回来  2
	uint16_t	ASK_floatID;    // 提问流水号
	uint8_t		ASK_infolen;    // 信息长度
	uint8_t		ASK_answerID;   // 回复ID
	uint8_t		ASK_info[30];   //  信息内容
	uint8_t		ASK_answer[30]; // 候选答案
}CENTRE_ASK;

//------ 事件  -------
typedef struct _EVENT           //  name: event
{
	uint8_t Event_ID;           //  事件ID
	uint8_t Event_Len;          //  事件长度
	uint8_t Event_Effective;    //  事件是否有效，   1 为要显示  0
	uint8_t Event_Str[20];      //  事件内容
}EVENT;

//----- 信息 ----
typedef struct _MSG_BROADCAST   // name: msg_broadcast
{
	uint8_t		INFO_TYPE;      //  信息类型
	uint16_t	INFO_LEN;       //  信息长度
	uint8_t		INFO_PlyCancel; // 点播/取消标志      0 取消  1  点播
	uint8_t		INFO_SDFlag;    //  发送标志位
	uint8_t		INFO_Effective; //  显示是否有效   1 显示有效    0  显示无效
	uint8_t		INFO_STR[30];   //  信息内容
}MSG_BRODCAST;

//------ 电话本 -----
typedef struct _PHONE_BOOK      // name: phonebook
{
	uint8_t CALL_TYPE;          // 呼入类型  1 呼入 2 呼出 3 呼入/呼出
	uint8_t NumLen;             // 号码长度
	uint8_t UserLen;            // 联系人长度
	uint8_t Effective_Flag;     // 有效标志位   无效 0 ，有效  1
	uint8_t NumberStr[20];      // 电话号码
	uint8_t UserStr[10];        // 联系人名称  GBK 编码
}PHONE_BOOK;

typedef struct _MULTIMEDIA
{
	u32 Media_ID;               //   多媒体数据ID
	u8	Media_Type;             //   0:   图像    1 : 音频    2:  视频
	u8	Media_CodeType;         //   编码格式  0 : JPEG  1:TIF  2:MP3  3:WAV  4: WMV
	u8	Event_Code;             //   事件编码  0: 平台下发指令  1: 定时动作  2 : 抢劫报警触发 3: 碰撞侧翻报警触发 其他保留
	u8	Media_Channel;          //   通道ID
	//----------------------
	u8	SD_Eventstate;          // 发送事件信息上传状态    0 表示空闲   1  表示处于发送状态
	u8	SD_media_Flag;          // 发送没提事件信息标志位
	u8	SD_Data_Flag;           // 发送数据标志位
	u8	SD_timer;               // 发送定时器
	u8	MaxSd_counter;          // 最大发送次数
	u8	Media_transmittingFlag; // 多媒体传输数据状态  1: 多媒体传输前发送1包定位信息    2 :多媒体数据传输中  0:  未进行多媒体数据传输
	u16 Media_totalPacketNum;   // 多媒体总包数
	u16 Media_currentPacketNum; // 多媒体当前报数
	//----------------------
	u8	RSD_State;              //  重传状态   0 : 重传没有启用   1 :  重传开始    2  : 表示顺序传完但是还没收到中心的重传命令
	u8	RSD_Timer;              //  传状态下的计数器
	u8	RSD_Reader;             //  重传计数器当前数值
	u8	RSD_total;              //  重传选项数目

	u8 Media_ReSdList[10];      //  多媒体重传消息列表
}MULTIMEDIA;

//--------------每分钟的平均速度
typedef  struct AvrgMintSpeed
{
	uint8_t datetime[6];        //current
	uint8_t datetime_Bak[6];    //Bak
	uint8_t avgrspd[60];
	uint8_t saveFlag;
}Avrg_MintSpeed;

extern TEXT_INFO	TextInfo;
//-------文本信息-------
extern MSG_TEXT		TEXT_Obj;
extern MSG_TEXT		TEXT_Obj_8[8], TEXT_Obj_8bak[8];

//------ 提问  --------
extern CENTRE_ASK ASK_Centre;                       // 中心提问

//------- 事件 ----
extern EVENT	EventObj;                           // 事件
extern EVENT	EventObj_8[8];                      // 事件

//------  信息点播  ---
extern MSG_BRODCAST MSG_BroadCast_Obj;              // 信息点播
extern MSG_BRODCAST MSG_Obj_8[8];                   // 信息点播

//------  电话本  -----
extern PHONE_BOOK		PhoneBook, Rx_PhoneBOOK;    //  电话本
extern PHONE_BOOK		PhoneBook_8[8];

extern MULTIMEDIA		MediaObj;                   // 多媒体信息

extern uint8_t			CarLoadState_Flag;          //选中车辆状态的标志   1:空车   2:半空   3:重车
extern uint8_t			Warn_Status[4];

extern u16				ISP_total_packnum;          // ISP  总包数
extern u16				ISP_current_packnum;        // ISP  当前包数

extern u8				APN_String[30];

extern u8				Camera_Number;
extern u8				Duomeiti_sdFlag;

extern Avrg_MintSpeed	Avrgspd_Mint;
extern u8				avgspd_Mint_Wr; // 填写每分钟平均速度记录下标

/*for new use*/

typedef struct
{
	int		id;
	short	attr;
	int		latitute;   /*以度位单位的纬度值乘以10的6次方，精确到百万分之一度*/
	int		longitute;
	int		radius;     /*单位为米m，路段为该拐点到下一拐点*/
	char	start[6];
	char	end[6];
	short	speed;
	char	interval;   /*持续时间,秒*/
}GPS_AREA_CIRCLE;

typedef enum
{
	T_NODEF = 1,
	T_BYTE,
	T_WORD,
	T_DWORD,
	T_STRING,
}PARAM_TYPE;

/*终端参数类型*/
typedef  struct
{
	uint8_t		id;
	PARAM_TYPE	type;
	void		* pvalue;
}PARAM;

/*终端参数类型*/
typedef __packed struct
{
	PARAM_TYPE	type;
	void		* pvalue;
}PARAM_BODY;

#if 0
typedef struct
{
	uint32_t ver; /*版本信息四个字节yy_mm_dd_build,比较大小*/
/*车辆注册信息*/
	uint16_t	id0100_1_w;
	uint16_t	id0100_2_w;
	uint8_t		id0100_3_s[5];
	uint8_t		id0100_4_s[8];
	uint8_t		id0100_5_s[7];
	uint8_t		id0100_6_b;
	uint8_t		id0100_7_s[12];

/*网络有关*/
	char	apn[32];
	char	user[32];
	char	psw[32];
	char	mobile[6];
/*传输相关*/
	uint32_t	timeout_udp;    /*udp传输超时时间*/
	uint32_t	retry_udp;      /*udp传输重传次数*/
	uint32_t	timeout_tcp;    /*udp传输超时时间*/
	uint32_t	retry_tcp;      /*udp传输重传次数*/
}JT808_PARAM;

#endif

typedef enum
{
	GET_DATA=1,					/*获取数据*/
	IDLE,						/*空闲等待发送*/
	WAIT_ACK,                   /*等待ACK中*/
	ACK_OK,                     /*已收到ACK应答*/
} JT808_MSG_STATE;

typedef enum
{
	TERMINAL_CMD = 1,
	TERMINAL_ACK,
	CENTER_CMD,
	CENTER_ACK
}JT808_MSG_TYPE;

typedef __packed struct
{
	uint16_t	id;
	uint16_t	attr;
	uint8_t		mobile[6];
	uint16_t	seq;
}JT808_MSG_HEAD;


/*
   存储jt808发送的相关信息
 */

typedef __packed struct _jt808_tx_msg_nodedata_old
{
/*发送机制相关*/
	uint8_t			linkno;                             /*传输使用的link,包括了协议和远端socket*/
	JT808_MSG_TYPE	type;                               /*发送消息的类型*/
	JT808_MSG_STATE state;                              /*发送状态*/
	uint32_t		retry;                              /*重传次数,递增，递减找不到*/
	uint32_t		max_retry;                          /*最大重传次数*/
	uint32_t		timeout;                            /*超时时间*/
	uint32_t		tick;                               /*发送时间*/
/*接收的处理判断相关*/
	void ( *cb_tx_timeout )( __packed struct _jt808_tx_msg_nodedata *pnodedata );
	void ( *cb_tx_response )( uint8_t linkno, uint8_t *pmsg );
	uint16_t	head_id;                                /*消息ID*/
	uint16_t	head_sn;                                /*消息流水号*/
/*真实的发送数据*/
	uint16_t	msg_len;                                /*消息长度*/
	uint8_t		*pmsg;                                  /*发送消息体,真实的要发送的数据格式，经过转义和FCS后的<7e>为标志*/
}JT808_TX_MSG_NODEDATA_OLD;

typedef __packed struct _jt808_tx_msg_nodedata
{
/*发送机制相关*/
	uint8_t			linkno;                             /*传输使用的link,包括了协议和远端socket*/
	uint8_t			multipacket;                        /*是不是多包发送*/
	JT808_MSG_TYPE	type;                               /*发送消息的类型*/
	JT808_MSG_STATE state;                              /*发送状态*/
	uint32_t		retry;                              /*重传次数,递增，递减找不到*/
	uint32_t		max_retry;                          /*最大重传次数*/
	uint32_t		timeout;                            /*超时时间*/
	uint32_t		tick;                               /*发送时间*/
/*接收的处理判断相关*/
	void ( *cb_tx_timeout )( struct _jt808_tx_msg_nodedata *pnodedata );
	void ( *cb_tx_response )( struct _jt808_tx_msg_nodedata *pnodedata ,uint8_t linkno, uint8_t *pmsg );
	uint16_t	head_id;                                /*消息ID*/
	uint16_t	head_sn;                                /*消息流水号*/
	
/*单包真实的发送数据-消息体*/
	uint16_t	msg_len;                                /*消息长度*/
	uint8_t		*pmsg;                                  /*原始信息,需要在发送时转义,因为多包发送时得到的是原始信息。
                                                       ，包括808转义和M66的HEX转义，这样，减少RAM使用*/
/*多包发送的处理*/
	//uint8_t		stage;                                  /*阶段*/
	uint16_t	packet_num;		/*总包数*/
	uint16_t	packet_no;		/*当前包数*/
	uint32_t	size;                                   /*总得数据大小*/
	void 		*user_para;								/*get_data函数需要的关键原始数据参数，通过该参数获取正确的数据*/
	//void		*user_data;								/*user_data,该数据区域表示需要发送的数据包列表，第一个字节的bit0为1表示需要发送该包数据，以此类推*/
	void (*init)(uint8_t id,...);
	uint16_t ( *get_data )( struct _jt808_tx_msg_nodedata *pnodedata ); /*获取要发送的信息*/
}JT808_TX_MSG_NODEDATA;

rt_err_t gprs_rx( uint8_t linkno, uint8_t *pinfo, uint16_t length );

rt_err_t jt808_add_tx_data( uint8_t linkno, JT808_MSG_TYPE type, uint16_t id, uint8_t *pinfo, uint16_t len );


#endif

/************************************** The End Of File **************************************/
