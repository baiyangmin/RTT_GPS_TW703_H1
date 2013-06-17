/**
 ******************************************************************************
 * @file    usbh_usr.c
 * @author  MCD Application Team
 * @version V2.1.0
 * @date    19-March-2012
 * @brief   This file includes the usb host library user callbacks
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "usbh_usr.h"
#include "ff.h"       /* FATFS */
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"


/** @addtogroup USBH_USER
 * @{
 */


/** @addtogroup USBH_MSC_DEMO_USER_CALLBACKS
 * @{
 */


/** @defgroup USBH_USR
 * @brief    This file includes the usb host stack user callbacks
 * @{
 */


/** @defgroup USBH_USR_Private_TypesDefinitions
 * @{
 */


/**
 * @}
 */


/** @defgroup USBH_USR_Private_Defines
 * @{
 */


/**
 * @}
 */


/** @defgroup USBH_USR_Private_Macros
 * @{
 */
extern USB_OTG_CORE_HANDLE	USB_OTG_Core;
extern USBH_HOST			USB_Host;


/**
 * @}
 */


/** @defgroup USBH_USR_Private_Variables
 * @{
 */
//uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;

/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

USBH_Usr_cb_TypeDef USR_cb =
{
	USBH_USR_Init,
	USBH_USR_DeInit,
	USBH_USR_DeviceAttached,
	USBH_USR_ResetDevice,
	USBH_USR_DeviceDisconnected,
	USBH_USR_OverCurrentDetected,
	USBH_USR_DeviceSpeedDetected,
	USBH_USR_Device_DescAvailable,
	USBH_USR_DeviceAddressAssigned,
	USBH_USR_Configuration_DescAvailable,
	USBH_USR_Manufacturer_String,
	USBH_USR_Product_String,
	USBH_USR_SerialNum_String,
	USBH_USR_EnumerationDone,
	USBH_USR_UserInput,
	USBH_USR_MSC_Application,
	USBH_USR_DeviceNotSupported,
	USBH_USR_UnrecoveredError
};


/**
 * @}
 */


/** @defgroup USBH_USR_Private_Constants
 * @{
 */
/*--------------- LCD Messages ---------------*/
const uint8_t	MSG_HOST_INIT[]			= "> Host Library Initialized\n";
const uint8_t	MSG_DEV_ATTACHED[]		= "> Device Attached \n";
const uint8_t	MSG_DEV_DISCONNECTED[]	= "> Device Disconnected\n";
const uint8_t	MSG_DEV_ENUMERATED[]	= "> Enumeration completed \n";
const uint8_t	MSG_DEV_HIGHSPEED[]		= "> High speed device detected\n";
const uint8_t	MSG_DEV_FULLSPEED[]		= "> Full speed device detected\n";
const uint8_t	MSG_DEV_LOWSPEED[]		= "> Low speed device detected\n";
const uint8_t	MSG_DEV_ERROR[]			= "> Device fault \n";

const uint8_t	MSG_MSC_CLASS[]		= "> Mass storage device connected\n";
const uint8_t	MSG_HID_CLASS[]		= "> HID device connected\n";
const uint8_t	MSG_DISK_SIZE[]		= "> Size of the disk in MBytes: \n";
const uint8_t	MSG_LUN[]			= "> LUN Available in the device:\n";
const uint8_t	MSG_ROOT_CONT[]		= "> Exploring disk flash ...\n";
const uint8_t	MSG_WR_PROTECT[]	= "> The disk is write protected\n";
const uint8_t	MSG_UNREC_ERROR[]	= "> UNRECOVERED ERROR STATE\n";


/**
 * @}
 */


/** @defgroup USBH_USR_Private_Functions
 * @{
 */


/**
 * @brief  USBH_USR_Init
 *         Displays the message on LCD for host lib initialization
 * @param  None
 * @retval None
 */
void USBH_USR_Init( void )
{
	static uint8_t startup = 0;

	if( startup == 0 )
	{
		startup = 1;
		rt_kprintf( "> USB Host library started.\n" );
	}
}

/**
 * @brief  USBH_USR_DeviceAttached
 *         Displays the message on LCD on device attached
 * @param  None
 * @retval None
 */
void USBH_USR_DeviceAttached( void )
{
	rt_kprintf( "%s\n", MSG_DEV_ATTACHED );

}

/**
 * @brief  USBH_USR_UnrecoveredError
 * @param  None
 * @retval None
 */
void USBH_USR_UnrecoveredError( void )
{
	/* Set default screen color*/
	rt_kprintf( "%s\n", MSG_UNREC_ERROR );
}

/**
 * @brief  USBH_DisconnectEvent
 *         Device disconnect event
 * @param  None
 * @retval Staus
 */
