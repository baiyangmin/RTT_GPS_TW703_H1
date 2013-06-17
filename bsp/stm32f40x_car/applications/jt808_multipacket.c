/*
   jt808多包发送和补传的实现

 */

#include "jt808_multipacket.h"
#include "jt808.h"
#include "stm32f4xx.h"
#include <rtthread.h>


typedef __packed struct _jt808_tx_msg_multipacket_nodedata
{
/*发送机制相关*/
	uint8_t			linkno;     /*传输使用的link,包括了协议和远端socket*/
	JT808_MSG_TYPE	type;
	JT808_MSG_STATE state;      /*发送状态*/
	uint32_t		retry;      /*重传次数,递增，递减找不到*/
	uint32_t		max_retry;  /*最大重传次数*/
	uint32_t		timeout;    /*超时时间*/
	uint32_t		tick;       /*发送时间*/
/*接收的处理判断相关*/
	uint16_t	head_id;        /*消息ID*/
	uint16_t	head_sn;        /*消息流水号*/
/*真实的发送数据*/
	uint16_t	msg_len;        /*消息长度*/
	void	(*get_data)(void *pmsgout);  /*获取要发送的信息*/
	
}JT808_TX_MSG_MULTIPACKET_NODEDATA;




typedef __packed struct
{
	uint16_t id;
	uint16_t attr;
	uint8_t  mobile[6];
	uint16_t seq;
}MSG_HEAD;


typedef __packed struct
{
	uint16_t id;
	uint16_t attr;
	uint8_t  mobile[6];
	uint16_t seq;
	uint16_t packet_count;	/*总包数*/
	uint16_t packet_no;		/*当前包序号*/
}MSG_HEAD_EX;




typedef __packed struct
{
	uint8_t chn;	/*通道*/
	uint16_t cmd;
	uint16_t interval;
	uint8_t flag_save;
	uint8_t	not_used[6];
}CMD_0x8801;















/*
多包消息的接收处理，只要看到消息体属性中有分包项

 0x8700 行车记录仪数据采集

 0x8800 多媒体数据上传应答-多包补传

 0x8801 摄像头立即拍照，

 0x8804 录音

*/
int handle_rx_multipacket( uint8_t linkno, uint8_t *pmsg )
{
	uint16_t id;
	uint8_t *psrc;

	psrc=pmsg;
	id=(*psrc<<8)|(*(psrc+1));

	switch(id)
	{
		case 0x8801:		/*立即拍照*/
			
			break;


	}




}





void jt808_tx_response_multipacket( uint8_t linkno, uint8_t *pmsg )
{


}


void jt808_tx_timeout_multipacket( uint8_t linkno, uint8_t *pmsg )
{


}




/***********************************************************
* Function: jt808多包处理，处理发送过程
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void jt808_multipacket_process( void )
{




}



/*
发送行车记录仪数据0x0700
*/

void multipacket_send_0x0700(uint16_t center_sq,uint8_t *pinfo,uint16_t len)
{



}





/************************************** The End Of File **************************************/
