#include "Menu_Include.h"



unsigned char Menu_Multimedia=0;
unsigned char Multimedia_change=1;//选择
unsigned char Multimedia_screen=0;//界面切换使用


void Multimedia(unsigned char type)
{
	lcd_fill(0);
	lcd_text12(18,3,"多媒体信息选择",14,LCD_MODE_SET);	
	if(type==1)
		{
		lcd_text12(24,19,"音频",4,LCD_MODE_INVERT);
		lcd_text12(72,19,"视频",4,LCD_MODE_SET);
		}
	else if(type==2)
		{
		lcd_text12(24,19,"音频",4,LCD_MODE_SET);
		lcd_text12(72,19,"视频",4,LCD_MODE_INVERT);
		}
	lcd_update_all();

}

static void msg( void *p)
{
}
static void show(void)
{
	Multimedia(1);
}


static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		pMenuItem=&Menu_3_InforInteract;
		pMenuItem->show();
		CounterBack=0;
		
		Menu_Multimedia=0;
		Multimedia_change=1;//选择
		Multimedia_screen=0;//界面切换使用
		break;
	case KeyValueOk:
		if(Multimedia_screen==0)
			{
			Multimedia_screen=1;
			lcd_fill(0);
			
			if(Multimedia_change==1)
				{
				//CarLoadState_Write();
				lcd_text12(7,10,"多媒体数据类型:音频",19,LCD_MODE_SET);  
				}
			else if(Multimedia_change==2)
				{
				//CarLoadState_Write(); 
				lcd_text12(7,10,"多媒体数据类型:视频",19,LCD_MODE_SET);  
				}
			lcd_update_all();
			}
		else if(Multimedia_screen==1)
			{
			Multimedia_screen=2;

			MediaObj.SD_Data_Flag=1;//发送多媒体数据标志
			if(Multimedia_change==1)
				{
				MediaObj.Media_Type=1;//音频1
				/*memset(send_data,0,sizeof(send_data));
				send_data[0]=0x08;
				send_data[1]=0x01;
				send_data[2]=0x00;
				send_data[3]=0x01;
				send_data[4]=0x01;//音频
				rt_mb_send(&mb_hmi, (rt_uint32_t)&send_data[0]);*/
				
				lcd_fill(0);
				lcd_text12(18,10,"音频  开始发送",14,LCD_MODE_SET);
				lcd_update_all();
				}
			else if(Multimedia_change==2)
				{
				MediaObj.Media_Type=2;//音频2
				/*memset(send_data,0,sizeof(send_data));
				send_data[0]=0x08;
				send_data[1]=0x01;
				send_data[2]=0x00;
				send_data[3]=0x01;
				send_data[4]=0x02;//视频
				rt_mb_send(&mb_hmi, (rt_uint32_t)&send_data[0]);*/
				
				lcd_fill(0);
				lcd_text12(18,10,"视频  开始发送",14,LCD_MODE_SET);	
				lcd_update_all();
				}
			
			}
		break;
	case KeyValueUP:
		if(Multimedia_screen==0)
			{
			Multimedia_change=1;
			Multimedia(1);
			}
		break;
	case KeyValueDown:
		if(Multimedia_screen==0)
			{
			Multimedia_change=2;
			Multimedia(2);
			}
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

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_3_4_Multimedia=
{
	"发送多媒体数据",
	14,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

