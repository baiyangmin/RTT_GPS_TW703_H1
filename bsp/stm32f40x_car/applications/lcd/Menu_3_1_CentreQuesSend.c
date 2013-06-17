#include "Menu_Include.h"
#include <stdio.h>
#include <string.h>
#include "app_moduleConfig.h"

/*unsigned char Question_menu=0;//=0时显示提问问题然后让其=1，=2显示答案，再按确认键把问题答案发送出来(前提:有中心提问消息)
unsigned char Question_Len=0;//问题长度
unsigned char Question_Counter=0;//问题显示第n屏
unsigned char Question_screen=0;//问题需要显示总屏数
unsigned int  Answer1=0,Answer2=0;//答案1的长度和答案2的长度
unsigned char Answer1_screen=0,Answer2_screen=0;//答案需要显示几屏
unsigned char Question_stor=0;//有中心下发的消息
*/

typedef struct _DIS_QUESTION_INFOR
{
unsigned char DIS_QUESTION_answer;//=0时显示提问问题然后让其=1，=2显示答案，再按确认键把问题答案发送出来(前提:有中心提问消息)
unsigned char DIS_QUESTION_len;//问题长度
unsigned char DIS_QUESTION_current;//问题需要显示总屏数
unsigned char DIS_QUESTION_total;//问题显示第n屏
unsigned char  DIS_ANSWER1_ID;//答案1的长度
unsigned char  DIS_ANSWER2_ID;//答案2的长度
unsigned int  DIS_ANSWER1_len;//答案1的长度
unsigned int  DIS_ANSWER2_len;//答案2的长度
unsigned char  DIS_ANSWER1[20];//答案1的长度
unsigned char  DIS_ANSWER2[20];//答案2的长度
unsigned char DIS_ANSWER1_current;//答案1需要显示几屏
unsigned char DIS_ANSWER2_current;//答案2需要显示几屏
unsigned char DIS_QUESTION_effic;//有中心下发的消息
}DIS_QUESTION_INFOR;

DIS_QUESTION_INFOR DIS_QUESTION_INFOR_temp;
u8 mark_first=0;
//-----  提问 ------
/*typedef struct _CENTER_ASK
{
  unsigned char  ASK_SdFlag; //  标志位           发给 TTS  1  ；   TTS 回来  2
  unsigned int   ASK_floatID; // 提问流水号
  unsigned char  ASK_infolen;// 信息长度  
  unsigned char  ASK_answerID;    // 回复ID
  unsigned char  ASK_info[30];//  信息内容
  unsigned char  ASK_answer[30];  // 候选答案  
}CENTRE_ASK;

CENTRE_ASK     ASK_Centre;  // 中心提问
*/
static void msg( void *p)
{
}
static void show(void)
{
if(mark_first==0)
	{
	mark_first=1;
	lcd_fill(0);
	lcd_text12(24,3,"中心提问消息",12,LCD_MODE_SET);
	lcd_text12(6,18,"按确认键查看并回复",18,LCD_MODE_SET);
	lcd_update_all();
	}
}