void USBH_USR_DeviceDisconnected( void )
{
	/* Set default screen color*/
	rt_kprintf( "%s\n", MSG_DEV_DISCONNECTED );
}

/**
 * @brief  USBH_USR_ResetUSBDevice
 * @param  None
 * @retval None
 */
void USBH_USR_ResetDevice( void )
{
	/* callback for USB-Reset */
	rt_kprintf( "%s\n", __func__ );
}

/**
 * @brief  USBH_USR_DeviceSpeedDetected
 *         Displays the message on LCD for device speed
 * @param  Device speed
 * @retval None
 */
void USBH_USR_DeviceSpeedDetected( uint8_t DeviceSpeed )
{
	if( DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED )
	{
		rt_kprintf( "%s\n", MSG_DEV_HIGHSPEED );
	}else if( DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED )
	{
		rt_kprintf( "%s\n", MSG_DEV_FULLSPEED );
	}else if( DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED )
	{
		rt_kprintf( "%s\n", MSG_DEV_LOWSPEED );
	}else
	{
		rt_kprintf( "%s\n", MSG_DEV_ERROR );
	}
}

/**
 * @brief  USBH_USR_Device_DescAvailable
 *         Displays the message on LCD for device descriptor
 * @param  device descriptor
 * @retval None
 */
void USBH_USR_Device_DescAvailable( void *DeviceDesc )
{
	USBH_DevDesc_TypeDef *hs;
	hs = DeviceDesc;

	rt_kprintf( "VID : %04Xh\n", (uint32_t)( *hs ).idVendor );
	rt_kprintf( "PID : %04Xh\n", (uint32_t)( *hs ).idProduct );
}

/**
 * @brief  USBH_USR_DeviceAddressAssigned
 *         USB device is successfully assigned the Address
 * @param  None
 * @retval None
 */
void USBH_USR_DeviceAddressAssigned( void )
{
	rt_kprintf( "%s\n", __func__ );
}

/**
 * @brief  USBH_USR_Conf_Desc
 *         Displays the message on LCD for configuration descriptor
 * @param  Configuration descriptor
 * @retval None
 */
void USBH_USR_Configuration_DescAvailable( USBH_CfgDesc_TypeDef * cfgDesc,
                                           USBH_InterfaceDesc_TypeDef *itfDesc,
                                           USBH_EpDesc_TypeDef *epDesc )
{
	USBH_InterfaceDesc_TypeDef *id;

	id = itfDesc;

	if( ( *id ).bInterfaceClass == 0x08 )
	{
		rt_kprintf( "%s\n", MSG_MSC_CLASS );
	}else if( ( *id ).bInterfaceClass == 0x03 )
	{
		rt_kprintf( "%s\n", MSG_HID_CLASS );
	}
}

/**
 * @brief  USBH_USR_Manufacturer_String
 *         Displays the message on LCD for Manufacturer String
 * @param  Manufacturer String
 * @retval None
 */
void USBH_USR_Manufacturer_String( void *ManufacturerString )
{
	rt_kprintf( "Manufacturer : %s\n", (char*)ManufacturerString );
}

/**
 * @brief  USBH_USR_Product_String
 *         Displays the message on LCD for Product String
 * @param  Product String
 * @retval None
 */
void USBH_USR_Product_String( void *ProductString )
{
	rt_kprintf( "Product : %s\n", (char*)ProductString );
}

/**
 * @brief  USBH_USR_SerialNum_String
 *         Displays the message on LCD for SerialNum_String
 * @param  SerialNum_String
 * @retval None
 */
void USBH_USR_SerialNum_String( void *SerialNumString )
{
	rt_kprintf( "Serial Number : %s\n", (char*)SerialNumString );
}

/**
 * @brief  EnumerationDone
 *         User response request is displayed to ask application jump to class
 * @param  None
 * @retval None
 */
void USBH_USR_EnumerationDone( void )
{
	/* Enumeration complete */
	rt_kprintf( "%s\n", MSG_DEV_ENUMERATED );
}

/**
 * @brief  USBH_USR_DeviceNotSupported
 *         Device is not supported
 * @param  None
 * @retval None
 */
void USBH_USR_DeviceNotSupported( void )
{
	rt_kprintf( "> Device not supported." );
}

/**
 * @brief  USBH_USR_UserInput
 *         User Action for application state entry
 * @param  None
 * @retval USBH_USR_Status : User response for key button
 */
USBH_USR_Status USBH_USR_UserInput( void )
{
	USBH_USR_Status usbh_usr_status;

	usbh_usr_status = USBH_USR_RESP_OK; //USBH_USR_NO_RESP;

	/*Key B3 is in polling mode to detect user action */
#if 0
	if( STM_EVAL_PBGetState( Button_KEY ) == RESET )
	{
		usbh_usr_status = USBH_USR_RESP_OK;
	}
#endif
	return usbh_usr_status;
}

