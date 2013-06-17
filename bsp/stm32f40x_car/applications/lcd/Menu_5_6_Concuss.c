#include "Menu_Include.h"


u8 concuss_screen=0;
u8 concuss_step=40;
u8 concuss_dis[11]={"震动级别:  "};

static void msg( void *p)
{
}
static void show(void)
{
concuss_step=JT808Conf_struct.concuss_step;
concuss_dis[9]=concuss_step/10+'0';
concuss_dis[10]=concuss_step%10+'0';
lcd_fill(0);
lcd_text12(30,10,(char *)concuss_dis,sizeof(concuss_dis),LCD_MODE_SET);
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

			concuss_screen=0;
			break;
			
		case KeyValueOk:
			if(concuss_screen==0)
				{
				concuss_screen=1;
				JT808Conf_struct.concuss_step=concuss_step;
				Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
			

                concuss_dis[9]=concuss_step/10+'0';
				concuss_dis[10]=concuss_step%10+'0';
				lcd_fill(0);
				lcd_text12(20,3,"震动级别设置成功",16,LCD_MODE_SET);
				lcd_text12(30,19,(char *)concuss_dis,sizeof(concuss_dis),LCD_MODE_SET);
				lcd_update_all();
				}
			break;
			
		case KeyValueUP:
			 if(concuss_step==0)
				concuss_step=79;
			else if(concuss_step>=1)
				concuss_step--;
			
			concuss_dis[9]=concuss_step/10+'0';
			concuss_dis[10]=concuss_step%10+'0';
			lcd_fill(0);
			lcd_text12(30,10,(char *)concuss_dis,sizeof(concuss_dis),LCD_MODE_SET);
			lcd_update_all();
						
			break;
			
		case KeyValueDown:
			concuss_step++;
			if(concuss_step>79)
				concuss_step=0;
			
			concuss_dis[9]=concuss_step/10+'0';
			concuss_dis[10]=concuss_step%10+'0';
			lcd_fill(0);
			lcd_text12(30,10,(char *)concuss_dis,sizeof(concuss_dis),LCD_MODE_SET);
			lcd_update_all();
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
MENUITEM	Menu_5_6_Concuss=
{
"震动级别",
	8,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

