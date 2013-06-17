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

#include "jt808_param.h"
#include <finsh.h>

JT808_PARAM jt808_param =
{
	0x130060600,         /*0x0000 版本*/
	5,                  /*0x0001 心跳发送间隔*/
	5,                  /*0x0002 TCP应答超时时间*/
	3,                  /*0x0003 TCP超时重传次数*/
	3,                  /*0x0004 UDP应答超时时间*/
	5,                  /*0x0005 UDP超时重传次数*/
	3,                  /*0x0006 SMS消息应答超时时间*/
	5,                  /*0x0007 SMS消息重传次数*/
	"CMNET",            /*0x0010 主服务器APN*/
	"",                 /*0x0011 用户名*/
	"",                 /*0x0012 密码*/
	"60.28.50.210",     /*0x0013 主服务器地址*/
	"CMNET",            /*0x0014 备份APN*/
	"",                 /*0x0015 备份用户名*/
	"",                 /*0x0016 备份密码*/
	"www.google.com",   /*0x0017 备份服务器地址，ip或域名*/
	9131,               /*0x0018 TCP端口*/
	5678,               /*0x0019 UDP端口*/
	"www.google.com",   /*0x001A ic卡主服务器地址，ip或域名*/
	9901,               /*0x001B ic卡服务器TCP端口*/
	8875,                /*0x001C ic卡服务器UDP端口*/
	"www.google.com",   /*0x001D ic卡备份服务器地址，ip或域名*/
	0,                  /*0x0020 位置汇报策略*/
	1,                  /*0x0021 位置汇报方案*/
	30,                 /*0x0022 驾驶员未登录汇报时间间隔*/
	120,                /*0x0027 休眠时汇报时间间隔*/
	5,                  /*0x0028 紧急报警时汇报时间间隔*/
	30,                 /*0x0029 缺省时间汇报间隔*/
	500,                /*0x002c 缺省距离汇报间隔*/
	1000,               /*0x002d 驾驶员未登录汇报距离间隔*/
	1000,               /*0x002e 休眠时距离汇报间隔*/
	100,                /*0x002f 紧急时距离汇报间隔*/
	270,                /*0x0030 拐点补传角度*/
	500,                /*0x0031 电子围栏半径（非法位移阈值），单位为米*/
	"1008611",          /*0x0040 监控平台电话号码*/
	"",                 /*0x0041 复位电话号码*/
	"",                 /*0x0042 恢复出厂设置电话号码*/
	"",                 /*0x0043 监控平台SMS号码*/
	"",                 /*0x0044 接收终端SMS文本报警号码*/
	5,                  /*0x0045 终端接听电话策略*/
	3,                  /*0x0046 每次通话时长*/
	3,                  /*0x0047 当月通话时长*/
	"",                 /*0x0048 监听电话号码*/
	"",                 /*0x0049 监管平台特权短信号码*/
	5,                  /*0x0050 报警屏蔽字*/
	3,                  /*0x0051 报警发送文本SMS开关*/
	5,                  /*0x0052 报警拍照开关*/
	3,                  /*0x0053 报警拍摄存储标志*/
	5,                  /*0x0054 关键标志*/
	3,                  /*0x0055 最高速度kmh*/
	5,                  /*0x0056 超速持续时间*/
	3,                  /*0x0057 连续驾驶时间门限*/
	5,                  /*0x0058 当天累计驾驶时间门限*/
	3,                  /*0x0059 最小休息时间*/
	5,                  /*0x005A 最长停车时间*/
	900,                /*0x0005B 超速报警预警差值，单位为 1/10Km/h */
	90,                 /*0x005C 疲劳驾驶预警差值，单位为秒（s），>0*/
	0x200a,             /*0x005D 碰撞报警参数设置:*/
	30,                 /*0x005E 侧翻报警参数设置： 侧翻角度，单位 1 度，默认为 30 度*/
	0,                  /*0x0064 定时拍照控制*/
	0,                  /*0x0065 定距拍照控制*/
	3,                  /*0x0070 图像视频质量(1-10)*/
	5,                  /*0x0071 亮度*/
	3,                  /*0x0072 对比度*/
	5,                  /*0x0073 饱和度*/
	3,                  /*0x0074 色度*/
	5,                  /*0x0080 车辆里程表读数0.1km*/
	3,                  /*0x0081 省域ID*/
	5,                  /*0x0082 市域ID*/
	"津O-00001",        /*0x0083 机动车号牌*/
	1,                  /*0x0084 车牌颜色	1蓝色 2黄色 3黑色 4白色 9其他*/
	0x0f,               /*0x0090 GNSS 定位模式*/
	0x01,               /*0x0091 GNSS 波特率*/
	0x01,               /*0x0092 GNSS 模块详细定位数据输出频率*/
	0x01,               /*0x0093	 GNSS 模块详细定位数据采集频率*/
	0x01,               /*0x0094	 GNSS 模块详细定位数据上传方式*/
	0x01,               /*0x0095 GNSS 模块详细定位数据上传设置*/
	0,                  /*0x0100 CAN 总线通道 1 采集时间间隔(ms)，0 表示不采集*/
	0,                  /*0x0101 CAN 总线通道 1 上传时间间隔(s)，0 表示不上传*/
	0,                  /*0x0102 CAN 总线通道 2 采集时间间隔(ms)，0 表示不采集*/
	0,                  /*0x0103 CAN 总线通道 2 上传时间间隔(s)，0 表示不上传*/
	{ 0, 0 },           /*0x0110 CAN 总线 ID 单独采集设置*/
	{ 0, 0 },           /*0x0111 其他CAN 总线 ID 单独采集设置*/
	{ 0, 0 },           /*0x0112 其他CAN 总线 ID 单独采集设置*/
	{ 0, 0 },           /*0x0113 其他CAN 总线 ID 单独采集设置*/
	{ 0, 0 },           /*0x0114 其他CAN 总线 ID 单独采集设置*/
	{ 0, 0 },           /*0x0115 其他CAN 总线 ID 单独采集设置*/
	{ 0, 0 },           /*0x0116 其他CAN 总线 ID 单独采集设置*/
	{ 0, 0 },           /*0x0117 其他CAN 总线 ID 单独采集设置*/
	{ 0, 0 },           /*0x0118 其他CAN 总线 ID 单独采集设置*/
	{ 0, 0 },           /*0x0119 其他CAN 总线 ID 单独采集设置*/
};

