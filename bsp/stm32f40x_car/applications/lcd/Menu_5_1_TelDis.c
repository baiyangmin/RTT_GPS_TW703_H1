#include "Menu_Include.h"


unsigned char Menu_TelText=0;
unsigned char TelText_scree=0;

unsigned char Tel_num_code[22][3]={"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20"};



void Dis_TelText(unsigned char screen)
	{
	u8 len=0;
	
	if(screen<10)
		len=1;
	else
		len=2;
	lcd_fill(0);
	lcd_text12(0,3,(char *)Tel_num_code[screen],len,LCD_MODE_SET);
	if(PhoneBook_8[screen-1].Effective_Flag)
		{
		lcd_text12(15,3,(char*)PhoneBook_8[screen-1].UserStr,strlen((char*)PhoneBook_8[screen-1].UserStr),LCD_MODE_SET);
		lcd_text12(40,19,(char *)PhoneBook_8[screen-1].NumberStr,strlen((char*)PhoneBook_8[screen-1].NumberStr),LCD_MODE_SET);
		}
	else
		lcd_text12(50,19,"Null",4, LCD_MODE_SET);
	lcd_update_all();
	}

static void msg( void *p)
{
}
static void show(void)
	{
	lcd_fill(0);
	lcd_text12(20,10,"电话本记录查看",14,LCD_MODE_SET);
	lcd_update_all();
	Menu_TelText=1;
	}

static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_5_other;
			pMenuItem->show();
			CounterBack=0;

			Menu_TelText=0;
            TelText_scree=0;
			break;
		case KeyValueOk:
			if(Menu_TelText==1)
				{
				PhoneBook_Read();
				Menu_TelText=2;
				TelText_scree=1;
				Dis_TelText(1);
				}
			break;
		case KeyValueUP:
			if(Menu_TelText==2)
				{
				TelText_scree--;
				if(TelText_scree<=1)
					TelText_scree=1;
				Dis_TelText(TelText_scree);
				}
			break;
		case KeyValueDown:
			if(Menu_TelText==2)
				{
				TelText_scree++;
				if(TelText_scree>=20)
					TelText_scree=20;
				Dis_TelText(TelText_scree);
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
		}
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_5_1_TelDis=
{
    "电话本查看",
	10,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


