/*
     SMS.C
 */
#include <rtthread.h>
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>
#include <finsh.h>

#include  <stdlib.h> //数字转换成字符串
#include  <stdio.h>
#include  <string.h>
#include  "jt808_sms.h"
#include "m66.h"

#define PHONEMAXNUM 15
#define PHONEMAXSTR 150

typedef enum
{
	SMS_TX_IDLE = 0,
	SMS_TX_START,
	SMS_TX_WAITACK,
	SMS_TX_OK,
	SMS_TX_FALSE,
}SMS_TX_STATE;

typedef enum
{
	SMS_RX_IDLE = 0,
	SMS_RX_START,
	SMS_RX_READ,
	SMS_RX_WAITACK,
	SMS_RX_OK,
	SMS_RX_DEL,
	SMS_RX_FALSE,
}SMS_RX_STATE;

typedef struct
{
	char	SCA[15];                                ///短信中心号码
	char	TPA[15];                                ///短信发送方号码
	char	TP_SCTS[14];                            ///事件戳信息
	u8		TP_PID;                                 ///协议标识
	u8		TP_DCS;                                 ///协议编码类型
}SmsType;

typedef  struct
{
	u8	SMIndex;                                    // 短信记录
	u8	SMS_read;                                   // 读取短信标志位
	u8	SMS_delALL;                                 // 删除所有短信标志位
	u8	SMS_come;                                   // 有短信过来了
	u8	SMS_Style;                                  // 消息类型，1为pdu，0为text
	u8	SMS_delayCounter;                           // 短信延时器
	u8	SMS_waitCounter;                            ///短信等待
	u8	SMSAtSend[45];                              //短信AT命令寄存器

	u8				SMS_destNum[15];                //  发送短息目的号码
	u8				SMS_sendFlag;                   //  短息发送标志位
	u8				SMS_sd_Content[PHONEMAXSTR];    // 短息发送内容
	u8				SMS_rx_Content[PHONEMAXSTR];    // 短息接收内容
	SMS_RX_STATE	rx_state;                       //接收状态
	SMS_TX_STATE	tx_state;                       //发送状态
	u8				rx_retry;                       ///重复接收次数
	u8				tx_retry;                       ///重复发送次数
	//------- self sms protocol  ----
	SmsType Sms_Info;                               //解析的PDU消息的参数信息
} SMS_Style;

typedef __packed struct
{
	u8		SMS_destNum[PHONEMAXNUM];               //  发送短息目的号码
	char	*pstr;                                  // 短息发送内容
} SMS_Send_Msg;

/* 消息邮箱控制块*/
static struct rt_mailbox mb_smsdata;
#define MB_SMSDATA_POOL_SIZE 32
/* 消息邮箱中用到的放置消息的内存池*/
static uint8_t		mb_smsdata_pool[MB_SMSDATA_POOL_SIZE];

static SMS_Style	SMS_Service;   //  短息相关

static const char	GB_DATA[]	= "京津沪宁渝琼藏川粤青贵闽吉陕蒙晋甘桂鄂赣浙苏新鲁皖湘黑辽云豫冀";
static const char	UCS2_CODE[] = "4EAC6D256CAA5B816E1D743C85CF5DDD7CA497528D3595FD540996558499664B7518684291028D636D5982CF65B09C8176966E589ED18FBD4E918C6B5180";

static void SMS_protocol( u8 *instr, u16 len, u8 ACKstate );