TERM_PARAM term_param =
{
	0x07,
	{ 0x11,0x22,	0x33, 0x44, 0x55, 0x66 },
	{ "TCBBD" },
	{ "TW701-BD" },
	{ 0x00,0x99,	0xaa, 0xbb, 0xcc, 0xdd, 0xee},
};

#define FLAG_DISABLE_REPORT_INVALID 1       /*设备非法*/
#define FLAG_DISABLE_REPORT_AREA	2       /*区域内禁止上报*/

static uint32_t flag_disable_report = 0;    /*禁止上报的标志位*/

/*保存参数到serialflash*/
void param_save( void )
{
	rt_kprintf( "parma_save size=%d\r\n", sizeof( jt808_param ) );
	sst25_write_back( ADDR_PARAM, (uint8_t*)&jt808_param, sizeof( jt808_param ) );
}

/*加载参数从serialflash*/
void param_load( void )
{
	/*预读一部分数据*/
	uint8_t		ver8[4];
	uint32_t	ver32;
	sst25_read( ADDR_PARAM, ver8, 4 );
	ver32 = ( ver8[0] ) | ( ver8[1] << 8 ) | ( ver8[2] << 16 ) | ( ver8[3] << 24 );
	rt_kprintf( "param_load ver=%08x\r\n", ver32 );
	if( jt808_param.id_0x0000 != ver32 ) /*不管是不是未初始化*/
	{
		rt_kprintf( "%s(%d)param_save\r\n", __func__, __LINE__ );
		param_save( );
	}
	sst25_read( ADDR_PARAM, (uint8_t*)&jt808_param, sizeof( jt808_param ) );
}

#define TYPE_BYTE	0x01    /*固定为1字节,小端对齐*/
#define TYPE_WORD	0x02    /*固定为2字节,小端对齐*/
#define TYPE_DWORD	0x04    /*固定为4字节,小端对齐*/
#define TYPE_STR	0x80    /*固定为32字节,网络顺序*/
#define TYPE_CAN_ID 0x48    /*固定为8字节,当前存储CAN_ID参数*/

