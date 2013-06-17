#include  <string.h>
#include "Menu_Include.h"

#define  Sim_width1  6


static u8 Sim_SetFlag=1;
static u8 Sim_SetCounter=0;


unsigned char select_Sim[]={0x0C,0x06,0xFF,0x06,0x0C};

DECL_BMP(8,5,select_Sim);


void Sim_Set(u8 par,u8 type1_2)
{
	lcd_fill(0);
	lcd_text12(0,3,(char *)Menu_Sim_Code,Sim_SetFlag-1,LCD_MODE_SET);
	
	if(type1_2==1)
		{
		lcd_bitmap(par*Sim_width1, 14, &BMP_select_Sim, LCD_MODE_SET);
		lcd_text12(0,19,"0123456789",10,LCD_MODE_SET);
		}
	lcd_update_all();
}

static void msg( void *p)
{

}
static void show(void)
{
CounterBack=0;
Sim_Set(Sim_SetCounter,1);
}


static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_0_loggingin;
			pMenuItem->show();
			memset(Menu_Sim_Code,0,sizeof(Menu_Sim_Code));
			Sim_SetFlag=1;
			Sim_SetCounter=0;
			break;
		case KeyValueOk:
			if((Sim_SetFlag>=1)&&(Sim_SetFlag<=12))
				{
				if(Sim_SetCounter<=9)
					Menu_Sim_Code[Sim_SetFlag-1]=Sim_SetCounter+'0';
				
				Sim_SetFlag++;	
				Sim_SetCounter=0;
				Sim_Set(0,1);
				}		
			else if(Sim_SetFlag==13)
				{
				Sim_SetFlag=14;
				lcd_fill(0);
				lcd_text12(0,5,(char *)Menu_Sim_Code,12,LCD_MODE_SET);
				lcd_text12(25,19,"ID设置完成",10,LCD_MODE_SET);
				lcd_update_all();		
				}
			else if(Sim_SetFlag==14)
				{
				Sim_SetFlag=1;
				CarSet_0_counter=4;

				pMenuItem=&Menu_0_loggingin;
				pMenuItem->show();
				}
			
			break;
		case KeyValueUP:
			if((Sim_SetFlag>=1)&&(Sim_SetFlag<=12))
				{
				if(Sim_SetCounter==0)
					Sim_SetCounter=9;
				else if(Sim_SetCounter>=1)
					Sim_SetCounter--;
				Sim_Set(Sim_SetCounter,1);
				}
			break;
		case KeyValueDown:
			if((Sim_SetFlag>=1)&&(Sim_SetFlag<=12))
				{
				Sim_SetCounter++;
				if(Sim_SetCounter>9)
					Sim_SetCounter=0;
				Sim_Set(Sim_SetCounter,1);	
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

	Sim_SetFlag=1;
	Sim_SetCounter=0;
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_0_3_Sim=
{
"SIM卡卡号设置",
	13,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


