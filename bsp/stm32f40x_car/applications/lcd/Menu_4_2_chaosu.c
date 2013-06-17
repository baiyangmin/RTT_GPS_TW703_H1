#include  <string.h>
#include "Menu_Include.h"
#include "Lcd.h"



static unsigned char ChaoSuNumScreen=1;
static unsigned char ChaoScreen=1;//新加

static void drawChaoSu_1(void)
{
	unsigned char screenNUMchaosu=0;
	unsigned char Num[20]={"1.    驾驶证号码"};
	unsigned char Code[19]={"                  "};//

	screenNUMchaosu=ChaoSuNumScreen/2;
	Num[0]+=screenNUMchaosu;
	
	memcpy(Code,ChaosuJilu[screenNUMchaosu].PCard,18);
	lcd_fill(0);
	lcd_text12( 0, 3,( char*)Num,sizeof(Num),LCD_MODE_SET);
	lcd_text12(15,20,(char*)Code,sizeof(Code),LCD_MODE_SET);
	lcd_update_all();


}
static void drawChaoSu_2(void)
{
	unsigned char t_s[30],t_e[30],sp[30];
	unsigned char screenNUMchaosu=0;
	
	screenNUMchaosu=ChaoSuNumScreen/2;
	
	lcd_fill(0);
	rt_kprintf((char*)t_s,"S: %02d/%02d/%02d %02d:%02d:%02d",ChaosuJilu[screenNUMchaosu-1].StartTime[0],ChaosuJilu[screenNUMchaosu-1].StartTime[1],ChaosuJilu[screenNUMchaosu-1].StartTime[2],ChaosuJilu[screenNUMchaosu-1].StartTime[3],ChaosuJilu[screenNUMchaosu-1].StartTime[4],ChaosuJilu[screenNUMchaosu-1].StartTime[5]);
	rt_kprintf((char*)t_e,"E: %02d/%02d/%02d %02d:%02d:%02d",ChaosuJilu[screenNUMchaosu-1].EndTime[0],ChaosuJilu[screenNUMchaosu-1].EndTime[1],ChaosuJilu[screenNUMchaosu-1].EndTime[2],ChaosuJilu[screenNUMchaosu-1].EndTime[3],ChaosuJilu[screenNUMchaosu-1].EndTime[4],ChaosuJilu[screenNUMchaosu-1].EndTime[5]);
	rt_kprintf((char*)sp,"MaxSpeed:%04dKm/h",ChaosuJilu[screenNUMchaosu-1].Speed);
	lcd_text12(2, 2,(char *)t_s,strlen((char *)t_s),LCD_MODE_SET);
	lcd_text12(2,12,(char *)t_e,strlen((char *)t_e),LCD_MODE_SET);
	lcd_text12(2,22,(char *)sp,strlen((char *)sp),LCD_MODE_SET);
	lcd_update_all();
}
static void msg( void *p)
{
}
static void show(void)
{
ErrorRecord=0;//疲劳超速记录错误清0
StartDisTiredExpspeed=0;
expsp_Flag=0;//查看疲劳超速报警记录过程标志清0;


lcd_fill(0);
lcd_text12(0,10,"按确认键查看超速记录",20,LCD_MODE_SET);
lcd_update_all();

//读超速驾驶记录

}


static void keypress(unsigned int key)
{
	unsigned char temp=0;
	unsigned char exspeed_num=0;

	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_4_InforTirExspd;
			pMenuItem->show();

			
			CounterBack=0;
			ErrorRecord=0;//疲劳超速记录错误清0
			StartDisTiredExpspeed=0;
			expsp_Flag=0;//查看疲劳报警记录过程标志清0;

			ChaoScreen=1;

			break;
		case KeyValueOk:
			if(ChaoScreen==1)
				{
				ChaoScreen=2;
				exspeed_num=Api_DFdirectory_Query(tired_warn,0);   //查询当前疲劳驾驶记录数目
                if(exspeed_num>0)
					{
					expsp_Flag=1;
					//rt_kprintf("\r\n  已有  疲劳驾驶 的记录 %d 条\r\n",TiredDrv_write);
					if(exspeed_num>=3)
						ReadEXspeed(3);
					else	
						ReadEXspeed(exspeed_num);
					Dis_chaosu(data_tirexps);
					}
				else
					{
					expsp_Flag=2;
					//rt_kprintf("\r\n无超速驾驶记录  read\r\n");
					}
				}
			else if(ChaoScreen==2)
				{
				ChaoScreen=3;
				if(expsp_Flag==3)
					{
					expsp_Flag=4;
					ChaoSuNumScreen=0;
					StartDisTiredExpspeed=1;
			        lcd_fill(0);	
					lcd_text12(18,10,"按下翻逐条查看",14,LCD_MODE_SET);
					lcd_update_all();
					}
				}
			
			break;
		case KeyValueUP:
			if(expsp_Flag==4)
				{
				if(ChaoSuNumScreen>0)
					ChaoSuNumScreen--;
				if(ChaoSuNumScreen<1)
					ChaoSuNumScreen=1;
				if(ChaoSuNumScreen%2==1)
					drawChaoSu_1();
				else
					{
					if(ErrorRecord==0)
						{
						StartDisTiredExpspeed=0;
						drawChaoSu_2();//开始时间  结束时间
						}
					}
				}
			break;
		case KeyValueDown:
			if(expsp_Flag==4)
				{
				ChaoSuNumScreen++;
				
				/*if(ExpSpdRec_write>=3)
					temp=6;
				else
					temp=ExpSpdRec_write*2;*/
				if(ChaoSuNumScreen>=temp)
					ChaoSuNumScreen=temp;
				
				if(ChaoSuNumScreen%2==1)
					drawChaoSu_1();
				else
					{
					if(ErrorRecord==0)
						{
						StartDisTiredExpspeed=0;
						drawChaoSu_2();//开始时间  结束时间
						}
					}
				}
			break;
		}
	KeyValue=0;
}


static void timetick(unsigned int systick)
{
   if(expsp_Flag==1)//超速驾驶记录
		{
		ChaoSuNumScreen=0;//显示速度第几屏的序号
		expsp_Flag=3;//不LCD_MODE_SET能再此清0，还要进入下一级菜单
		lcd_fill(0);
		lcd_text12(0,10,"按确认键查看超速记录",20,LCD_MODE_SET);
		lcd_update_all();
		}
	else if(expsp_Flag==2)//无超速驾驶记录
		{
		expsp_Flag=0;
		lcd_fill(0);
		lcd_text12(18,10,"无超速驾驶记录",14,LCD_MODE_SET);
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
	expsp_Flag=0;//查看超速报警记录过程标志清0;
	ChaoScreen=1;
}



ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_4_2_chaosu=
{   
    "超速驾驶查看",
    12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


