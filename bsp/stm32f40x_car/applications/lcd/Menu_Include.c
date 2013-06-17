#include <stdio.h>

#include "Menu_Include.h"
#include <string.h>

#include "stm32f4xx.h"
unsigned char XinhaoStatus[20]={"信号线状态:00000000"};

unsigned int  tzxs_value=6000;
unsigned char send_data[10];
MB_SendDataType mb_senddata;

unsigned int CounterBack=0;
unsigned char UpAndDown=1;//参数设置主菜单选择序号

unsigned char Dis_date[22]={"2000/00/00  00:00:00"};//20
unsigned char Dis_speDer[20]={"000 km/h    000 度"};

unsigned char GPS_Flag=0,Gprs_Online_Flag=0;//记录gps gprs状态的标志


unsigned char speed_time_rec[15][6];//年  月  日  时  分  速度
unsigned char DriverName[22],DriverCardNUM[20];//从IC卡里读出的驾驶员姓名和驾驶证号码
unsigned char ServiceNum[13];//设备的唯一性编码,IMSI号码的后12位

unsigned char KeyValue=0;
unsigned char KeyCheck_Flag[4]={0,0,0,0};             

unsigned char ErrorRecord=0;//疲劳超速记录   疲劳时间错误为1超速时间错误为2,按任意键清0
PilaoRecord PilaoJilu[12];
ChaosuRecord ChaosuJilu[20];

unsigned char StartDisTiredExpspeed=0;//开始显示疲劳或者超速驾驶的记录,再判断提示时间信息错误时用
unsigned char tire_Flag=0,expsp_Flag=0;//疲劳驾驶/超速驾驶  有记录为1(显示有几条记录)，无记录为2，查看记录变为3(显示按down逐条查看)
unsigned char pilaoCounter=0,chaosuCounter=0;//记录返回疲劳驾驶和超速驾驶的条数
unsigned char pilaoCouAscii[2],chaosuCouAscii[2];
DispMailBoxInfor LCD_Post,GPStoLCD,OtherToLCD,PiLaoLCD,ChaoSuLCD;

unsigned char SetVIN_NUM=1;// :设置车牌号码  2:设置VIN
unsigned char OK_Counter=0;//记录在快捷菜单下ok键按下的次数
unsigned char Screen_In=0,Screen_in0Z=0; //记录备选屏内选中的汉字

unsigned char OKorCancel=1,OKorCancel2=1,OKorCancelFlag=1;
unsigned char SetTZXSFlag=0,SetTZXSCounter=0;//SetTZXSFlag  1:校准车辆特征系数开始  2:校准车辆特征系数结束
                                                 //    1数据导出(... .../完成)  2:usb设备拔出
unsigned char OUT_DataCounter=0;//指定导出数据类型  1、2、3
unsigned char DataOutStartFlag=0;//数据导出标志
unsigned char DataOutOK=0;

unsigned char Rx_TZXS_Flag=0;

unsigned char battery_flag=0,tz_flag=0;
unsigned char USB_insertFlag=0;


unsigned char BuzzerFlag=0;//=1响1声  ＝11响2声

unsigned char DaYin=0;//待机按下打印键为101，2s后为1，开始打印(判断是否取到数据，没有提示错误，渠道数据打印)
unsigned char DaYinDelay=0;

unsigned char FileName_zk[11];

//==============12*12========读字库中汉字的点阵==========
unsigned char test_00[24],Read_ZK[24];
unsigned char DisComFlag=0;
unsigned char ICcard_flag=0;



unsigned char DisInfor_Menu[8][20];
unsigned char DisInfor_Affair[8][20];

unsigned char UpAndDown_nm[4]={0xA1,0xFC,0xA1,0xFD};//↑ ↓

//========================================================================
unsigned char UpdataDisp[8]={"001/000"};//北斗升级进度
unsigned char BD_updata_flag=0;//北斗升级度u盘文件的标志
unsigned int  FilePageBD_Sum=0;//记录文件大小，读文件大小/514
unsigned int  bd_file_exist=0;//读出存在要升级的文件
unsigned char device_version[30]={"主机版本:V BD 1.00"};  
unsigned char bd_version[20]={"模块版本:V 00.00.000"};


unsigned char ISP_Updata_Flag=0; //远程升级主机程序进度显示标志   1:开始升级  2:升级完成

unsigned char data_tirexps[120];
unsigned char OneKeyCallFlag=0;   //  一键拨号
unsigned char BD_upgrad_contr=0; //  北斗升级控制
unsigned char print_rec_flag=0;  // 打印记录标志 
u8 CarSet_0_counter=1;//记录设置车辆信息的设置内容1:车牌号2:类型3:颜色

//------------ 使用前锁定相关 ------------------
unsigned char Menu_Car_license[10];//存放车牌号码
u8 Menu_Sim_Code[12];
u8 Menu_VechileType[10];  //  车辆类型
u8 Menu_VecLogoColor[10]; // 车牌颜色
u8 Menu_color_num=0;   // JT415    1  蓝 2 黄 3 黑 4 白 9其他
u8 menu_speedtype=0;  //速度获取方式   0:gps速度     1:传感器速度
u8 menu_color_flag=0; //颜色设置完成

u8 Antenna_open_flag=0;
u8 Antenna_open_counter=0;


MENUITEM *pMenuItem;   


