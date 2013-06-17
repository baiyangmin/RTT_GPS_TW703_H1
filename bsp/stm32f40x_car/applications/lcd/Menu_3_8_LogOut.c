#include "Menu_Include.h"
#include <stdio.h>
#include <string.h>

	   
unsigned char Menu_Logout=0;  //  1:准备注册再按确认键=2   =2:发送注册
unsigned char LogInorOut_screen=0;//选择鉴权/注销,选择好以后按确认键清0
unsigned char LogInorOut=0;//  1:选择鉴权	2:选择注册


void confirm_login(unsigned char par)
{
lcd_fill(0);
if(par==1)
	{
	lcd_text12(0, 3,"清空鉴权码",10,LCD_MODE_INVERT);
	lcd_text12(60,3," 鉴权 注册",10,LCD_MODE_SET);
	lcd_text12(0,18,"终端注销",8,LCD_MODE_SET);
	}
else if(par==2)
	{
	lcd_text12(0, 3,"清空鉴权码",10,LCD_MODE_SET);
	lcd_text12(66,3,"鉴权",4,LCD_MODE_INVERT);
	lcd_text12(96,3,"注册",4,LCD_MODE_SET);
	lcd_text12(0,18,"终端注销",8,LCD_MODE_SET);
	}
else if(par==3)
	{
	lcd_text12(0, 3,"清空鉴权码 鉴权 ",16,LCD_MODE_SET);
	lcd_text12(96,3,"注册",4,LCD_MODE_INVERT);
	lcd_text12(0,18,"终端注销",8,LCD_MODE_SET);
	}
else if(par==4)
	{
	lcd_text12(0, 3,"清空鉴权码 鉴权 注册",20,LCD_MODE_SET);
	lcd_text12(0,18,"终端注销",8,LCD_MODE_INVERT);
	}
lcd_update_all();
}
static void msg( void *p)
{
}
static void show(void)
   {
   confirm_login(1);
   LogInorOut_screen=1;
   LogInorOut=1;
   }

static void keypress(unsigned int key)
{
     u8  Reg_buf[40];
	 
   switch(KeyValue)
	   {
	   case KeyValueMenu:
		   pMenuItem=&Menu_3_InforInteract;
		   pMenuItem->show();
		   CounterBack=0;

		   Menu_Logout=0;
		   LogInorOut_screen=0;
		   LogInorOut=0;//	1:鉴权	 2:注销
		   break;
	   case KeyValueOk:
	   	   //清空鉴权码
	   	   if(LogInorOut==1)//清空鉴权码
			   {
			   LogInorOut_screen=0;
			   LogInorOut=0;
			   lcd_fill(0);
			   lcd_text12(6,10,"按确认键清空鉴权码",18,LCD_MODE_SET);
			   lcd_update_all();
			   Menu_Logout=1;
			   }
		   else if(Menu_Logout==1)
			   	{
			   	Menu_Logout=0;
				
		              //-----------------------------------------------------------	
				    JT808Conf_struct.Regsiter_Status=0; 
				    memset(Reg_buf,0,sizeof(Reg_buf));
				    memcpy(Reg_buf,JT808Conf_struct.ConfirmCode,20);
				    Reg_buf[20]=JT808Conf_struct.Regsiter_Status;			
		                  Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));   
				//---------------------------------------------------------------
				lcd_fill(0); 
				lcd_text12(24,10,"鉴权码已清空",12,LCD_MODE_SET);
				lcd_update_all();
			   	}
		   //鉴权
		  else if(LogInorOut==2)//鉴权
			   {
			   LogInorOut_screen=0;
			   LogInorOut=0;
			   lcd_fill(0);
			   lcd_text12(0,10,"按确认键发送车台鉴权",20,LCD_MODE_SET);
			   lcd_update_all();
			   Menu_Logout=2;
			   }
		   else if(Menu_Logout==2)//鉴权已发送 
			   {
			   Menu_Logout=0;

			   lcd_fill(0);
			   lcd_text12(30,10,"鉴权已发送",10,LCD_MODE_SET);
			   lcd_update_all();
			   
			   DEV_Login.Operate_enable=1;
			   
			   DEV_Login.Enable_sd=1;
			   }
		   //注册
		   else if(LogInorOut==3)//注册
			   {
			   LogInorOut_screen=0;
			   LogInorOut=0;
			   lcd_fill(0);
			   lcd_text12(0,10,"按确认键发送车台注册",20,LCD_MODE_SET);
			   lcd_update_all();
			   Menu_Logout=3;
			   }
		   else if(Menu_Logout==3)
			   {
			   Menu_Logout=0;

				DEV_regist.Enable_sd=1; // set 发送注册标志位

			   lcd_fill(0);
			   lcd_text12(30,10,"注册已发送",10,LCD_MODE_SET);
			   lcd_update_all();
			   }
		   //终端注销
		     else if(LogInorOut==4)
			   {
			   LogInorOut_screen=0;
			   LogInorOut=0;
			   lcd_fill(0);
			   lcd_text12(0,10,"按确认键发送终端注销",20,LCD_MODE_SET);
			   lcd_update_all();
			   Menu_Logout=4;
			   }
		   else if(Menu_Logout==4)
			   {
			   Menu_Logout=0;

			   DEV_regist.DeRegst_sd=1; // set 发送终端注销标志位

			   lcd_fill(0);
			   lcd_text12(18,10,"终端注销已发送",14,LCD_MODE_SET);
			   lcd_update_all();
			   }
		   break;
	   case KeyValueUP:
		   if(LogInorOut_screen==1)
			   {
			   LogInorOut--;
			   if(LogInorOut<=1)
				   LogInorOut=1;
			   confirm_login(LogInorOut);
			   }
		   break;
		   
	   case KeyValueDown:
		   if(LogInorOut_screen==1)
			   {
			   LogInorOut++;
			   if(LogInorOut>=4)
				   LogInorOut=4;
			   confirm_login(LogInorOut);
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
   else
	   {
	   pMenuItem=&Menu_1_Idle;
	   pMenuItem->show();
	   CounterBack=0;

	   Menu_Logout=0;
	   LogInorOut_screen=0;
	   LogInorOut=0;//	1:鉴权	 2:注册
	   }
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM    Menu_3_8_LogOut=
{
   "鉴权注册",
   8,
   &show,
   &keypress,
   &timetick,
   &msg,
   (void*)0
};

