/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// 文件名
 * Author:			// 作者
 * Date:			// 日期
 * Description:		// 模块描述
 * Version:			// 版本信息
 * Function List:	// 主要函数及其功能
 *     1. -------
 * History:			// 历史修改记录
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/
#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>

#include "stm32f4xx.h"
#include "mma8451.h"
#include "math.h"


/*
   软件模拟I2C 同MMA8451通讯

   SDA  PB3
   SCL  PB4
   INT1  PA14
   INT2  PA15
 */
#define TRUE	1
#define FALSE	0

#define MMA845X_ADDR 0x1d

#define I2C_Speed			100000
#define I2C1_SLAVE_ADDRESS7 0x42
#define I2C_PageSize		256

#define SCL_H		( GPIOB->BSRRL = GPIO_Pin_5 )
#define SCL_L		( GPIOB->BSRRH = GPIO_Pin_5 )
#define SCL_read	( GPIOB->IDR & GPIO_Pin_5 )

#define SDA_H		( GPIOB->BSRRL = GPIO_Pin_4 )
#define SDA_L		( GPIOB->BSRRH = GPIO_Pin_4 )
#define SDA_read	( GPIOB->IDR & GPIO_Pin_4 )

#define ERR_NONE	0x00
#define ERR_START1	0x01
#define ERR_START2	0x02
#define ERR_WAITACK 0x11

#define ERR_INITFAILED 0x80

#define STATUS_00_REG 0x00


/*
**  XYZ Data Registers
*/
#define OUT_X_MSB_REG	0x01
#define OUT_X_LSB_REG	0x02
#define OUT_Y_MSB_REG	0x03
#define OUT_Y_LSB_REG	0x04
#define OUT_Z_MSB_REG	0x05
#define OUT_Z_LSB_REG	0x06


/*
**  WHO_AM_I Device ID Register
*/
#define WHO_AM_I_REG	0x0D
#define MMA8451Q_ID		0x1A
#define MMA8452Q_ID		0x2A
#define MMA8453Q_ID		0x3A

#define F_SETUP_REG 0x09

#define TRIG_CFG_REG		0x0A
#define SYSMOD_REG			0x0B
#define INT_SOURCE_REG		0x0C
#define XYZ_DATA_CFG_REG	0x0E
//
//
#define HPF_OUT_MASK 0x10

#define FULL_SCALE_8G	0x02
#define FULL_SCALE_4G	0x01
#define FULL_SCALE_2G	0x00


/*
**  HP_FILTER_CUTOFF High Pass Filter Register
*/
#define HP_FILTER_CUTOFF_REG 0x0F

//
#define PULSE_HPF_BYP_MASK	0x20
#define PULSE_LPF_EN_MASK	0x10
#define SEL1_MASK			0x02
#define SEL0_MASK			0x01
#define SEL_MASK			0x03

#define PL_STATUS_REG	0x10
#define PL_CFG_REG		0x11
#define PL_COUNT_REG	0x12
#define PL_BF_ZCOMP_REG 0x13
#define PL_P_L_THS_REG	0x14
#define FF_MT_CFG_1_REG 0x15
#define FF_MT_CFG_2_REG 0x19

#define FF_MT_SRC_1_REG 0x16
#define FF_MT_SRC_2_REG 0x1A

#define FT_MT_THS_1_REG 0x17
#define FT_MT_THS_2_REG 0x1B

#define FF_MT_COUNT_1_REG	0x18
#define FF_MT_COUNT_2_REG	0x1C
#define TRANSIENT_CFG_REG	0x1D
#define TRANSIENT_SRC_REG	0x1E
#define TRANSIENT_THS_REG	0x1F

#define TRANSIENT_COUNT_REG 0x20
#define PULSE_CFG_REG		0x21
#define PULSE_SRC_REG		0x22
#define PULSE_THSX_REG		0x23
#define PULSE_THSY_REG		0x24
#define PULSE_THSZ_REG		0x25
#define PULSE_TMLT_REG		0x26
#define PULSE_LTCY_REG		0x27
#define PULSE_WIND_REG		0x28
#define ASLP_COUNT_REG		0x29


/*
**  CTRL_REG1 System Control 1 Register
*/
#define CTRL_REG1 0x2A
//