struct _tbl_id_lookup
{
	uint16_t	id;
	uint8_t		type;
	uint8_t		* val;
} tbl_id_lookup[] = {
	{ 0x0000, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0000 ) },    //uint32_t  id_0x0000;   /*0x0000 版本*/
	{ 0x0001, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0001 ) },    //uint32_t  id_0x0001;   /*0x0001 心跳发送间隔*/
	{ 0x0002, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0002 ) },    //uint32_t  id_0x0002;   /*0x0002 TCP应答超时时间*/
	{ 0x0003, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0003 ) },    //uint32_t  id_0x0003;   /*0x0003 TCP超时重传次数*/
	{ 0x0004, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0004 ) },    //uint32_t  id_0x0004;   /*0x0004 UDP应答超时时间*/
	{ 0x0005, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0005 ) },    //uint32_t  id_0x0005;   /*0x0005 UDP超时重传次数*/
	{ 0x0006, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0006 ) },    //uint32_t  id_0x0006;   /*0x0006 SMS消息应答超时时间*/
	{ 0x0007, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0007 ) },    //uint32_t  id_0x0007;   /*0x0007 SMS消息重传次数*/
	{ 0x0010, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0010 ) },    //char   id_0x0010[32];  /*0x0010 主服务器APN*/
	{ 0x0011, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0011 ) },    //char   id_0x0011[32];  /*0x0011 用户名*/
	{ 0x0012, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0012 ) },    //char   id_0x0012[32];  /*0x0012 密码*/
	{ 0x0013, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0013 ) },    //char   id_0x0013[32];  /*0x0013 主服务器地址*/
	{ 0x0014, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0014 ) },    //char   id_0x0014[32];  /*0x0014 备份APN*/
	{ 0x0015, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0015 ) },    //char   id_0x0015[32];  /*0x0015 备份用户名*/
	{ 0x0016, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0016 ) },    //char   id_0x0016[32];  /*0x0016 备份密码*/
	{ 0x0017, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0017 ) },    //char   id_0x0017[32];  /*0x0017 备份服务器地址，ip或域名*/
	{ 0x0018, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0018 ) },    //uint32_t  id_0x0018;   /*0x0018 TCP端口*/
	{ 0x0019, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0019 ) },    //uint32_t  id_0x0019;   /*0x0019 UDP端口*/
	{ 0x001A, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x001A ) },    //char   id_0x001A[32];  /*0x001A ic卡主服务器地址，ip或域名*/
	{ 0x001B, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x001B ) },    //uint32_t  id_0x001B;   /*0x001B ic卡服务器TCP端口*/
	{ 0x001C, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x001C ) },    //uint32_t  id_0x001C;   /*0x001C ic卡服务器UDP端口*/
	{ 0x001D, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x001D ) },    //char   id_0x001D[32];  /*0x001D ic卡备份服务器地址，ip或域名*/
	{ 0x0020, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0020 ) },    //uint32_t  id_0x0020;   /*0x0020 位置汇报策略*/
	{ 0x0021, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0021 ) },    //uint32_t  id_0x0021;   /*0x0021 位置汇报方案*/
	{ 0x0022, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0022 ) },    //uint32_t  id_0x0022;   /*0x0022 驾驶员未登录汇报时间间隔*/
	{ 0x0027, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0027 ) },    //uint32_t  id_0x0027;   /*0x0027 休眠时汇报时间间隔*/
	{ 0x0028, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0028 ) },    //uint32_t  id_0x0028;   /*0x0028 紧急报警时汇报时间间隔*/
	{ 0x0029, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0029 ) },    //uint32_t  id_0x0029;   /*0x0029 缺省时间汇报间隔*/
	{ 0x002C, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x002C ) },    //uint32_t  id_0x002C;   /*0x002c 缺省距离汇报间隔*/
	{ 0x002D, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x002D ) },    //uint32_t  id_0x002D;   /*0x002d 驾驶员未登录汇报距离间隔*/
	{ 0x002E, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x002E ) },    //uint32_t  id_0x002E;   /*0x002e 休眠时距离汇报间隔*/
	{ 0x002F, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x002F ) },    //uint32_t  id_0x002F;   /*0x002f 紧急时距离汇报间隔*/
	{ 0x0030, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0030 ) },    //uint32_t  id_0x0030;   /*0x0030 拐点补传角度*/
	{ 0x0031, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0031 ) },    //uint16_t  id_0x0031;   /*0x0031 电子围栏半径（非法位移阈值），单位为米*/
	{ 0x0040, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0040 ) },    //char   id_0x0040[32];  /*0x0040 监控平台电话号码*/
	{ 0x0041, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0041 ) },    //char   id_0x0041[32];  /*0x0041 复位电话号码*/
	{ 0x0042, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0042 ) },    //char   id_0x0042[32];  /*0x0042 恢复出厂设置电话号码*/
	{ 0x0043, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0043 ) },    //char   id_0x0043[32];  /*0x0043 监控平台SMS号码*/
	{ 0x0044, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0044 ) },    //char   id_0x0044[32];  /*0x0044 接收终端SMS文本报警号码*/
	{ 0x0045, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0045 ) },    //uint32_t  id_0x0045;   /*0x0045 终端接听电话策略*/
	{ 0x0046, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0046 ) },    //uint32_t  id_0x0046;   /*0x0046 每次通话时长*/
	{ 0x0047, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0047 ) },    //uint32_t  id_0x0047;   /*0x0047 当月通话时长*/
	{ 0x0048, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0048 ) },    //char   id_0x0048[32];  /*0x0048 监听电话号码*/
	{ 0x0049, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0049 ) },    //char   id_0x0049[32];  /*0x0049 监管平台特权短信号码*/
	{ 0x0050, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0050 ) },    //uint32_t  id_0x0050;   /*0x0050 报警屏蔽字*/
	{ 0x0051, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0051 ) },    //uint32_t  id_0x0051;   /*0x0051 报警发送文本SMS开关*/
	{ 0x0052, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0052 ) },    //uint32_t  id_0x0052;   /*0x0052 报警拍照开关*/
	{ 0x0053, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0053 ) },    //uint32_t  id_0x0053;   /*0x0053 报警拍摄存储标志*/
	{ 0x0054, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0054 ) },    //uint32_t  id_0x0054;   /*0x0054 关键标志*/
	{ 0x0055, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0055 ) },    //uint32_t  id_0x0055;   /*0x0055 最高速度kmh*/
	{ 0x0056, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0056 ) },    //uint32_t  id_0x0056;   /*0x0056 超速持续时间*/
	{ 0x0057, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0057 ) },    //uint32_t  id_0x0057;   /*0x0057 连续驾驶时间门限*/
	{ 0x0058, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0058 ) },    //uint32_t  id_0x0058;   /*0x0058 当天累计驾驶时间门限*/
	{ 0x0059, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0059 ) },    //uint32_t  id_0x0059;   /*0x0059 最小休息时间*/
	{ 0x005A, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x005A ) },    //uint32_t  id_0x005A;   /*0x005A 最长停车时间*/
	{ 0x005B, TYPE_WORD,	 (uint8_t*)&( jt808_param.id_0x005B ) },    //uint16_t  id_0x005B;   /*0x005B 超速报警预警差值，单位为 1/10Km/h */
	{ 0x005C, TYPE_WORD,	 (uint8_t*)&( jt808_param.id_0x005C ) },    //uint16_t  id_0x005C;   /*0x005C 疲劳驾驶预警差值，单位为秒（s），>0*/
	{ 0x005D, TYPE_WORD,	 (uint8_t*)&( jt808_param.id_0x005D ) },    //uint16_t  id_0x005D;   /*0x005D 碰撞报警参数设置:b7..0：碰撞时间(4ms) b15..8：碰撞加速度(0.1g) 0-79 之间，默认为10 */
	{ 0x005E, TYPE_WORD,	 (uint8_t*)&( jt808_param.id_0x005E ) },    //uint16_t  id_0x005E;   /*0x005E 侧翻报警参数设置： 侧翻角度，单位 1 度，默认为 30 度*/
	{ 0x0064, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0064 ) },    //uint32_t  id_0x0064;   /*0x0064 定时拍照控制*/
	{ 0x0065, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0065 ) },    //uint32_t  id_0x0065;   /*0x0065 定距拍照控制*/
	{ 0x0070, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0070 ) },    //uint32_t  id_0x0070;   /*0x0070 图像视频质量(1-10)*/
	{ 0x0071, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0071 ) },    //uint32_t  id_0x0071;   /*0x0071 亮度*/
	{ 0x0072, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0072 ) },    //uint32_t  id_0x0072;   /*0x0072 对比度*/
	{ 0x0073, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0073 ) },    //uint32_t  id_0x0073;   /*0x0073 饱和度*/
	{ 0x0074, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0074 ) },    //uint32_t  id_0x0074;   /*0x0074 色度*/
	{ 0x0080, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0080 ) },    //uint32_t  id_0x0080;   /*0x0080 车辆里程表读数0.1km*/
	{ 0x0081, TYPE_WORD,	 (uint8_t*)&( jt808_param.id_0x0081 ) },    //uint16_t  id_0x0081;   /*0x0081 省域ID*/
	{ 0x0082, TYPE_WORD,	 (uint8_t*)&( jt808_param.id_0x0082 ) },    //uint16_t  id_0x0082;   /*0x0082 市域ID*/
	{ 0x0083, TYPE_STR,		 (uint8_t*)&( jt808_param.id_0x0083 ) },    //char   id_0x0083[32];  /*0x0083 机动车号牌*/
	{ 0x0084, TYPE_BYTE,	 (uint8_t*)&( jt808_param.id_0x0084 ) },    //uint8_t		 id_0x0084;      /*0x0084 车牌颜色	1蓝色 2黄色 3黑色 4白色 9其他*/
	{ 0x0090, TYPE_BYTE,	 (uint8_t*)&( jt808_param.id_0x0090 ) },    //uint8_t		 id_0x0090;      /*0x0090 GNSS 定位模式*/
	{ 0x0091, TYPE_BYTE,	 (uint8_t*)&( jt808_param.id_0x0091 ) },    //uint8_t		 id_0x0091;      /*0x0091 GNSS 波特率*/
	{ 0x0092, TYPE_BYTE,	 (uint8_t*)&( jt808_param.id_0x0092 ) },    //uint8_t		 id_0x0092;      /*0x0092 GNSS 模块详细定位数据输出频率*/
	{ 0x0093, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0093 ) },    //uint32_t  id_0x0093;   /*0x0093 GNSS 模块详细定位数据采集频率*/
	{ 0x0094, TYPE_BYTE,	 (uint8_t*)&( jt808_param.id_0x0094 ) },    //uint8_t		 id_0x0094;      /*0x0094 GNSS 模块详细定位数据上传方式*/
	{ 0x0095, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0095 ) },    //uint32_t  id_0x0095;   /*0x0095 GNSS 模块详细定位数据上传设置*/
	{ 0x0100, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0100 ) },    //uint32_t  id_0x0100;   /*0x0100 CAN 总线通道 1 采集时间间隔(ms)，0 表示不采集*/
	{ 0x0101, TYPE_WORD,	 (uint8_t*)&( jt808_param.id_0x0101 ) },    //uint16_t  id_0x0101;   /*0x0101 CAN 总线通道 1 上传时间间隔(s)，0 表示不上传*/
	{ 0x0102, TYPE_DWORD,	 (uint8_t*)&( jt808_param.id_0x0102 ) },    //uint32_t  id_0x0102;   /*0x0102 CAN 总线通道 2 采集时间间隔(ms)，0 表示不采集*/
	{ 0x0103, TYPE_WORD,	 (uint8_t*)&( jt808_param.id_0x0103 ) },    //uint16_t  id_0x0103;   /*0x0103 CAN 总线通道 2 上传时间间隔(s)，0 表示不上传*/
	{ 0x0110, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0110 ) },    //uint8_t		 id_0x0110[8];	 /*0x0110 CAN 总线 ID 单独采集设置*/
	{ 0x0111, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0111 ) },    //uint8_t		 id_0x0111[8];	 /*0x0111 其他CAN 总线 ID 单独采集设置*/
	{ 0x0112, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0112 ) },    //uint8_t		 id_0x0112[8];	 /*0x0112 其他CAN 总线 ID 单独采集设置*/
	{ 0x0113, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0113 ) },    //uint8_t		 id_0x0113[8];	 /*0x0113 其他CAN 总线 ID 单独采集设置*/
	{ 0x0114, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0114 ) },    //uint8_t		 id_0x0114[8];	 /*0x0114 其他CAN 总线 ID 单独采集设置*/
	{ 0x0115, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0115 ) },    //uint8_t		 id_0x0115[8];	 /*0x0115 其他CAN 总线 ID 单独采集设置*/
	{ 0x0116, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0116 ) },    //uint8_t		 id_0x0116[8];	 /*0x0116 其他CAN 总线 ID 单独采集设置*/
	{ 0x0117, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0117 ) },    //uint8_t		 id_0x0117[8];	 /*0x0117 其他CAN 总线 ID 单独采集设置*/
	{ 0x0118, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0118 ) },    //uint8_t		 id_0x0118[8];	 /*0x0118 其他CAN 总线 ID 单独采集设置*/
	{ 0x0119, TYPE_BYTE | 8, (uint8_t*)&( jt808_param.id_0x0119 ) } //uint8_t		 id_0x0119[8];	 /*0x0119 其他CAN 总线 ID 单独采集设置*/
};


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
uint8_t param_put( uint16_t id, uint8_t len, uint8_t* value )
{
	int		i, j;
	uint8_t *psrc, *pdst;

	for( i = 0; i < sizeof( tbl_id_lookup ) / sizeof( struct _tbl_id_lookup ); i++ )
	{
		if( id == tbl_id_lookup[i].id )
		{
			if( ( tbl_id_lookup[i].type == TYPE_DWORD ) && ( len == 4 ) )
			{
				psrc = value;
				//rt_kprintf("psrc=%02x %02x %02x %02x \r\n",*(psrc+3),*(psrc+2),*(psrc+1),*(psrc+0));
				pdst	= tbl_id_lookup[i].val;
				*pdst++ = *( psrc + 3 );
				*pdst++ = *( psrc + 2 );
				*pdst++ = *( psrc + 1 );
				*pdst	= *( psrc + 0 );
				return 0;
			}
			if( ( tbl_id_lookup[i].type == TYPE_WORD ) && ( len == 2 ) )
			{
				psrc	= value;
				pdst	= tbl_id_lookup[i].val;
				*pdst++ = *( psrc + 1 );
				*pdst	= *psrc;
				return 0;
			}
			if( ( tbl_id_lookup[i].type == TYPE_BYTE ) && ( len == 1 ) )
			{
				psrc	= value;
				pdst	= tbl_id_lookup[i].val;
				*pdst	= *psrc;
				return 0;
			}
			if( tbl_id_lookup[i].type == TYPE_CAN_ID )
			{
				psrc	= value;
				pdst	= tbl_id_lookup[i].val;
				*pdst++ = *( psrc + 3 );
				*pdst++ = *( psrc + 2 );
				*pdst++ = *( psrc + 1 );
				*pdst++ = *psrc;
				*pdst++ = *( psrc + 7 );
				*pdst++ = *( psrc + 6 );
				*pdst++ = *( psrc + 5 );
				*pdst	= *( psrc + 4 );
				return 0;
			}
			if( tbl_id_lookup[i].type == TYPE_STR )
			{
				psrc = tbl_id_lookup[i].val;
				strncpy( psrc, value, 32 );
				*( psrc + 31 ) = 0;
				return 0;
			}
		}
	}
	return 1;
}