/*获取一个指定字符的位置，中文字符作为一个字符计算*/
static int StringFind( const char* string, const char* find, int number )
{
	char* pos;
	char* p;
	int count = 0;
	//pos=string;
	//p = string;
	pos = strstr( string, find );
	if( pos == (void*)0 )
	{
		return -1;
	}
	count = pos - string;
	return count;
#ifdef 0
	while( number > 0 )
	{
		/*定义查找到的字符位置的指针，以便临时指针进行遍历*/
		pos = strstr( p, find );
		/*当位置指针为0时，说明没有找到这个字符*/
		if( pos == (void*)0 )
		{
			return -1;
		}
		/*当位置指针和临时指针相等说明下一个字符就是要找的字符，如果临时指针小于位置指针，则进行遍历字符串操作，并将count增1*/
		while( p <= pos )
		{
			if( *p > 0x80 || *p < 0 )
			{
				p++;
			}
			p++;
			count++;
		}
		/*对要查找的次数减一*/
		number--;
	}
	return count;
#endif
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
u16 Hex_To_Ascii( const u8* pSrc, u8* pDst, u16 nSrcLength )
{
	const u8	tab[] = "0123456789ABCDEF"; // 0x0-0xf的字符查找表
	u16			i;

	for( i = 0; i < nSrcLength; i++ )
	{
		// 输出低4位
		*pDst++ = tab[*pSrc >> 4];

		// 输出高4位
		*pDst++ = tab[*pSrc & 0x0f];

		pSrc++;
	}

	// 输出字符串加个结束符
	*pDst = '\0';

	// 返回目标字符串长度
	return ( nSrcLength << 1 );
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
u16 Ascii_To_Hex( const u8* pSrc, u8* pDst, u16 nSrcLength )
{
	u16 i;
	for( i = 0; i < nSrcLength; i += 2 )
	{
		// 输出高4位
		if( *pSrc >= '0' && *pSrc <= '9' )
		{
			*pDst = ( *pSrc - '0' ) << 4;
		}     else if( *pSrc >= 'A' && *pSrc <= 'F' )
		{
			*pDst = ( *pSrc - 'A' + 10 ) << 4;
		}else if( *pSrc >= 'a' && *pSrc <= 'f' )
		{
			*pDst = ( *pSrc - 'a' + 10 ) << 4;
		}else
		{
			return false;
		}
		pSrc++;
		// 输出低4位
		if( *pSrc >= '0' && *pSrc <= '9' )
		{
			*pDst |= *pSrc - '0';
		}else if( *pSrc >= 'A' && *pSrc <= 'F' )
		{
			*pDst |= *pSrc - 'A' + 10;
		}else if( *pSrc >= 'a' && *pSrc <= 'f' )
		{
			*pDst |= ( *pSrc - 'a' + 10 );
		}else
		{
			return false;
		}
		pSrc++;
		pDst++;
	}

	// 返回目标数据长度
	return ( nSrcLength >> 1 );
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
void printer_data_hex(u8 *pSrc,u16 nSrcLength)
{
	char pDst[3];
 	const u8	tab[] = "0123456789ABCDEF"; // 0x0-0xf的字符查找表
	u16			i;
	
	pDst[2]=0;
	for( i = 0; i < nSrcLength; i++ )
	{
		// 输出低4位
		pDst[0] = tab[*pSrc >> 4];
		// 输出高4位
		pDst[1] = tab[*pSrc & 0x0f];
		rt_kprintf("%s",pDst);
		pSrc++;
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
u16 GsmDecode8bit( const u8 *pSrc, u8 *pDst, u16 nSrcLength )
{
	u16 m;
	// 简单复制
	for( m = 0; m < nSrcLength; m++ )
	{
		*pDst++ = *pSrc++;
	}
	// 输出字符串加个结束符
	*pDst = '\0';
	return nSrcLength;
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
u16  GsmEncode8bit( const u8 *pSrc, u8 *pDst, u16 nSrcLength )
{
	u16 m;
	// 简单复制
	for( m = 0; m < nSrcLength; m++ )
	{
		*pDst++ = *pSrc++;
	}

	return nSrcLength;
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
u16 GsmDecodeUcs2_old( const u8* pSrc, u8* pDst, u16 nSrcLength )
{
	u16 nDstLength = nSrcLength;   // UNICODE宽字符数目
	u16 i;
	// INT16U wchar[128];      // UNICODE串缓冲区

	// 高低字节对调，拼成UNICODE
	for( i = 0; i < nSrcLength; i += 2 )
	{
		// 先高位字节,因为是数据。高字节为0
		pSrc++;
		// 后低位字节
		*pDst++ = *pSrc++;
	}
	// UNICODE串-.字符串
	//nDstLength = ::WideCharToMultiByte(CP_ACP, 0, wchar, nSrcLength/2, pDst, 160, NULL, NULL);
	// 输出字符串加个结束符
	//pDst[nDstLength] = '\0';
	// 返回目标字符串长度
	return ( nDstLength >> 1 );
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
u16 GsmEncodeUcs2_old( const u8* pSrc, u8* pDst, u16 nSrcLength )
{
	u16 nDstLength = nSrcLength;   // UNICODE宽字符数目
	u16 i;
	//INT16U wchar[128];      // UNICODE串缓冲区

	// 字符串-.UNICODE串
	// nDstLength = ::MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);

	// 高低字节对调，输出
	for( i = 0; i < nDstLength; i++ )
	{
		// 先输出高位字节
		*pDst++ = 0x00;
		// 后输出低位字节
		*pDst++ = *pSrc++;
	}

	// 返回目标编码串长度
	return ( nDstLength << 1 );
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
u16 GsmDecodeUcs2( const u8* pSrc, u8* pDst, u16 nSrcLength )
{
	u16			nDstLength = 0; // UNICODE宽字符数目
	u16			i;
	s16			indexNum;
	char		strTemp[5];
	const u8	*p;
	// INT16U wchar[128];      // UNICODE串缓冲区

	// 高低字节对调，拼成UNICODE
	for( i = 0; i < nSrcLength; i += 2 )
	{
		if( *pSrc ) ///汉字编码
		{
			p = pSrc;
			Hex_To_Ascii( p, strTemp, 2 );
			strTemp[4]	= 0;
			indexNum	= StringFind( UCS2_CODE, strTemp, strlen( UCS2_CODE ) );
			if( indexNum >= 0 )
			{
				indexNum	= indexNum >> 1;
				*pDst++		= GB_DATA[indexNum];
				*pDst++		= GB_DATA[1 + indexNum];
			}else
			{
				*pDst++ = 0x20; ///不可识别的汉子用"※"表示
				*pDst++ = 0x3B;
				//*pDst++ = *pSrc;		///不可识别的汉子用UCS2源码表示
				//*pDst++ = *(pSrc+1);
			}
			pSrc		+= 2;
			nDstLength	+= 2;
		}else ///英文编码
		{
			// 先高位字节,因为是数据。高字节为0
			pSrc++;
			// 后低位字节
			*pDst++ = *pSrc++;
			nDstLength++;
		}
	}
	// UNICODE串-.字符串
	//nDstLength = ::WideCharToMultiByte(CP_ACP, 0, wchar, nSrcLength/2, pDst, 160, NULL, NULL);
	// 输出字符串加个结束符
	//pDst[nDstLength] = '\0';
	// 返回目标字符串长度
	return nDstLength;
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
u16 GsmEncodeUcs2( const u8* pSrc, u8* pDst, u16 nSrcLength )
{
	u16		nDstLength = nSrcLength; // UNICODE宽字符数目
	u16		i;
	s16		indexNum;
	char	strTemp[3];
	//INT16U wchar[128];      // UNICODE串缓冲区
	nDstLength = 0;
	// 字符串-.UNICODE串
	// nDstLength = ::MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);

	// 高低字节对调，输出
	for( i = 0; i < nSrcLength; i++ )
	{
		if( ( *pSrc & 0x80 ) == 0x80 ) ///汉字编码
		{
			strTemp[0]	= *pSrc;
			strTemp[1]	= *( pSrc + 1 );
			strTemp[2]	= 0;
			indexNum	= StringFind( GB_DATA, strTemp, strlen( GB_DATA ) );
			if( indexNum >= 0 )
			{
				indexNum = indexNum * 2;
				Ascii_To_Hex( &UCS2_CODE[indexNum], strTemp, 4 );
				*pDst++ = strTemp[0];
				*pDst++ = strTemp[1];
			}else   ///不可识别的汉子用"※"表示
			{
				*pDst++ = 0x20;
				*pDst++ = 0x3B;
			}
			pSrc += 2;
			i++;
		}else       ///英文编码
		{
			// 先输出高位字节
			*pDst++ = 0x00;
			// 后输出低位字节
			*pDst++ = *pSrc++;
		}
		nDstLength++;
	}

	// 返回目标编码串长度
	return ( nDstLength << 1 );
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
u16 GsmDecode7bit( const u8* pSrc, u8* pDst, u16 nSrcLength )
{
	u16 nSrc;       // 源字符串的计数值
	u16 nDst;       // 目标解码串的计数值
	u16 nByte;      // 当前正在处理的组内字节的序号，范围是0-6
	u8	nLeft;      // 上一字节残余的数据

	// 计数值初始化
	nSrc	= 0;
	nDst	= 0;

	// 组内字节序号和残余数据初始化
	nByte	= 0;
	nLeft	= 0;

	// 将源数据每7个字节分为一组，解压缩成8个字节
	// 循环该处理过程，直至源数据被处理完
	// 如果分组不到7字节，也能正确处理
	while( nSrc < nSrcLength )
	{
		// 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
		*pDst = ( ( *pSrc << nByte ) | nLeft ) & 0x7f;
		// 将该字节剩下的左边部分，作为残余数据保存起来
		nLeft = *pSrc >> ( 7 - nByte );

		// 修改目标串的指针和计数值
		pDst++;
		nDst++;
		// 修改字节计数值
		nByte++;

		// 到了一组的最后一个字节
		if( nByte == 7 )
		{
			// 额外得到一个目标解码字节
			*pDst = nLeft;

			// 修改目标串的指针和计数值
			pDst++;
			nDst++;

			// 组内字节序号和残余数据初始化
			nByte	= 0;
			nLeft	= 0;
		}

		// 修改源串的指针和计数值
		pSrc++;
		nSrc++;
	}

	*pDst = 0;

	// 返回目标串长度
	return nDst;
}

//将每个ascii8位编码的Bit8去掉，依次将下7位编码的后几位逐次移到前面，形成新的8位编码。
u16 GsmEncode7bit( const u8* pSrc, u8* pDst, u16 nSrcLength )
{
	u16 nSrc;       // 源字符串的计数值
	u16 nDst;       // 目标编码串的计数值
	u16 nChar;      // 当前正在处理的组内字符字节的序号，范围是0-7
	u8	nLeft;      // 上一字节残余的数据

	// 计数值初始化
	nSrc	= 0;
	nDst	= 0;

	// 将源串每8个字节分为一组，压缩成7个字节
	// 循环该处理过程，直至源串被处理完
	// 如果分组不到8字节，也能正确处理
	while( nSrc < nSrcLength + 1 )
	{
		// 取源字符串的计数值的最低3位
		nChar = nSrc & 7;
		// 处理源串的每个字节
		if( nChar == 0 )
		{
			// 组内第一个字节，只是保存起来，待处理下一个字节时使用
			nLeft = *pSrc;
		}else
		{
			// 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
			*pDst = ( *pSrc << ( 8 - nChar ) ) | nLeft;
			// 将该字节剩下的左边部分，作为残余数据保存起来
			nLeft = *pSrc >> nChar;
			// 修改目标串的指针和计数值 pDst++;
			pDst++;
			nDst++;
		}

		// 修改源串的指针和计数值
		pSrc++; nSrc++;
	}

	// 返回目标串长度
	return nDst;
}

// 两两颠倒的字符串转换为正常顺序的字符串
// 如："8613693092030" -. "683196032930F0"
// pSrc: 源字符串指针
// pDst: 目标字符串指针
// nSrcLength: 源字符串长度
// 返回: 目标字符串长度
u8 Hex_Num_Encode( const u8 *pSrc, u8 *pDst, u8 nSrcLength )
{
	u8	nDstLength = nSrcLength;
	u8	i;
	if( nDstLength & 0x01 )
	{
		nDstLength += 1;
	}
	for( i = 0; i < nDstLength; i += 2 )
	{
		*pDst = ( *pSrc << 4 ) | ( *pSrc >> 4 );
		pDst++;
		pSrc++;
	}
	if( nSrcLength & 1 )
	{
		*( pDst - 1 )	&= 0x0f;
		*( pDst - 1 )	|= 0xf0;
	}
	return ( nDstLength >> 1 );
}

// 两两颠倒的字符串转换为正常顺序的字符串
// 如："8613693092030" -. "683196032930F0"
// pSrc: 源字符串指针
// pDst: 目标字符串指针
// nSrcLength: 源字符串长度
// 返回: 目标字符串长度
u8 Hex_Num_Decode( const u8 *pSrc, u8 *pDst, u8 nSrcLength )
{
	u8	nDstLength = nSrcLength;
	u8	i;
	if( nDstLength & 0x01 )
	{
		nDstLength += 1;
	}
	for( i = 0; i < nDstLength; i += 2 )
	{
		*pDst = ( *pSrc << 4 ) | ( *pSrc >> 4 );
		pDst++;
		pSrc++;
	}
	if( nSrcLength & 1 )
	{
		*( pDst - 1 )	&= 0xf0;
		*( pDst - 1 )	|= 0x0f;
	}
	return ( nDstLength >> 1 );
}

//返回号码长度
u8 Que_Number_Length( const u8 *Src )
{
	u8	n = 0;
	u8	m;
	for( m = 0; m < 8; m++ )
	{
		if( ( Src[m] & 0xf0 ) == 0xf0 )
		{
			break;
		}
		n++;
		if( ( Src[m] & 0x0f ) == 0x0f )
		{
			break;
		}
		n++;
	}
	return n;
}

/*********************************************************************************
  *函数名称:u16 SetPhoneNumToPDU(u8 *pDest,u8 *pSrc,u16 len)
  *功能描述:将字符串格式转换成BCD码格式的PDU电话号码，如原始号码为"13820554863"转为13820554863F
  *输    入:pSrc 原始数据，len输出的最大长度
  *输    出:pDest 输出字符串
  *返 回 值:	有效的号码长度
  *作    者:白养民
  *创建日期:2013-05-28
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
u16 SetPhoneNumToPDU( u8 *pDest, char *pSrc, u16 len )
{
	u16 i, j;

	memset( pDest, 0xff, len );
	for( i = 0; i < len; i++ )
	{
		if( ( *pSrc >= 0x30 ) && ( *pSrc <= 0x39 ) )
		{
			if( i % 2 == 0 )
			{
				*pDest	&= 0x0F;
				*pDest	|= ( ( *pSrc - 0x30 ) << 4 );
			}else
			{
				*pDest	&= 0xF0;
				*pDest	|= *pSrc - 0x30;
				pDest++;
			}
		}else
		{
			return i;
		}
		pSrc++;
	}
	return i;
}

/*********************************************************************************
  *函数名称:void GetPhoneNumFromPDU(u8 *pDest,u8 *pSrc,u16 len)
  *功能描述:将BCD码格式的PDU电话号码转换成字符串格式，如原始号码为13820554863F转为"13820554863"
  *输    入:pSrc 原始数据，len输入的最大长度
  *输    出:pDest 输出字符串
  *返 回 值:	有效的号码长度
  *作    者:白养民
  *创建日期:2013-05-28
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
u16 GetPhoneNumFromPDU( char *pDest, u8 *pSrc, u16 len )
{
	u16 i, j;
	for( j = 0, i = 0; i < len; i++ )
	{
		if( ( pSrc[i] & 0x0f ) != 0x0f )
		{
			pDest[j++]	= ( pSrc[i] >> 4 ) + 0x30;
			pDest[j++]	= ( pSrc[i] & 0x0f ) + 0x30;
		}else
		{
			pDest[j++]	= ( pSrc[i] >> 4 ) + 0x30;
			pDest[j]	= 0;
			break;
		}
	}
	return j;
}

//ok_bym
//解码程序
u16   GsmDecodePdu( const u8* pSrc, u16 pSrcLength, SmsType *pSmstype, u8 *DataDst )
{
	u8	nDstLength = 0;                                             // 目标PDU串长度
	u8	tmp;                                                        // 内部用的临时字节变量
	u8	buf[256];                                                   // 内部用的缓冲区
	u16 templen = 0;
	u16 tmp16;
	///0891683108200245F320 0D90 683128504568F3 0008315032419430235E00540057003700300033002300440045005600490043004500490044002800310033003300340035003600370034003800360033002900230049005000310028003100320035002E00330038002E003100380032002E0036003000296D25
	//---SCA
	// SMSC地址信息段
	Ascii_To_Hex( pSrc, &tmp, 2 );                                  // 取长度
	if( tmp > 0 && tmp <= 12 )
	{
		tmp		= ( tmp - 1 ) * 2;                                  // SMSC号码串长度,去掉了91;
		pSrc	+= 4;                                               // 指针后移,长度两个字节，91两个字节。共4个字节
		templen += 4;
		Ascii_To_Hex( pSrc, buf, tmp );                             // 转换SMSC号码到目标PDU串
		Hex_Num_Decode( buf, ( *pSmstype ).SCA, tmp );              //取短信中心号码,保存，回复时用,是HEX码保存的
		pSrc	+= tmp;                                             // 指针后移,此时到了PDUType
		templen += tmp;

		// TPDU段基本参数、回复地址等
		//--PDUType
		Ascii_To_Hex( pSrc, &tmp, 2 );                              // 取基本参数
		pSrc	+= 2;                                               // 指针后移
		templen += 2;
		//--OA----star
		// 包含回复地址，取回复地址信息
		Ascii_To_Hex( pSrc, &tmp, 2 );                              // 取长度,OA的长度是指具体的号码长度，
		if( tmp & 1 )
		{
			tmp += 1;                                               // 调整奇偶性
		}
		pSrc	+= 4;                                               // 指针后移，除去长度，和91,共4个字节
		templen += 4;
		if( tmp > 0 && tmp <= 12 * 2 )
		{
			Ascii_To_Hex( pSrc, buf, tmp );
			Hex_Num_Decode( buf, ( *pSmstype ).TPA, tmp );          // 取TP-RA号码,保存回复地址
			pSrc	+= tmp;                                         // 指针后移
			templen += tmp;
			//--OA---End-------

			///0891683108200245F320 0D90 683128504568F3 0008 31503241943023 5E 00540057003700300033002300440045005600490043004500490044002800310033003300340035003600370034003800360033002900230049005000310028003100320035002E00330038002E003100380032002E0036003000296D25
			//---SCA
			// TPDU段协议标识、编码方式、用户信息等
			Ascii_To_Hex( pSrc, (u8*)&( *pSmstype ).TP_PID, 2 );    // 取协议标识(TP-PID)
			pSrc	+= 2;
			templen += 2;                                           // 指针后移
			Ascii_To_Hex( pSrc, (u8*)&( *pSmstype ).TP_DCS, 2 );    // 取编码方式(TP-DCS),这个解码时和回复时用
			pSrc	+= 2;                                           // 指针后移
			templen += 2;
			//GsmSerializeNumbers(pSrc, m_SmsType.TP_SCTS, 14);        // 服务时间戳字符串(TP_SCTS)
			pSrc	+= 14;                                          // 指针后移
			templen += 14;
			Ascii_To_Hex( pSrc, &tmp, 2 );                          // 用户信息长度(TP-UDL)
			pSrc	+= 2;                                           // 指针后移
			templen += 2;
			// pDst.TP_DCS=8;
			if( ( ( *pSmstype ).TP_DCS & 0x0c ) == GSM_7BIT )
			{
				// 7-bit解码
				tmp16	= tmp % 8 ? ( (u16)tmp * 7 / 8 + 1 ) : ( (u16)tmp * 7 / 8 );
				tmp16	*= 2;
				if( ( templen + tmp16 <= pSrcLength ) && ( tmp16 < 400 ) )
				{
					nDstLength = Ascii_To_Hex( pSrc, buf, tmp16 );  // 格式转换
					GsmDecode7bit( buf, DataDst, nDstLength );      // 转换到TP-DU
					nDstLength = tmp;
				}
			}else if( ( ( *pSmstype ).TP_DCS & 0x0c ) == GSM_UCS2 )
			{
				// UCS2解码
				tmp16 = tmp * 2;
				if( ( templen + tmp16 <= pSrcLength ) && ( tmp16 < 400 ) )
				{
					nDstLength	= Ascii_To_Hex( pSrc, buf, tmp16 );             // 格式转换
					nDstLength	= GsmDecodeUcs2( buf, DataDst, nDstLength );    // 转换到TP-DU
				}
			}else
			{
				// 8-bit解码
				tmp16 = tmp * 2;
				if( ( templen + tmp16 <= pSrcLength ) && ( tmp16 < 512 ) )
				{
					nDstLength	= Ascii_To_Hex( pSrc, buf, tmp16 );             // 格式转换
					nDstLength	= GsmDecode8bit( buf, DataDst, nDstLength );    // 转换到TP-DU
				}
			}
		}
	}
	// 返回目标字符串长度
	return nDstLength;
}

//不加短信中心号码
u16   GsmEncodePdu_NoCenter( const SmsType pSrc, const u8 *DataSrc, u16 datalen, u8* pDst )
{
	u16 nLength;                                                    // 内部用的串长度
	u16 nDstLength = 0;                                             // 目标PDU串长度
	u8	buf[256];                                                   // 内部用的缓冲区

	// SMSC地址信息段
	buf[0]		= 0;                                                //SCA
	nDstLength	+= Hex_To_Ascii( buf, pDst, 1 );
	// TPDU段基本参数、目标地址等
	buf[0]		= 0x11;                                             //PDUTYPE
	buf[1]		= 0x00;                                             //MR
	nDstLength	+= Hex_To_Ascii( buf, &pDst[nDstLength], 2 );
	// SMSC地址信息段
	nLength		= Que_Number_Length( pSrc.TPA );                    // TP-DA地址字符串的长度
	buf[0]		= (u8)nLength;                                      // 目标地址数字个数(TP-DA地址字符串真实长度)
	buf[1]		= 0x91;                                             // 固定: 用国际格式号码
	nDstLength	+= Hex_To_Ascii( buf, &pDst[nDstLength], 2 );
	nLength		= Hex_Num_Encode( pSrc.TPA, buf, nLength );
	nDstLength	+= Hex_To_Ascii( buf, &pDst[nDstLength], nLength ); // 转换TP-DA到目标PDU串

	// TPDU段协议标识、编码方式、用户信息等
	buf[0]		= 0;                                                // 协议标识(TP-PID)
	buf[1]		= pSrc.TP_DCS & 0x0c;                               // 用户信息编码方式(TP-DCS)
	buf[2]		= 0x8f;                                             // 有效期(TP-VP)为12小时
	nDstLength	+= Hex_To_Ascii( buf, &pDst[nDstLength], 3 );
	if( ( pSrc.TP_DCS & 0x0c ) == GSM_7BIT )
	{
		// 7-bit编码方式
		buf[0]	= datalen;                                          // 编码前长度.7位方式表示编码前的长度
		nLength = GsmEncode7bit( DataSrc, &buf[1], datalen );
		nLength += 1;
		// 转换		TP-DA到目标PDU串
	}else if( ( pSrc.TP_DCS & 0x0c ) == GSM_UCS2 )
	{
		// UCS2编码方式
		buf[0]	= GsmEncodeUcs2( DataSrc, &buf[1], datalen );       // 转换TP-DA到目标PDU串
		nLength = buf[0] + 1;                                       // nLength等于该段数据长度
	}else
	{
		// 8-bit编码方式
		buf[0]	= GsmEncode8bit( DataSrc, &buf[1], datalen );       // 转换TP-DA到目标PDU串
		nLength = buf[0] + 1;                                       // nLength等于该段数据长度
	}
	nDstLength += Hex_To_Ascii( buf, &pDst[nDstLength], nLength );  // 转换该段数据到目标PDU串

	// 返回目标字符串长度
	return nDstLength;
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
u16   AnySmsEncode_NoCenter( const u8 *SrcNumber, u8 type, const u8 *DataSrc, u16 datalen, u8* pDst )
{
	SmsType tmpSms;
	u8		len8;
	len8 = Que_Number_Length( SrcNumber );
	if( *SrcNumber == 0x86 )    ///有0x86标记
	{
		len8 = ( len8 + 1 ) >> 1;
		//tmpSms.TPA[0]=0x86;
		memcpy( &tmpSms.TPA[0], SrcNumber, len8 );
	}else                       ///没有0x86标记
	{
		len8			= ( len8 + 1 ) >> 1;
		tmpSms.TPA[0]	= 0x86;
		memcpy( &tmpSms.TPA[1], SrcNumber, len8 );
	}
	tmpSms.TP_DCS	= type & 0x0c;
	tmpSms.TP_PID	= 0;
	return ( GsmEncodePdu_NoCenter( tmpSms, DataSrc, datalen, pDst ) );
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
u16 GsmEncodePdu_Center( const SmsType pSrc, const u8 *DataSrc, u16 datalen, u8* pDst )
{
	u16 nLength;                                                                // 内部用的串长度
	u16 nDstLength = 0;                                                         // 目标PDU串长度
	u8	buf[256];                                                               // 内部用的缓冲区

	// SMSC地址信息段
	nLength		= Que_Number_Length( pSrc.SCA );                                // SMSC地址字符串的长度
	buf[0]		= (u8)( ( nLength & 1 ) == 0 ? nLength : nLength + 1 ) / 2 + 1; // SMSC地址信息长度
	buf[1]		= 0x91;                                                         // 固定: 用国际格式号码
	nDstLength	= Hex_To_Ascii( buf, pDst, 2 );                                 // 转换2个字节到目标PDU串
	nLength		= Hex_Num_Encode( pSrc.SCA, buf, nLength );
	nDstLength	+= Hex_To_Ascii( buf, &pDst[nDstLength], nLength );             // 转换SMSC到目标PDU串
	// TPDU段基本参数、目标地址等
	buf[0]		= 0x11;                                                         //PDUTYPE
	buf[1]		= 0x00;                                                         //MR
	nDstLength	+= Hex_To_Ascii( buf, &pDst[nDstLength], 2 );
	// SMSC地址信息段
	nLength		= Que_Number_Length( pSrc.TPA );                                // TP-DA地址字符串的长度
	buf[0]		= (u8)nLength;                                                  // 目标地址数字个数(TP-DA地址字符串真实长度)
	buf[1]		= 0x91;                                                         // 固定: 用国际格式号码
	nDstLength	+= Hex_To_Ascii( buf, &pDst[nDstLength], 2 );
	nLength		= Hex_Num_Encode( pSrc.TPA, buf, nLength );
	nDstLength	+= Hex_To_Ascii( buf, &pDst[nDstLength], nLength );             // 转换TP-DA到目标PDU串

	// TPDU段协议标识、编码方式、用户信息等
	buf[0]		= 0;                                                            // 协议标识(TP-PID)
	buf[1]		= pSrc.TP_DCS & 0x0c;                                           // 用户信息编码方式(TP-DCS)
	buf[2]		= 0x8f;                                                         // 有效期(TP-VP)为12小时
	nDstLength	+= Hex_To_Ascii( buf, &pDst[nDstLength], 3 );
	if( ( pSrc.TP_DCS & 0x0c ) == GSM_7BIT )
	{
		// 7-bit编码方式
		buf[0]	= datalen;                                                      // 编码前长度.7位方式表示编码前的长度
		nLength = GsmEncode7bit( DataSrc, &buf[1], datalen );
		nLength += 1;
		// 转换		TP-DA到目标PDU串
	}else if( ( pSrc.TP_DCS & 0x0c ) == GSM_UCS2 )
	{
		// UCS2编码方式
		buf[0]	= GsmEncodeUcs2( DataSrc, &buf[1], datalen );                   // 转换TP-DA到目标PDU串
		nLength = buf[0] + 1;                                                   // nLength等于该段数据长度
	}else
	{
		// 8-bit编码方式
		buf[0]	= GsmEncode8bit( DataSrc, &buf[1], datalen );                   // 转换TP-DA到目标PDU串
		nLength = buf[0] + 1;                                                   // nLength等于该段数据长度
	}
	nDstLength += Hex_To_Ascii( buf, &pDst[nDstLength], nLength );              // 转换该段数据到目标PDU串

	// 返回目标字符串长度
	return nDstLength;
}

/*
   // 两两颠倒的字符串转换为正常顺序的字符串
   // 如："8613693092030" -. "683196032930F0"
   // pSrc: 源字符串指针
   // pDst: 目标字符串指针
   // nSrcLength: 源字符串长度
   // 返回: 目标字符串长度
   INT16U  GsmSerializeNumbers(const INT8U* pSrc, INT8U* pDst, INT16U nSrcLength)
   {

   INT16U nDstLength;   // 目标字符串长度
    INT8U ch;          // 用于保存一个字符

    // 复制串长度
    nDstLength = nSrcLength;
   // 两两颠倒
    for(INT16U i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;        // 保存先出现的字符
   *pDst++ = *pSrc++;   // 复制后出现的字符
   *pDst++ = ch;        // 复制先出现的字符
    }

    // 最后的字符是'F'吗？
    if(*(pDst-1) == 'F')
    {
        pDst--;
        nDstLength--;        // 目标字符串长度减1
    }

    // 输出字符串加个结束符
   *pDst = '\0';

    // 返回目标字符串长度
    return nDstLength;
   }

   //PDU串中的号码和时间，都是两两颠倒的字符串。利用下面两个函数可进行正反变换：
   // 正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
   // 如："8613693092030" -. "683196032930F0"
   // pSrc: 源字符串指针
   // pDst: 目标字符串指针
   // nSrcLength: 源字符串长度
   // 返回: 目标字符串长度
   INT16U  GsmInvertNumbers(const INT8U* pSrc, INT8U* pDst, INT16U nSrcLength)
   {

   INT16U nDstLength;   // 目标字符串长度
    INT8U ch;          // 用于保存一个字符

    // 复制串长度
    nDstLength = nSrcLength;

    // 两两颠倒
    for(INT16U i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;        // 保存先出现的字符
   *pDst++ = *pSrc++;   // 复制后出现的字符
   *pDst++ = ch;        // 复制先出现的字符
    }

    // 源串长度是奇数吗？
    if(nSrcLength & 1)
   {
   *(pDst-2) = 'F';     // 补'F'
        nDstLength++;        // 目标串长度加1
    }

    // 输出字符串加个结束符
   *pDst = '\0';

    // 返回目标字符串长度
    return nDstLength;
   }

   //加短信中心号码
   INT16U  GsmEncodePdu(const SM_PARAM* pSrc, INT8U* pDst)
   {

   INT16U nLength;             // 内部用的串长度
    INT16U nDstLength;          // 目标PDU串长度
    INT8U buf[256];  // 内部用的缓冲区

    // SMSC地址信息段
    nLength = strlen(pSrc.SCA);    // SMSC地址字符串的长度
    buf[0] = (INT8U)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;    // SMSC地址信息长度
    buf[1] = 0x91;        // 固定: 用国际格式号码
    nDstLength = Hex_To_Ascii(buf, pDst, 2);        // 转换2个字节到目标PDU串
    nDstLength += GsmInvertNumbers(pSrc.SCA, &pDst[nDstLength], nLength);    // 转换SMSC到目标PDU串
     // TPDU段基本参数、目标地址等
    nLength = strlen(pSrc.TPA);    // TP-DA地址字符串的长度
   CString rec_number=CString(pSrc.TPA);
    buf[0] = 0x11;            // 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
    buf[1] = 0;               // TP-MR=0
    buf[2] = (INT8U)nLength;   // 目标地址数字个数(TP-DA地址字符串真实长度)
    buf[3] = 0x91;            // 固定: 用国际格式号码
    nDstLength += Hex_To_Ascii(buf, &pDst[nDstLength], 4);  // 转换4个字节到目标PDU串
    nDstLength += GsmInvertNumbers(pSrc.TPA, &pDst[nDstLength], nLength); // 转换TP-DA到目标PDU串

    // TPDU段协议标识、编码方式、用户信息等
    nLength = strlen(pSrc.TP_UD);    // 用户信息字符串的长度
    buf[0] = pSrc.TP_PID;        // 协议标识(TP-PID)
    buf[1] = pSrc.TP_DCS;        // 用户信息编码方式(TP-DCS)
    buf[2] = 0;            // 有效期(TP-VP)为5分钟
    if(pSrc.TP_DCS == GSM_7BIT)
    {
        // 7-bit编码方式
        buf[3] = nLength;            // 编码前长度
        nLength = GsmEncode7bit(pSrc.TP_UD, &buf[4], nLength+1) + 4;
   // 转换		TP-DA到目标PDU串
    }
    else if(pSrc.TP_DCS == GSM_UCS2)
    {
        // UCS2编码方式
        buf[3] = GsmEncodeUcs2(pSrc.TP_UD, &buf[4], nLength);    // 转换TP-DA到目标PDU串
        nLength = buf[3] + 4;        // nLength等于该段数据长度
    }
    else
    {
        // 8-bit编码方式
        buf[3] = GsmEncode8bit(pSrc.TP_UD, &buf[4], nLength);    // 转换TP-DA到目标PDU串
        nLength = buf[3] + 4;        // nLength等于该段数据长度
    }
    nDstLength += Hex_To_Ascii(buf, &pDst[nDstLength], nLength);        // 转换该段数据到目标PDU串

    // 返回目标字符串长度
    return nDstLength;
   }
 */


/*********************************************************************************
  *函数名称:static void SMS_SendConsoleStr(char *s)
  *功能描述:向调试口发送长字符串，因为rt_kprintf有字节长度限制
  *输    入:s 需要输出的字符串
  *输    出:none
  *返 回 值:none
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
static void SMS_SendConsoleStr( char *s )
{
	u16 i;
	for( i = 0; i < strlen( s ); i++ )
	{
		rt_kprintf( "%c", *( s + i ) );
	}
}

/*********************************************************************************
  *函数名称:void SMS_timer(u8 *instr,u16 len)
  *功能描述:短信处理函数，这个函数需要在一个1秒的定时器里面调用，用于函数"SMS_Process"的定时处理等
  *输    入:none
  *输    出:none
  *返 回 值:none
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
void SMS_timer( void )
{
	if( SMS_Service.SMS_waitCounter )
	{
		SMS_Service.SMS_waitCounter--;
	}


	/*
	   if(SMS_Service.SMS_delALL>1)
	   {
	   SMS_Service.SMS_delALL--;
	   }
	 */
	//-------- 短信相关 ------------------


	/*
	   if(SMS_Service.SMS_come==1)
	   {
	   SMS_Service.SMS_delayCounter++;
	   if(SMS_Service.SMS_delayCounter)
	   {
	   SMS_Service.SMS_delayCounter=0;
	   SMS_Service.SMS_come=0;
	   SMS_Service.SMS_read=3;	  // 使能读取
	   }
	   }
	 */
}

/*********************************************************************************
  *函数名称:u8 SMS_Rx_Text(char *instr,char *strDestNum)
  *功能描述:接收到TEXT格式的短信处理函数
  *输    入:instr 原始短信数据，strDestNum接收到得信息的发送方号码
  *输    出:none
  *返 回 值:	1:正常完成，
   0:表示失败
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
u8 SMS_Rx_Text( char *instr, char *strDestNum )
{
	u16 len;
	u8	ret = 0;
	len = strlen( strDestNum );
	memset( SMS_Service.SMS_destNum, 0, sizeof( SMS_Service.SMS_destNum ) );
	if( len > sizeof( SMS_Service.SMS_destNum ) )
	{
		len = sizeof( SMS_Service.SMS_destNum );
	}
	memcpy( SMS_Service.SMS_destNum, strDestNum, len );

	len = strlen( instr );
#if 1
	memset( SMS_Service.SMS_rx_Content, 0, sizeof( SMS_Service.SMS_rx_Content ) );
	if( len > sizeof( SMS_Service.SMS_rx_Content ) )
	{
		len = sizeof( SMS_Service.SMS_rx_Content );
	}
	memcpy( SMS_Service.SMS_rx_Content, instr, len );
	SMS_Service.rx_state = SMS_RX_OK;
#endif
#if 0
	rt_kprintf( "\r\n  短息来源号码:%s", SMS_Service.SMS_destNum );
	rt_kprintf( "\r\n 短信收到消息: " );
	rt_kprintf( instr );
	if( strncmp( (char*)instr, "TW703#", 6 ) == 0 )                                             //短信修改UDP的IP和端口
	{
		//-----------  自定义 短息设置修改 协议 ----------------------------------
		SMS_protocol( instr + 5, len - 5, SMS_ACK_msg );
		ret = 1;
	}
#endif
	SMS_Service.SMS_read		= 0;
	SMS_Service.SMS_waitCounter = 0;
	SMS_Service.SMS_Style		= 0;
	//SMS_Service.SMS_delALL		= 1;
	return ret;
}

/*********************************************************************************
  *函数名称:u8 SMS_Rx_PDU(char *instr,u16 len)
  *功能描述:接收到PDU格式的短信处理函数
  *输    入:instr 原始短信数据，len接收到得信息长度，单位为字节
  *输    出:none
  *返 回 值:	1:正常完成，
   0:表示失败
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
u8 SMS_Rx_PDU( char *instr, u16 len )
{
	char	*pstrTemp;
	u8		ret = 0;
#if 1
	//rt_kprintf( "\r\n PDU_短信: " );
	//SMS_SendConsoleStr( instr );
	memset( SMS_Service.SMS_rx_Content, 0, sizeof( SMS_Service.SMS_rx_Content ) );

	len = GsmDecodePdu( instr, len, &SMS_Service.Sms_Info, SMS_Service.SMS_rx_Content );
	GetPhoneNumFromPDU( SMS_Service.SMS_destNum, SMS_Service.Sms_Info.TPA, sizeof( SMS_Service.Sms_Info.TPA ) );
	SMS_Service.rx_state = SMS_RX_OK;
#endif
#if 0
	memset( SMS_Service.SMS_destNum, 0, sizeof( SMS_Service.SMS_destNum ) );
	pstrTemp = (char*)rt_malloc( 200 ); ///短信解码后的完整内容，解码后汉子为GB码
	memset( pstrTemp, 0, 200 );
	//rt_kprintf( "\r\n 短信原始消息: " );
	//SMS_SendConsoleStr( instr );

	len = GsmDecodePdu( instr, len, &SMS_Service.Sms_Info, pstrTemp );
	GetPhoneNumFromPDU( SMS_Service.SMS_destNum, SMS_Service.Sms_Info.TPA, sizeof( SMS_Service.Sms_Info.TPA ) );

	rt_kprintf( "\r\n  短息来源号码:%s \r\n", SMS_Service.SMS_destNum );
	rt_kprintf( "\r\n 短信消息: " );
	SMS_SendConsoleStr( pstrTemp );
	//rt_hw_console_output(instr);
	if( strncmp( (char*)pstrTemp, "TW703#", 6 ) == 0 )                                             //短信修改UDP的IP和端口
	{
		//-----------  自定义 短息设置修改 协议 ----------------------------------
		SMS_protocol( pstrTemp + 5, len - 5, SMS_ACK_msg );
		ret = 1;
	}
#endif
	SMS_Service.SMS_read		= 0;
	SMS_Service.SMS_waitCounter = 0;
	SMS_Service.SMS_Style		= 1;
	//SMS_Service.SMS_delALL		= 1;
	//rt_free( pstrTemp );
	//pstrTemp = RT_NULL;
	//////
	return ret;
}

/*********************************************************************************
  *函数名称:u8 SMS_Tx_Text(char *strDestNum,char *s)
  *功能描述:发送TEXT格式的短信函数
  *输    入:s 原始短信数据，strDestNum接收方号码
  *输    出:none
  *返 回 值:	1:正常完成，
   0:表示失败
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
u8 SMS_Tx_Text( char *strDestNum, char *s )
{
	u16		len;
	char	*pstrTemp;

	memset( SMS_Service.SMSAtSend, 0, sizeof( SMS_Service.SMSAtSend ) );
	strcpy( ( char* )SMS_Service.SMSAtSend, "AT+CMGS=\"" );
	//strcat(SMS_Service.SMSAtSend,"8613820554863");// Debug
	strcat( SMS_Service.SMSAtSend, strDestNum );
	strcat( SMS_Service.SMSAtSend, "\"\r\n" );

	rt_kprintf( "\r\n%s", SMS_Service.SMSAtSend );
	at( ( char* )SMS_Service.SMSAtSend );

	rt_thread_delay( 50 );

	pstrTemp = rt_malloc( PHONEMAXSTR );
	memset( pstrTemp, 0, PHONEMAXSTR );
	len = strlen( s );
	memcpy( pstrTemp, s, len );
	pstrTemp[len++] = 0x1A; // message  end

	///发送调试信息
	SMS_SendConsoleStr( pstrTemp );
	///发送到GSM模块
	at( pstrTemp, len );
	rt_free( pstrTemp );
	pstrTemp = RT_NULL;
	return 1;
}

FINSH_FUNCTION_EXPORT( SMS_Tx_Text, SMS_Tx_Text );


/*********************************************************************************
  *函数名称:u8 SMS_Tx_PDU(char *strDestNum,char *s)
  *功能描述:发送PDU格式的短信函数
  *输    入:s 原始短信数据，strDestNum接收方号码
  *输    出:none
  *返 回 值:	1:正常完成，
   0:表示失败
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
u8 SMS_Tx_PDU( char *strDestNum, char *s )
{
	u16		len;
	u16		i;
	char	*pstrTemp;
	memset( SMS_Service.SMSAtSend, 0, sizeof( SMS_Service.SMSAtSend ) );
	pstrTemp = rt_malloc( 400 );
	memset( pstrTemp, 0, 400 );
	i = 0;
	SetPhoneNumToPDU( SMS_Service.Sms_Info.TPA, strDestNum, sizeof( SMS_Service.Sms_Info.TPA ) );
	len = AnySmsEncode_NoCenter( SMS_Service.Sms_Info.TPA, GSM_UCS2, s, strlen( s ), pstrTemp );
	//len=strlen(pstrTemp);
	pstrTemp[len++] = 0x1A; // message  end
	//////
	sprintf( ( char* )SMS_Service.SMSAtSend, "AT+CMGS=%d\r\n", ( len - 2 ) / 2 );
	//rt_kprintf("%s",SMS_Service.SMSAtSend);
	//at( ( char * ) SMS_Service.SMSAtSend );

	if( gsm_send( SMS_Service.SMSAtSend, RT_NULL, ">", RESP_TYPE_STR, RT_TICK_PER_SECOND, 1 ) == RT_EOK )
	{
		//rt_thread_delay(50);
		//////
		//SMS_SendConsoleStr( pstrTemp );
		//rt_hw_console_output(pstrTemp);
		//at(pstrTemp, len);

		gsm_send( pstrTemp, RT_NULL, RT_NULL, RESP_TYPE_STR, RT_TICK_PER_SECOND / 2, 1 );
	}
	rt_free( pstrTemp );
	pstrTemp = RT_NULL;
	return 1;
}

FINSH_FUNCTION_EXPORT( SMS_Tx_PDU, SMS_Tx_PDU );


/*********************************************************************************
  *函数名称:u8 SMS_Rx_Notice(u16 indexNum)
  *功能描述:模块收到新短信通知
  *输    入:新短信的索引号
  *输    出:none
  *返 回 值:	1:正常完成，
   0:表示失败
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
u8 SMS_Rx_Notice( u16 indexNum )
{
	SMS_Service.SMIndex = indexNum;
	rt_kprintf( " index=%d", SMS_Service.SMIndex );
	SMS_Service.SMS_read		= 3;
	SMS_Service.SMS_waitCounter = 1;
	return 1;
}

///测试函数，测试SMS短信接收处理命令解析的正确性
void SMS_Test( char * s )
{
	SMS_protocol( s, strlen( s ), SMS_ACK_msg );
}

FINSH_FUNCTION_EXPORT( SMS_Test, SMS_Test );

///测试函数，测试PDU数据包的接收解析功能
void SMS_PDU( char *s )
{
	u16		len;
	u16		i, j;
	char	*pstrTemp;
	pstrTemp	= (char*)rt_malloc( 160 ); ///短信解码后的完整内容，解码后汉子为GB码
	len			= GsmDecodePdu( s, strlen( s ), &SMS_Service.Sms_Info, pstrTemp );
	GetPhoneNumFromPDU( SMS_Service.SMS_destNum, SMS_Service.Sms_Info.SCA, sizeof( SMS_Service.Sms_Info.SCA ) );
	rt_kprintf( "\r\n  短息中心号码:%s \r\n", SMS_Service.SMS_destNum );
	GetPhoneNumFromPDU( SMS_Service.SMS_destNum, SMS_Service.Sms_Info.TPA, sizeof( SMS_Service.Sms_Info.TPA ) );
	rt_kprintf( "\r\n  短息来源号码:%s \r\n", SMS_Service.SMS_destNum );
	rt_kprintf( "\r\n 短信消息:\"%s\"\r\n", pstrTemp );
	rt_free( pstrTemp );
	pstrTemp = RT_NULL;
}

FINSH_FUNCTION_EXPORT( SMS_PDU, SMS_PDU );


/*********************************************************************************
  *函数名称:rt_err_t sms_init(void)
  *功能描述:短信初始化程序，完成短信消息队列及相关参数的初始化
  *输    入:
  *输    出:
  *返 回 值:	rt_err_t
  *作    者:白养民
  *创建日期:2013-06-8
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
rt_err_t sms_init( void )
{
	SMS_Service.tx_state	= SMS_TX_IDLE;
	SMS_Service.rx_state	= SMS_RX_IDLE;
	return rt_mb_init( &mb_smsdata, "smsdata", &mb_smsdata_pool, MB_SMSDATA_POOL_SIZE / 4, RT_IPC_FLAG_FIFO );
}

/*********************************************************************************
  *函数名称:rt_err_t sms_tx_sms(char *strDestNum,char *s)
  *功能描述:短信发送函数，该函数并不是直接将短信发出，而是将短信放入发送队列中，
   等待之前的短信成功发送后在按照先后顺序发送。
  *输    入:
  *输    出:
  *返 回 值:	rt_err_t
  *作    者:白养民
  *创建日期:2013-06-8
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
rt_err_t sms_tx_sms( char *strDestNum, char *s )
{
	u8				temp_u8;
	SMS_Send_Msg	*pmsg = RT_NULL;
	pmsg = rt_malloc( strlen( s ) + PHONEMAXNUM ); /*包含长度信息*/
	if( pmsg != RT_NULL )
	{
		if( strlen( strDestNum ) > PHONEMAXNUM )
		{
			temp_u8 = PHONEMAXNUM;
		} else
		{
			temp_u8 = strlen( strDestNum );
		}
		memset( pmsg->SMS_destNum, 0, PHONEMAXNUM );
		memcpy( pmsg->SMS_destNum, strDestNum, temp_u8 );
		memcpy( pmsg->pstr, s, strlen( s ) );
		return rt_mb_send( &mb_smsdata, ( rt_uint32_t )pmsg );
	}
	return RT_ERROR;
}

/*********************************************************************************
  *函数名称:u8 sms_rx_pro(u8 *psrc,u16 len)
  *功能描述:M66接收模块数据分发线程调用该函数，完成SMS短信的接收处理过程，该函数会和sms_process()函
   数配合使用完成短信的发送和接收过程
  *输    入:psrc M66模块接收到得一条字符串的指针，len 字符串长度
  *输    出:
  *返 回 值:	1:表示该消息属于短信消息
   0:表示该消息不可识别
  *作    者:白养民
  *创建日期:2013-06-8
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
u8 SMS_rx_pro( char *psrc, u16 len )
{
	u16			i, j, q;
	static u8	SMS_Rx_State = 0;
	static char strTempBuf[15];

	if( SMS_Rx_State )
	{
		if( SMS_Rx_State == 1 ) ///PDU模式
		{
			SMS_Rx_PDU( psrc, len );
		}else ///TEXT模式
		{
			SMS_Rx_Text( psrc, strTempBuf );
		}
		SMS_Rx_State = 0;
		return 1;
	}else if( strncmp( psrc, "+CMT: ", 6 ) == 0 )
	{
		if( psrc[6] == ',' ) ///PDU模式
		{
			SMS_Rx_State = 1;
		}else ///TEXT模式
		{
			SMS_Rx_State	= 2;
			j				= 0;
			q				= 0;
			memset( strTempBuf, 0, sizeof( strTempBuf ) );
			for( i = 6; i < len; i++ )
			{
				if( ( j == 1 ) && ( psrc[i] != '"' ) )
				{
					strTempBuf[q++] = psrc[i];
				}
				if( psrc[i] == '"' )
				{
					j++;
					if( j > 1 )
					{
						break;
					}
				}
			}
			//rt_kprintf( "\r\n  短息来源号码:%s \r\n", reg_str );
		}
		return 1;
	}else if( strncmp( psrc, "+CMTI: \"SM\",", 12 ) == 0 )
	{
		rt_kprintf( "\r\n收到短信:" );
		j = sscanf( psrc + 12, "%d", &i );
		if( j )
		{
			SMS_Service.SMIndex		= j;
			SMS_Service.rx_state	= SMS_RX_START;
			return 1;
		}
	}
#ifdef SMS_TYPE_PDU
	else if( strncmp( (char*)psrc, "+CMGR:", 6 ) == 0 )
	{
		SMS_Rx_State = 1;
		return 1;
	}
#else
	else if( strncmp( (char*)psrc, "+CMGR:", 6 ) == 0 )
	{
		//+CMGR: "REC UNREAD","8613602069191", ,"13/05/16,13:05:29+35"
		// 获取要返回短息的目的号码
		j	= 0;
		q	= 0;
		memset( strTempBuf, 0, sizeof( strTempBuf ) );
		for( i = 6; i < 50; i++ )
		{
			if( ( j == 3 ) && ( psrc[i] != '"' ) )
			{
				strTempBuf[q++] = psrc[i];
			}
			if( psrc[i] == '"' )
			{
				j++;
			}
		}
		SMS_Rx_State = 2;
		return 1;
	}
#endif

	////发送短信相关
	else if( strncmp( (char*)psrc, "+CMGS:", 6 ) == 0 )
	{
		if( SMS_Service.tx_state == SMS_TX_WAITACK )
		{
			SMS_Service.tx_state = SMS_TX_OK;
		}
		return 1;
	}else if( strncmp( (char*)psrc, "+CMS ERROR:", 11 ) == 0 )
	{
		if( SMS_Service.tx_state == SMS_TX_WAITACK )
		{
			SMS_Service.tx_state = SMS_TX_START;
		}
		return 1;
	}
	return 0;
}

/*********************************************************************************
  *函数名称:void SMS_protocol(u8 *instr,u16 len)
  *功能描述:短信处理函数，这个函数需要在一个线程里面调用，进行相关处理(短信读取，删除和自动发送相关)
  *输    入:none
  *输    出:none
  *返 回 值:none
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
void SMS_Process_old( void )
{
	u16				ContentLen = 0;
	u16				i, j, k;
	char			*pstrTemp;
	SMS_Send_Msg	*pmsg = RT_NULL;

	if( SMS_Service.SMS_waitCounter )
	{
		return;
	}
	//-----------  短信处理相关 --------------------------------------------------------
	//---------------------------------
	if( SMS_Service.SMS_read ) // 读取短信
	{
		memset( SMS_Service.SMSAtSend, 0, sizeof( SMS_Service.SMSAtSend ) );
		sprintf( SMS_Service.SMSAtSend, "AT+CMGR=%d\r\n", SMS_Service.SMIndex );
		rt_kprintf( "%s", SMS_Service.SMSAtSend );
		at( ( char* )SMS_Service.SMSAtSend );
		SMS_Service.SMS_read--;
		SMS_Service.SMS_waitCounter = 3;
	}
	//-------------------------------
	//       发送短息确认
	else if( SMS_Service.SMS_sendFlag == 1 )
	{
		//#ifdef SMS_TYPE_PDU
		if( SMS_Service.SMS_Style == 1 )
		{
			memset( SMS_Service.SMSAtSend, 0, sizeof( SMS_Service.SMSAtSend ) );
			///申请600字节空间
			pstrTemp = rt_malloc( 400 );
			memset( pstrTemp, 0, 400 );
			///将字符串格式的目的电话号码设置为PDU格式的号码
			SetPhoneNumToPDU( SMS_Service.Sms_Info.TPA, SMS_Service.SMS_destNum, sizeof( SMS_Service.Sms_Info.TPA ) );
			///生成PDU格式短信内容
			ContentLen = AnySmsEncode_NoCenter( SMS_Service.Sms_Info.TPA, GSM_UCS2, SMS_Service.SMS_sd_Content, strlen( SMS_Service.SMS_sd_Content ), pstrTemp );
			//ContentLen=strlen(pstrTemp);
			///添加短信尾部标记"esc"
			pstrTemp[ContentLen] = 0x1A; // message  end
			//////
			sprintf( ( char* )SMS_Service.SMSAtSend, "AT+CMGS=%d\r\n", ( ContentLen - 2 ) / 2 );
			rt_kprintf( "%s", SMS_Service.SMSAtSend );
			at( ( char* )SMS_Service.SMSAtSend );
			rt_thread_delay( 50 );
			//////
			//rt_kprintf("%s",pstrTemp);
			SMS_SendConsoleStr( pstrTemp );
			at( ( char* )pstrTemp, ContentLen + 1 );
			rt_free( pstrTemp );
			pstrTemp = RT_NULL;
		}
		//#else
		else
		{
			memset( SMS_Service.SMSAtSend, 0, sizeof( SMS_Service.SMSAtSend ) );
			strcpy( ( char* )SMS_Service.SMSAtSend, "AT+CMGS=\"" );
			//strcat(SMS_Service.SMSAtSend,"8613820554863");// Debug
			strcat( SMS_Service.SMSAtSend, SMS_Service.SMS_destNum );
			strcat( SMS_Service.SMSAtSend, "\"\r\n" );

			rt_kprintf( "\r\n%s", SMS_Service.SMSAtSend );
			at( ( char* )SMS_Service.SMSAtSend );

			rt_thread_delay( 50 );
			ContentLen								= strlen( SMS_Service.SMS_sd_Content );
			SMS_Service.SMS_sd_Content [ContentLen] = 0x1A; // message  end
			rt_kprintf( "%s", SMS_Service.SMS_sd_Content );
			at( ( char* )SMS_Service.SMS_sd_Content, ContentLen + 1 );
		}
		//#endif
		SMS_Service.SMS_sendFlag	= 0;                    // clear
		SMS_Service.SMS_waitCounter = 3;
	}


	/*
	   else if(SMS_Service.SMS_delALL==1)	  //删除短信
	   {
	   memset(SMS_Service.SMSAtSend,0,sizeof(SMS_Service.SMSAtSend));
	   ///
	   //sprintf(SMS_Service.SMSAtSend,"AT+CMGD=%d\r\n",SMS_Service.SMIndex);
	   sprintf(SMS_Service.SMSAtSend,"AT+CMGD=0,4\r\n",SMS_Service.SMIndex);
	   rt_kprintf("%s",SMS_Service.SMSAtSend);
	   ///
	   at( ( char * )SMS_Service.SMSAtSend );
	   SMS_Service.SMS_delALL=0;
	   SMS_Service.SMS_waitCounter=3;
	   }
	 */
}

/*********************************************************************************
  *函数名称:void SMS_protocol(u8 *instr,u16 len)
  *功能描述:短信处理函数，这个函数需要在一个线程里面调用，进行相关处理(短信读取，删除和自动发送相关)
  *输    入:none
  *输    出:none
  *返 回 值:none
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
void SMS_Process( void )
{
	u16				ContentLen = 0;
	u16				i, j, k;
	char			*pstrTemp;
	SMS_Send_Msg	*pmsg = RT_NULL;
	static uint32_t rx_tick;
	static uint32_t tx_tick;
	static char		tx_dest_num[PHONEMAXNUM];
	static char		tx_dest_str[PHONEMAXSTR];

	///短信发送相关处理
	switch( SMS_Service.tx_state )
	{
		case SMS_TX_IDLE:
		{
			if( RT_EOK == rt_mb_recv( &mb_smsdata, ( rt_uint32_t* )&pmsg, 0 ) )
			{
				if( RT_NULL != pmsg )
				{
					pstrTemp = (char*)pmsg;
					memset( tx_dest_num, 0, sizeof( tx_dest_num ) );
					memset( tx_dest_str, 0, sizeof( tx_dest_str ) );
					memcpy( tx_dest_num, pstrTemp, PHONEMAXNUM );
					pstrTemp += PHONEMAXNUM;
					memcpy( tx_dest_str, pstrTemp, strlen( pstrTemp ) );
					SMS_Service.tx_state	= SMS_TX_START;
					SMS_Service.tx_retry	= 0;
				}
				rt_free( (void*)pmsg );
			}
			break;
		}
		case SMS_TX_START:
		{
			if( SMS_Service.tx_retry >= 3 )
			{
				SMS_Service.tx_state = SMS_TX_FALSE;
				break;
			}
			SMS_Tx_PDU( tx_dest_num, tx_dest_str );
			SMS_Service.tx_retry++;
			//SMS_Service.tx_state=SMS_TX_WAITACK;	///判断是否成功发送
			SMS_Service.tx_state	= SMS_TX_WAITACK;                        ///不判断是否成功发送，直接返回成功
			tx_tick					= rt_tick_get( );
			break;
		}
		case SMS_TX_WAITACK:
		{
			if( rt_tick_get( ) - tx_tick > RT_TICK_PER_SECOND * 20 )    ///判读是否超时
			{
				SMS_Service.tx_state = SMS_TX_START;
			}
			break;
		}
		case SMS_TX_OK:
		{
			SMS_Service.tx_state = SMS_TX_IDLE;
			break;
		}
		case SMS_TX_FALSE:
		{
			SMS_Service.tx_state = SMS_TX_START;
			break;
		}
		default:
		{
			SMS_Service.tx_state = SMS_TX_IDLE;
		}
	}

	if( SMS_Service.SMS_waitCounter )
	{
		return;
	}
	///短信接收相关处理
	switch( SMS_Service.rx_state )
	{
		case SMS_RX_IDLE:
		{
			SMS_Service.rx_retry = 0;
			break;
		}
		case SMS_RX_START:
		{
			rx_tick					= rt_tick_get( );
			SMS_Service.rx_retry	= 0;
			SMS_Service.rx_state	= SMS_RX_READ;
			break;
		}
		case SMS_RX_READ:
		{
			if( rt_tick_get( ) - rx_tick > RT_TICK_PER_SECOND / 2 ) ///等待一段时间后再读取数据
			{
				if( SMS_Service.rx_retry >= 3 )
				{
					SMS_Service.rx_state = SMS_RX_FALSE;
					break;
				}
				memset( SMS_Service.SMSAtSend, 0, sizeof( SMS_Service.SMSAtSend ) );
				sprintf( SMS_Service.SMSAtSend, "AT+CMGR=%d\r\n", SMS_Service.SMIndex );
				rt_kprintf( "%s", SMS_Service.SMSAtSend );
				at( ( char* )SMS_Service.SMSAtSend );
				SMS_Service.rx_retry++;
				SMS_Service.tx_state	= SMS_RX_WAITACK;               ///判断是否成功读取
				rx_tick					= rt_tick_get( );
			}
			break;
		}
		case SMS_RX_WAITACK:
		{
			if( rt_tick_get( ) - rx_tick > RT_TICK_PER_SECOND * 10 )    ///判读是否超时
			{
				SMS_Service.rx_state = SMS_RX_READ;
			}
			break;
		}
		case SMS_RX_OK:
		{
			if( strncmp( (char*)SMS_Service.SMS_rx_Content, "TW703#", 6 ) == 0 )                                        //短信修改UDP的IP和端口
			{
				ContentLen = strlen( SMS_Service.SMS_rx_Content );
				//-----------  自定义 短息设置修改 协议 ----------------------------------
				SMS_protocol( SMS_Service.SMS_rx_Content + 5, ContentLen - 5, SMS_ACK_msg );
			}
			SMS_Service.rx_state = SMS_RX_IDLE;
			//SMS_Service.rx_state=SMS_RX_DEL;
			break;
		}
		case SMS_RX_DEL:
		{
			memset( SMS_Service.SMSAtSend, 0, sizeof( SMS_Service.SMSAtSend ) );
			//sprintf(SMS_Service.SMSAtSend,"AT+CMGD=%d\r\n",SMS_Service.SMIndex);	///删除指定短信
			sprintf( SMS_Service.SMSAtSend, "AT+CMGD=0,4\r\n" ); ///删除所有短信
			rt_kprintf( "%s", SMS_Service.SMSAtSend );
			SMS_Service.rx_state = SMS_RX_IDLE;
			break;
		}
		case SMS_RX_FALSE:
		{
			SMS_Service.rx_state = SMS_RX_IDLE;
			break;
		}
		default:
		{
			SMS_Service.rx_state = SMS_RX_IDLE;
		}
	}


	/*
	   //-------------------------------
	   //       发送短息确认
	   else if(SMS_Service.SMS_sendFlag==1)
	   {
	   //#ifdef SMS_TYPE_PDU
	   if(SMS_Service.SMS_Style==1)
	   {
	   SMS_Tx_PDU(SMS_Service.SMS_destNum,SMS_Service.SMS_sd_Content);
	   }
	   //#else
	   else
	   {
	   SMS_Tx_Text(SMS_Service.SMS_destNum,SMS_Service.SMS_sd_Content);
	   }
	   //#endif
	   SMS_Service.SMS_sendFlag=0;  // clear
	   SMS_Service.SMS_waitCounter=3;
	   }
	   else if(SMS_Service.SMS_delALL==1)	  //删除短信
	   {
	   memset(SMS_Service.SMSAtSend,0,sizeof(SMS_Service.SMSAtSend));
	   ///
	   //sprintf(SMS_Service.SMSAtSend,"AT+CMGD=%d\r\n",SMS_Service.SMIndex);
	   sprintf(SMS_Service.SMSAtSend,"AT+CMGD=0,4\r\n",SMS_Service.SMIndex);
	   rt_kprintf("%s",SMS_Service.SMSAtSend);
	   ///
	   at( ( char * )SMS_Service.SMSAtSend );
	   SMS_Service.SMS_delALL=0;
	   SMS_Service.SMS_waitCounter=3;
	   }
	 */
}

///增加发送短信区域的内容，并置位发送短息标记，成功返回true，失败返回false
u8 Add_SMS_Ack_Content( char * instr, u8 ACKflag )
{
	if( ACKflag == 0 )
	{
		return false;
	}

	if( strlen( instr ) + strlen( SMS_Service.SMS_sd_Content ) < sizeof( SMS_Service.SMS_sd_Content ) )
	{
		strcat( (char*)SMS_Service.SMS_sd_Content, instr );
		SMS_Service.SMS_sendFlag = 1;
		return true;
	}
	return false;
}

/*********************************************************************************
  *函数名称:void SMS_protocol(u8 *instr,u16 len)
  *功能描述:接收到短信后参数处理函数
  *输    入:instr原始短信数据，len长度
  *输    出:none
  *返 回 值:none
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
void SMS_protocol( u8 *instr, u16 len, u8 ACKstate )
{
	SMS_Tx_PDU( SMS_Service.SMS_destNum, instr );
}

#if 0
/*********************************************************************************
  *函数名称:void SMS_protocol(u8 *instr,u16 len)
  *功能描述:接收到短信后参数处理函数
  *输    入:instr原始短信数据，len长度
  *输    出:none
  *返 回 值:none
  *作    者:白养民
  *创建日期:2013-05-29
  *---------------------------------------------------------------------------------
  *修 改 人:
  *修改日期:
  *修改描述:
*********************************************************************************/
void   SMS_protocol( u8 *instr, u16 len, u8 ACKstate )  //  ACKstate
{
	char	sms_content[60];                            ///短信命令区"()"之间的内容
	char	sms_ack_data[60];                           ///短信每个命令包括'#'符的完整内容
	u8		u8TempBuf[6];
	u16		i = 0, j = 0;
	u16		cmdLen, u16Temp;
	char	*p_Instr;
	char	*pstrTemp, *pstrTempStart, *pstrTempEnd;

	//SYSID		///修改该值，保存flash
	///应答短信包头部分
	memset( SMS_Service.SMS_sd_Content, 0, sizeof( SMS_Service.SMS_sd_Content ) );
	strcpy( SMS_Service.SMS_sd_Content, JT808Conf_struct.Vechicle_Info.Vech_Num );
	strcat( SMS_Service.SMS_sd_Content, "#" );                  // Debug
	strcat( SMS_Service.SMS_sd_Content, SIM_CardID_JT808 );     // Debug
	/*************************处理信息****************************/
	p_Instr = (char*)instr;
	for( i = 0; i < len; i++ )
	{
		pstrTemp = strchr( p_Instr, '#' );                      ///查找命令是否存在
		//instr++;
		if( pstrTemp )
		{
			p_Instr			= pstrTemp + 1;
			pstrTempStart	= strchr( (char*)pstrTemp, '(' );   ///查找命令内容开始位置
			pstrTempEnd		= strchr( (char*)pstrTemp, ')' );   ///查找命令内容结束位置
			if( ( NULL == pstrTempStart ) || ( NULL == pstrTempEnd ) )
			{
				break;
			}
			rt_kprintf( "\r\n短信命令格式有效 !" );
			///获取命令内容
			memset( sms_ack_data, 0, sizeof( sms_ack_data ) );
			memcpy( sms_ack_data, pstrTemp, pstrTempEnd - pstrTemp + 1 );

			///获取命令参数区域的内容以及参数长度
			pstrTempStart++;
			pstrTemp++;
			cmdLen = pstrTempEnd - pstrTempStart;
			memset( sms_content, 0, sizeof( sms_content ) );
			rt_memcpy( sms_content, pstrTempStart, cmdLen );

			///进行命令匹配
			if( strncmp( pstrTemp, "DNSR", 4 ) == 0 )   ///  1. 设置域名
			{
				if( cmdLen <= sizeof( DomainNameStr ) )
				{
					if( pstrTemp[4] == '1' )            ///主域名
					{
						rt_kprintf( "\r\n设置主域名 !" );
						memset( DomainNameStr, 0, sizeof( DomainNameStr ) );
						memset( SysConf_struct.DNSR, 0, sizeof( DomainNameStr ) );
						memcpy( DomainNameStr, (char*)pstrTempStart, cmdLen );
						memcpy( SysConf_struct.DNSR, (char*)pstrTempStart, cmdLen );
						Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
						//----- 传给 GSM 模块------
						DataLink_DNSR_Set( SysConf_struct.DNSR, 1 );

						///
						Add_SMS_Ack_Content( sms_ack_data, ACKstate );

						//------- add on 2013-6-6
						if( ACKstate == SMS_ACK_none )
						{
							SD_ACKflag.f_CentreCMDack_0001H = 2;    //DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断
						}
					}else if( pstrTemp[4] == '2' )                  ///备用域名
					{
						rt_kprintf( "\r\n设置备用域名 !" );
						memset( DomainNameStr_aux, 0, sizeof( DomainNameStr_aux ) );
						memset( SysConf_struct.DNSR_Aux, 0, sizeof( DomainNameStr_aux ) );
						memcpy( DomainNameStr_aux, (char*)pstrTempStart, cmdLen );
						memcpy( SysConf_struct.DNSR_Aux, (char*)pstrTempStart, cmdLen );
						Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
						//----- 传给 GSM 模块------
						DataLink_DNSR2_Set( SysConf_struct.DNSR_Aux, 1 );

						///
						Add_SMS_Ack_Content( sms_ack_data, ACKstate );
					}else
					{
						continue;
					}
				}
			}else if( strncmp( pstrTemp, "PORT", 4 ) == 0 ) ///2. 设置端口
			{
				j = sscanf( sms_content, "%u", &u16Temp );
				if( j )
				{
					if( pstrTemp[4] == '1' )                ///主端口
					{
						rt_kprintf( "\r\n设置主端口=%d!", u16Temp );
						RemotePort_main				= u16Temp;
						SysConf_struct.Port_main	= u16Temp;
						Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
						//----- 传给 GSM 模块------
						DataLink_MainSocket_set( RemoteIP_main, RemotePort_main, 1 );
						///
						Add_SMS_Ack_Content( sms_ack_data, ACKstate );

						//------- add on 2013-6-6
						if( ACKstate == SMS_ACK_none )
						{
							SD_ACKflag.f_CentreCMDack_0001H = 2;    //DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断
						}
					}else if( pstrTemp[4] == '2' )                  ///备用端口
					{
						rt_kprintf( "\r\n设置备用端口=%d!", u16Temp );
						RemotePort_aux			= u16Temp;
						SysConf_struct.Port_Aux = u16Temp;
						Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
						//----- 传给 GSM 模块------
						DataLink_AuxSocket_set( RemoteIP_aux, RemotePort_aux, 1 );
						///
						Add_SMS_Ack_Content( sms_ack_data, ACKstate );
					}else
					{
						continue;
					}
				}
			}else if( strncmp( pstrTemp, "DUR", 3 ) == 0 ) ///3. 修改发送间隔
			{
				j = sscanf( sms_content, "%u", &u16Temp );
				if( j )
				{
					rt_kprintf( "\r\n修改发送间隔! %d", u16Temp );
					dur( sms_content );


					/*
					   JT808Conf_struct.DURATION.Default_Dur=u16Temp;
					   Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
					 */

					///
					Add_SMS_Ack_Content( sms_ack_data, ACKstate );
				}
			}else if( strncmp( pstrTemp, "DEVICEID", 8 ) == 0 ) ///4. 修改终端ID
			{
				if( cmdLen <= sizeof( DeviceNumberID ) )
				{
					rt_kprintf( "\r\n修改终端ID  !" );
					memset( DeviceNumberID, 0, sizeof( DeviceNumberID ) );
					memcpy( DeviceNumberID, pstrTempStart, cmdLen );
					DF_WriteFlashSector( DF_DeviceID_offset, 0, DeviceNumberID, 13 );
					///
					Add_SMS_Ack_Content( sms_ack_data, ACKstate );
				}else
				{
					continue;
				}
			} //SIM_CardID_JT808
			else if( strncmp( pstrTemp, "SIMID", 5 ) == 0 ) ///4. 修改终端ID
			{
				if( cmdLen <= sizeof( DeviceNumberID ) )
				{
					memset( SIM_CardID_JT808, 0, sizeof( SIM_CardID_JT808 ) );
					memcpy( SIM_CardID_JT808, pstrTempStart, cmdLen );
					rt_kprintf( "\r\n修改SIMID  !%s,%s,%d", SIM_CardID_JT808, pstrTempStart, cmdLen );
					simid( SIM_CardID_JT808 );


					/*
					   DF_WriteFlashSector(DF_SIMID_offset,0,SIM_CardID_JT808,13);
					   ///*/
					Add_SMS_Ack_Content( sms_ack_data, ACKstate );

					//------- add on 2013-6-6
					if( ACKstate == SMS_ACK_none )
					{
						SD_ACKflag.f_CentreCMDack_0001H = 2;    //DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断
					}
				}else
				{
					continue;
				}
			}else if( strncmp( pstrTemp, "IP", 2 ) == 0 )       ///5.设置IP地址
			{
				j = sscanf( sms_content, "%u.%u.%u.%u", (u32*)&u8TempBuf[0], (u32*)&u8TempBuf[1], (u32*)&u8TempBuf[2], (u32*)&u8TempBuf[3] );
				//j=str2ip(sms_content, u8TempBuf);
				if( j == 4 )
				{
					rt_kprintf( "\r\n设置IP地址!" );
					if( pstrTemp[2] == '1' )
					{
						memcpy( SysConf_struct.IP_Main, u8TempBuf, 4 );
						memcpy( RemoteIP_main, u8TempBuf, 4 );
						SysConf_struct.Port_main = RemotePort_main;
						Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
						rt_kprintf( "\r\n短信设置主服务器 IP: %d.%d.%d.%d : %d ", RemoteIP_main[0], RemoteIP_main[1], RemoteIP_main[2], RemoteIP_main[3], RemotePort_main );
						//-----------  Below add by Nathan  ----------------------------
						DataLink_MainSocket_set( RemoteIP_main, RemotePort_main, 1 );
						///
						Add_SMS_Ack_Content( sms_ack_data, ACKstate );

						//------- add on 2013-6-6
						if( ACKstate == SMS_ACK_none )
						{
							SD_ACKflag.f_CentreCMDack_0001H = 2; //DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断
						}
					}else if( pstrTemp[2] == '2' )
					{
						memcpy( SysConf_struct.IP_Aux, u8TempBuf, 4 );
						memcpy( RemoteIP_aux, u8TempBuf, 4 );
						SysConf_struct.Port_Aux = RemotePort_aux;
						Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
						rt_kprintf( "\r\n短信设置备用服务器 IP: %d.%d.%d.%d : %d ", RemoteIP_aux[0], RemoteIP_aux[1], RemoteIP_aux[2], RemoteIP_aux[3], RemotePort_aux );
						//-----------  Below add by Nathan  ----------------------------
						DataLink_AuxSocket_set( RemoteIP_aux, RemotePort_aux, 1 );
						///
						Add_SMS_Ack_Content( sms_ack_data, ACKstate );
					}
				}
			}else if( strncmp( pstrTemp, "MODE", 4 ) == 0 ) ///6. 设置定位模式
			{
				if( strncmp( sms_content, "BD", 2 ) == 0 )
				{
					gps_mode( "1" );
				}
				if( strncmp( sms_content, "GP", 2 ) == 0 )
				{
					gps_mode( "2" );
				}
				if( strncmp( sms_content, "GN", 2 ) == 0 )
				{
					gps_mode( "3" );
				}
				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "VIN", 3 ) == 0 )          ///7.设置车辆VIN
			{
				vin_set( sms_content );
				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "TIREDCLEAR", 10 ) == 0 )  ///8.清除疲劳驾驶记录
			{
				TiredDrv_write	= 0;
				TiredDrv_read	= 0;
				DF_Write_RecordAdd( TiredDrv_write, TiredDrv_read, TYPE_TiredDrvAdd );
				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "DISCLEAR", 8 ) == 0 )     ///9清除里程
			{
				JT808Conf_struct.DayStartDistance_32	= 0;
				JT808Conf_struct.Distance_m_u32			= 0;
				Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "RESET", 5 ) == 0 )        ///10.终端重启
			{
				reset( );
			}else if( strncmp( pstrTemp, "RELAY", 5 ) == 0 )        ///11.继电器控制
			{
				if( sms_content[0] == '0' )
				{
					debug_relay( "0" );
				}
				if( sms_content[0] == '1' )
				{
					debug_relay( "1" );
				}

				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "TAKE", 4 ) == 0 ) //12./拍照
			{
				switch( sms_content[0] )
				{
					case '1':
						takephoto( "1" );
						break;
					case '2':
						takephoto( "2" );
						break;
					case '3':
						takephoto( "3" );
						break;
					case '4':
						takephoto( "4" );
						break;
				}
				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "PLAY", 4 ) == 0 )                                                                     ///13.语音播报
			{
				TTS_Get_Data( sms_content, strlen( sms_content ) );
				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "QUERY", 5 ) == 0 )                                                                    ///14.车辆状态查询
			{
				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "ISP", 3 ) == 0 )                                                                      ///15.远程下载IP 端口
			{
				;
				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "PLATENUM", 8 ) == 0 )
			{
				rt_kprintf( "Vech_Num is %s", sms_content );
				memset( (u8*)&JT808Conf_struct.Vechicle_Info.Vech_Num, 0, sizeof( JT808Conf_struct.Vechicle_Info.Vech_Num ) );  //clear
				rt_memcpy( JT808Conf_struct.Vechicle_Info.Vech_Num, sms_content, strlen( sms_content ) );
				Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
				Add_SMS_Ack_Content( sms_ack_data, ACKstate );
			}else if( strncmp( pstrTemp, "COLOR", 5 ) == 0 )
			{
				j = sscanf( sms_content, "%d", &u16Temp );
				if( j )
				{
					JT808Conf_struct.Vechicle_Info.Dev_Color = u16Temp;
					rt_kprintf( "\r\n 车辆颜色: %s ,%d \r\n", sms_content, JT808Conf_struct.Vechicle_Info.Dev_Color );
					Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
					Add_SMS_Ack_Content( sms_ack_data, ACKstate );
				}
			}else
			{
				;
			}
		}else
		{
			break;
		}
	}
}

#endif

/************************************** The End Of File **************************************/
