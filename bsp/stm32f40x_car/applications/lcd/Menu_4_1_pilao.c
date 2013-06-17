#include  <string.h>
#include "Menu_Include.h"
#include "Lcd.h"
#include <string.h>




unsigned char DisOverSpeedS[25]={"S: 11/02/12  15:01:02"};
unsigned char DisOverSpeedE[25]={"E: 11/02/12  20:21:03"};


static unsigned char PiLaoNumScreen=1;

static unsigned char PiLaoSreen=1;

static void drawPiLao_1(void)
{
unsigned char screenNUM=0;
unsigned char Num[20]={"1.    驾驶证号码"};
unsigned char Code[19]={"                  "};//

screenNUM=PiLaoNumScreen/2;
Num[0]+=screenNUM;

memcpy(Code,PilaoJilu[screenNUM].PCard,18);
lcd_fill(0);
lcd_text12( 0, 3,( char*)Num,sizeof(Num),LCD_MODE_SET);
lcd_text12(15,20,(char*)Code,sizeof(Code),LCD_MODE_SET);
lcd_update_all();
}

static void drawPiLao_2()
{
unsigned char t_s[35],t_e[35];
unsigned char screenNUM=0;

screenNUM=PiLaoNumScreen/2;
lcd_fill(0);
rt_kprintf((char*)t_s,"S: %02d/%02d/%02d %02d:%02d:%02d",PilaoJilu[screenNUM-1].StartTime[0],PilaoJilu[screenNUM-1].StartTime[1],PilaoJilu[screenNUM-1].StartTime[2],PilaoJilu[screenNUM-1].StartTime[3],PilaoJilu[screenNUM-1].StartTime[4],PilaoJilu[screenNUM-1].StartTime[5]);
rt_kprintf((char*)t_e,"E: %02d/%02d/%02d %02d:%02d:%02d",PilaoJilu[screenNUM-1].EndTime[0],PilaoJilu[screenNUM-1].EndTime[1],PilaoJilu[screenNUM-1].EndTime[2],PilaoJilu[screenNUM-1].EndTime[3],PilaoJilu[screenNUM-1].EndTime[4],PilaoJilu[screenNUM-1].EndTime[5]);
lcd_text12(0, 5,(char *)t_s,strlen((char *)t_s),LCD_MODE_SET);
lcd_text12(0,19,(char *)t_s,strlen((char *)t_e),LCD_MODE_SET);

lcd_update_all();
}
static void msg( void *p)
{
}
static void show(void)
{
	ErrorRecord=0;//疲劳超速记录错误清0
	StartDisTiredExpspeed=0;
	tire_Flag=0;//查看疲劳超速报警记录过程标志清0;


	lcd_fill(0);
	lcd_text12(0,10,"按确认键查看疲劳记录",20,LCD_MODE_SET);
	lcd_update_all();
	
    //读疲劳驾驶记录
}


static void keypress(unsigned int key)
{
unsigned char temp=0;
unsigned char tired_num=0;



	switch(KeyValue)
		{
		case KeyValueMenu:
			CounterBack=0;
			ErrorRecord=0;//疲劳超速记录错误清0
			StartDisTiredExpspeed=0;
			tire_Flag=0;//查看疲劳报警记录过程标志清0;
			PiLaoSreen=1;
			
			pMenuItem=&Menu_4_InforTirExspd;
			pMenuItem->show();
			break;
		case KeyValueOk:
			if(PiLaoSreen==1)
				{
				PiLaoSreen=2;
				tired_num=Api_DFdirectory_Query(tired_warn,0);   //查询当前疲劳驾驶记录数目
                if(tired_num>0)
					{
					tire_Flag=1;
					//rt_kprintf("\r\n  已有  疲劳驾驶 的记录 %d 条\r\n",TiredDrv_write);
					if(tired_num>=3)
						ReadPiLao(3);
					else	
						ReadPiLao(tired_num);
					Dis_pilao(data_tirexps);
					}
				else
					{
					tire_Flag=2;
					//rt_kprintf("\r\n无疲劳驾驶记录,取超速驾驶记录\r\n");
					}
				}
			else if(PiLaoSreen==2)
				{
				PiLaoSreen=3;
				if(tire_Flag==3)
					{
					tire_Flag=4;
					PiLaoNumScreen=0;
					StartDisTiredExpspeed=1;
			        lcd_fill(0);	
					lcd_text12(20,10,"按下翻逐条查看",14,LCD_MODE_SET);
					lcd_update_all();
					}
				}

			break;
		case KeyValueUP:
			if(tire_Flag==4)
				{
				if(PiLaoNumScreen>0)
					PiLaoNumScreen--;
				if(PiLaoNumScreen<1)
					PiLaoNumScreen=1;
				if(PiLaoNumScreen%2==1)
					drawPiLao_1();//驾驶证代码
				else
					{
					if(ErrorRecord==0)
						{
						StartDisTiredExpspeed=0;
						drawPiLao_2();//开始时间  结束时间
						}
					}
				}
			break;
		case KeyValueDown:
			if(tire_Flag==4)
				{
				PiLaoNumScreen++;
               /* if(TiredDrv_write>=3)
					temp=6;
				else
					temp=TiredDrv_write*2;*/
				if(PiLaoNumScreen>=temp)
					PiLaoNumScreen=temp;
					
				if(PiLaoNumScreen%2==1)
					drawPiLao_1();//驾驶证代码
				else
					{
					if(ErrorRecord==0)
						{
						StartDisTiredExpspeed=0;
						drawPiLao_2();//开始时间  结束时间
						}
					}
				}
			break;
		}
	KeyValue=0;

}


static void timetick(unsigned int systick)
{
	if(tire_Flag==1)//疲劳驾驶记录
		{
		PiLaoNumScreen=0;//显示速度第几屏的序号
		tire_Flag=3;//不能再此清0，还要进入下一级菜单
		lcd_fill(0);
		//疲劳16
		lcd_text12(0,10,"按确认键查看疲劳记录",20,LCD_MODE_SET);
		lcd_update_all();
		}
	else if(tire_Flag==2)//无疲劳驾驶记录
		{
		tire_Flag=0;
		lcd_fill(0);
		lcd_text12(18,10,"无疲劳驾驶记录",14,LCD_MODE_SET);
		lcd_update_all();
		}


	CounterBack++;
	if(CounterBack!=MaxBankIdleTime)
		return;
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();
    CounterBack=0;
	
	ErrorRecord=0;//疲劳超速记录错误清0
	StartDisTiredExpspeed=0;
	tire_Flag=0;//查看疲劳报警记录过程标志清0;
	PiLaoSreen=1;
}




ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_4_1_pilao=
{
    "疲劳驾驶查看",
	12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


