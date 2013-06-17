#include "Menu_Include.h"
#include "Lcd.h"

u8  SensorUPCounter=0;
static void msg( void *p)
{
char *pinfor;
char len=0;

pinfor=(char *)p;
len=strlen(pinfor);

lcd_fill(0);
lcd_text12(0,10,pinfor,len,LCD_MODE_SET);
lcd_update_all();
}
static void show(void)
{
	msg(XinhaoStatus);
}


static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_2_InforCheck;
			pMenuItem->show();
			CounterBack=0;
			break;
		case KeyValueOk:
			msg(XinhaoStatus);
			break;
		case KeyValueUP:
			msg(XinhaoStatus);
			break;
		case KeyValueDown:
			msg(XinhaoStatus);
			break;
		}
 KeyValue=0;
}


static void timetick(unsigned int systick)
{
u8 i=0,SensorFlag=0;

SensorUPCounter++;
if(SensorUPCounter>=10)
	{
	SensorUPCounter=0;
	Vehicle_sensor=Get_SensorStatus();
	SensorFlag=0x80;
	for(i=1;i<8;i++)  
		{
		if(Vehicle_sensor&SensorFlag)   
			XinhaoStatus[10+i]=0x31;
		else
			XinhaoStatus[10+i]=0x30; 
		SensorFlag=SensorFlag>>1;   
		} 
	lcd_fill(0);
	lcd_text12(0,10,XinhaoStatus,strlen(XinhaoStatus),LCD_MODE_SET);
	lcd_update_all();

	}
	CounterBack++;
	if(CounterBack!=MaxBankIdleTime)
		return;
	CounterBack=0;
	
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_2_1_Status8=
{
    "ÐÅºÅÏß×´Ì¬",
	10,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