static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_3_InforInteract;
			pMenuItem->show();

			mark_first=0;			
			CounterBack=0;			
			memset(&DIS_QUESTION_INFOR_temp,0,sizeof(DIS_QUESTION_INFOR_temp));
			break;
		case KeyValueOk:
			if(mark_first==1)
				{
				mark_first=2;
				//Question_Read();
				Api_RecordNum_Read(ask_quesstion,1, (u8*)&ASK_Centre,sizeof(ASK_Centre)); 
				rt_kprintf("\r\n信息内容: %s",ASK_Centre.ASK_info); 
	            rt_kprintf("\r\n候选答案: %s",ASK_Centre.ASK_answer); 
				if(ASK_Centre.ASK_SdFlag==1)
					{
					DIS_QUESTION_INFOR_temp.DIS_QUESTION_effic=1;
					if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer==0)
						{
						DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer=1;
			            DIS_QUESTION_INFOR_temp.DIS_QUESTION_len=ASK_Centre.ASK_infolen;//问题长度
			            DIS_QUESTION_INFOR_temp.DIS_ANSWER1_ID=ASK_Centre.ASK_answer[0];
						DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len=(((u16)ASK_Centre.ASK_answer[1])<<8)+((u16)ASK_Centre.ASK_answer[2]);
						memcpy(DIS_QUESTION_INFOR_temp.DIS_ANSWER1,ASK_Centre.ASK_answer+3,DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len);
			            DIS_QUESTION_INFOR_temp.DIS_ANSWER2_ID=ASK_Centre.ASK_answer[3+DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len];
						DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len=(((u16)ASK_Centre.ASK_answer[4+DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len])<<8)+((u16)ASK_Centre.ASK_answer[5+DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len]);
						memcpy(DIS_QUESTION_INFOR_temp.DIS_ANSWER2,ASK_Centre.ASK_answer+6+DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len,DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len);		
						DIS_QUESTION_INFOR_temp.DIS_QUESTION_current++;
						if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_len%40)//问题需要显示几屏?
							DIS_QUESTION_INFOR_temp.DIS_QUESTION_total=DIS_QUESTION_INFOR_temp.DIS_QUESTION_len/40+1;
						else
							DIS_QUESTION_INFOR_temp.DIS_QUESTION_total=DIS_QUESTION_INFOR_temp.DIS_QUESTION_len/40;

						if(DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len%40)//答案1需要显示几屏?
							DIS_QUESTION_INFOR_temp.DIS_ANSWER1_current=DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len/40+1;
						else
							DIS_QUESTION_INFOR_temp.DIS_ANSWER1_current=DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len/40;

						if(DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len%40)//答案2需要显示几屏?
							DIS_QUESTION_INFOR_temp.DIS_ANSWER1_current=DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len/40+1;
						else
							DIS_QUESTION_INFOR_temp.DIS_ANSWER1_current=DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len/40; 

						lcd_fill(0);
						if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_len<=40)
							{
							if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_len<=20)
								lcd_text12(0,0,(char *)ASK_Centre.ASK_info,ASK_Centre.ASK_infolen,LCD_MODE_SET);
							else
								{
								lcd_text12(0, 0,(char *)ASK_Centre.ASK_info,20,LCD_MODE_SET);
								lcd_text12(0,15,(char *)ASK_Centre.ASK_info,(ASK_Centre.ASK_infolen-20),LCD_MODE_SET);
								}
							}
						lcd_update_all();
						}		
					}
				/*else
					{
					lcd_fill(0);
					lcd_text12(24,10,"没有提问消息",12,LCD_MODE_SET);
					lcd_update_all();
					}*/
				}
			else if(mark_first==2)
				{
				mark_first=0;
				if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_effic==1)//把选择的结果发给中心
				{
				lcd_fill(0);
				lcd_text12(12,10,"问题答案发送成功",16,LCD_MODE_SET);
				lcd_update_all();
				if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer==2)
					ASK_Centre.ASK_answerID=DIS_QUESTION_INFOR_temp.DIS_ANSWER1_ID; 
				if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer==3)
					ASK_Centre.ASK_answerID=DIS_QUESTION_INFOR_temp.DIS_ANSWER2_ID;	
				rt_kprintf("\r\n发到中心的答案ID:%d",ASK_Centre.ASK_answerID);
				ASK_Centre.ASK_SdFlag=2;//   把结果发送给中心  
					}
				}
			break;
		case KeyValueUP:
			if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_effic==1)
				{
				if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer==3)
					{
					DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer=2;
					if((DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len<20)&&(DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len<20))
						{
						lcd_fill(0);
						lcd_text12(0,0,(char *)DIS_QUESTION_INFOR_temp.DIS_ANSWER1,DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len,LCD_MODE_INVERT);
						lcd_text12(0,16,(char *)DIS_QUESTION_INFOR_temp.DIS_ANSWER2,DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len,LCD_MODE_SET);
						lcd_update_all();
						}
					}
				}
			break;
		case KeyValueDown:
			if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_effic==1)
				{
				if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer==1)
					{
					DIS_QUESTION_INFOR_temp.DIS_QUESTION_current++;
					if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_current>=DIS_QUESTION_INFOR_temp.DIS_QUESTION_total)
						{
						DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer=2;//显示答案
						rt_kprintf("\r\ndown:答案1_len=%d,str1=%s\r\n",DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len,DIS_QUESTION_INFOR_temp.DIS_ANSWER1);
			            rt_kprintf("\r\ndown:答案2_len=%d,str2=%s\r\n",DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len,DIS_QUESTION_INFOR_temp.DIS_ANSWER2);		
			  
						if((DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len<20)&&(DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len<20))
							{
							lcd_fill(0);
							lcd_text12(0,0,(char *)DIS_QUESTION_INFOR_temp.DIS_ANSWER1,DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len,LCD_MODE_INVERT);
							lcd_text12(0,16,(char *)DIS_QUESTION_INFOR_temp.DIS_ANSWER2,DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len,LCD_MODE_SET);
							lcd_update_all();
							}
						}
					 /*else//问题显示其他部分
					 	{
					 	}*/
					}
				else if(DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer==2)
					{
					DIS_QUESTION_INFOR_temp.DIS_QUESTION_answer=3;
					if((DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len<20)&&(DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len<20))
						{
						lcd_fill(0);
						lcd_text12(0,0,(char *)DIS_QUESTION_INFOR_temp.DIS_ANSWER1,DIS_QUESTION_INFOR_temp.DIS_ANSWER1_len,LCD_MODE_SET);
						lcd_text12(0,16,(char *)DIS_QUESTION_INFOR_temp.DIS_ANSWER2,DIS_QUESTION_INFOR_temp.DIS_ANSWER2_len,LCD_MODE_INVERT);
						lcd_update_all();
						}
					}
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
		mark_first=0;
		memset(&DIS_QUESTION_INFOR_temp,0,sizeof(DIS_QUESTION_INFOR_temp));
		}
}



ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_3_1_CenterQuesSend=
{
	"中心提问消息",
	12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

