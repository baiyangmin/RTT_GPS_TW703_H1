#include "Menu_Include.h"

unsigned char car_status_str[3][4]={"空车","半空","重车"};

unsigned char CarStatus_change=1;//状态选择
unsigned char CarStatus_screen=0;//界面切换使用



void CarStatus(unsigned char Status)
{
unsigned char i=0;

	lcd_fill(0);
	lcd_text12(12,3,"车辆负载状态选择",16,LCD_MODE_SET);
	for(i=0;i<3;i++)
		lcd_text12(20+i*30,19,(char *)car_status_str[i],4,LCD_MODE_SET);
	lcd_text12(20+30*Status,19,(char *)car_status_str[Status],4,LCD_MODE_INVERT);
	lcd_update_all();
}
static void msg( void *p)
{
}
static void show(void)
{
	CarStatus(1);
}


static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		pMenuItem=&Menu_3_InforInteract;
		pMenuItem->show();
		CounterBack=0;
		
		CarStatus_change=1;//选择
		CarStatus_screen=0;//界面切换使用
		break;
	case KeyValueOk:
		
		if(CarStatus_screen==0)
			{
			CarStatus_screen=1;
			lcd_fill(0);
			lcd_text12(12,10,"发送车辆状态",12,LCD_MODE_SET);
			lcd_text12(88,10,(char *)car_status_str[CarStatus_change],4,LCD_MODE_SET);
			lcd_update_all();
			}
		else if(CarStatus_screen==1)
			{
			CarStatus_screen=2;
			
		    JT808Conf_struct.LOAD_STATE=CarStatus_change;
			Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
            Car_Status[2]&=~0x03;      //  空载
	        if(CarStatus_change==1)
				Car_Status[2]|=0x01;   //半载
			else if(CarStatus_change==2)
				Car_Status[2]|=0x03;   //满载

            //上报位置信息
			PositionSD_Enable();
			Current_UDP_sd=1;
			
			lcd_fill(0);
			lcd_text12(20,10,(char *)car_status_str[CarStatus_change],4,LCD_MODE_SET);
			lcd_text12(48,10,"发送成功",8,LCD_MODE_SET);
			lcd_update_all();
			}
		break;
	case KeyValueUP:
		if(CarStatus_screen==0)
			{			
			CarStatus_change--;
			if(CarStatus_change<=0)
				CarStatus_change=0;
			CarStatus(CarStatus_change);
			}
		break;
	case KeyValueDown:
		if(CarStatus_screen==0)
			{		
			CarStatus_change++;
			if(CarStatus_change>=2)
				CarStatus_change=2;
			CarStatus(CarStatus_change);
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
MENUITEM	Menu_3_2_FullorEmpty= 
{

    "车辆状态发送",
    12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