/**
 * @brief  USBH_USR_OverCurrentDetected
 *         Over Current Detected on VBUS
 * @param  None
 * @retval Staus
 */
void USBH_USR_OverCurrentDetected( void )
{
	rt_kprintf( "Overcurrent detected." );
}

static uint8_t			diskinited = 0;
static struct rt_device mscdev;


/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static rt_err_t msc_init( rt_device_t dev )
{
	rt_kprintf( "%s\r\n", __func__ );
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
static rt_size_t msc_read( rt_device_t dev, rt_off_t sector, void* buff, rt_size_t count )
{
	__IO uint8_t status = USBH_MSC_OK;
	if( HCD_IsDeviceConnected( &USB_OTG_Core ) )
	{
		do
		{
			status = USBH_MSC_Read10( &USB_OTG_Core, buff, sector, 512 * count );
			USBH_MSC_HandleBOTXfer( &USB_OTG_Core, &USB_Host );

			if( !HCD_IsDeviceConnected( &USB_OTG_Core ) )
			{
				//return RES_ERROR;
				rt_kprintf( "%s error\r\n", __func__ );
				return USBH_MSC_FAIL;
			}
		}
		while( status == USBH_MSC_BUSY );
	}
	if( status == USBH_MSC_OK )
	{
		return count;
	}
	return 0xff;
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
static rt_size_t msc_write( rt_device_t dev, rt_off_t sector, const void* buff, rt_size_t count )
{
	BYTE status = USBH_MSC_OK;
	rt_kprintf( "%s\r\n", __func__ );
	if( HCD_IsDeviceConnected( &USB_OTG_Core ) )
	{
		do
		{
			status = USBH_MSC_Write10( &USB_OTG_Core, (BYTE*)buff, sector, 512 * count );
			USBH_MSC_HandleBOTXfer( &USB_OTG_Core, &USB_Host );

			if( !HCD_IsDeviceConnected( &USB_OTG_Core ) )
			{
				//return RES_ERROR;
				rt_kprintf( "%s error\r\n", __func__ );
				return USBH_MSC_FAIL;
			}
		}

		while( status == USBH_MSC_BUSY );
	}
	if( status == USBH_MSC_OK )
	{
		return count;
	}
	return 0xff;
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
rt_err_t msc_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
	struct rt_device_blk_geometry *pgeometry = (struct rt_device_blk_geometry*)arg;
	switch( cmd )
	{
		case RT_DEVICE_CTRL_BLK_GETGEOME:
			pgeometry->sector_count		= USBH_MSC_Param.MSCapacity;
			pgeometry->bytes_per_sector = USBH_MSC_Param.MSPageLength;
			pgeometry->block_size		= 0;
			break;
		case RT_DEVICE_CTRL_BLK_SYNC:

			break;
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
static rt_err_t msc_close( rt_device_t dev )
{
	rt_kprintf( "%s\r\n", __func__ );

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
static rt_err_t msc_open( rt_device_t dev, rt_uint16_t oflag )
{
	rt_kprintf( "%s\r\n", __func__ );

	return RT_EOK;
}

/**
 * @brief  USBH_USR_MSC_Application
 *         Demo application for mass storage
 * @param  None
 * @retval Staus
 */
int USBH_USR_MSC_Application( void )
{
	FRESULT res;
	int		ret;

	if( diskinited )
	{
		return 0;
	}

	mscdev.type			= RT_Device_Class_Block;
	mscdev.init			= msc_init;
	mscdev.open			= msc_open;
	mscdev.close		= msc_close;
	mscdev.read			= msc_read;
	mscdev.write		= msc_write;
	mscdev.control		= msc_control;
	mscdev.user_data	= RT_NULL;
	diskinited			= 1;

	rt_device_register( &mscdev, "udisk", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE );
	rt_device_init( &mscdev );
	rt_thread_delay( RT_TICK_PER_SECOND / 10 );
	ret = dfs_mount( "udisk", "/udisk", "elm", 0, 0 );
	
	rt_kprintf( "dfs_mount ret=%x  now udisk start\r\n", ret );

	return 0;
}

/**
 * @brief  USBH_USR_DeInit
 *         Deint User state and associated variables
 * @param  None
 * @retval None
 */
void USBH_USR_DeInit( void )
{
	rt_kprintf( "%s\r\n", __func__ );
	//USBH_USR_ApplicationState = USH_USR_FS_INIT;
	dfs_unmount( "/udisk" );
	rt_device_unregister( &mscdev );
	diskinited = 0;
}

/**
 * @}
 */


/**
 * @}
 */


/**
 * @}
 */


/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

