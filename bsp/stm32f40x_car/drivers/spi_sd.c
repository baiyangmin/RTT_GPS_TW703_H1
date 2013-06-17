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
#include <rtthread.h>
#include "spi_sd.h"
#include "stm32f4xx.h"

#include <dfs_fs.h>
#include <dfs_elm.h>
#include <finsh.h>

enum _CS_HOLD
{
	HOLD	= 0,
	RELEASE = 1,
};

#define false	0
#define true	1

/* 512 bytes for each sector */
#define SD_SECTOR_SIZE 512

/* token for write operation */
#define TOKEN_SINGLE_BLOCK	0xFE
#define TOKEN_MULTI_BLOCK	0xFC
#define TOKEN_STOP_TRAN		0xFD

/* Local variables */
static rt_uint8_t			CardType;
static SDCFG				SDCfg;
static struct rt_device		dev_spi_sd;
static struct dfs_partition part;


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void _spi1_baud_rate( uint16_t SPI_BaudRatePrescaler )
{
	SPI1->CR1	&= ~SPI_BaudRatePrescaler_256;
	SPI1->CR1	|= SPI_BaudRatePrescaler;
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

void _spi1_init( )
{
	SPI_InitTypeDef		SPI_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE );

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE );

	GPIO_PinAFConfig( GPIOA, GPIO_PinSource5, GPIO_AF_SPI1 );
	GPIO_PinAFConfig( GPIOA, GPIO_PinSource6, GPIO_AF_SPI1 );
	GPIO_PinAFConfig( GPIOA, GPIO_PinSource7, GPIO_AF_SPI1 );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_DOWN;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	//_card_power_off();
	_card_disable( );

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode		= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize	= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL		= SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA		= SPI_CPHA_1Edge;
//	SPI_InitStructure.SPI_CPOL				= SPI_CPOL_High;
//	SPI_InitStructure.SPI_CPHA				= SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS				= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;

	SPI_InitStructure.SPI_FirstBit		= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init( SPI1, &SPI_InitStructure );

	SPI_Cmd( SPI1, ENABLE );
}



/*
   Send and Recv a byte
 */

__inline rt_uint8_t _spi_send_recv( rt_uint8_t data )
{
	rt_uint8_t dat;
#if 1
	while( ( SPI1->SR & SPI_I2S_FLAG_TXE ) == RESET )
	{
		;
	}
	SPI1->DR = data;
	while( ( SPI1->SR & SPI_I2S_FLAG_RXNE ) == RESET )
	{
		;
	}
	dat = SPI1->DR;
	return dat;
#endif
#if 0
	while( SPI_I2S_GetFlagStatus( SPI1, SPI_I2S_FLAG_TXE ) == RESET )
	{
		;
	}
	SPI_I2S_SendData( SPI1, data );
	while( SPI_I2S_GetFlagStatus( SPI1, SPI_I2S_FLAG_RXNE ) == RESET )
	{
		;
	}
	dat = SPI_I2S_ReceiveData( SPI1 );
	return dat;
#endif

#if 0
	SPI1->DR = data;
	while( SPI1->SR & ( 1 << SPI_I2S_FLAG_BSY ) )
	{
		;
	}
	return SPI1->DR;
#endif
}

/* wait until the card is not busy */
static rt_uint8_t _spi_sd_wait4ready( void )
{
	rt_uint8_t	res;
	rt_uint32_t timeout = 400000;
	do
	{
		res = _spi_send_recv( DUMMY_BYTE );
	}
	while( ( res != DUMMY_BYTE ) && timeout-- );

	return ( res == 0xFF ? true : false );
}

