#include "Menu_Include.h"

u8 updataResult_screen=0;
u8 updataResult=0;
static void msg( void *p)
{
}
static void show(void)
	{
	if(updataResult_screen==0)
		{
		updataResult_screen=1;
		updataResult=1;
		lcd_fill(0);
		lcd_text12(12,3,"升级结果上报中心",16,LCD_MODE_SET);
		lcd_text12(12,18,"成功",4,LCD_MODE_INVERT);
		lcd_text12(84,18,"失败",4,LCD_MODE_SET);
		lcd_update_all();
		}
	}

static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_8_bd808new;
			pMenuItem->show();

			updataResult_screen=0;
		    updataResult=0;
			break;
		case KeyValueOk:
			if(updataResult_screen==1)
				{
				updataResult_screen=2;
				//之相应的标志上传给中心
				lcd_fill(0);
				lcd_text12(12,10,"升级结果上报中心",16,LCD_MODE_SET);
				if(updataResult==1)
					{
					updataResult=0;
					lcd_text12(36,10,"成功",4,LCD_MODE_INVERT);
					}
				else
					{
					updataResult=0;
					lcd_text12(36,10,"失败",4,LCD_MODE_INVERT);
					}
				lcd_update_all();
				}
			else if(updataResult_screen==2)
				{
				updataResult_screen=0;
				//
				}
			break;
		case KeyValueUP:
			if(updataResult_screen==1)
				{
				updataResult=1;
				lcd_fill(0);
				lcd_text12(12,3,"升级结果上报中心",16,LCD_MODE_SET);
				lcd_text12(12,18,"成功",4,LCD_MODE_INVERT);
				lcd_text12(84,18,"失败",4,LCD_MODE_SET);
				lcd_update_all();
				}
			break;
		case KeyValueDown:
			if(updataResult_screen==1)
				{
				updataResult=0;
				lcd_fill(0);
				lcd_text12(12,3,"升级结果上报中心",16,LCD_MODE_SET);
				lcd_text12(12,18,"成功",4,LCD_MODE_SET);
				lcd_text12(84,18,"失败",4,LCD_MODE_INVERT);
				lcd_update_all();
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
	else
		{
		pMenuItem=&Menu_1_Idle;
		pMenuItem->show();
		CounterBack=0;
		
		updataResult_screen=0;
		updataResult=0;
		}
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_8_1_updataResult=
{
    "升级结果",
	8,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


