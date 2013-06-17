#include <rtthread.h>
#include <stm32f4xx.h>
#include "rtc.h"


__IO uint32_t AsynchPrediv = 0, SynchPrediv = 0;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_InitTypeDef RTC_InitStructure;
RTC_DateTypeDef RTC_DateStructure;
static struct rt_device rtc;

static rt_err_t rt_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* Open Interrupt */
    }

    return RT_EOK;
}

static rt_size_t rt_rtc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    return 0;
}

static rt_err_t rt_rtc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    //rt_time_t *time;
    RT_ASSERT(dev != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
       // time = (rt_time_t *)args;
        /* read device */
        RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
    {
//        time = (rt_time_t *)args;

        /* Enable PWR and BKP clocks */
        //RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        /* Allow access to BKP Domain */
        //PWR_BackupAccessCmd(ENABLE);
        PWR_BackupAccessCmd(ENABLE);
        /* Wait until last write operation on RTC registers has finished */
        //RTC_WaitForLastTask();
			   RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
        if(RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure) == ERROR)
						{
							rt_kprintf("\n\r>> !! RTC Set Time failed. !! <<\n\r");
						} 
			  else
						{
							rt_kprintf("\n\r>> !! RTC Set Time success.~^_^~ !! <<\n\r");
							RTC_TimeShow();
						}
			RTC_WaitForSynchro();
			
        /* Change the current time */
        //RTC_SetCounter(*time);

        /* Wait until last write operation on RTC registers has finished */
        //RTC_WaitForLastTask();

        RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
    }
    break;
		case RT_DEVICE_CTRL_RTC_SET_DATE:
		{
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); 
			PWR_BackupAccessCmd(ENABLE);
			RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
			if(RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure) == ERROR)
						{
							rt_kprintf("\n\r>> !! RTC Set Date failed. !! <<\n\r");
						} 
			  else
						{
							rt_kprintf("\n\r>> !! RTC Set Date success.~^_^~ !! <<\n\r");
							RTC_TimeShow();
						}
			RTC_WaitForSynchro();
    }
		break;
    }

    return RT_EOK;
}


/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : 0 reday,-1 error.
*******************************************************************************/
int RTC_Config(void)
{
    u32 count=0x200000;
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to RTC */
  PWR_BackupAccessCmd(ENABLE);
    
  RCC_LSEConfig(RCC_LSE_ON);

  /* Wait till LSE is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && (--count) );
  if ( count == 0 )
    {
        return -1;
    }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  
  SynchPrediv = 0xFF;
  AsynchPrediv = 0x7F;
  
  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();
  return 0;
}
/***********************************************RTC注册******************************************************/
void rt_hw_rtc_init(void)
{
    rtc.type	= RT_Device_Class_RTC;

    if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F2)
			{
						rt_kprintf("rtc is not configured\n");
						//rt_kprintf("please configure with set_date and set_time\n");
		/**************************************************************RTC First	Init****************************/	
						 RTC_Config();
					if ( RTC_Config() != 0)
						{
								rt_kprintf("rtc configure fail...\r\n");
								return ;
						}
					else
					{
						/* Configure the RTC data register and RTC prescaler */
							RTC_InitStructure.RTC_AsynchPrediv = AsynchPrediv;
							RTC_InitStructure.RTC_SynchPrediv = SynchPrediv;
							RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
			 
							/* Check on RTC init */
							if (RTC_Init(&RTC_InitStructure) == ERROR)
							{
								rt_kprintf("\n\r  /!\\***** RTC Prescaler Config failed ********/!\\ \n\r");
							}			
					 }
			rt_kprintf("NOW  set  Arbitrary initial time 11:11:11\n");	
      RTC_TimeStructure.RTC_Hours = 11;
      RTC_TimeStructure.RTC_Minutes = 11;
      RTC_TimeStructure.RTC_Seconds = 11;					 
			}
    else
			{
					/* Wait for RTC registers synchronization */
					RTC_WaitForSynchro();
			}

    /* register rtc device */
    rtc.init 	= RT_NULL;
    rtc.open 	= rt_rtc_open;
    rtc.close	= RT_NULL;
    rtc.read 	= rt_rtc_read;
    rtc.write	= RT_NULL;
    rtc.control = rt_rtc_control;

    /* no private */
    //rtc.user_data = RT_NULL;

    rt_device_register(&rtc, "rtc", RT_DEVICE_FLAG_RDWR);
#ifdef RT_USING_FINSH
	//RTC_TimeShow();
#endif

    return;
}

void RTC_TimeShow(void)
{
  /* Get the current Time */
	//RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
  rt_kprintf("\n\r  The current time is :  %0.2d:%0.2d:%0.2d\n\r", RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
	RTC_GetDate(RTC_Format_BIN,  &RTC_DateStructure);
	rt_kprintf("\n\r  The current Date is :  20%0.2d-%0.2d-%0.2d\n\r", RTC_DateStructure.RTC_Year, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date);
	//rt_kprintf("xiao,xiao");
}

/**********************************************************可调用标准函数接口*****************************************/

void set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second)
{
	 rt_device_t device;

    RTC_TimeStructure.RTC_Hours = hour;
    RTC_TimeStructure.RTC_Minutes 	= minute;
    RTC_TimeStructure.RTC_Seconds 	= second;
    
  
    device = rt_device_find("rtc");
    if (device != RT_NULL)
    {
        rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &RTC_TimeStructure);
    }
}

