#include "Menu_Include.h"


unsigned char CarMulTrans_screen=0;


static void msg( void *p)
{
}
static void show(void)
{
	lcd_fill(0);
	lcd_text12(20,3,"多媒体事件上传",14,LCD_MODE_SET);
	lcd_text12(20,18,"按确认键上传信息",16,LCD_MODE_SET);
	lcd_update_all();
}


static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		pMenuItem=&Menu_3_InforInteract;
		pMenuItem->show();
		CounterBack=0;

		CarMulTrans_screen=0;
		break;
	case KeyValueOk:
		if(CarMulTrans_screen==0)
			{
			CarMulTrans_screen=1;
			/*memset(send_data,0,sizeof(send_data));
			send_data[0]=0x08;
			send_data[1]=0x00;
			send_data[2]=0x00;
			send_data[3]=0x00;
			rt_mb_send(&mb_hmi, (rt_uint32_t)&send_data[0]);*/
			
			lcd_fill(0);
			lcd_text12(7,10,"多媒体事件上传成功",18,LCD_MODE_INVERT);
			lcd_update_all();
            //置上传多媒体信息的标志
			rt_kprintf("\r\n -----  多媒体事件上传成功 \r\n");  
			MediaObj.Media_Type=0; //图像
		    MediaObj.Media_totalPacketNum=1;  // 图片总包数
		    MediaObj.Media_currentPacketNum=1;	// 图片当前报数
		    MediaObj.Media_ID=1;   //  多媒体ID
		    MediaObj.Media_Channel=Camera_Number;  // 图片摄像头通道号
		    MediaObj.SD_media_Flag=1; //多媒体事件信息上传标志
		    Duomeiti_sdFlag=1; 
			}
		break;
	case KeyValueUP:
		break;
	case KeyValueDown:
		break;
	}
KeyValue=0;
}

	

static void timetick(unsigned int systick)
{
	CounterBack++;
	if(CounterBack!=MaxBankIdleTime)
		return;
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();
	CounterBack=0;					

	CarMulTrans_screen=0;

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_3_5_MultimediaTrans=
{
"多媒体事件上传",
	14,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

