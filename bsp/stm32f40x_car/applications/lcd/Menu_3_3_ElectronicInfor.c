#include "Menu_Include.h"

static void msg( void *p)
{
}
static void show(void)
{
	lcd_fill(0);
	lcd_text12(0,10,"按确认键发送电子运单",20,LCD_MODE_SET);
	lcd_update_all();
}


static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		pMenuItem=&Menu_3_InforInteract;//scr_CarMulTrans;
		pMenuItem->show();
		CounterBack=0;

		break;
	case KeyValueOk:
		/*memset(send_data,0,sizeof(send_data));
		send_data[0]=0x07;
		send_data[1]=0x01;
		send_data[2]=0x00;
		send_data[3]=0x00;
		rt_mb_send(&mb_hmi, (rt_uint32_t)&send_data[0]);*/

		lcd_fill(0);
		lcd_text12(10,10,"电子运单发送成功",16,LCD_MODE_SET);
		lcd_update_all();
		SD_ACKflag.f_Worklist_SD_0701H=1;//按键选择发送电子运单标志
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
	if(CounterBack!=MaxBankIdleTime)
		return;
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();
	CounterBack=0;

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_3_3_ElectronicInfor=
{
    "电子运单发送",
	12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

