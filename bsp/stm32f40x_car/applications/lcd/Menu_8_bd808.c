#include "Menu_Include.h"
#include <string.h>


unsigned char noselect_8[]={0x3C,0x7E,0xC3,0xC3,0xC3,0xC3,0x7E,0x3C};//空心
unsigned char select_8[]={0x3C,0x7E,0xFF,0xFF,0xFF,0xFF,0x7E,0x3C};//实心

DECL_BMP(8,8,select_8); DECL_BMP(8,8,noselect_8); 

static unsigned char menu_pos_8=0;
static PMENUITEM psubmenu[3]=
{
	&Menu_8_1_updataResult,
	&Menu_8_2_BDbatchTrans,
	&Menu_8_3_electronicDisp
};
static void menuswitch(void)
{
unsigned char i=0;
	
lcd_fill(0);
lcd_text12(0,3,"新增",4,LCD_MODE_SET);
lcd_text12(0,17,"信息",4,LCD_MODE_SET);
for(i=0;i<3;i++)
	lcd_bitmap(30+i*11, 5, &BMP_noselect_8, LCD_MODE_SET);
lcd_bitmap(30+menu_pos_8*11,5,&BMP_select_8,LCD_MODE_SET);
lcd_text12(30,19,(char *)(psubmenu[menu_pos_8]->caption),psubmenu[menu_pos_8]->len,LCD_MODE_SET);
lcd_update_all();
}
static void msg( void *p)
{
}
static void show(void)
{
    //menu_pos_8=0;
	menuswitch();
}


static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		pMenuItem=&Menu_1_Idle;
		pMenuItem->show();
		CounterBack=0;
		break;
	case KeyValueOk:
			pMenuItem=psubmenu[menu_pos_8];//疲劳驾驶
			pMenuItem->show();
		break;
	case KeyValueUP:
		if(menu_pos_8==0) 
			menu_pos_8=2;
		else
			menu_pos_8--;
		menuswitch();		
		break;
	case KeyValueDown:
		menu_pos_8++;
		if(menu_pos_8>2)
			menu_pos_8=0;
		menuswitch();
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
MENUITEM	Menu_8_bd808new=
{
    "新增信息",
	8,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

