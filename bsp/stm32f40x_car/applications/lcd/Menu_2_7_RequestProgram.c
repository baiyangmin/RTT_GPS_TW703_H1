#include "Menu_Include.h"
#include "App_moduleConfig.h"


unsigned char Menu_dianbo=0;
unsigned char dianbo_scree=1;
unsigned char MSG_TypeToCenter=0;//发送给中心的序号

/*
typedef struct _MSG_BROADCAST
{
unsigned char	 INFO_TYPE; 	//	信息类型
unsigned int     INFO_LEN;		//	信息长度
unsigned char	 INFO_PlyCancel; // 点播/取消标志	   0 取消  1  点播
unsigned char	 INFO_SDFlag;	 //  发送标志位
unsigned char	 INFO_Effective; //  显示是否有效	1 显示有效	  0  显示无效	  
unsigned char	 INFO_STR[30];	//	信息内容
}MSG_BRODCAST;

MSG_BRODCAST	 MSG_Obj;	 // 信息点播		 
MSG_BRODCAST	 MSG_Obj_8[8];	// 信息点播    

*/
void SenddianboMeun(unsigned char screen,unsigned char SendOK)
{
if(SendOK==1)
  {
  MSG_Obj_8[screen-1].INFO_TYPE=screen;
  if(MSG_Obj_8[screen-1].INFO_Effective)
	  {
	  lcd_fill(0);
	  lcd_text12(30,3,(char*)MSG_Obj_8[screen-1].INFO_STR,MSG_Obj_8[screen-1].INFO_LEN,LCD_MODE_INVERT);
	  if(MSG_Obj_8[screen-1].INFO_PlyCancel==1)
		 lcd_text12(30,19,"点播成功",8,LCD_MODE_INVERT);
	  else if(MSG_Obj_8[screen-1].INFO_PlyCancel==0)
		 lcd_text12(30,19,"取消成功",8,LCD_MODE_INVERT);
	  lcd_update_all();
	  }
  MSG_TypeToCenter=screen;//发送给中心点播信息的序号
  }
else
  {
  if((screen>=1)&&(screen<=8))
	  {
	  if(MSG_Obj_8[screen-1].INFO_Effective)
		  {
		  lcd_fill(0);
		  lcd_text12(15,5,(char *)MSG_Obj_8[screen-1].INFO_STR,MSG_Obj_8[screen-1].INFO_LEN,LCD_MODE_INVERT);
		  if(MSG_Obj_8[screen-1].INFO_PlyCancel)
			  {
			  lcd_text12(15,20,"点播",4,LCD_MODE_INVERT);
			  lcd_text12(80,20,"取消",4,LCD_MODE_SET);
			  }
		  else
			  {
			  lcd_text12(15,20,"点播",4,LCD_MODE_SET);
			  lcd_text12(80,20,"取消",4,LCD_MODE_INVERT);
			  }
		  lcd_update_all();
		  }
	  }
  }
}
  
  void Dis_dianbo(unsigned char screen)
  {
  switch(screen)
	  {
	  case 1:
		  lcd_fill(0);
		  lcd_text12(0,0,"1.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[0].INFO_Effective)
			  lcd_text12(15,0,(char *)MSG_Obj_8[0].INFO_STR,MSG_Obj_8[0].INFO_LEN,LCD_MODE_INVERT);
		  else
			  lcd_text12(15,0,"无",2,LCD_MODE_INVERT);
		  
		  lcd_text12(0,16,"2.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[1].INFO_Effective)
			  lcd_text12(15,16,(char *)MSG_Obj_8[1].INFO_STR,MSG_Obj_8[1].INFO_LEN,LCD_MODE_SET);
		  else
			  lcd_text12(15,16,"无",2,LCD_MODE_SET);
  
		  if(MSG_Obj_8[0].INFO_Effective)
			  {
			  if(MSG_Obj_8[0].INFO_PlyCancel==1)
				  lcd_text12(95,0,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,0,"取消",4,LCD_MODE_INVERT);
  
			  if(MSG_Obj_8[1].INFO_PlyCancel==1)
				  lcd_text12(95,16,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,16,"取消",4,LCD_MODE_INVERT);
			  }
		  lcd_update_all();
		  break;
	  case 2:
		  lcd_fill(0);
		  lcd_text12(0,0,"1.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[0].INFO_Effective)
			  lcd_text12(15,0,(char *)MSG_Obj_8[0].INFO_STR,MSG_Obj_8[0].INFO_LEN,LCD_MODE_SET);
		  else
			  lcd_text12(15,0,"无",2,LCD_MODE_SET);
		  lcd_text12(0,16,"2.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[1].INFO_Effective)
			  lcd_text12(15,16,(char *)MSG_Obj_8[1].INFO_STR,MSG_Obj_8[1].INFO_LEN,LCD_MODE_INVERT);
		  else
			  lcd_text12(15,16,"无",2,LCD_MODE_INVERT);
		  if(MSG_Obj_8[1].INFO_Effective)
			  {
			  if(MSG_Obj_8[0].INFO_PlyCancel==1)
				  lcd_text12(95,0,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,0,"取消",4,LCD_MODE_INVERT);
  
			  if(MSG_Obj_8[1].INFO_PlyCancel==1)
				  lcd_text12(95,16,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,16,"取消",4,LCD_MODE_INVERT);
			  }
		  lcd_update_all();
		  break;
	  case 3:
		  lcd_fill(0);
		  lcd_text12(0,0,"3.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[2].INFO_Effective)
			  lcd_text12(15,0,(char *)MSG_Obj_8[2].INFO_STR,MSG_Obj_8[2].INFO_LEN,LCD_MODE_INVERT);
		  else
			  lcd_text12(15,0,"无",2,LCD_MODE_INVERT);
		  lcd_text12(0,16,"4.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[3].INFO_Effective)
			  lcd_text12(15,16,(char *)MSG_Obj_8[3].INFO_STR,MSG_Obj_8[3].INFO_LEN,LCD_MODE_SET);
		  else
			  lcd_text12(15,16,"无",2,LCD_MODE_SET);
		  if(MSG_Obj_8[2].INFO_Effective)
			  {
			  if(MSG_Obj_8[2].INFO_PlyCancel==1)
				  lcd_text12(95,0,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,0,"取消",4,LCD_MODE_INVERT);
  
			  if(MSG_Obj_8[3].INFO_PlyCancel==1)
				  lcd_text12(95,16,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,16,"取消",4,LCD_MODE_INVERT);
			  }
		  lcd_update_all();
		  break;
	  case 4: 
		  lcd_fill(0);
		  lcd_text12(0,0,"3.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[2].INFO_Effective)
			  lcd_text12(15,0,(char *)MSG_Obj_8[2].INFO_STR,MSG_Obj_8[2].INFO_LEN,LCD_MODE_SET);
		  else
			  lcd_text12(15,0,"无",2,LCD_MODE_SET);
		  lcd_text12(0,16,"4.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[3].INFO_Effective)
			  lcd_text12(15,16,(char *)MSG_Obj_8[3].INFO_STR,MSG_Obj_8[3].INFO_LEN,LCD_MODE_INVERT);
		  else
			  lcd_text12(15,16,"无",2,LCD_MODE_INVERT);
		  if(MSG_Obj_8[3].INFO_Effective)
			  {
			  if(MSG_Obj_8[2].INFO_PlyCancel==1)
				  lcd_text12(95,0,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,0,"取消",4,LCD_MODE_INVERT);
  
			  if(MSG_Obj_8[3].INFO_PlyCancel==1)
				  lcd_text12(95,16,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,16,"取消",4,LCD_MODE_INVERT);
			  }
		  lcd_update_all();
		  break;
	  case 5:
		  lcd_fill(0);
		  lcd_text12(0,0,"5.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[4].INFO_Effective)
			  lcd_text12(15,0,(char *)MSG_Obj_8[4].INFO_STR,MSG_Obj_8[4].INFO_LEN,LCD_MODE_INVERT);
		  else
			  lcd_text12(15,0,"无",2,LCD_MODE_INVERT);
		  lcd_text12(0,16,"6.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[5].INFO_Effective)
			  lcd_text12(15,16,(char *)MSG_Obj_8[5].INFO_STR,MSG_Obj_8[5].INFO_LEN,LCD_MODE_SET);
		  else
			  lcd_text12(15,16,"无",2,LCD_MODE_SET);
		  if(MSG_Obj_8[4].INFO_Effective)
			  {
			  if(MSG_Obj_8[4].INFO_PlyCancel==1)
				  lcd_text12(95,0,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,0,"取消",4,LCD_MODE_INVERT);
  
			  if(MSG_Obj_8[5].INFO_PlyCancel==1)
				  lcd_text12(95,16,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,16,"取消",4,LCD_MODE_INVERT);
			  }
		  lcd_update_all();
		  break;
	  case 6: 
		  lcd_fill(0);
		  lcd_text12(0,0,"5.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[4].INFO_Effective)
			  lcd_text12(15,0,(char *)MSG_Obj_8[4].INFO_STR,MSG_Obj_8[4].INFO_LEN,LCD_MODE_SET);
		  else
			  lcd_text12(15,0,"无",2,LCD_MODE_SET);
		  lcd_text12(0,16,"6.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[5].INFO_Effective)
			  lcd_text12(15,16,(char *)MSG_Obj_8[5].INFO_STR,MSG_Obj_8[5].INFO_LEN,LCD_MODE_INVERT);
		  else
			  lcd_text12(15,16,"无",2,LCD_MODE_INVERT);
		  if(MSG_Obj_8[5].INFO_Effective)
			  {
			  if(MSG_Obj_8[4].INFO_PlyCancel==1)
				  lcd_text12(95,0,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,0,"取消",4,LCD_MODE_INVERT);
  
			  if(MSG_Obj_8[5].INFO_PlyCancel==1)
				  lcd_text12(95,16,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,16,"取消",4,LCD_MODE_INVERT);
			  }
		  lcd_update_all();
		  break;
	  case 7:
		  lcd_fill(0);
		  lcd_text12(0,0,"7.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[6].INFO_Effective)
			  lcd_text12(15,0,(char *)MSG_Obj_8[6].INFO_STR,MSG_Obj_8[6].INFO_LEN,LCD_MODE_INVERT);
		  else
			  lcd_text12(15,0,"无",2,LCD_MODE_INVERT);
		  lcd_text12(0,16,"8.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[7].INFO_Effective)
			  lcd_text12(15,16,(char *)MSG_Obj_8[7].INFO_STR,MSG_Obj_8[7].INFO_LEN,LCD_MODE_SET);
		  else
			  lcd_text12(15,16,"无",2,LCD_MODE_SET);
		  if(MSG_Obj_8[6].INFO_Effective)
			  {
			  if(MSG_Obj_8[6].INFO_PlyCancel==1)
				  lcd_text12(95,0,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,0,"取消",4,LCD_MODE_INVERT);
  
			  if(MSG_Obj_8[7].INFO_PlyCancel==1)
				  lcd_text12(95,16,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,16,"取消",4,LCD_MODE_INVERT);
			  }
		  lcd_update_all();
		  break;
	  case 8: 
		  lcd_fill(0);
		  lcd_text12(0,0,"7.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[6].INFO_Effective)
			  lcd_text12(15,0,(char *)MSG_Obj_8[6].INFO_STR,MSG_Obj_8[6].INFO_LEN,LCD_MODE_SET);
		  else
			  lcd_text12(15,0,"无",2,LCD_MODE_SET);
		  lcd_text12(0,16,"8.",2,LCD_MODE_SET);
		  if(MSG_Obj_8[7].INFO_Effective)
			  lcd_text12(15,16,(char *)MSG_Obj_8[7].INFO_STR,MSG_Obj_8[7].INFO_LEN,LCD_MODE_INVERT);
		  else
			  lcd_text12(15,16,"无",2,LCD_MODE_INVERT);
		  if(MSG_Obj_8[7].INFO_Effective)
			  {
			  if(MSG_Obj_8[6].INFO_PlyCancel==1)
				  lcd_text12(95,0,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,0,"取消",4,LCD_MODE_INVERT);
  
			  if(MSG_Obj_8[7].INFO_PlyCancel==1)
				  lcd_text12(95,16,"点播",4,LCD_MODE_INVERT);
			  else
				  lcd_text12(95,16,"取消",4,LCD_MODE_INVERT);
			  }
		  lcd_update_all();
		  break;
	  default :
		  break;
	  
	  }
  
  }
  static void msg( void *p)
{
}
  static void show(void)
	  {
	  //u8 i=0;
	  //读出8条消息在判断显示
	  MSG_BroadCast_Read();
	  Dis_dianbo(1);
      Menu_dianbo=1;
	  }
  
  static void keypress(unsigned int key)
  {
  //u8 result=0;
	  switch(KeyValue)
		  {
		  case KeyValueMenu:
			  pMenuItem=&Menu_2_InforCheck;
			  pMenuItem->show();
			  CounterBack=0;
  
			  Menu_dianbo=0;
			  dianbo_scree=1;
              MSG_TypeToCenter=0;//发送给中心的序号
			  break; 
		  case KeyValueOk:
			  if(Menu_dianbo==1)
				  {
				  Menu_dianbo=2;
				  //将选中的序号数据显示发送的界面
				  SenddianboMeun(dianbo_scree,0);
				  }
			  else if(Menu_dianbo==2)
				  {
				  SD_ACKflag.f_MsgBroadCast_0303H=1;
				  //--- 更新最新的状态 ----------- 
				  Api_RecordNum_Write(msg_broadcast, MSG_Obj_8[dianbo_scree-1].INFO_TYPE, (u8*)&MSG_Obj_8[dianbo_scree-1],sizeof(MSG_Obj_8[dianbo_scree-1])); 
				  
				  //发送出去的界面
				  SenddianboMeun(dianbo_scree,1);
				   
				  Menu_dianbo=3;//按确认返回信息查看界面
				  dianbo_scree=1;
				  }
			  else if(Menu_dianbo==3)
				  {
				  Menu_dianbo=0;  //   1
				  dianbo_scree=1;
				  }
			  break;
		  case KeyValueUP:
			  if(Menu_dianbo==1)
				  {
				  dianbo_scree--;
				  if(dianbo_scree<=1)
					  dianbo_scree=1;
				  Dis_dianbo(dianbo_scree);
				  }
			  else if(Menu_dianbo==2)
				  {
				  MSG_Obj_8[dianbo_scree-1].INFO_PlyCancel=1;//点播
				  SenddianboMeun(dianbo_scree,0);
				  }
			  break;
		  case KeyValueDown:
			  if(Menu_dianbo==1)
				  {
				  dianbo_scree++;
				  if(dianbo_scree>=8)
					  dianbo_scree=8;
				  Dis_dianbo(dianbo_scree);
				  }
			  else if(Menu_dianbo==2)
				  {
				  MSG_Obj_8[dianbo_scree-1].INFO_PlyCancel=0;//取消
				  SenddianboMeun(dianbo_scree,0);
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
	  }
  }
  
  
  ALIGN(RT_ALIGN_SIZE)
  MENUITEM	  Menu_2_7_RequestProgram=
  {
	  "信息点播查看",
	  12,
	  &show,
	  &keypress,
	  &timetick,
	  &msg,
	  (void*)0
  };

