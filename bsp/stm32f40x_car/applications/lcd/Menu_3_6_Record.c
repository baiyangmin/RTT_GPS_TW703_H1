#include "Menu_Include.h"



unsigned char Record_screen=0;//进入录音界面=1,准备发送开始/结束时为2   开始/结束已发送为3    到下一界面恢复初试值
unsigned char Record_StartEnd=0;//==1录音开始   ==2录音结束   到下一界面恢复初试值

void record_sel(unsigned char sel)
{
	lcd_fill(0);
	lcd_text12(36,3,"录音选择",8,LCD_MODE_SET);					  
	if(sel==1)
		{
		lcd_text12(24,19,"开始",4,LCD_MODE_INVERT);
		lcd_text12(72,19,"结束",4,LCD_MODE_SET);
		}
    else
    	{
		lcd_text12(24,19,"开始",4,LCD_MODE_SET);
		lcd_text12(72,19,"结束",4,LCD_MODE_INVERT);
    	}
    lcd_update_all();
}

static void msg( void *p)
{
}
static void show(void)
{
	record_sel(1);
	Record_screen=1;
	Record_StartEnd=1;
}


static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		pMenuItem=&Menu_3_InforInteract;
		pMenuItem->show();
		CounterBack=0;
		    
		Record_screen=0;//进入录音界面=1,准备发送开始/结束时为2	 开始/结束已发送为3    到下一界面恢复初试值
		Record_StartEnd=0;//==1录音开始	 ==2录音结束   到下一界面恢复初试值
		break;
	case KeyValueOk:
		if(Record_screen==1)
			{
			Record_screen=2;
			lcd_fill(0);
			if(Record_StartEnd==1)
				lcd_text12(10,10,"发送录音选择:开始",17,LCD_MODE_SET);
			else if(Record_StartEnd==2)
				lcd_text12(10,10,"发送录音选择:结束",17,LCD_MODE_SET);
			lcd_update_all();
			}
		else if(Record_screen==2)
			{
			Record_screen=3;
			lcd_fill(0);
			if(Record_StartEnd==1)
				lcd_text12(18,10,"开始录音已发送",14,LCD_MODE_SET);
			else if(Record_StartEnd==2)
				lcd_text12(18,10,"结束录音已发送",14,LCD_MODE_SET);
			lcd_update_all();
			}
		else if(Record_screen==3)//回到录音开始/录音结束界面
			{
			Record_screen=1;
			Record_StartEnd=1;

			record_sel(1);
			}

		break;
	case KeyValueUP:
        if(Record_screen==1)
        	{
        	Record_StartEnd=1;
			record_sel(1);
        	}
		break;
	case KeyValueDown:
         if(Record_screen==1)
        	{
        	Record_StartEnd=2;
			record_sel(2);
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
MENUITEM	Menu_3_6_Record=
{
	"录音开始结束",
	12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

