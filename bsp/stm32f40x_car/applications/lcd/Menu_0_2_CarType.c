#include  <string.h>
#include "Menu_Include.h"


struct IMG_DEF test_scr_CarType={12,12,test_00};

unsigned char CarType_counter=0;
unsigned char CarType_Type=0;


void CarType(unsigned char type_Sle,unsigned char sel)
{
switch(type_Sle)
	{
	case 1:
		lcd_fill(0);
		if(sel==0)
			{
			lcd_text12(24,3,"车辆类型选择",12,LCD_MODE_SET); 
		    lcd_text12(0,19,"1.  客运车",10,LCD_MODE_SET);
			}
		else
			lcd_text12(12,10,"车辆类型:客运车",15,LCD_MODE_SET);
		lcd_update_all();
		break;
	case 2:
		lcd_fill(0);
		if(sel==0)
			{
			lcd_text12(24,3,"车辆类型选择",12,LCD_MODE_SET); 
		    lcd_text12(0,19,"2.  货运车",10,LCD_MODE_SET);
			}
		else
			lcd_text12(12,10,"车辆类型:货运车",15,LCD_MODE_SET);
		lcd_update_all();
		break;
	case 3:
		lcd_fill(0);
		if(sel==0)
			{
			lcd_text12(24,3,"车辆类型选择",12,LCD_MODE_SET); 
		    lcd_text12(0,19,"3.  危品车",10,LCD_MODE_SET);
			}
		else
			lcd_text12(12,10,"车辆类型:危品车",15,LCD_MODE_SET);
		lcd_update_all();
		break;
	case 4:
		lcd_fill(0);
		if(sel==0)
			{
			lcd_text12(24,3,"车辆类型选择",12,LCD_MODE_SET); 
		    lcd_text12(0,19,"4.  大型车",10,LCD_MODE_SET);
			}
		else
			lcd_text12(12,10,"车辆类型:大型车",15,LCD_MODE_SET);
		lcd_update_all();
		break;
	case 5:
		lcd_fill(0);
		if(sel==0)
			{
			lcd_text12(24,3,"车辆类型选择",12,LCD_MODE_SET); 
		    lcd_text12(0,19,"5.  中型车",10,LCD_MODE_SET);
			}
		else
			lcd_text12(12,10,"车辆类型:中型车",15,LCD_MODE_SET);
		lcd_update_all();
		break;
	case 6:
		lcd_fill(0);
		if(sel==0)
			{
			lcd_text12(24,3,"车辆类型选择",12,LCD_MODE_SET); 
		    lcd_text12(0,19,"6.  小型车",10,LCD_MODE_SET);
			}
		else
			lcd_text12(12,10,"车辆类型:小型车",15,LCD_MODE_SET);
		lcd_update_all();
		break;
	case 7:
		lcd_fill(0);
		if(sel==0)
			{
			lcd_text12(24,3,"车辆类型选择",12,LCD_MODE_SET); 
		    lcd_text12(0,19,"7.  微型车",10,LCD_MODE_SET);
			}
		else
			lcd_text12(12,10,"车辆类型:微型车",15,LCD_MODE_SET);
		lcd_update_all();
		break;
	case 8:
		lcd_fill(0);
		if(sel==0)
			{
			lcd_text12(24,3,"车辆类型选择",12,LCD_MODE_SET); 
		    lcd_text12(0,19,"8.  出租车",10,LCD_MODE_SET);
			}
		else
			lcd_text12(12,10,"车辆类型:出租车",15,LCD_MODE_SET);
		lcd_update_all();
		break;
	}
}

static void msg( void *p)
{

}
static void show(void)
{
CounterBack=0;
	lcd_fill(0);
	lcd_text12(24,3,"车辆类型选择",12,LCD_MODE_SET);
	lcd_text12(0,19,"按确认键选择车辆类型",20,LCD_MODE_SET);
	lcd_update_all();
	
	CarType_counter=1;
	CarType_Type=1;

	CarType(CarType_counter,0);
	//--printf("\r\n车辆类型选择 = %d",CarType_counter);
}


static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_0_loggingin;
			pMenuItem->show();
			CarType_counter=0;
			CarType_Type=0;
			break;
		case KeyValueOk:
			if(CarType_Type==1)
				{
				CarType_Type=2;
				CarType(CarType_counter,1);
				//printf("\r\nCarType_Type = %d",CarType_Type);
				}
			else if(CarType_Type==2)
				{
				CarType_Type=3;
				lcd_fill(0);
				lcd_text12(12,10,"车辆类型选择完毕",16,LCD_MODE_SET);
				lcd_update_all();
				
				//写入车辆类型
				if((CarType_counter>=1)&&(CarType_counter<=8))
					memset(Menu_VechileType,0,sizeof(Menu_VechileType));
				
				if(CarType_counter==1)
					memcpy(Menu_VechileType,"客运车",6); 
				else if(CarType_counter==2)
					memcpy(Menu_VechileType,"货运车",6); 
				else if(CarType_counter==3)
					memcpy(Menu_VechileType,"危品车",6);
				else if(CarType_counter==4)
					memcpy(Menu_VechileType,"大型车",6); 
				else if(CarType_counter==5)
					memcpy(Menu_VechileType,"中型车",6); 
				else if(CarType_counter==6)
					memcpy(Menu_VechileType,"小型车",6); 
				else if(CarType_counter==6)
					memcpy(Menu_VechileType,"微型车",6); 
				else if(CarType_counter==8)
					memcpy(Menu_VechileType,"出租车",6);  

				}
			else if(CarType_Type==3)
				{
				CarSet_0_counter=3;//设置第3项
				pMenuItem=&Menu_0_loggingin;
				pMenuItem->show();
				
				CarType_counter=0;
				CarType_Type=0;
				}
			break;
		case KeyValueUP:
			if(	CarType_Type==1)
				{
				CarType_counter--;
				if(CarType_counter<1)
					CarType_counter=8;
				//printf("\r\n  up  车辆类型选择 = %d",CarType_counter);
				CarType(CarType_counter,0);
				}
			break;
		case KeyValueDown:
			if(	CarType_Type==1)
				{
				CarType_counter++;
				if(CarType_counter>8)
					CarType_counter=1;
				//printf("\r\n down 车辆类型选择 = %d",CarType_counter);
				CarType(CarType_counter,0);
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
	CounterBack=0;
	pMenuItem=&Menu_0_loggingin;
	pMenuItem->show();

	
}


MENUITEM	Menu_0_2_CarType=
{
    "车辆类型设置",
	12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


