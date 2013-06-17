#include "Menu_Include.h"

u8 can_screen=0;
u8 can_counter=1;
u8 can_ID_counter=0;
u8 CAN_baud[13]={"波特率:009600"};
u8 CAN_ID1[13]={"CAN1:00000001"};
u8 CAN_ID2[13]={"CAN2:00000002"};

u8 canid_check[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void can_select(u8  par )
{
lcd_fill(0);
if(par==1)
	{	
	lcd_text12(20, 3,"CAN ID    查询",14,LCD_MODE_INVERT);
	lcd_text12(20,19,"CAN 波特率查询",14,LCD_MODE_SET);
	}
else
    {
	lcd_text12(20, 3,"CAN ID    查询",14,LCD_MODE_SET);
	lcd_text12(20,19,"CAN 波特率查询",14,LCD_MODE_INVERT);
	}
lcd_update_all();
}

void can_set_check(u8  par )
{
lcd_fill(0);
if(par==1)
	{	
	CAN_ID1[5]= canid_check[(u8)(BD_EXT.CAN_1_ID&0xF0000000)>>28];
	CAN_ID1[6]= canid_check[(u8)(BD_EXT.CAN_1_ID&0x0F000000)>>24];
	CAN_ID1[7]= canid_check[(u8)(BD_EXT.CAN_1_ID&0x00F00000)>>20];
	CAN_ID1[8]= canid_check[(u8)(BD_EXT.CAN_1_ID&0x000F0000)>>16];
	CAN_ID1[9]= canid_check[(u8)(BD_EXT.CAN_1_ID&0x0000F000)>>12];
	CAN_ID1[10]=canid_check[(u8)(BD_EXT.CAN_1_ID&0x00000F00)>>8];
	CAN_ID1[11]=canid_check[(u8)(BD_EXT.CAN_1_ID&0x000000F0)>>4];
	CAN_ID1[12]=canid_check[(u8)(BD_EXT.CAN_1_ID&0x0000000F)];

	CAN_ID2[5]= canid_check[(u8)(BD_EXT.CAN_2_ID&0xF0000000)>>28];
	CAN_ID2[6]= canid_check[(u8)(BD_EXT.CAN_2_ID&0x0F000000)>>24];
	CAN_ID2[7]= canid_check[(u8)(BD_EXT.CAN_2_ID&0x00F00000)>>20];
	CAN_ID2[8]= canid_check[(u8)(BD_EXT.CAN_2_ID&0x000F0000)>>16];
	CAN_ID2[9]= canid_check[(u8)(BD_EXT.CAN_2_ID&0x0000F000)>>12];
	CAN_ID2[10]=canid_check[(u8)(BD_EXT.CAN_2_ID&0x00000F00)>>8];
	CAN_ID2[11]=canid_check[(u8)(BD_EXT.CAN_2_ID&0x000000F0)>>4];
	CAN_ID2[12]=canid_check[(u8)(BD_EXT.CAN_2_ID&0x0000000F)];
	
	lcd_text12(0, 3,(char *)CAN_ID1,13,LCD_MODE_SET);
	lcd_text12(0,19,(char *)CAN_ID2,13,LCD_MODE_SET);
	}
else
	{
	CAN_baud[7]= JT808Conf_struct.BD_EXT.GNSS_Baud_Value/100000+'0';
	CAN_baud[8]= JT808Conf_struct.BD_EXT.GNSS_Baud_Value%100000/10+'0';
	CAN_baud[9]= JT808Conf_struct.BD_EXT.GNSS_Baud_Value%10000/10+'0';
	CAN_baud[10]=JT808Conf_struct.BD_EXT.GNSS_Baud_Value%1000/10+'0';
	CAN_baud[11]=JT808Conf_struct.BD_EXT.GNSS_Baud_Value%100/10+'0';
	CAN_baud[12]=JT808Conf_struct.BD_EXT.GNSS_Baud_Value%10+'0'; 
	lcd_text12(0,10,(char *)CAN_baud,13,LCD_MODE_SET);
	}
lcd_update_all();
}
static void msg( void *p)
{
}
static void show(void)
{
	can_select(1);

}


static void keypress(unsigned int key)
{

	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_5_other;
			pMenuItem->show();
			CounterBack=0;

			can_screen=0;
			can_counter=1;
			can_ID_counter=0;
			break;
		case KeyValueOk:
			if(can_screen==0)
				{
			       can_screen=1;
				can_set_check(can_counter);
				}
			else if(can_screen==1)
				{
				can_screen=0;
				can_select(can_counter);
				}
			break;
		case KeyValueUP:
			if(can_screen==0)
				{
				can_counter=1;
				can_select(can_counter);
				}
			if((can_screen==1)&&(can_counter==1))
				{
				}
			break;
		case KeyValueDown:
			if(can_screen==0)
				{
				can_counter=2;
				can_select(can_counter);
				}
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

	can_screen=0;
	can_counter=1;
	can_ID_counter=0;
	
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_5_5_can=
{
"CAN参数查询",
	11,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

