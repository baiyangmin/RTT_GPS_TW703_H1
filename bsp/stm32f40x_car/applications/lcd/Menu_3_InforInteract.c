#include "Menu_Include.h"
#include <string.h>


#define  DIS_Dur_width_inter 11


unsigned char noselect_inter[]={0x3C,0x7E,0xC3,0xC3,0xC3,0xC3,0x7E,0x3C};//空心
unsigned char select_inter[]={0x3C,0x7E,0xFF,0xFF,0xFF,0xFF,0x7E,0x3C};//实心
DECL_BMP(8,8,select_inter); 
DECL_BMP(8,8,noselect_inter); 

static unsigned char menu_pos_3=0;
static PMENUITEM psubmenu[8]=
{
	&Menu_3_1_CenterQuesSend,
	&Menu_3_2_FullorEmpty,
	&Menu_3_3_ElectronicInfor,
	&Menu_3_4_Multimedia,
	&Menu_3_5_MultimediaTrans,
	&Menu_3_6_Record,
	&Menu_3_7_Affair,
	&Menu_3_8_LogOut,
};


void menuswitch(void)
{
unsigned char i=0;

	lcd_fill(0);
	lcd_text12(0,3,"信息",4,LCD_MODE_SET);
	lcd_text12(0,17,"交互",4,LCD_MODE_SET);
	for(i=0;i<8;i++)
		lcd_bitmap(30+i*DIS_Dur_width_inter, 5, &BMP_noselect_inter, LCD_MODE_SET);
    lcd_bitmap(30+menu_pos_3*DIS_Dur_width_inter, 5, &BMP_select_inter, LCD_MODE_SET);
    lcd_text12(30,19,(char *)(psubmenu[menu_pos_3]->caption),psubmenu[menu_pos_3]->len,LCD_MODE_SET);
	lcd_update_all();
}


static void msg( void *p)
{
}
static void show(void)
{
	//menu_pos_3=0;
	menuswitch();
}



static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		CounterBack=0;
		pMenuItem=&Menu_4_InforTirExspd;//
		pMenuItem->show();
		break;
	case KeyValueOk:
		pMenuItem=psubmenu[menu_pos_3];//鉴权注册
		pMenuItem->show();
		break;
	case KeyValueUP:
		if(menu_pos_3==0) 
			menu_pos_3=7;
		else
			menu_pos_3--;
		menuswitch();		
		break;
	case KeyValueDown:
		menu_pos_3++;
		if(menu_pos_3>7)
			menu_pos_3=0;
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
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();
	CounterBack=0;

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_3_InforInteract=
{
    "交互信息",
	8,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


