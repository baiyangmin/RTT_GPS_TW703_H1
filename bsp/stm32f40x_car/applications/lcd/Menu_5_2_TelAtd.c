#include "Menu_Include.h"



u8 tel_screen=0;
static void msg( void *p)
{
}
static void show(void)
{
	lcd_fill(0);
	lcd_text12(36,10,"一键回拨",8,LCD_MODE_SET);
	lcd_update_all();
}


static void keypress(unsigned int key)
{

	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_5_other;
			pMenuItem->show();
			CounterBack=0;

			tel_screen=0;
			break;
		case KeyValueOk:
			if(tel_screen==0)
				{
				tel_screen=1;
				

				OneKeyCallFlag=1;

				lcd_fill(0);
				lcd_text12(42,10,"回拨中",6,LCD_MODE_SET);
				lcd_update_all();
				//---------  一键拨号------
				/*OneKeyCallFlag=1;    
				One_largeCounter=0;
				One_smallCounter=0;*/
				//-----------------------------
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
	if(CounterBack!=MaxBankIdleTime*5)
		return;
	CounterBack=0;
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_5_2_TelAtd=
{
"一键回拨",
	8,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