//
#define ASLP_RATE1_MASK 0x80
#define ASLP_RATE0_MASK 0x40
#define DR2_MASK		0x20
#define DR1_MASK		0x10
#define DR0_MASK		0x08
#define LNOISE_MASK		0x04
#define FREAD_MASK		0x02
#define ACTIVE_MASK		0x01
#define ASLP_RATE_MASK	0xC0
#define DR_MASK			0x38
//
#define ASLP_RATE_20MS	0x00
#define ASLP_RATE_80MS	ASLP_RATE0_MASK
#define ASLP_RATE_160MS ASLP_RATE1_MASK
#define ASLP_RATE_640MS ASLP_RATE1_MASK + ASLP_RATE0_MASK
//
#define DATA_RATE_1250US	0x00
#define DATA_RATE_2500US	DR0_MASK
#define DATA_RATE_5MS		DR1_MASK
#define DATA_RATE_10MS		DR1_MASK + DR0_MASK
#define DATA_RATE_20MS		DR2_MASK
#define DATA_RATE_80MS		DR2_MASK + DR0_MASK
#define DATA_RATE_160MS		DR2_MASK + DR1_MASK
#define DATA_RATE_640MS		DR2_MASK + DR1_MASK + DR0_MASK


/*
**  CTRL_REG2 System Control 2 Register
*/
#define CTRL_REG2 0x2B
//
//
#define ST_MASK		0x80
#define BOOT_MASK	0x40
#define SMODS1_MASK 0x20
#define SMODS0_MASK 0x10
#define SLPE_MASK	0x04
#define MODS1_MASK	0x02
#define MODS0_MASK	0x01
#define SMODS_MASK	0x18
#define MODS_MASK	0x03


/*
**  CTRL_REG3 Interrupt Control Register
*/
#define CTRL_REG3	0x2C
#define CTRL_REG4	0x2D
#define CTRL_REG5	0x2E
#define OFF_X_REG	0x2F
#define OFF_Y_REG	0x30
#define OFF_Z_REG	0x31


static struct rt_device dev_mma8451;

