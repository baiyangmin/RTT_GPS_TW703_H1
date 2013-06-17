#include "Menu_Include.h"

unsigned char noselect_log[]={0x3C,0x7E,0xC3,0xC3,0xC3,0xC3,0x7E,0x3C};//空心
unsigned char select_log[]={0x3C,0x7E,0xFF,0xFF,0xFF,0xFF,0x7E,0x3C};//实心

unsigned char CarSet_0=1;
//unsigned char CarSet_0_Flag=1;

DECL_BMP(8,8,select_log); DECL_BMP(8,8,noselect_log); 

void Selec_123(u8 par)
{
u8 i=0;
if(par==1)
	{
	lcd_bitmap(35, 5, &BMP_select_log, LCD_MODE_SET);
	for(i=0;i<4;i++)
		lcd_bitmap(47+i*12, 5, &BMP_noselect_log ,LCD_MODE_SET);
	}
else if(par==2)
	{
	lcd_bitmap(35, 5, &BMP_noselect_log, LCD_MODE_SET);
	lcd_bitmap(47, 5, &BMP_select_log, LCD_MODE_SET);
	for(i=0;i<3;i++)
	lcd_bitmap(59+i*12, 5, &BMP_noselect_log, LCD_MODE_SET);
	}
else if(par==3)
	{
	for(i=0;i<2;i++)
		lcd_bitmap(35+i*12, 5, &BMP_noselect_log, LCD_MODE_SET);
	lcd_bitmap(59, 5, &BMP_select_log, LCD_MODE_SET);
	for(i=0;i<2;i++)
	lcd_bitmap(71+i*12, 5, &BMP_noselect_log, LCD_MODE_SET);
	}
else if(par==4)
	{
	for(i=0;i<3;i++)
		lcd_bitmap(35+i*12, 5, &BMP_noselect_log, LCD_MODE_SET);
	lcd_bitmap(71, 5, &BMP_select_log, LCD_MODE_SET);
	lcd_bitmap(83, 5, &BMP_noselect_log, LCD_MODE_SET);
	}
else if(par==5)
	{
	for(i=0;i<4;i++)
		lcd_bitmap(35+i*12, 5, &BMP_noselect_log, LCD_MODE_SET);
	lcd_bitmap(83, 5, &BMP_select_log, LCD_MODE_SET);
	}
}
void CarSet_0_fun(u8 set_type)
{
//u8 i=0;
switch(set_type)
	{
	case 1:
		lcd_fill(0);
		lcd_text12( 0, 3,"注册",4,LCD_MODE_SET);
		lcd_text12( 0,17,"输入",4,LCD_MODE_SET);
		Selec_123(set_type);
		lcd_text12(35,19,"车牌号输入",10,LCD_MODE_INVERT);
		lcd_update_all();
		break;
	case 2:
		lcd_fill(0);
		lcd_text12( 0, 3,"注册",4,LCD_MODE_SET);
		lcd_text12( 0,17,"输入",4,LCD_MODE_SET);	
              Selec_123(set_type);
		lcd_text12(35,19,"车辆类型选择",12,LCD_MODE_INVERT);		
		lcd_update_all();
		break;
	case 3:
		lcd_fill(0);
		lcd_text12( 0, 3,"注册",4,LCD_MODE_SET);
		lcd_text12( 0,17,"输入",4,LCD_MODE_SET);
              Selec_123(set_type);
		lcd_text12(35,19,"SIM卡号设置",11,LCD_MODE_INVERT);
		lcd_update_all();
		break;
	case 4:
		lcd_fill(0);
		lcd_text12( 0, 3,"注册",4,LCD_MODE_SET);
		lcd_text12( 0,17,"输入",4,LCD_MODE_SET);
              Selec_123(set_type);
		lcd_text12(35,19,"速度获取方式",12,LCD_MODE_INVERT);
		lcd_update_all();
		break;
	case 5:
		lcd_fill(0);
		lcd_text12( 0, 3,"注册",4,LCD_MODE_SET);
		lcd_text12( 0,17,"输入",4,LCD_MODE_SET);
              Selec_123(CarSet_0_counter);
		lcd_text12(35,19,"颜色输入",8,LCD_MODE_INVERT);
		lcd_update_all();
		break;
	default	 :
		break;
	}
}
static void msg( void *p)
{

}
static void show(void)
{
 CounterBack=0;
 CarSet_0_fun(CarSet_0_counter);
}


static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		//pMenuItem=&Menu_1_Idle;//进入信息查看界面
		//pMenuItem->show();
		if(menu_color_flag)
			{
			menu_color_flag=0;
			
			pMenuItem=&Menu_1_Idle;//进入信息查看界面
			pMenuItem->show();
			}
		break;
	case KeyValueOk:
				
		if(CarSet_0_counter==1)
          	{
			pMenuItem=&Menu_0_1_license;//车牌号输入
	        pMenuItem->show();
          	}
		else if(CarSet_0_counter==2)
			{
			pMenuItem=&Menu_0_2_CarType;//type
		    pMenuItem->show();
			}
		else if(CarSet_0_counter==3)
			{
			pMenuItem=&Menu_0_3_Sim;//颜色
		    pMenuItem->show();
			}
		else if(CarSet_0_counter==4)
			{
			pMenuItem=&Menu_0_5_speedtype;//获取速度方式
		    pMenuItem->show();
			}
		else if(CarSet_0_counter==5)
			{
			pMenuItem=&Menu_0_4_Colour;//颜色
		    pMenuItem->show();
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
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_0_loggingin=
{
   "车辆设置",
	8,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