//中心下发消息或者条件触发显示消息函数
void Cent_To_Disp(void)
{
Antenna_open_counter++;
if(Antenna_open_counter>=10)
	{
	Antenna_open_counter=0;
	if(Antenna_open_flag==1)
		{
		lcd_fill(0);
		lcd_text12(36,10,"天线开路",8,LCD_MODE_SET);
		lcd_update_all();
		}
	else if(Antenna_open_flag==2)
		{
		lcd_fill(0);
		lcd_text12(24,10,"天线恢复正常",12,LCD_MODE_SET);
		lcd_update_all();
		}
	}
}
void version_disp(void)
{
	lcd_fill(0);
	lcd_text12(0, 3,(char *)device_version,strlen((const char*)device_version),LCD_MODE_SET); 
	lcd_text12(0,19,(char *)bd_version,sizeof(bd_version),LCD_MODE_SET);
	lcd_update_all();
}
//  0   1            34             67
//(3)  1 [2-33]   2 [35-66]   3 [68-99]
void ReadPiLao(unsigned char NumPilao)
{
unsigned char i=0,j=0;
unsigned char Read_PilaoData[32];

data_tirexps[0]=NumPilao;//总条数
for(i=0,j=0;i<NumPilao;i++,j++)
	data_tirexps[1+j*31]=i+1;	
for(i=0;i<NumPilao;i++)
	{
	Api_DFdirectory_Read(tired_warn,Read_PilaoData,31,0,i); // 从new-->old  读取
	memcpy(&data_tirexps[i*31+2],Read_PilaoData,30);
	}
}
void ReadEXspeed(unsigned char NumExspeed)
{
unsigned char i=0,j=0;
unsigned char Read_ChaosuData[32];

data_tirexps[0]=NumExspeed;//总条数
for(i=0,j=0;i<NumExspeed;i++,j++)
	data_tirexps[1+j*32]=i+1;	
for(i=0;i<NumExspeed;i++)
	{
	Api_DFdirectory_Read(spd_warn,Read_ChaosuData,32,0,i); // 从new-->old  读取
    memcpy(&data_tirexps[i*32+2],Read_ChaosuData,31);
	}
}
void Dis_pilao(unsigned char *p)
{
unsigned char i,j;
pilaoCounter=*p;
if(pilaoCounter==0) return;
if(pilaoCounter>12) return;


pilaoCouAscii[0]=pilaoCounter/10+0x30;
pilaoCouAscii[1]=pilaoCounter%10+0x30;
for(i=0;i<pilaoCounter;i++)
	{
	PilaoJilu[i].Num=*(p+1+i*31);
	memcpy(PilaoJilu[i].PCard,p+2+i*31,18);
	memcpy(PilaoJilu[i].StartTime,p+20+i*31,6);
	memcpy(PilaoJilu[i].EndTime,p+26+i*31,6);
	for(j=0;j<6;j++)
		PilaoJilu[i].StartTime[j]=(PilaoJilu[i].StartTime[j]>>4)*10+(PilaoJilu[i].StartTime[j]&0x0f);
	for(j=0;j<6;j++)
		PilaoJilu[i].EndTime[j]=(PilaoJilu[i].EndTime[j]>>4)*10+(PilaoJilu[i].EndTime[j]&0x0f);

	if((PilaoJilu[i].StartTime[0]>99)||(PilaoJilu[i].StartTime[1]>12)||(PilaoJilu[i].StartTime[2]>31)||PilaoJilu[i].StartTime[3]>23||PilaoJilu[i].StartTime[4]>59||PilaoJilu[i].StartTime[5]>59)
		ErrorRecord=1;
	if((PilaoJilu[i].EndTime[0]>99)||(PilaoJilu[i].EndTime[1]>12)||(PilaoJilu[i].EndTime[2]>31)||PilaoJilu[i].EndTime[3]>23||PilaoJilu[i].EndTime[4]>59||PilaoJilu[i].EndTime[5]>59)
		ErrorRecord=1;
	}


}
void Dis_chaosu(unsigned char *p)
{
unsigned char i,j;
chaosuCounter=*p;
if(chaosuCounter==0) return;
if(chaosuCounter>20) return;

chaosuCouAscii[0]=chaosuCounter/10+0x30;
chaosuCouAscii[1]=chaosuCounter%10+0x30;

for(i=0;i<chaosuCounter;i++)
	{
	ChaosuJilu[i].Num=*(p+1+i*46);
	memcpy(ChaosuJilu[i].PCard,p+2+i*32,18);
	memcpy(ChaosuJilu[i].StartTime,p+20+i*32,6);
	memcpy(ChaosuJilu[i].EndTime,p+26+i*32,6);

	for(j=0;j<6;j++)
		ChaosuJilu[i].StartTime[j]=(ChaosuJilu[i].StartTime[j]>>4)*10+(ChaosuJilu[i].StartTime[j]&0x0f);
	for(j=0;j<6;j++)
		ChaosuJilu[i].EndTime[j]=(ChaosuJilu[i].EndTime[j]>>4)*10+(ChaosuJilu[i].EndTime[j]&0x0f);	
	ChaosuJilu[i].Speed=*(p+32+i*32);

	if((ChaosuJilu[i].StartTime[0]>99)||(ChaosuJilu[i].StartTime[1]>12)||(ChaosuJilu[i].StartTime[2]>31)||ChaosuJilu[i].StartTime[3]>23||ChaosuJilu[i].StartTime[4]>59||ChaosuJilu[i].StartTime[5]>59)
		ErrorRecord=2;
	if((ChaosuJilu[i].EndTime[0]>99)||(ChaosuJilu[i].EndTime[1]>12)||(ChaosuJilu[i].EndTime[2]>31)||ChaosuJilu[i].EndTime[3]>23||ChaosuJilu[i].EndTime[4]>59||ChaosuJilu[i].EndTime[5]>59)
		ErrorRecord=2;
	}
}