/*是否是首次运行，首次运行不上报状态*/
static uint8_t firstrun=1;

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static void I2C_delay( void )
{
	//这里可以优化速度
	//缺省150
	__IO uint8_t i = 30;
	while( i )
	{
		i--;
	}
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static void I2C_delay5ms( void )
{
	int i = 5000;
	while( i )
	{
		i--;
	}
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static uint8_t I2C_Start( void )
{
	SDA_H;
//	I2C_delay();
	SCL_H;
	I2C_delay( );
	if( !SDA_read )
	{
		return ERR_START1;  //SDA线为低电平则总线忙,退出
	}
	SDA_L;
	I2C_delay( );
	if( SDA_read )
	{
		return ERR_START2;  //SDA线为高电平则总线出错,退出
	}
	SDA_L;
	I2C_delay( );
	return ERR_NONE;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static void I2C_Stop( void )
{
	SCL_L;
	I2C_delay( );
	SDA_L;
	I2C_delay( );
	SCL_H;
	I2C_delay( );
	SDA_H;
	I2C_delay( );
}

/*
   I2C的Master 通知 Slave 正常回复
 */
static void I2C_Ack( void )
{
	SCL_L;
	I2C_delay( );
	SDA_L;
	I2C_delay( );
	SCL_H;
	I2C_delay( );
	SCL_L;
	I2C_delay( );
}

/*
   I2C的Master 通知 Slave 停止发送
 */
static void I2C_NoAck( void )
{
	SCL_L;
	I2C_delay( );
	SDA_H;
	I2C_delay( );
	SCL_H;
	I2C_delay( );
	SCL_L;
	I2C_delay( );
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static uint8_t I2C_WaitAck( void )   //返回为:=1有ACK,=0无ACK
{
	SCL_L;
	I2C_delay( );
	SDA_H;
	I2C_delay( );
	SCL_H;
	I2C_delay( );
	if( SDA_read )
	{
		SCL_L;
		I2C_delay( );
		return ERR_WAITACK;
	}
	SCL_L;
	I2C_delay( );
	return ERR_NONE;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static void I2C_SendByte( u8 SendByte ) //数据从高位到低位//
{
	uint8_t i = 8;
	while( i-- )
	{
		SCL_L;
		I2C_delay( );
		if( SendByte & 0x80 )
		{
			SDA_H;
		} else
		{
			SDA_L;
		}
		SendByte <<= 1;
		I2C_delay( );
		SCL_H;
		I2C_delay( );
	}
	SCL_L;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static uint8_t I2C_ReceiveByte( void )  //数据从高位到低位//
{
	uint8_t i			= 8;
	uint8_t ReceiveByte = 0;

	SDA_H;
	while( i-- )
	{
		ReceiveByte <<= 1;
		SCL_L;
		I2C_delay( );
		SCL_H;
		I2C_delay( );
		if( SDA_read )
		{
			ReceiveByte |= 0x01;
		}
	}
	SCL_L;
	return ReceiveByte;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static uint8_t IIC_RegWrite( uint8_t address, uint8_t reg, uint8_t val )
{
	uint8_t err = ERR_NONE;
	err = I2C_Start( );
	if( err )
	{
		goto lbl_ERR_REG_WR;
	}
	I2C_SendByte( MMA845X_ADDR << 1 );
	err = I2C_WaitAck( );
	if( err )
	{
		goto lbl_ERR_REG_WR_STOP;
	}

	I2C_SendByte( reg );
	err = I2C_WaitAck( );
	if( err )
	{
		goto lbl_ERR_REG_WR_STOP;
	}

	I2C_SendByte( val );
	err = I2C_WaitAck( );
	if( err )
	{
		goto lbl_ERR_REG_WR_STOP;
	}
	I2C_Stop( );
	//todo:这里需要延时5ms吗?
	return ERR_NONE;

lbl_ERR_REG_WR_STOP:
	I2C_Stop( );
lbl_ERR_REG_WR:
	rt_kprintf( "reg_wr error=%02x reg=%02x value=%02x\r\n", err, reg, val );
	return err;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static uint8_t IIC_RegRead( uint8_t address, uint8_t reg, uint8_t* value )
{
	uint8_t err = ERR_NONE;

	err = I2C_Start( );
	if( err )
	{
		goto lbl_ERR_REG_RD_STOP;
	}
	I2C_SendByte( address << 1 );
	err = I2C_WaitAck( );
	if( err )
	{
		goto lbl_ERR_REG_RD_STOP;
	}

	I2C_SendByte( reg );
	err = I2C_WaitAck( );
	if( err )
	{
		goto lbl_ERR_REG_RD_STOP;
	}

	err = I2C_Start( );
	if( err )
	{
		goto lbl_ERR_REG_RD;
	}

	I2C_SendByte( ( address << 1 ) | 0x01 );
	err = I2C_WaitAck( );
	if( err )
	{
		goto lbl_ERR_REG_RD_STOP;
	}

	*value = I2C_ReceiveByte( );
	I2C_NoAck( );
	I2C_Stop( );
	return ERR_NONE;

lbl_ERR_REG_RD_STOP:
	I2C_Stop( );
lbl_ERR_REG_RD:
	printf( "reg_rd error=%02x reg=%02x\r\n", err, reg );
	return err;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static uint8_t IIC_RegRead_Data( uint8_t reg, uint8_t *pdata, uint8_t count )
{
	uint8_t err				= ERR_NONE;
	uint8_t NumBytesToRead	= count;
	uint8_t * pBuffer		= pdata;

	err = I2C_Start( );
	if( err )
	{
		goto lbl_IIC_RegRead_Data_err;
	}
	I2C_SendByte( MMA845X_ADDR << 1 );
	err = I2C_WaitAck( );
	if( err )
	{
		goto lbl_IIC_RegRead_Data_err_stop;
	}

	I2C_SendByte( reg );
	err = I2C_WaitAck( );
	if( err )
	{
		goto lbl_IIC_RegRead_Data_err_stop;
	}

	err = I2C_Start( );
	if( err )
	{
		goto lbl_IIC_RegRead_Data_err;
	}

	I2C_SendByte( ( MMA845X_ADDR << 1 ) | 0x01 );
	err = I2C_WaitAck( );
	if( err )
	{
		goto lbl_IIC_RegRead_Data_err_stop;
	}

	while( NumBytesToRead )
	{
		*pBuffer = I2C_ReceiveByte( );
		if( NumBytesToRead == 1 )
		{
			I2C_NoAck( );
		} else
		{
			I2C_Ack( );
		}
		pBuffer++;
		NumBytesToRead--;
	}
	I2C_Stop( );
	return ERR_NONE;
lbl_IIC_RegRead_Data_err_stop:
	I2C_Stop( );
lbl_IIC_RegRead_Data_err:
	return err;
}

/*
   利用systick定时查询，打印中使用了systick的查询方式
 */

volatile uint32_t	ticks = 0;

uint32_t			lastsend_rotation_ticks = 0;
uint32_t			lastsend_crash_ticks	= 0;
uint8_t				last_plstatus			= 0;

uint16_t			last_g_value = 0;

uint16_t			param_mma8451_word1 = 0x8000;
uint16_t			param_mma8451_word2 = 0x1E02; /*0x1e:单位4ms的倍数  0x02:碰撞加速度门限 0.1g的倍数*/

uint8_t				crash_counter = 0;

uint8_t				CTRL_REG1_value			= 0x10;
uint8_t				PL_BF_ZCOMP_REG_value	= 0xc7;
uint8_t				PL_P_L_THS_REG_vlaue	= 0xcf;


/*
读取
 */
void EXTI9_5_IRQHandler( void )
{
	uint8_t ret,value1,value2,value3;
	rt_interrupt_enter( );
	if( EXTI_GetITStatus( EXTI_Line5 ) != RESET )
	{
		ret=IIC_RegRead( MMA845X_ADDR, INT_SOURCE_REG, &value1 );
		ret=IIC_RegRead( MMA845X_ADDR, PL_STATUS_REG, &value2 );
		ret=IIC_RegRead( MMA845X_ADDR, PULSE_SRC_REG, &value3 );
		rt_kprintf("\nINT=%02x %02x %02x\n",value1,value2,value3);
		EXTI_ClearITPendingBit( EXTI_Line5 );
	}
	rt_interrupt_leave( );
}

/*
   使用外部中断触发 PD5
 */

void EXTILine5_Config( void )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;

	/* Enable GPIOD clock */
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE );
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, ENABLE );

	/* Configure PD5 pin as input floating */
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_5;
	GPIO_Init( GPIOD, &GPIO_InitStructure );

	/* Connect EXTI Line5 to PD5 pin */
	SYSCFG_EXTILineConfig( EXTI_PortSourceGPIOD, EXTI_PinSource5 );

	/* Configure EXTI Line5 */
	EXTI_InitStructure.EXTI_Line	= EXTI_Line5;
	EXTI_InitStructure.EXTI_Mode	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init( &EXTI_InitStructure );

	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel						= EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init( &NVIC_InitStructure );
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static uint8_t mma8451_config( uint16_t param1, uint16_t param2 )
{
	unsigned char res, value;
	param_mma8451_word1=param1;
	param_mma8451_word2=param2;

//standby
	if( IIC_RegWrite( MMA845X_ADDR, CTRL_REG1, ( CTRL_REG1_value & ~ACTIVE_MASK ) ) )
	{
		goto lbl_mma8451_config_err;
	}


/*
   res = IIC_RegWrite( MMA845X_ADDR, CTRL_REG1, CTRL_REG1_value);
   if( res )
   {
   goto lbl_mma8451_config_err;
   }
 */
	if( IIC_RegWrite( MMA845X_ADDR, XYZ_DATA_CFG_REG, FULL_SCALE_4G ) )
	{
		goto lbl_mma8451_config_err;
	}


	/*Step 2: Set the data rate to 50 Hz (for example, but can choose any sample rate).
	   CTRL_REG1_Data = IIC_RegRead(0x2A); //Note: Can combine this step with above
	   CTRL_REG1_Data& = 0xC7; //Clear the sample rate bits
	   CTRL_REG1_Data | = 0x20; //Set the sample rate bits to 50 Hz
	   IC_RegWrite(0x2A, CTRL_REG1_Data); //Write updated value into the register.
	 */


/*
   res = IIC_RegRead( MMA845X_ADDR, CTRL_REG1, &value );
   if( res )
   {
   goto lbl_mma8451_config_err;
   }
   value	&= 0xC7;
   value	|= 0x20;
   res		= IIC_RegWrite( MMA845X_ADDR, CTRL_REG1, value );
   if( res )
   {
   goto lbl_mma8451_config_err;
   }
 */


	/*
	   Step 3: Set the PL_EN bit in Register 0x11 PL_CFG. This will enable the orientation detection.
	   PLCFG_Data = IIC_RegRead (0x11);
	   PLCFG_Data | = 0x40;
	   IIC_RegWrite(0x11, PLCFG_Data);
	 */

	if( IIC_RegWrite( MMA845X_ADDR, PL_CFG_REG, 0xc0 ) )
	{
		goto lbl_mma8451_config_err;
	}


	/*Step 4: Set the Back/Front Angle trip points in register 0x13 following the table in the data sheet.
	   NOTE: This register is readable in all versions of MMA845xQ but it is only modifiable in the MMA8451Q.
	   PL_BF_ZCOMP_Data = IIC_RegRead(0x13);
	   PL_BF_ZCOMP_Data& = 0x3F; //Clear bit 7 and 6
	   Select one of the following to set the B/F angle value:
	   PL_BF_ZCOMP_Data | = 0x00; //This does nothing additional and keeps bits [7:6] = 00
	   PL_BF_ZCOMP_Data | = 0x40; //Sets bits[7:6] = 01
	   PL_BF_ZCOMP_Data | = 0x80; //Sets bits[7:6] = 02
	   PL_BF_ZCOMP_Data | = 0xC0; //Sets bits[7:6] = 03
	   IIC_RegWrite(0x13, PL_BF_ZCOMP_Data); //Write in the updated Back/Front Angle

	   Step 5: Set the Z-Lockout angle trip point in register 0x13 following the table in the data sheet.
	   //NOTE: This register is readable in all versions of MMA845xQ but it is only modifiable in the MMA8451Q.
	   PL_BF_ZCOMP_Data = IIC_RegRead(0x1C); //Read out contents of the register (can be read by all
	   versions of MMA845xQ)
	   The remaining parts of this step only apply to MMA8451Q
	   PL_BF_ZCOMP_Data& = 0xF8; //Clear the last three bits of the register
	   Select one of the following to set the Z-lockout value
	   PL_BF_ZCOMP_Data | = 0x00; //This does nothing additional but the Z-lockout selection will remain at
	   14°
	   PL_BF_ZCOMP_Data | = 0x01; //Set the Z-lockout angle to 18°
	   PL_BF_ZCOMP_Data | = 0x02; //Set the Z-lockout angle to 21°
	   PL_BF_ZCOMP_Data | = 0x03; //Set the Z-lockout angle to 25°
	   PL_BF_ZCOMP_Data | = 0x04; //Set the Z-lockout angle to 29°
	   PL_BF_ZCOMP_Data | = 0x05; //Set the Z-lockout angle to 33°
	   PL_BF_ZCOMP_Data | = 0x06; //Set the Z-lockout angle to 37°
	   PL_BF_ZCOMP_Data | = 0x07; //Set the Z-lockout angle to 42°
	   IIC_RegWrite(0x13, PL_BF_ZCOMP_Data); //Write in the updated Z-lockout angle
	 */

	if( IIC_RegWrite( MMA845X_ADDR, PL_BF_ZCOMP_REG, PL_BF_ZCOMP_REG_value ) )
	{
		goto lbl_mma8451_config_err;
	}


	/*
	   Step 6: Set the Trip Threshold Angle
	   NOTE: This register is readable in all versions of MMA845xQ but it is only modifiable in the MMA8451Q.
	   Select the angle desired in the table, and, Enter in the values given in the table for the corresponding angle.
	   Refer to Figure 7 for the reference frame of the trip angles.
	   P_L_THS_Data = IIC_RegRead(0x14); (can be read by all versions of MMA845xQ)
	   The remaining parts of this step only apply to MMA8451Q
	   P_L_THS_Data& = 0x07; //Clear the Threshold values
	   Choose one of the following options
	   P_L_THS_Data | = (0x07)<<3; //Set Threshold to 15°
	   P_L_THS_Data | = (0x09)<<3; //Set Threshold to 20°
	   P_L_THS_Data | = (0x0C)<<3; //Set Threshold to 30°
	   P_L_THS_Data | = (0x0D)<<3; //Set Threshold to 35°
	   P_L_THS_Data | = (0x0F)<<3; //Set Threshold to 40°
	   P_L_THS_Data | = (0x10)<<3; //Set Threshold to 45°
	   P_L_THS_Data | = (0x13)<<3; //Set Threshold to 55°
	   P_L_THS_Data | = (0x14)<<3; //Set Threshold to 60°
	   P_L_THS_Data | = (0x17)<<3; //Set Threshold to 70°
	   P_L_THS_Data | = (0x19)<<3; //Set Threshold to 75°
	   IIC_RegWrite(0x14, P_L_THS_Data);

	   Step 7: Set the Hysteresis Angle
	   NOTE: This register is readable in all versions of MMA845xQ but it is only modifiable in the MMA8451Q.
	   Select the hysteresis value based on the desired final trip points (threshold + hysteresis)
	   Enter in the values given in the table for that corresponding angle.
	   Note: Care must be taken. Review the final resulting angles. Make sure there isn’t a resulting trip value
	   greater than 90 or less than 0.
	   The following are the options for setting the hysteresis.
	   P_L_THS_Data = IIC_RegRead(0x14);
	   NOTE: The remaining parts of this step only apply to the MMA8451Q.
	   P_L_THS_Data& = 0xF8; //Clear the Hysteresis values
	   P_L_THS_Data | = 0x01; //Set Hysteresis to ±4°
	   P_L_THS_Data | = 0x02; //Set Threshold to ±7°
	   P_L_THS_Data | = 0x03; //Set Threshold to ±11°
	   P_L_THS_Data | = 0x04; //Set Threshold to ±14°
	   P_L_THS_Data | = 0x05; //Set Threshold to ±17°
	   P_L_THS_Data | = 0x06; //Set Threshold to ±21°
	   P_L_THS_Data | = 0x07; //Set Threshold to ±24°
	   IIC_RegWrite(0x14, P_L_THS_Data);
	 */

	if( IIC_RegWrite( MMA845X_ADDR, PL_P_L_THS_REG, PL_P_L_THS_REG_vlaue ) )
	{
		goto lbl_mma8451_config_err;
	}


	/*
	   Step 8: Register 0x2D, Control Register 4 configures all embedded features for interrupt
	   detection.
	   To set this device up to run an interrupt service routine:
	   Program the Orientation Detection bit in Control Register 4.
	   Set bit 4 to enable the orientation detection “INT_EN_LNDPRT”.
	   CTRL_REG4_Data = IIC_RegRead(0x2D); //Read out the contents of the register
	   CTRL_REG4_Data | = 0x10; //Set bit 4
	   IIC_RegWrite(0x2D, CTRL_REG4_Data); //Set the bit and write into CTRL_REG4
	 */

	if( IIC_RegWrite( MMA845X_ADDR, CTRL_REG4, 0x10 ) )
	{
		goto lbl_mma8451_config_err;
	}


	/*
	   Step 9: Register 0x2E is Control Register 5 which gives the option of routing the interrupt to
	   either INT1 or INT2
	   Depending on which interrupt pin is enabled and configured to the processor:
	   Set bit 4 “INT_CFG_LNDPRT” to configure INT1, or,
	   Leave the bit clear to configure INT2.
	   CTRL_REG5_Data = IIC_RegRead(0x2E);
	   In the next two lines choose to clear bit 4 to route to INT2 or set bit 4 to route to INT1
	   CTRL_REG5_Data& = 0xEF; //Clear bit 4 to choose the interrupt to route to INT2
	   CTRL_REG5_Data | = 0x10; //Set bit 4 to choose the interrupt to route to INT1
	   IIC_RegWrite(0x2E, CTRL_REG5_Data); //Write in the interrupt routing selection
	 */
	//value=IIC_RegRead(MMA845X_ADDR,CTRL_REG5);
	////value&=0xEF;
	//value |= 0x10;  // INT1
	//IIC_RegWrite(MMA845X_ADDR,CTRL_REG5,value);


	/*
	   Step 10: Set the debounce counter in register 0x12
	   This value will scale depending on the application-specific required ODR.
	   If the device is set to go to sleep, reset the debounce counter before the device goes to sleep. This setting
	   helps avoid long delays since the debounce will always scale with the current sample rate. The debounce
	   can be set between 50 ms - 100 ms to avoid long delays.
	   IIC_RegWrite(0x12, 0x05); //This sets the debounce counter to 100 ms at 50 Hz
	 */

	res = IIC_RegWrite( MMA845X_ADDR, PL_COUNT_REG, 0x02 );
	if( res )
	{
		goto lbl_mma8451_config_err;
	}


	/*
	   Step 11: Put the device in Active Mode
	   CTRL_REG1_Data = IIC_RegRead(0x2A); //Read out the contents of the register
	   CTRL_REG1_Data | = 0x01; //Change the value in the register to Active Mode.
	   IIC_RegWrite(0x2A, CTRL_REG1_Data); //Write in the updated value to put the device in Active Mode
	 */

//	value=IIC_RegRead(MMA845X_ADDR,CTRL_REG1);
//	value |= 0x01;
//	IIC_RegWrite(MMA845X_ADDR,CTRL_REG1,value);
//active

/*TAP设置*/
	if( IIC_RegWrite( MMA845X_ADDR, PULSE_CFG_REG, 0xd5 ) )
	{
		goto lbl_mma8451_config_err;
	}

	//加速度门限 0.1g 1-79	扩展为0.0625  1-127  扩大1.6倍
/*
	res = ( ( param_mma8451_word2 & 0xff00 ) >> 8 ) * 1.6;

	if( IIC_RegWrite( MMA845X_ADDR, PULSE_THSX_REG, res ) )
	{
		goto lbl_mma8451_config_err;
	}
	if( IIC_RegWrite( MMA845X_ADDR, PULSE_THSY_REG, res ) )
	{
		goto lbl_mma8451_config_err;
	}
	if( IIC_RegWrite( MMA845X_ADDR, PULSE_THSZ_REG, res ) )
	{
		goto lbl_mma8451_config_err;
	}

	res = param_mma8451_word2 & 0xff;
	if( IIC_RegWrite( MMA845X_ADDR, PULSE_TMLT_REG, res ) )
	{
		goto lbl_mma8451_config_err;
	}
*/

	res = ( param_mma8451_word2 & 0xff)*1.6;

	if( IIC_RegWrite( MMA845X_ADDR, PULSE_THSX_REG, res ) )
	{
		goto lbl_mma8451_config_err;
	}
	if( IIC_RegWrite( MMA845X_ADDR, PULSE_THSY_REG, res ) )
	{
		goto lbl_mma8451_config_err;
	}
	if( IIC_RegWrite( MMA845X_ADDR, PULSE_THSZ_REG, res ) )
	{
		goto lbl_mma8451_config_err;
	}

	res = (param_mma8451_word2 & 0xff00)>>8;
	if( IIC_RegWrite( MMA845X_ADDR, PULSE_TMLT_REG, res ) )
	{
		goto lbl_mma8451_config_err;
	}



	if( IIC_RegWrite( MMA845X_ADDR, CTRL_REG1, ( CTRL_REG1_value | ACTIVE_MASK ) ) )
	{
		goto lbl_mma8451_config_err;
	}

	if( IIC_RegWrite( MMA845X_ADDR, CTRL_REG4, 0x18 ) )
	{
		goto lbl_mma8451_config_err;
	}


	return ERR_NONE;

lbl_mma8451_config_err:
	return res;
}

FINSH_FUNCTION_EXPORT(mma8451_config ,setup sensor);




/*
   碰撞时间门限
   碰撞加速度门限

   指定时间门限内，加速度持续大于门限即为碰撞
 */

static uint8_t sensor_info[2] = { 0 };



/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t mma8451_init( rt_device_t dev )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	uint16_t			res;
	uint16_t			param1	= 0x8000, param2 = 0x1e02;
	uint16_t			i		= 0;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );
	/*去掉JTAG功能*/
	GPIO_PinAFConfig( GPIOB, GPIO_Pin_4, 1 );

	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;   /*SCL*/
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_OType	= GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;   /*SDA*/
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	SCL_H;
	SDA_H;
	mma8451_config( param_mma8451_word1, param_mma8451_word2 );
	EXTILine5_Config( );
	return RT_EOK;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static rt_err_t mma8451_open( rt_device_t dev, rt_uint16_t oflag )
{
	return RT_EOK;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static rt_size_t mma8451_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{
	return RT_EOK;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static rt_size_t mma8451_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
{
	return RT_EOK;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static rt_err_t mma8451_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
	uint8_t code = *(uint8_t*)arg;
	int		i;
	switch( cmd )
	{
	}
	return RT_EOK;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static rt_err_t mma8451_close( rt_device_t dev )
{
	return RT_EOK;
}

/*

   传感器处理线程
   原先为函数方式，现在采用线程，使用状态机
 */

#define MMA8451_CONFIG	1
#define MMA8451_TILT	2   /*倾斜检测*/
#define MMA8451_TAP		3   /*震动检测*/

ALIGN( RT_ALIGN_SIZE )
static char thread_sensor_stack[512];
struct rt_thread thread_sensor;

static void rt_thread_entry_sensor( void* parameter )
{
	uint8_t		int_source, pl;
	uint16_t	res;
	uint8_t		status = MMA8451_CONFIG;
	while( 1 )
	{
		if( ( param_mma8451_word1 & 0x8000 ) == 0x0 )
		{
			continue;
		}
		switch( status )
		{
			case MMA8451_CONFIG:
				res = mma8451_config( param_mma8451_word1, param_mma8451_word2 );
				if( res == 0 )
				{
					status = MMA8451_TILT;
				}
				break;
			case MMA8451_TILT:
				res = IIC_RegRead( MMA845X_ADDR, INT_SOURCE_REG, &int_source );
				if( res )
				{
					status = MMA8451_CONFIG;
					break;;
				}

				if( ( int_source & 0x10 ) == 0x10 )
				{
					res = IIC_RegRead( MMA845X_ADDR, PL_STATUS_REG, &pl );
					if( res )
					{
						status = MMA8451_CONFIG;
						break;;
					}

					if( ( sensor_info[0] & 0x40 ) == 0x40 ) //首次使用,不发送
					{
						if( ( rt_tick_get( ) - lastsend_rotation_ticks ) > 1000 )
						{
							rt_kprintf( "\r\n%d>pl=%02x", rt_tick_get( ), pl );
							if( pl != last_plstatus )
							{
								//uart2_send_frame( '2', 2, sensor_info, 1 );
								last_plstatus = pl;
							}
							lastsend_rotation_ticks = rt_tick_get( );
						}
					}else
					{
						rt_kprintf( "\r\n%d>First pl=%02x", rt_tick_get( ), pl );
					}
					sensor_info[0] |= 0x40;
				}
				break;

			case MMA8451_TAP:
				res = IIC_RegRead( MMA845X_ADDR, PULSE_SRC_REG, &int_source );
				if( res )
				{
					status = MMA8451_CONFIG;
					break;;
				}

				if( int_source >= 0x80 )
				{
					if( ( rt_tick_get( ) - lastsend_crash_ticks ) > 1000 )
					{
						sensor_info[0] |= 0x80;
						//uart2_send_frame( '2', 2, sensor_info, 1 );
						lastsend_crash_ticks = rt_tick_get( );
					}
					rt_kprintf( "\r\n%d>PULSE_SRC_REG=%02x", rt_tick_get( ), int_source );
				}
				break;
		}
		rt_thread_delay( RT_TICK_PER_SECOND / 50 );
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void mma8451_driver_init( void )
{
/*
   rt_thread_init( &thread_sensor,
                 "sensor",
                 rt_thread_entry_sensor,
                 RT_NULL,
                 &thread_sensor_stack[0],
                 sizeof( thread_sensor_stack ), 22, 5 );
   rt_thread_startup( &thread_sensor );
 */
	dev_mma8451.type		= RT_Device_Class_Char;
	dev_mma8451.init		= mma8451_init;
	dev_mma8451.open		= mma8451_open;
	dev_mma8451.close		= mma8451_close;
	dev_mma8451.read		= mma8451_read;
	dev_mma8451.write		= mma8451_write;
	dev_mma8451.control		= mma8451_control;
	dev_mma8451.user_data	= RT_NULL;

	rt_device_register( &dev_mma8451, "sensor", RT_DEVICE_FLAG_RDWR );
	rt_device_init( &dev_mma8451 );
}







/************************************** The End Of File **************************************/
