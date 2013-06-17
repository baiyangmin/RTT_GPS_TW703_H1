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
#ifndef _H_SMS
#define _H_SMS

#define SMS_TYPE_PDU

#define GSM_7BIT	0x00
#define GSM_UCS2	0x08

#define  SMS_ACK_msg          1      // 需哟返回短息
#define  SMS_ACK_none         0      // 不需要返回短息

#ifndef true
#define true	1
#define false	0
#endif


/////////////////////////////////////////////////////////////////////////////////////////////////
///接口函数
/////////////////////////////////////////////////////////////////////////////////////////////////

extern void printer_data_hex(u8 *pSrc,u16 nSrcLength);

/*jt808主线程调用前初始化*/
extern rt_err_t sms_init(void);
/*jt808主线程调用*/
void SMS_Process(void);
/*m66接收线程调用*/
u8 	SMS_rx_pro(char *psrc,u16 len);
#endif
/************************************** The End Of File **************************************/