/*设置参数*/
void param_put_int( uint16_t id, uint32_t value )
{
	uint32_t	i, j;
	int			len;
	uint8_t		*p;

	for( i = 0; i < sizeof( tbl_id_lookup ) / sizeof( struct _tbl_id_lookup ); i++ )
	{
		if( id == tbl_id_lookup[i].id )
		{
			p		= tbl_id_lookup[i].val;
			j		= value;
			*p++	= ( j & 0xff );
			*p++	= j >> 8;
			*p++	= j >> 16;
			*p		= j >> 24;
			break;
		}
	}
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
static void param_put_str( uint16_t id, uint8_t* value )
{
	int		i, j;
	int		len;
	uint8_t *p;

	for( i = 0; i < sizeof( tbl_id_lookup ) / sizeof( struct _tbl_id_lookup ); i++ )
	{
		if( id == tbl_id_lookup[i].id )
		{
			p = tbl_id_lookup[i].val;
			strncpy( p, (char*)value, 32 );
			break;
		}
	}
}

/*读取参数,返回参数类型参数*/
uint8_t param_get( uint16_t id, uint8_t* value )
{
	int			i;
	uint8_t		*p;
	uint32_t	val;
	for( i = 0; i < sizeof( tbl_id_lookup ) / sizeof( struct _tbl_id_lookup ); i++ )
	{
		if( id == tbl_id_lookup[i].id )
		{
			if( tbl_id_lookup[i].type == TYPE_DWORD )
			{
				p = tbl_id_lookup[i].val;
				memcpy( value, p, 4 );
				return 4;
			}

			if( tbl_id_lookup[i].type == TYPE_WORD )
			{
				p = tbl_id_lookup[i].val;
				memcpy( value, p, 2 );
				return 2;
			}

			if( tbl_id_lookup[i].type == TYPE_BYTE )
			{
				p		= tbl_id_lookup[i].val;
				*value	= *p;
				return 1;
			}
			if( tbl_id_lookup[i].type == TYPE_STR )
			{
				p = tbl_id_lookup[i].val;
				memcpy( value, p, strlen( p ) );
				return strlen( p );
			}
			if( tbl_id_lookup[i].type == TYPE_CAN_ID )
			{
				p = tbl_id_lookup[i].val;
				memcpy( value, p, 8 );
				return 8;
			}
		}
	}
	return 0;
}

/*读取参数*/
uint32_t param_get_int( uint16_t id )
{
	int			i;
	uint8_t		*p;
	uint32_t	val = 0;
	for( i = 0; i < sizeof( tbl_id_lookup ) / sizeof( struct _tbl_id_lookup ); i++ )
	{
		if( id == tbl_id_lookup[i].id )
		{
			p	= tbl_id_lookup[i].val;
			val |= ( *p++ );
			val |= ( ( *p++ ) << 8 );
			val |= ( ( *p++ ) << 16 );
			val |= ( ( *p ) << 24 );
			return val;
		}
	}
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
void param_print( void )
{
	int			i, j, id;
	int			type;
	uint8_t		*p;
	uint32_t	val = 0;

	for( i = 0; i < sizeof( tbl_id_lookup ) / sizeof( struct _tbl_id_lookup ); i++ )
	{
		id	= tbl_id_lookup[i].id;
		p	= tbl_id_lookup[i].val;
		val = 0;
		switch( tbl_id_lookup[i].type )
		{
			case TYPE_DWORD: /*字节对齐方式 little_endian*/
				val |= ( *p++ );
				val |= ( ( *p++ ) << 8 );
				val |= ( ( *p++ ) << 16 );
				val |= ( ( *p ) << 24 );
				rt_kprintf( "\r\nid=%04x value=%08x\r\n", id, val );
				break;
			case TYPE_CAN_ID:
				val |= ( *p++ );
				val |= ( ( *p++ ) << 8 );
				val |= ( ( *p++ ) << 16 );
				val |= ( ( *p++ ) << 24 );
				rt_kprintf( "\r\nid=%04x value=%08x", id, val );
				val = 0;
				val |= ( *p++ );
				val |= ( ( *p++ ) << 8 );
				val |= ( ( *p++ ) << 16 );
				val |= ( ( *p ) << 24 );
				rt_kprintf( " %08x\r\n", val );
				break;

				break;
			case TYPE_STR:
				rt_kprintf( "\r\nid=%04x value=%s\r\n", id, p );
				break;
		}
	}
}

FINSH_FUNCTION_EXPORT( param_print, print param );

/*打印参数信息*/
void param_dump( void )
{
	uint8_t tbl[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	int		i, count = 0;
	uint8_t c;
	uint8_t *p = (uint8_t*)&jt808_param;
	uint8_t printbuf[70];
	int32_t len = sizeof( jt808_param );

	while( len > 0 )
	{
		count = ( len < 16 ) ? len : 16;
		memset( printbuf, 0x20, 70 );
		for( i = 0; i < count; i++ )
		{
			c					= *p;
			printbuf[i * 3]		= tbl[c >> 4];
			printbuf[i * 3 + 1] = tbl[c & 0x0f];
			if( c < 0x20 )
			{
				c = '.';
			}
			if( c > 0x7f )
			{
				c = '.';
			}
			printbuf[50 + i] = c;
			p++;
		}
		printbuf[69] = 0;
		rt_kprintf( "%s\r\n", printbuf );
		len -= count;
	}
}

FINSH_FUNCTION_EXPORT( param_dump, dump param );

/************************************** The End Of File **************************************/
