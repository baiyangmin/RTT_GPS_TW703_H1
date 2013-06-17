#include  <string.h>
#include "Menu_Include.h"
#include "Lcd.h"


/*
static unsigned char Jiayuan_screen=0;  //  显示查看/发送界面   =1时选择是查看还是发送界面
static unsigned char CheckJiayuanFlag=0;//  1:进入显示驾驶员信息   2:进入发送驾驶员信息
static unsigned char Jiayuan_1_2=0;     // 0:表示在查看界面   1:表示在发送界面
*/
typedef struct _DIS_DIRVER_INFOR
{
unsigned char DIS_SELECT_check_send;
unsigned char DIS_ENTER_check_send;
unsigned char DIS_SHOW_check_send;
}DIS_DIRVER_INFOR;

DIS_DIRVER_INFOR DIS_DRIVER_inform_temp;



//驾驶员代码 
void Display_jiayuan(unsigned char NameCode)
{
//unsigned char i=0;
lcd_fill(0);
if(NameCode==1)
	{
	lcd_text12(30,3,"驾驶员姓名",10,LCD_MODE_SET);
	lcd_text12(30,19,(char *)JT808Conf_struct.Driver_Info.BD_DriveName,strlen((char *)JT808Conf_struct.Driver_Info.BD_DriveName),LCD_MODE_SET);
	}
else
	{
	lcd_text12(30,3,"驾驶员从业资格证",16,LCD_MODE_SET);
	lcd_text12(0,19,(char *)JT808Conf_struct.Driver_Info.BD_Drv_CareerID,20,LCD_MODE_SET);
	}
lcd_update_all();
} 

static void Dis_DriverInfor(unsigned char type,unsigned char disscreen)
{
lcd_fill(0);
if(type==1)
	{
	if(disscreen==1)
		{
		lcd_text12(0, 3,"1.驾驶员信息查看",16,LCD_MODE_INVERT); 
	    lcd_text12(0,19,"2.驾驶员信息发送",16,LCD_MODE_SET);
		}
	else if(disscreen==2)
		{
		lcd_text12(0, 3,"1.驾驶员信息查看",16,LCD_MODE_SET); 
		lcd_text12(0,19,"2.驾驶员信息发送",16,LCD_MODE_INVERT);
		}
	}
else if(type==2)
	{
	if(disscreen==1)
		lcd_text12(0,10,"按确认发送驾驶员信息",20,LCD_MODE_SET);
	else if(disscreen==2)
		lcd_text12(5,10,"驾驶员信息发送成功",18,LCD_MODE_SET);
	}	
lcd_update_all();
}

static void msg( void *p)
{
}
static void show(void)
{
	Dis_DriverInfor(1,1);
	DIS_DRIVER_inform_temp.DIS_SELECT_check_send=1;
	DIS_DRIVER_inform_temp.DIS_ENTER_check_send=1;
}


static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_2_InforCheck;
			pMenuItem->show();
			CounterBack=0;

			memset(&DIS_DRIVER_inform_temp,0,sizeof(DIS_DRIVER_inform_temp));
			break;
		case KeyValueOk:
			if(DIS_DRIVER_inform_temp.DIS_ENTER_check_send==1)
				{
				DIS_DRIVER_inform_temp.DIS_ENTER_check_send=2;
				DIS_DRIVER_inform_temp.DIS_SELECT_check_send=0;//查看或者发送已经选好
				
				if(DIS_DRIVER_inform_temp.DIS_SHOW_check_send==0)//进入查看驾驶员信息界面
					Display_jiayuan(1);
				else if(DIS_DRIVER_inform_temp.DIS_SHOW_check_send==1)//进入发送驾驶员信息界面
					{
					Dis_DriverInfor(2,1);
					}
				}
			else if(DIS_DRIVER_inform_temp.DIS_ENTER_check_send==2)
				{
				DIS_DRIVER_inform_temp.DIS_ENTER_check_send=3;
				if(DIS_DRIVER_inform_temp.DIS_SHOW_check_send==0)//返回查看和发送界面
					{
					Dis_DriverInfor(1,1);
					DIS_DRIVER_inform_temp.DIS_SELECT_check_send=1;
					DIS_DRIVER_inform_temp.DIS_ENTER_check_send=1;
					}
				else if(DIS_DRIVER_inform_temp.DIS_SHOW_check_send==1)//提示发送成功
					{
					Dis_DriverInfor(2,2);
					SD_ACKflag.f_DriverInfoSD_0702H=1;
					DIS_DRIVER_inform_temp.DIS_ENTER_check_send=0;//    1
					DIS_DRIVER_inform_temp.DIS_SELECT_check_send=0;
					DIS_DRIVER_inform_temp.DIS_SHOW_check_send=0;
					}
				}
			break;
		case KeyValueUP:
			if(DIS_DRIVER_inform_temp.DIS_ENTER_check_send==2)
				{
				if(DIS_DRIVER_inform_temp.DIS_SHOW_check_send==0)//查看
					Display_jiayuan(1);
				else if(DIS_DRIVER_inform_temp.DIS_SHOW_check_send==1)//发送
					{
					Dis_DriverInfor(2,1);
					}
				}
			else if(DIS_DRIVER_inform_temp.DIS_SELECT_check_send==1)//选择进入查看或者发送
				{
				DIS_DRIVER_inform_temp.DIS_SHOW_check_send=0;
				Dis_DriverInfor(1,1);
				}
			break;
		case KeyValueDown:
			if(DIS_DRIVER_inform_temp.DIS_ENTER_check_send==2)
				{
				if(DIS_DRIVER_inform_temp.DIS_SHOW_check_send==0)//查看
					Display_jiayuan(2);
				else if(DIS_DRIVER_inform_temp.DIS_SHOW_check_send==1)//发送
					Dis_DriverInfor(2,1);
				}
			else if(DIS_DRIVER_inform_temp.DIS_SELECT_check_send==1)//选择进入查看或者发送
				{
				DIS_DRIVER_inform_temp.DIS_SHOW_check_send=1;
				Dis_DriverInfor(1,2);
				
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
    memset(&DIS_DRIVER_inform_temp,0,sizeof(DIS_DRIVER_inform_temp));
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_2_5_DriverInfor=
{	
	"驾驶员信息查看",
    14,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


