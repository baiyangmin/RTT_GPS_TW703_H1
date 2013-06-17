#include "Menu_Include.h"

#define  DIS_Dur_width_check 11

unsigned char noselect_check[]={0x3C,0x7E,0xC3,0xC3,0xC3,0xC3,0x7E,0x3C};//空心
unsigned char select_check[]={0x3C,0x7E,0xFF,0xFF,0xFF,0xFF,0x7E,0x3C};//实心

static unsigned char menu_pos_2=0;


DECL_BMP(8,8,select_check); DECL_BMP(8,8,noselect_check); 

	
static PMENUITEM psubmenu[8]=
{
	&Menu_2_1_Status8,			//信号线
	&Menu_2_2_Speed15,			//15speed
	&Menu_2_3_CentreTextStor,	//文本消息(消息1-消息8)
	&Menu_2_4_CarInfor,		//车辆信息
	&Menu_2_5_DriverInfor,		//驾驶员信息
	&Menu_2_6_Mileage,		//里程信息
	&Menu_2_7_RequestProgram,//中心信息点播
	&Menu_2_8_DnsIpDisplay
};



static void menuswitch(void)
{
	unsigned char i;
	lcd_fill(0);
	lcd_text12(0,3,"信息",4,LCD_MODE_SET);
	lcd_text12(0,17,"查看",4,LCD_MODE_SET);
	for(i=0;i<8;i++) 
		lcd_bitmap(30+i*DIS_Dur_width_check, 5, &BMP_noselect_check, LCD_MODE_SET);
	lcd_bitmap(30+menu_pos_2*DIS_Dur_width_check,5, &BMP_select_check, LCD_MODE_SET);
	lcd_text12(30,19,(char *)(psubmenu[menu_pos_2]->caption),psubmenu[menu_pos_2]->len,LCD_MODE_SET);
	lcd_update_all();
}
static void msg( void *p)
{
}
static void show(void)
{
	//menu_pos_2=0;
	menuswitch();
}


static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		CounterBack=0;

		pMenuItem=&Menu_3_InforInteract;//scr_CarMulTrans;
		pMenuItem->show();
		break;
	case KeyValueOk:
		pMenuItem=psubmenu[menu_pos_2];
		pMenuItem->show();
		break;
	case KeyValueUP:
		if(menu_pos_2==0) 
			menu_pos_2=7;
		else
			menu_pos_2--;
		menuswitch();		
		break;
	case KeyValueDown:
		menu_pos_2++;
		if(menu_pos_2>7)
			menu_pos_2=0;
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
MENUITEM	Menu_2_InforCheck=
{
	"查看信息",
	8,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