void set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t date)
{
	 
	  rt_device_t device;

    RTC_DateStructure.RTC_Year = year;
    RTC_DateStructure.RTC_Month 	= month;
    RTC_DateStructure.RTC_Date 	= date;
    
  
    device = rt_device_find("rtc");
    if (device != RT_NULL)
    {
        rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_DATE, &RTC_TimeStructure);
    }
	
}


/*****************************************************串口时间设置****************************************************/
// void RTC_TimeRegulate(void)
// {
//   uint32_t tmp_hh = 0xFF, tmp_mm = 0xFF, tmp_ss = 0xFF;
//   /*uint32_t tmp_yy=0xFF,  tmp_month = 0xFF, tmp_date = 0xFF;*/
//   printf("\n\r==============Time Settings=====================================\n\r");
//   RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
//   printf("  Please Set Hours:\n\r");
//   while (tmp_hh == 0xFF)
//   {
//     tmp_hh = USART_Scanf(23);
//     RTC_TimeStructure.RTC_Hours = tmp_hh;
//   }
//   printf("  %0.2d\n\r", tmp_hh);
//   
//   printf("  Please Set Minutes:\n\r");
//   while (tmp_mm == 0xFF)
//   {
//     tmp_mm = USART_Scanf(59);
//     RTC_TimeStructure.RTC_Minutes = tmp_mm;
//   }
//   printf("  %0.2d\n\r", tmp_mm);
//   
//   printf("  Please Set Seconds:\n\r");
//   while (tmp_ss == 0xFF)
//   {
//     tmp_ss = USART_Scanf(59);
//     RTC_TimeStructure.RTC_Seconds = tmp_ss;
// 		Temp_second = tmp_ss;
//   }
//   printf("  %0.2d\n\r", tmp_ss);

//   /* Configure the RTC time register */
//   if(RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure) == ERROR)
//   {
//     printf("\n\r>> !! RTC Set Time failed. !! <<\n\r");
//   } 
//   else
//   {
//     printf("\n\r>> !! RTC Set Time success. !! <<\n\r");
//     RTC_TimeShow();
//     /* Indicator for the RTC configuration */
//     RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
//   }

// // 	/***************************************************xiao日期****************************************/
// // 	  printf("\n\r==============Date Settings=====================================\n\r");
// //   //RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
// //   printf("  Please Set yy:\n\r");
// //   while (tmp_yy == 0xFF)
// //   {
// //     tmp_yy = USART_Scanf(99);
// //     RTC_DateStructure.RTC_Year = tmp_yy;
// //   }
// //   printf("  %0.2d\n\r", tmp_yy);
// //   
// //   printf("  Please Set Month:\n\r");
// //   while (tmp_month == 0xFF)
// //   {
// //     tmp_month = USART_Scanf(12);
// //     RTC_DateStructure.RTC_Month = tmp_month;
// //   }
// //   printf("  %0.2d\n\r", tmp_month);
// //   
// //   printf("  Please Set Date:\n\r");
// //   while (tmp_date == 0xFF)
// //   {
// //     tmp_date = USART_Scanf(31);
// //     RTC_DateStructure.RTC_Date = tmp_date;
// // 		//Temp_second = tmp_ss;
// //   }
// //   printf("  %0.2d\n\r", tmp_date);

// //   /* Configure the RTC time register */
// //   if(RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure) == ERROR)
// //   {
// //     printf("\n\r>> !! RTC Set Date failed. !! <<\n\r");
// //   } 
// //   else
// //   {
// //     printf("\n\r>> !! RTC Set Date success. !! <<\n\r");
// //     RTC_TimeShow();
// //     /* Indicator for the RTC configuration */
// //     RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
// //   }
// // /***************************************************xiao日期****************************************/

// }
// /********************************************rtc显示相关********************************************************************/
// /* #ifdef RT_USING_FINSH
// #include <finsh.h>
// #include <time.h>
// time_t time(time_t* t)
// {
//     rt_device_t device;
//     time_t time;

//     device = rt_device_find("rtc");
//     if (device != RT_NULL)
//     {
//         rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
//         if (t != RT_NULL) *t = time;
//     }

//     return time;
// }

// void set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t day)
// {
//     time_t now;
//     struct tm* ti;
//     rt_device_t device;

//     ti = RT_NULL;
//     // get current time 
//     time(&now);

//     ti = localtime(&now);
//     if (ti != RT_NULL)
//     {
//         ti->tm_year = year - 1900;
//         ti->tm_mon 	= month - 1; // ti->tm_mon 	= month; 0~11 
//         ti->tm_mday = day;
//     }

//     now = mktime(ti);

//     device = rt_device_find("rtc");
//     if (device != RT_NULL)
//     {
//         rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &now);
//     }
// }
// FINSH_FUNCTION_EXPORT(set_date, set date. e.g: set_date(2010,2,28))

// void set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second)
// {
//     time_t now;
//     struct tm* ti;
//     rt_device_t device;

//     ti = RT_NULL;
//    
//     time(&now);

//     ti = localtime(&now);
//     if (ti != RT_NULL)
//     {
//         ti->tm_hour = hour;
//         ti->tm_min 	= minute;
//         ti->tm_sec 	= second;
//     }

//     now = mktime(ti);
//     device = rt_device_find("rtc");
//     if (device != RT_NULL)
//     {
//         rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &now);
//     }
// }
// FINSH_FUNCTION_EXPORT(set_time, set time. e.g: set_time(23,59,59))

// void list_date(void)
// {
//     time_t now;

//     time(&now);
//     rt_kprintf("%s\n", ctime(&now));
// }
// FINSH_FUNCTION_EXPORT(list_date, show date and time.)
// #endif */








	