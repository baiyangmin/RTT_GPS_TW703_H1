#include "Menu_Include.h"
#include  <string.h>


void Disp_DnsIP(u8 par)
{
u8 dns1[50]={"1. "},dns2[50]={"2. "},apn[20]={"APN:"};
u8 len1=0,len2=0;
u8 ip1[30]={"   .   .   .   :    "},ip2[30]={"   .   .   .   :    "};
u8 i=0;
if(par==1)
	{
	len1=strlen((char *)DomainNameStr);
	memcpy(dns1+3,DomainNameStr,len1);
	len2=strlen((char *)DomainNameStr_aux);
	memcpy(dns2+3,DomainNameStr_aux,len2);

	lcd_fill(0);
	lcd_text12(0,3,(char *)dns1,len1+3,LCD_MODE_SET);
	lcd_text12(0,18,(char *)dns2,len2+3,LCD_MODE_SET);
	lcd_update_all();
	}
else if(par==2)
	{
	for(i=0;i<4;i++)
		ip1[i*4]=RemoteIP_main[i]/100+'0';
	for(i=0;i<4;i++)
		ip1[1+i*4]=RemoteIP_main[i]%100/10+'0';
	for(i=0;i<4;i++)
		ip1[2+i*4]=RemoteIP_main[i]%10+'0';

	//ip1[21]=RemotePort_1/10000+'0';
	ip1[16]=RemotePort_main%10000/1000+'0';
	ip1[17]=RemotePort_main%1000/100+'0';
	ip1[18]=RemotePort_main%100/10+'0';
	ip1[19]=RemotePort_main%10+'0';

	for(i=0;i<4;i++)
		ip2[i*4]=RemoteIP_aux[i]/100+'0';
	for(i=0;i<4;i++)
		ip2[1+i*4]=RemoteIP_aux[i]%100/10+'0';
	for(i=0;i<4;i++)
		ip2[2+i*4]=RemoteIP_aux[i]%10+'0';

	//ip2[21]=RemotePort_2/10000+'0';
	ip2[16]=RemotePort_aux%10000/1000+'0';
	ip2[17]=RemotePort_aux%1000/100+'0';
	ip2[18]=RemotePort_aux%100/10+'0';
	ip2[19]=RemotePort_aux%10+'0';

	if(strncmp((char *)APN_String,"CMNET",5)==0)
		{
		memcpy(apn+4,"CMNET",5);
		len1=5;
		}
	else if(strncmp((char *)APN_String,"UNINET",6)==0)
		{
		memcpy(apn+4,"UNINET",6);
		len1=6;
		}
	lcd_fill(0);
	lcd_text12(0,0,(char *)apn,4+len1,LCD_MODE_SET);
	lcd_text12(0,11,(char *)ip1,20,LCD_MODE_SET);
	lcd_text12(0,22,(char *)ip2,20,LCD_MODE_SET);
	lcd_update_all();
	}
}
static void msg( void *p)
{
}
static void show(void)
	{
	lcd_fill(0);
	lcd_text12(24,3,"查看设置信息",12,LCD_MODE_SET);
	lcd_text12(30,18,"请按确认键",10,LCD_MODE_SET);
	lcd_update_all();
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
			Disp_DnsIP(1);
			break;
		case KeyValueUP:
			Disp_DnsIP(1);
			break;
		case KeyValueDown:
			Disp_DnsIP(2);
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


MENUITEM	Menu_2_8_DnsIpDisplay=
{
"DNS显示",
	7,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

