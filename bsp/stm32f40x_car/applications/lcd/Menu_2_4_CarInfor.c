#include  <string.h>
#include "Menu_Include.h"
#include "Lcd.h"

static unsigned char vech_num[16]={"车牌号:        "};
static unsigned char vech_type[25]={"车辆类型:            "};
static unsigned char  vech_ID[19]={"入网ID:            "}; 
static unsigned char  vech_sim[20]={"SIM卡号:            "};
static unsigned char updown_flag=0;

//驾驶员代码
void Display_driver(u8 drivercar)
{
    u8 color_disp[4];
switch(drivercar)
	{
	case 1:
	
		//车牌号JT808Conf_struct.Vechicle_Info.Vech_Num
		memcpy(vech_num+7,JT808Conf_struct.Vechicle_Info.Vech_Num,8); 
            //车辆颜色
               memset(color_disp,0,sizeof(color_disp));
               switch(JT808Conf_struct.Vechicle_Info.Dev_Color)
               	{
               	    case 1: memcpy(color_disp,"蓝色",4);break;
                         case 2:memcpy(color_disp,"黄色",4);break;
			    case 3:memcpy(color_disp,"黑色",4);break;
			    case 4:memcpy(color_disp,"白色",4);break;
			    case 9:memcpy(color_disp,"其他",4);break;
			    default:memcpy(color_disp,"蓝色",4);break;	
               	}
	      lcd_fill(0);
	      lcd_text12(10,3,(char *)vech_num,15,LCD_MODE_SET);	   
		lcd_text12(10,19,"车牌颜色:",9,LCD_MODE_SET);
		lcd_text12(64,19,color_disp,4,LCD_MODE_SET); 
		lcd_update_all();
		break;

	case 2://车辆ID Vechicle_Info.DevicePhone
		lcd_fill(0);
              memcpy(vech_type+9,JT808Conf_struct.Vechicle_Info.Vech_Type,6);
		lcd_text12(0,3,(char *)vech_type,19,LCD_MODE_SET);		
		//读取设备速度取得是GPS速度还是速度线速度
		if(JT808Conf_struct.DF_K_adjustState)
			lcd_text12(0,18,"设备速度:传感器速度",19,LCD_MODE_SET);
		else
			lcd_text12(0,18,"设备速度:GPS速度",16,LCD_MODE_SET);
		lcd_update_all();
		break;
	case 3:  //  车辆ID及车辆sim卡号码
		memcpy(vech_ID+7,IMSI_CODE+3,12); 
		memcpy(vech_sim+8,JT808Conf_struct.Vechicle_Info.Vech_sim,12);
		lcd_fill(0);
		lcd_text12(0,3,(char *)vech_ID,19,LCD_MODE_SET);
		lcd_text12(0,18,(char *)vech_sim,20,LCD_MODE_SET);
		lcd_update_all();   
		 break;
	default:
		break;
	}

}

static void msg( void *p)
{
}
static void show(void)
{
	lcd_fill(0);
	lcd_text12(24, 3,"车辆信息查看",12,LCD_MODE_SET);
	lcd_text12(24,19,"查看请按选择",12,LCD_MODE_SET);
	lcd_update_all();

}


static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_2_InforCheck;
			pMenuItem->show();
			CounterBack=0;

			updown_flag=0;
			break;
		case KeyValueOk:
			updown_flag=1;
			Display_driver(1);
			break;
		case KeyValueUP:
                    if(updown_flag==1)
				Display_driver(1);
			if(updown_flag==2)
			  {	Display_driver(2);updown_flag=1;}	
			break;
		case KeyValueDown:
			if(updown_flag==1)
			  {	Display_driver(2);updown_flag=2;}
			else
			if(updown_flag==2)
			  {	Display_driver(3);updown_flag=2;}	
			break;
		}
	KeyValue=0;
}


static void timetick(unsigned int systick)
{
	CounterBack++;
	if(CounterBack!=MaxBankIdleTime)
		return;
	CounterBack=0;
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();
}


ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_2_4_CarInfor=
{
	"车辆信息查看",
	12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