/*****************************************************************************
   Send a Command to Flash card and get a Response
   cmd:  cmd index
   arg: argument for the cmd
   return the received response of the commond
*****************************************************************************/
static rt_uint8_t _spi_sd_sendcmd( rt_uint8_t cmd, rt_uint32_t arg, rt_uint8_t crc )
{
	rt_uint32_t n;
	rt_uint8_t r1;
#if 1
	if( cmd & 0x80 )                                    /* ACMD<n> is the command sequence of CMD55-CMD<n> */
	{
		cmd &= 0x7F;
		r1	= _spi_sd_sendcmd( APP_CMD, 0, DUMMY_CRC ); /* CMD55 */
		if( r1 > 1 )
		{
			return r1;                                  /* cmd send failed */
		}
	}
#endif
	/* Select the card and wait for ready */
//	LPC17xx_SPI_DeSelect();
//	LPC17xx_SPI_Select();
//	if (_spi_sd_wait4ready() == false ) return 0xFF;

	_spi_send_recv( DUMMY_BYTE );
	_card_enable( );
	_spi_send_recv( cmd );
	_spi_send_recv( arg >> 24 );
	_spi_send_recv( arg >> 16 );
	_spi_send_recv( arg >> 8 );
	_spi_send_recv( arg );
	_spi_send_recv( crc );

#if 0
	n = 10; /* Wait for a valid response in timeout of 10 attempts */
	do
	{
		r1 = _spi_send_recv( DUMMY_BYTE );
	}
	while( ( r1 & 0x80 ) && --n );
#endif

#if 1
	for( n = 0; n < 200; n++ )
	{
		r1 = _spi_send_recv( DUMMY_BYTE );
		if( r1 != 0xFF )
		{
			break;
		}
	}
#endif	
	_card_disable( );
	_spi_send_recv( DUMMY_BYTE );
	return r1; /* Return with the response value */
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
static rt_uint8_t _spi_sd_sendcmd_hold( rt_uint8_t cmd, rt_uint32_t arg, rt_uint8_t crc )
{
	rt_uint32_t r1, n;
#if 1
	if( cmd & 0x80 )                                    /* ACMD<n> is the command sequence of CMD55-CMD<n> */
	{
		cmd &= 0x7F;
		r1	= _spi_sd_sendcmd( APP_CMD, 0, DUMMY_CRC ); /* CMD55 */
		if( r1 > 1 )
		{
			return r1;                                  /* cmd send failed */
		}
	}
#endif
	_spi_send_recv( DUMMY_BYTE );
	_card_enable( );
	_spi_send_recv( cmd );
	_spi_send_recv( arg >> 24 );
	_spi_send_recv( arg >> 16 );
	_spi_send_recv( arg >> 8 );
	_spi_send_recv( arg );
	_spi_send_recv( crc );

	for( n = 0; n < 200; n++ )
	{
		r1 = _spi_send_recv( DUMMY_BYTE );
		if( r1 != 0xFF )
		{
			break;
		}
	}
	return ( r1 ); /* Return with the response value */
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
static rt_uint8_t _spi_sd_readdatablock( rt_uint8_t *buff, rt_uint32_t cnt, rt_uint8_t release )
{
	rt_uint8_t	r1;
	rt_uint16_t retry;
	_card_enable( );
	for( retry = 0; retry < 2000; retry++ )
	{
		r1 = _spi_send_recv( DUMMY_BYTE );
		if( r1 == 0xFE )
		{
			retry = 0;
			break;
		}
	}
	if( retry == 2000 )
	{
		_card_disable( );
		return false;
	}
	for( retry = 0; retry < cnt; retry++ )
	{
		*( buff + retry ) = _spi_send_recv( DUMMY_BYTE );
	}

	// 2bytes dummy CRC
	_spi_send_recv( DUMMY_BYTE );
	_spi_send_recv( DUMMY_BYTE );

	// chip disable and dummy byte
	if( release )
	{
		_card_disable( );
		_spi_send_recv( DUMMY_BYTE );
	}
	return true;
}

/*****************************************************************************
   Write 512 bytes
   buffer: 512 byte data block to be transmitted
   token:  0xFE -> single block
   0xFC -> multi block
   0xFD -> Stop
*****************************************************************************/
static rt_uint8_t _spi_sd_writedatablock( const rt_uint8_t *buff, rt_uint8_t token )
{
	rt_uint8_t resp, i;

	i = i;                      // avoid warning
	_card_enable( );
	_spi_send_recv( DUMMY_BYTE );
	_spi_send_recv( DUMMY_BYTE );
	_spi_send_recv( DUMMY_BYTE );
	_spi_send_recv( token );    /* send data token first*/

	if( token != TOKEN_STOP_TRAN )
	{
		for( i = 512 / 4; i; i-- )
		{
			_spi_send_recv( *buff++ );
			_spi_send_recv( *buff++ );
			_spi_send_recv( *buff++ );
			_spi_send_recv( *buff++ );
		}
		_spi_send_recv( DUMMY_CRC );            /* 16-bit CRC (Dummy) */
		_spi_send_recv( DUMMY_CRC );

		resp = _spi_send_recv( DUMMY_BYTE );    /* Receive data response */
		if( ( resp & 0x1F ) != 0x05 )           /* If not accepted, return with error */
		{
			return false;
		}
		if( _spi_sd_wait4ready( ) == false )    /* Wait while Flash Card is busy. */
		{
			return false;
		}
	}

	return true;
}

/*****************************************************************************
   Read "count" Sector(s) starting from sector index "sector",
   buff <- [sector, sector+1, ... sector+count-1]
   if success, return true, otherwise return false
*****************************************************************************/
static rt_uint8_t _spi_sd_readsector( rt_uint32_t sector, rt_uint8_t * buff, rt_uint32_t count )
{
	/* Convert to byte address if needed */
	if( !( CardType & CT_BLOCK ) )
	{
		sector *= SD_SECTOR_SIZE;
	}

	if( count == 1 ) /* Single block read */
	{
		if( _spi_sd_sendcmd_hold( READ_BLOCK, sector, DUMMY_CRC ) != 0x0 )
		{
			return false;
		}
		if( !_spi_sd_readdatablock( buff, SD_SECTOR_SIZE, RELEASE ) )
		{
			return false;
		}
		//if (_spi_sd_sendcmd(STOP_TRAN, 0,DUMMY_CRC) != 0x0) return false;
		return true;
	} else   /* Multiple block read */
	{
		if( !_spi_sd_sendcmd( READ_MULT_BLOCK, sector, DUMMY_CRC ) )
		{
			return false;
		}
		do
		{
			if( !_spi_sd_readdatablock( buff, SD_SECTOR_SIZE, HOLD ) )
			{
				return false;
			}
			buff += SD_SECTOR_SIZE;
		}
		while( --count );
		_spi_sd_sendcmd( STOP_TRAN, 0, DUMMY_CRC ); /* STOP_TRANSMISSION */
	}
	//LPC17xx_SPI_Release();
	//SPI_SSOutputCmd(SPI1,DISABLE);
	_card_disable( );
	_spi_send_recv( DUMMY_BYTE );
	return true;
}

/*****************************************************************************
   Write "count" Sector(s) starting from sector index "sector",
   buff -> [sector, sector+1, ... sector+count-1]
   if success, return true, otherwise return false
*****************************************************************************/
static rt_uint8_t _spi_sd_writesector( rt_uint32_t sector, const rt_uint8_t *buff, rt_uint32_t count )
{
	if( !( CardType & CT_BLOCK ) )
	{
		sector *= 512;  /* Convert to byte address if needed */
	}
	if( count == 1 )    /* Single block write */
	{
		if( ( _spi_sd_sendcmd( WRITE_BLOCK, sector, DUMMY_CRC ) == 0 )
		    && _spi_sd_writedatablock( buff, TOKEN_SINGLE_BLOCK ) )
		{
			count = 0;
		}
	} else              /* Multiple block write */
	{
		if( CardType & CT_SDC )
		{
			_spi_sd_sendcmd( SET_WR_BLK_ERASE_COUNT, count, DUMMY_CRC );
		}
		if( _spi_sd_sendcmd( WRITE_MULT_BLOCK, sector, DUMMY_CRC ) == 0 )
		{
			do
			{
				if( !_spi_sd_writedatablock( buff, TOKEN_MULTI_BLOCK ) )
				{
					break;
				}
				buff += 512;
			}
			while( --count );
#if 1
			if( !_spi_sd_writedatablock( 0, TOKEN_STOP_TRAN ) ) /* STOP_TRAN token */
			{
				count = 1;
			}
#else
			_spi_send_recv( TOKEN_STOP_TRAN );
#endif
		}
	}
	//LPC17xx_SPI_Release();
	//SPI_SSOutputCmd(SPI1,DISABLE);
	_card_disable( );
	_spi_send_recv( DUMMY_BYTE );
	return count ? false : true;
}

/* Read MMC/SD Card device configuration. */
static rt_uint8_t _spi_sd_readcfg( SDCFG *cfg )
{
	rt_uint8_t	i;
	rt_uint16_t csize;
	rt_uint8_t	n, csd[16];
	rt_uint8_t	retv = false;

	/* Read the OCR - Operations Condition Register. */
	if( _spi_sd_sendcmd_hold( READ_OCR, 0, DUMMY_CRC ) != 0x00 )
	{
		goto x;
	}
	for( i = 0; i < 4; i++ )
	{
		cfg->ocr[i] = _spi_send_recv( DUMMY_BYTE );
	}
	_card_disable( );
	_spi_send_recv( DUMMY_BYTE );

	/* Read the CID - Card Identification. */
	if( _spi_sd_sendcmd_hold( SEND_CID, 0, DUMMY_CRC ) != 0x00 )
	{
		goto x;
	}
	if( _spi_sd_readdatablock( cfg->cid, 16, RELEASE ) == false )
	{
		goto x;
	}

	rt_kprintf("\r\nCID=");
	for (i=0;i<16;i++) rt_kprintf("%02x ",cfg->cid[i]);

	/* Read the CSD - Card Specific Data. */
	if( ( _spi_sd_sendcmd_hold( SEND_CSD, 0, DUMMY_CRC ) != 0x00 ) ||
	    ( _spi_sd_readdatablock( cfg->csd, 16, RELEASE ) == false ) )
	{
		goto x;
	}

	rt_kprintf("\r\nCSD=");
	for (i=0;i<16;i++) rt_kprintf("%02x ",cfg->csd[i]);

	cfg->sectorsize = SD_SECTOR_SIZE;

	/* Get number of sectors on the disk (DWORD) */
	if( ( cfg->csd[0] >> 6 ) == 1 )                                                                                                     /* SDC ver 2.00 */
	{
		csize			= cfg->csd[9] + ( (rt_uint16_t)cfg->csd[8] << 8 ) + 1;
		cfg->sectorcnt	= (rt_uint32_t)csize << 10;
	} else                                                                                                                              /* SDC ver 1.XX or MMC*/
	{
		n				= ( cfg->csd[5] & 15 ) + ( ( cfg->csd[10] & 128 ) >> 7 ) + ( ( cfg->csd[9] & 3 ) << 1 ) + 2;                    // 19
		csize			= ( cfg->csd[8] >> 6 ) + ( (rt_uint16_t)cfg->csd[7] << 2 ) + ( (rt_uint16_t)( cfg->csd[6] & 3 ) << 10 ) + 1;    // 3752
		cfg->sectorcnt	= (rt_uint32_t)csize << ( n - 9 );                                                                              // 3842048
	}

	cfg->size = cfg->sectorcnt * cfg->sectorsize;                                                                                       // 512*3842048=1967128576Byte (1.83GB)
	rt_kprintf("\r\nsectorcnt=%d  sectorsize=%d\r\n",cfg->sectorcnt, cfg->sectorsize);

	/* Get erase block size in unit of sector (DWORD) */
	if( CardType & CT_SD2 )                                                                                                             /* SDC ver 2.00 */
	{
		if( _spi_sd_sendcmd( SD_STATUS, 0, DUMMY_CRC ) == 0 )                                                                           /* Read SD status */
		{
			_spi_send_recv( DUMMY_BYTE );
			if( !_spi_sd_readdatablock( csd, 16, RELEASE ) )                                                                            /* Read partial block */
			{
				for( n = 64 - 16; n; n-- )
				{
					_spi_send_recv( DUMMY_BYTE );                                                                                       /* Purge trailing data */
				}
				cfg->blocksize	= 16UL << ( csd[10] >> 4 );
				retv			= true;
			}
		}
	} else                                                                                                                              /* SDC ver 1.XX or MMC */
	{
		if( ( _spi_sd_sendcmd( SEND_CSD, 0, DUMMY_CRC ) == 0 ) && ( _spi_sd_readdatablock( csd, 16, RELEASE ) ) )                       /* Read CSD */
		{
			if( CardType & CT_SD1 )                                                                                                     /* SDC ver 1.XX */
			{
				cfg->blocksize = ( ( ( csd[10] & 63 ) << 1 ) + ( (rt_uint16_t)( csd[11] & 128 ) >> 7 ) + 1 ) << ( ( csd[13] >> 6 ) - 1 );
			} else /* MMC */
			{ // cfg->blocksize = ((uint16_t)((buf[10] & 124) >> 2) + 1) * (((buf[11] & 3) << 3) + ((buf[11] & 224) >> 5) + 1);
				cfg->blocksize = ( (rt_uint16_t)( ( cfg->csd[10] & 124 ) >> 2 ) + 1 ) * ( ( ( cfg->csd[10] & 3 ) << 3 ) + ( ( cfg->csd[11] & 224 ) >> 5 ) + 1 );
			}
			retv = true;
		}
	}
	rt_kprintf("blocksize=%d\r\n",cfg->blocksize);
x:  //LPC17xx_SPI_Release();
	SPI_SSOutputCmd(SPI1,DISABLE);

	return ( retv );
}


/* Initialize SD/MMC card. */
static rt_uint8_t _spi_sd_init( void )
{
	rt_uint32_t i, timeout;
	rt_uint8_t	cmd, ct, ocr[4];
	rt_uint8_t	ret = false, r1;

	_spi1_init( );

	//_card_power_on();
	_card_disable();

	for( i = 0; i < 74; i++ )  /* 80 dummy clocks */
	{
		r1 = _spi_send_recv( DUMMY_BYTE );
	}

	ct = CT_NONE;
/*有的卡需要发送多次的CMD0，才能工作*/
	for( i = 1; i < 0xfff; i++ )
	{
		r1 = _spi_sd_sendcmd( GO_IDLE_STATE, 0, 0x95 );
		if( r1 == 0x01 )
		{
			i = 0;
			break;
		}
	}
	if( i == 0xff )
	{
		rt_kprintf( "CMD0 Timeout Error\r\n" );
		return ret;
	}

	timeout = 50000;
	r1		= _spi_sd_sendcmd_hold( CMD8, 0x1AA, 0x87 );
	rt_kprintf("r1=%d\n",r1);
	if( r1 == 0x01 )
	{
		for( i = 0; i < 4; i++ )
		{
			ocr[i] = _spi_send_recv( DUMMY_BYTE );
		}
		rt_kprintf( "CMD8=%02x %02x %02x %02x\r\n", ocr[0], ocr[1], ocr[2], ocr[3] );
		_card_disable( );
		_spi_send_recv( DUMMY_BYTE );
		if( ocr[2] == 0x01 && ocr[3] == 0xaa )
		{
			while( timeout-- && _spi_sd_sendcmd( SD_SEND_OP_COND, 1UL << 30, DUMMY_CRC ) )
			{
				;
			}
			if( timeout && _spi_sd_sendcmd_hold( READ_OCR, 0, DUMMY_CRC ) == 0 )
			{
				for( i = 0; i < 4; i++ )
				{
					ocr[i] = _spi_send_recv( DUMMY_BYTE );
				}
				rt_kprintf( "OCR=%02x %02x %02x %02x\r\n", ocr[0], ocr[1], ocr[2], ocr[3] );
				ct = ( ocr[0] & 0x40 ) ? CT_SD2 | CT_BLOCK : CT_SD2;
			}
		}else
		{
			r1 = _spi_sd_sendcmd( SD_SEND_OP_COND, 0, DUMMY_CRC );
			rt_kprintf( "ACMD r1=%02x\r\n", r1 );
			if( r1 <= 1 )
			{
				ct	= CT_SD1;
				cmd = SD_SEND_OP_COND;  /* SDSC */
			} else
			{
				ct	= CT_MMC;
				cmd = SEND_OP_COND;     /* MMC */
			}
			/* Wait for leaving idle state */
			while( timeout-- && _spi_sd_sendcmd( cmd, 0, DUMMY_CRC ) )
			{
				;
			}
			if( !timeout || _spi_sd_sendcmd( SET_BLOCKLEN, SD_SECTOR_SIZE, DUMMY_CRC ) != 0 )
			{
				ct = CT_NONE;
			}
		}
	}
	else
	{
		if( _spi_sd_sendcmd( SD_SEND_OP_COND, 0, DUMMY_CRC ) <= 1 )
		{
			ct	= CT_SD1;
			cmd = SD_SEND_OP_COND;  /* SDSC */
		} else
		{
			ct	= CT_MMC;
			cmd = SEND_OP_COND;     /* MMC */
		}
		/* Wait for leaving idle state */
		while( timeout-- && _spi_sd_sendcmd( cmd, 0, DUMMY_CRC ) )
		{
			;
		}
		/* Set R/W block length to 512 */
		if( !timeout || _spi_sd_sendcmd( SET_BLOCKLEN, SD_SECTOR_SIZE, DUMMY_CRC ) != 0 )
		{
			ct = CT_NONE;
		}
	}

#if 0
	if( _spi_sd_sendcmd( GO_IDLE_STATE, 0, 0x95 ) == 0x1 )
	{
		timeout = 50000;
		if( _spi_sd_sendcmd( CMD8, 0x1AA, 0x87 ) == 1 ) /* SDHC */
		{ /* Get trailing return value of R7 resp */
			for( i = 0; i < 4; i++ )
			{
				ocr[i] = _spi_send_recv( DUMMY_BYTE );
			}
			if( ocr[2] == 0x01 && ocr[3] == 0xAA )      /* The card can work at vdd range of 2.7-3.6V */
			{ /* Wait for leaving idle state (ACMD41 with HCS bit) */
				while( timeout-- && _spi_sd_sendcmd( SD_SEND_OP_COND, 1UL << 30, DUMMY_CRC ) )
				{
					;
				}
				/* Check CCS bit in the OCR */
				if( timeout && _spi_sd_sendcmd( READ_OCR, 0, DUMMY_CRC ) == 0 )
				{
					for( i = 0; i < 4; i++ )
					{
						ocr[i] = _spi_send_recv( DUMMY_BYTE );
					}
					ct = ( ocr[0] & 0x40 ) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			} else /* SDSC or MMC */
			{
				if( _spi_sd_sendcmd( SD_SEND_OP_COND, 0, DUMMY_CRC ) <= 1 )
				{
					ct	= CT_SD1;
					cmd = SD_SEND_OP_COND; /* SDSC */
				} else
				{
					ct	= CT_MMC;
					cmd = SEND_OP_COND; /* MMC */
				}
				/* Wait for leaving idle state */
				while( timeout-- && _spi_sd_sendcmd( cmd, 0, DUMMY_CRC ) )
				{
					;
				}
				/* Set R/W block length to 512 */
				if( !timeout || _spi_sd_sendcmd( SET_BLOCKLEN, SD_SECTOR_SIZE, DUMMY_CRC ) != 0 )
				{
					ct = CT_NONE;
				}
			}
		}else /* SDSC or MMC */
		{
			if( _spi_sd_sendcmd( SD_SEND_OP_COND, 0, DUMMY_CRC ) <= 1 )
			{
				ct	= CT_SD1;
				cmd = SD_SEND_OP_COND; /* SDSC */
			} else
			{
				ct	= CT_MMC;
				cmd = SEND_OP_COND; /* MMC */
			}
			/* Wait for leaving idle state */
			while( timeout-- && _spi_sd_sendcmd( cmd, 0, DUMMY_CRC ) )
			{
				;
			}
			/* Set R/W block length to 512 */
			if( !timeout || _spi_sd_sendcmd( SET_BLOCKLEN, SD_SECTOR_SIZE, DUMMY_CRC ) != 0 )
			{
				ct = CT_NONE;
			}
		}
	}
	CardType = ct;
	//LPC17xx_SPI_Release();
	//SPI_SSOutputCmd(SPI1,DISABLE);
#endif
	if( ct ) /* Initialization succeeded */
	{
		ret = true;
		if( ct == CT_MMC )
		{
			rt_kprintf("CT_MMC\n");
			_spi1_baud_rate( SPI_BaudRatePrescaler_16 );
		} 
		else
		{
			rt_kprintf("CT_SDSC\n");
			_spi1_baud_rate( SPI_BaudRatePrescaler_4 );
		}
		dfs_mount("","/tf","elmfat",0,0);
	} 
	else  /* Initialization failed */
	{ //LPC17xx_SPI_Select ();
		SPI_SSOutputCmd( SPI1, DISABLE );
		//LPC17xx_SD_WaitForReady ();
		_spi_sd_wait4ready( );
		//LPC17xx_SPI_DeInit();
	}

	return ret;
}


/***********************************************************
* Function:
* Description:配置spi接口即可
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t rt_sdcard_init( rt_device_t dev )
{
	return RT_EOK;
}

/*
打开tf卡
*/
static rt_err_t rt_sdcard_open( rt_device_t dev, rt_uint16_t oflag )
{
	rt_uint8_t	status;
	rt_uint8_t	*sector;


	if( !_spi_sd_init( ) ) 
	{
		rt_kprintf("_spi_sd_init fail\n");
		goto err;
	}
#if 0
	if( !_spi_sd_readcfg( &SDCfg ) )
	{
		rt_kprintf("_spi_sd_init fail\n");
		goto err;
	}

	/* get the first sector to read partition table */
	sector = (rt_uint8_t*)rt_malloc( 512 );
	if( sector == RT_NULL )
	{
		rt_kprintf( "allocate partition sector buffer failed\n" );
		return;
	}

	status = _spi_sd_readsector( 0, sector, 1 );
	if( status == true )
	{
		/* get the first partition */
		if( dfs_filesystem_get_partition( &part, sector, 0 ) != 0 )
		{
			/* there is no partition */
			part.offset = 0;
			part.size	= 0;
		}
	}else
	{
		/* there is no partition table */
		part.offset = 0;
		part.size	= 0;
	}

	/* release sector buffer */
	rt_free( sector );

#endif
	_spi_sd_readcfg(&SDCfg);

/*
在mount的时候还是会调用设备的open
*/
	dfs_mount("spi_sd","/sd","elm",0,0);
	return RT_EOK;

err:
	rt_kprintf( "sdcard init failed\n" );

	return RT_ERROR;
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
static rt_err_t rt_sdcard_close( rt_device_t dev )
{
	return RT_EOK;
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
static rt_size_t rt_sdcard_read( rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size )
{
	rt_uint8_t	status;
	//rt_uint32_t nr = size / SD_SECTOR_SIZE;

	//status = _spi_sd_readsector( part.offset + pos / SECTOR_SIZE, buffer, nr );
	status = _spi_sd_readsector( pos, buffer,size );

	if( status == true )
	{
		return size;
	}

	rt_kprintf( "read failed: %d, pos 0x%08x, size %d\n", status, pos, size );
	return 0;
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
static rt_size_t rt_sdcard_write( rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size )
{
	rt_uint8_t	status;
	rt_uint32_t nr = size / SD_SECTOR_SIZE;

	status = _spi_sd_writesector( part.offset + pos / SECTOR_SIZE, buffer, nr );

	if( status == true )
	{
		return nr * SECTOR_SIZE;
	}

	rt_kprintf( "write failed: %d, pos 0x%08x, size %d\n", status, pos, size );
	return 0;
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
static rt_err_t rt_sdcard_control( rt_device_t dev, rt_uint8_t cmd, void *args )
{
	return RT_EOK;
}


/***********************************************************
* Function:
* Description: 初始化一个设备，没有供电和插入指示
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void spi_sd_init( void )
{
	/* register sdcard device */
	dev_spi_sd.type		= RT_Device_Class_Block;
	dev_spi_sd.init		= rt_sdcard_init;
	dev_spi_sd.open		= rt_sdcard_open;
	dev_spi_sd.close	= rt_sdcard_close;
	dev_spi_sd.read		= rt_sdcard_read;
	dev_spi_sd.write	= rt_sdcard_write;
	dev_spi_sd.control	= rt_sdcard_control;

	/* no private */
	dev_spi_sd.user_data = &SDCfg;

	rt_device_register( &dev_spi_sd, "spi_sd",
	                    RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE );

	rt_device_init(&dev_spi_sd);

}



void tf_open( void )
{
	rt_sdcard_open( &dev_spi_sd,RT_DEVICE_FLAG_RDWR);
}

FINSH_FUNCTION_EXPORT( tf_open, open a tf card );


void tf_close( void )
{
	rt_sdcard_close( &dev_spi_sd);
}

FINSH_FUNCTION_EXPORT( tf_close, close a tf card );



/************************************** The End Of File **************************************/



#if 0

/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// 文件名
 * Author:			// 作者
 * Date:			// 日期
 * Description:		SPI接口的MicroSD设备
 * Version:			// 版本信息
 * Function List:	// 主要函数及其功能
 *     1. -------
 * History:			// 历史修改记录
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/


#include <rtthread.h>
#include "stm32f4xx.h"

#include <finsh.h>


static struct rt_device dev_spi_tf;

#define CS_PIN		GPIO_Pin_4
#define CS_GPIO_PORT GPIOA
#define CS_GPIO_CLK	RCC_AHB1Periph_GPIOA

#define CS_LOW( )	GPIO_ResetBits( CS_GPIO_PORT, CS_PIN )
#define CS_HIGH( )	GPIO_SetBits( CS_GPIO_PORT, CS_PIN )


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t  spi1_init( rt_device_t dev )
{
	SPI_InitTypeDef		SPI_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE );

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );

	GPIO_PinAFConfig( GPIOA, GPIO_PinSource5, GPIO_AF_SPI1 );
	GPIO_PinAFConfig( GPIOA, GPIO_PinSource6, GPIO_AF_SPI1 );
	GPIO_PinAFConfig( GPIOA, GPIO_PinSource7, GPIO_AF_SPI1 );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_DOWN;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	CS_HIGH();
		
	SPI_InitStructure.SPI_Direction			= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode				= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize			= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL				= SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA				= SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS				= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

	SPI_InitStructure.SPI_FirstBit		= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init( SPI1, &SPI_InitStructure );

	SPI_Cmd( SPI1, ENABLE );

	return RT_EOK;
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
rt_err_t  spi1_open( rt_device_t dev, rt_uint16_t oflag )
{
	CS_LOW();
	dfs_mount("tf","/","elm",0,0);
/*检查是否有卡存在*/
	return RT_EOK;
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
rt_err_t  spi1_close( rt_device_t dev )
{
	dfs_unmount("tf");
	CS_HIGH();
	return RT_EOK;
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
rt_size_t spi1_read( rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size )
{


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
rt_size_t spi1_write( rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size )
{
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
rt_err_t  spi1_control( rt_device_t dev, rt_uint8_t cmd, void *args )
{
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
void spi_tf_init( void )
{
	dev_spi_tf.type		= RT_Device_Class_Block;
	dev_spi_tf.init		= spi1_init;
	dev_spi_tf.open		= spi1_open;
	dev_spi_tf.close	= spi1_close;
	dev_spi_tf.read		= spi1_read;
	dev_spi_tf.write	= spi1_write;
	dev_spi_tf.control	= spi1_control;

	rt_device_register( &dev_spi_tf, "tf", RT_DEVICE_FLAG_RDWR );
	rt_device_init( &dev_spi_tf );
}




void tf_open( const char *str )
{
	spi1_open( &dev_spi_tf,RT_DEVICE_FLAG_RDWR|RT_DEVICE_FLAG_REMOVABLE);
}

FINSH_FUNCTION_EXPORT( tf_open, open a tf card );


void tf_close( const char *str )
{
	spi1_close( &dev_spi_tf);
}

FINSH_FUNCTION_EXPORT( tf_close, close a tf card );



/************************************** The End Of File **************************************/
#endif





