/**************************************************************************
 *                                                                         
 *   sed1520.c                                                             
 *   LCD display controller interface routines for graphics modules        
 *   with onboard SED1520 controller(s) in "write-only" setup              
 *                                                                         
 *   Version 1.02 (20051031)                                               
 *                                                                         
 *   For Atmel AVR controllers with avr-gcc/avr-libc                       
 *   Copyright (c) 2005                                                    
 *     Martin Thomas, Kaiserslautern, Germany                              
 *     <eversmith@heizung-thomas.de>                                       
 *     http://www.siwawi.arubi.uni-kl.de/avr_projects                      
 *
 *   Permission to use in NON-COMMERCIAL projects is herbey granted. For
 *   a commercial license contact the author.
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *   FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *   COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *                                                                         
 *
 *   partly based on code published by:                                    
 *   Michael J. Karas and Fabian "ape" Thiele
 *
 *
 ***************************************************************************/

/* 
   An Emerging Display EW12A03LY 122x32 Graphics module has been used
   for testing. This module only supports "write". There is no option
   to read data from the SED1520 RAM. The SED1520 R/W line on the 
   module is bound to GND according to the datasheet. Because of this 
   Read-Modify-Write using the LCD-RAM is not possible with the 12A03 
   LCD-Module. So this library uses a "framebuffer" which needs 
   ca. 500 bytes of the AVR's SRAM. The libray can of cause be used 
   with read/write modules too.
*/

/* tab-width: 4 */

//#include <LPC213x.H>      
//#include <includes.h>

#include <stdint.h>
#include "sed1520.h"
#include "bmp.h"
#include "board.h"
#include "stm32f4xx.h"

/* pixel level bit masks for display */
/* this array is setup to map the order */
/* of bits in a byte to the vertical order */
/* of bits at the LCD controller */
const unsigned char l_mask_array[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80}; /* TODO: avoid or PROGMEM */

/* the LCD display image memory */
/* buffer arranged so page memory is sequential in RAM */
static unsigned char l_display_array[LCD_Y_BYTES][LCD_X_BYTES];

/* control-lines hardware-interface (only "write") */
#define LCD_CMD_MODE()     LCDCTRLPORT &= ~(1<<LCDCMDPIN)
#define LCD_DATA_MODE()    LCDCTRLPORT |=  (1<<LCDCMDPIN)
#define LCD_ENABLE_E1()    LCDCTRLPORT &= ~(1<<LCDE1PIN)
#define LCD_DISABLE_E1()   LCDCTRLPORT |=  (1<<LCDE1PIN)
#define LCD_ENABLE_E2()    LCDCTRLPORT &= ~(1<<LCDE2PIN)
#define LCD_DISABLE_E2()   LCDCTRLPORT |=  (1<<LCDE2PIN)




#define MR		(1<<15)
#define SHCP	(1<<12)
#define DS		(1<<14)
#define STCP1	(1<<15)
#define STCP2	(1<<13)


#define RST0	(1<<0)
#define E1		(1<<1)
#define E2		(1<<2)
#define RW		(1<<3)
#define A0		(1<<4)


#define pgm_read_byte(a)	(*(a))
#define pgm_read_word(a) (*(a))




void ControlBitShift(unsigned char data)
{
	unsigned char i;
	
	//IOSET0	= STCP1;
	//IOCLR0 = STCP2;
	GPIO_SetBits(GPIOE,GPIO_Pin_15);
	GPIO_ResetBits(GPIOE,GPIO_Pin_13);
	
	for(i=0;i<8;i++)
		{
		//IOCLR0=SHCP;
		GPIO_ResetBits(GPIOE,GPIO_Pin_12);
		if(data&0x80)
			//IOSET0 = DS;
			GPIO_SetBits(GPIOE,GPIO_Pin_14);
		else
			//IOCLR0 = DS;
			GPIO_ResetBits(GPIOE,GPIO_Pin_14);
		//IOSET0 = SHCP;
		GPIO_SetBits(GPIOE,GPIO_Pin_12);
		data<<=1;
	}	
	//IOSET0 = STCP2;
	GPIO_SetBits(GPIOE,GPIO_Pin_13);
}


void DataBitShift(unsigned char data)
{
	unsigned char i;
	//IOCLR0 = STCP1;
	GPIO_SetBits(GPIOE,GPIO_Pin_13);
	GPIO_ResetBits(GPIOE,GPIO_Pin_15);
	for(i=0;i<8;i++)
		{
		//IOCLR0=SHCP;
		GPIO_ResetBits(GPIOE,GPIO_Pin_12);
		if(data&0x80)
			//IOSET0 = DS;
			GPIO_SetBits(GPIOE,GPIO_Pin_14);
		else
			//IOCLR0 = DS;
			GPIO_ResetBits(GPIOE,GPIO_Pin_14);
		//IOSET0 = SHCP;
		GPIO_SetBits(GPIOE,GPIO_Pin_12);
		data<<=1;
	}	
	//IOSET0 = STCP1;
	GPIO_SetBits(GPIOE,GPIO_Pin_15);
}
 
/* 
**
** low level routine to send a byte value 
** to the LCD controller control register. 
** entry argument is the data to output 
** and the controller to use
** 1: IC 1, 2: IC 2, 3: both ICs
**
*/
void lcd_out_ctl(const unsigned char cmd, const unsigned char ncontr)
{ 
	unsigned char ctr;
	unsigned int i;
// 	LCD_CMD_MODE();
//	LCDDATAPORT = cmd;

	ControlBitShift(RST0|0x0|0x60);
	DataBitShift(cmd);
	ctr=RST0;
	if(ncontr&0x01){
	  ctr|=E1;
	}
	if(ncontr&0x02){
		ctr|=E2;
	}	
	ControlBitShift(ctr|0x60);
	//delay(1);
	for(i=0;i<0xf;i++){}
	ControlBitShift(RST0|0|0x60);
}

/* 
**
** low level routine to send a byte value 
** to the LCD controller data register. entry argument
** is the data to output and the controller-number
**
*/
void lcd_out_dat(const unsigned char dat, const unsigned char ncontr)
{ 
	unsigned char ctr;
	unsigned int i;
//	LCD_DATA_MODE();
//   LCDDATAPORT = dat;

	ctr=RST0|A0;
	ControlBitShift(ctr|0x60);
	DataBitShift(dat);
	if(ncontr&0x01){
	  ctr|=E1;
	}
	if(ncontr&0x02){
		ctr|=E2;
	}
	ControlBitShift(ctr|0x60);
	//delay(1);
	for(i=0;i<0xf;i++){}
	ControlBitShift(RST0|A0|0x60);
}


/*
** 
** routine to initialize the operation of the LCD display subsystem
**
*/
void lcd_init(void)
 { 
	//IODIR0	= MR|SHCP|DS|STCP1|STCP2;
	//IOSET0	= MR;
	//GPIO_SetBits(GPIOA,GPIO_Pin_15);

	lcd_out_ctl(0,3);
    lcd_out_ctl(LCD_RESET,3);
    //delay_ms(1);//3
    
    lcd_out_ctl(LCD_DISP_ON,3);
    lcd_out_ctl(LCD_SET_ADC_NOR,3); // !
    lcd_out_ctl(LCD_SET_LINE+16,3);
    lcd_out_ctl(LCD_SET_PAGE+0,3);
    lcd_out_ctl(LCD_SET_COL,3);
	/*
	lcd_out_ctl(0,3);
	lcd_out_ctl(LCD_RESET,3);

	lcd_out_ctl(LCD_DISP_ON,3);
	lcd_out_ctl(LCD_SET_ADC_REV,3); // !
	lcd_out_ctl(LCD_SET_LINE+0,3);
	lcd_out_ctl(LCD_SET_PAGE+0,3);
	lcd_out_ctl(LCD_SET_COL+LCD_STARTCOL_REVERSE,3);
	*/
 } 


#ifdef LCD_DEBUG

/* fills complete area with pattern - direct to LCD-RAM */
void lcd_test(const unsigned char pattern)
 { unsigned char page, col;

	for (page=0; page<LCD_Y_BYTES; page++) 
    { lcd_out_ctl(LCD_SET_PAGE+page,3);
      lcd_out_ctl(LCD_SET_COL+LCD_STARTCOL_REVERSE,3); // !
      for (col=0; col<LCD_X_BYTES/2; col++) 
         lcd_out_dat(pattern,3);
		 
    }
 }

/* raw write to LCD-RAM - used for debugging */
void lcd_raw(const unsigned char page, const unsigned char col, const unsigned char nctrl, const unsigned char pattern)
 { lcd_out_ctl(LCD_SET_PAGE+page,nctrl);
   lcd_out_ctl(LCD_SET_COL+col,nctrl);	
   lcd_out_dat(pattern,nctrl);
 }

#endif /* LCD_DEBUG */


/* fill buffer and LCD with pattern */
void lcd_fill(const unsigned char pattern)
 { unsigned char page, col;
   lcd_init(); 
   lcd_out_ctl(LCD_DISP_OFF,3);
   for (page=0; page<LCD_Y_BYTES; page++) 
    { for (col=0; col<LCD_X_BYTES; col++) 
         l_display_array[page][col]=pattern;
    }
   lcd_update_all();
   lcd_out_ctl(LCD_DISP_ON,3);
 }

void lcd_fill_2(const unsigned char pattern,unsigned char Pag_start,unsigned char Pag_end,unsigned char Col_start,unsigned char col_end)
{
unsigned char page, col;

for (page=Pag_start; page<Pag_end; page++) 
	{ 
	for (col=Col_start; col<col_end; col++) 
		l_display_array[page][col]=pattern;
	}
lcd_update_all();
}

void lcd_fill_Page(const unsigned char pattern,unsigned char startpage,unsigned char endpage)
 { unsigned char page, col;

   for (page=startpage; page<endpage; page++) 
    { for (col=0; col<LCD_X_BYTES; col++) 
         l_display_array[page][col]=pattern;
    }
   lcd_update_all();
 }

void lcd_erase(void)
 { lcd_fill(0x00);
   lcd_update_all();
 }

/*
**
** Updates area of the display. Writes data from "framebuffer" 
** RAM to the lcd display controller RAM.
** 
** Arguments Used:
**    top     top line of area to update.
**    bottom  bottom line of area to update.
**    from MJK-Code
**
*/
void lcd_update(const unsigned char top, const unsigned char bottom)
 { unsigned char x;
   unsigned char y;
   unsigned char yt;
   unsigned char yb;
   unsigned char *colptr;
   
   /* setup bytes of range */
   yb = bottom >> 3;
   yt = top >> 3;		

   for(y = yt; y <= yb; y++)
    { lcd_out_ctl(LCD_SET_PAGE+y,3);	/* set page */
//     lcd_out_ctl(LCD_SET_COL+LCD_STARTCOL_REVERSE,3);
     lcd_out_ctl(LCD_SET_COL+0,3);
      colptr = &l_display_array[y][0];

      for (x=0; x < LCD_X_BYTES; x++)
       { if ( x < LCD_X_BYTES/2 ) 
            lcd_out_dat(*colptr++,1);
         else 
            lcd_out_dat(*colptr++,2);
       }
    }
 }



void lcd_update_all(void)
 { 
   lcd_update(SCRN_TOP,SCRN_BOTTOM); 
 }



/* sets/clears/switchs(XOR) dot at (x,y) */
void lcd_dot(const unsigned char x, const unsigned char y, const unsigned char mode) 
 { unsigned char bitnum, bitmask, yByte;
   unsigned char *pBuffer; /* pointer used for optimisation */

   if ( ( x > SCRN_RIGHT ) || ( y > SCRN_BOTTOM ) ) return;

   yByte   = y >> 3; 
   bitnum  = y & 0x07;
   bitmask = l_mask_array[bitnum]; // bitmask = ( 1 << (y & 0x07) );
   pBuffer = &(l_display_array[yByte][x]);
   switch (mode) 
    { case LCD_MODE_SET:
         *pBuffer |= bitmask;
         break;
      case LCD_MODE_CLEAR:
         *pBuffer &= ~bitmask;
         break;
      case LCD_MODE_XOR:
         *pBuffer ^= bitmask;
         break;
	  case LCD_MODE_INVERT:
	  	 if((*pBuffer) & bitmask>0){
		 	*pBuffer &=~bitmask;
	  	 }else{
			*pBuffer |= bitmask;
		 }
		 break;
      default: break;
    }
 }


/* line- and circle-function from a KS0108-library by F. Thiele */

void lcd_line( uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, const uint8_t mode ) 
 { uint8_t length, xTmp, yTmp, i, y, yAlt;
   int16_t m;

   if(x1 == x2) 
    { // vertical line
      // x1|y1 must be the upper point
      if(y1 > y2) 
       { xTmp = x1;
         yTmp = y1;
         x1 = x2;
         y1 = y2;
         x2 = xTmp;
         y2 = yTmp;
       }
      length = y2-y1;
      for(i=0; i<=length; i++) 
         lcd_dot(x1, y1+i, mode);
    } 
   else if(y1 == y2) 
    { // horizontal line
      // x1|y1 must be the left point
      if(x1 > x2) 
       { xTmp = x1;
         yTmp = y1;
         x1 = x2;
         y1 = y2;
         x2 = xTmp;
         y2 = yTmp;
       }

      length = x2-x1;
      for(i=0; i<=length; i++) 
         lcd_dot(x1+i, y1, mode);
       
    } 
   else 
    { // x1 must be smaller than x2
      if(x1 > x2) 
       { xTmp = x1;
         yTmp = y1;
         x1 = x2;
         y1 = y2;
         x2 = xTmp;
         y2 = yTmp;
       }
		
      if((y2-y1) >= (x2-x1) || (y1-y2) >= (x2-x1)) 
       { // angle larger or equal 45?
         length = x2-x1;								// not really the length :)
         m = ((y2-y1)*200)/length;
         yAlt = y1;
         for(i=0; i<=length; i++) 
          { y = ((m*i)/200)+y1;
            if((m*i)%200 >= 100)
               y++;
            else if((m*i)%200 <= -100)
               y--;
				
            lcd_line(x1+i, yAlt, x1+i, y, mode ); /* wuff wuff recurs. */
            if(length<=(y2-y1) && y1 < y2)
               yAlt = y+1;
            else if(length<=(y1-y2) && y1 > y2)
               yAlt = y-1;
            else
               yAlt = y;
          }
       } 
      else
       { // angle smaller 45?
         // y1 must be smaller than y2
         if(y1 > y2)
          { xTmp = x1;
            yTmp = y1;
            x1 = x2;
            y1 = y2;
            x2 = xTmp;
            y2 = yTmp;
          }
         length = y2-y1;
         m = ((x2-x1)*200)/length;
         yAlt = x1;
         for(i=0; i<=length; i++)
          { y = ((m*i)/200)+x1;

            if((m*i)%200 >= 100)
               y++;
            else if((m*i)%200 <= -100)
               y--;

            lcd_line(yAlt, y1+i, y, y1+i, mode); /* wuff */
            if(length<=(x2-x1) && x1 < x2)
               yAlt = y+1;
            else if(length<=(x1-x2) && x1 > x2)
               yAlt = y-1;
            else
               yAlt = y;
          }
       }
    }
 }


void lcd_circle(const uint8_t xCenter, const uint8_t yCenter, const uint8_t radius, const uint8_t mode) 
 { int16_t tSwitch, y, x = 0;
   uint8_t d;
   
   d = yCenter - xCenter;
   y = radius;
   tSwitch = 3 - 2 * radius;

   while (x <= y) 
    { lcd_dot(xCenter + x, yCenter + y, mode);
      lcd_dot(xCenter + x, yCenter - y, mode);
	
      lcd_dot(xCenter - x, yCenter + y, mode);
      lcd_dot(xCenter - x, yCenter - y, mode);
	
      lcd_dot(yCenter + y - d, yCenter + x, mode);
      lcd_dot(yCenter + y - d, yCenter - x, mode);
   
      lcd_dot(yCenter - y - d, yCenter + x, mode);
      lcd_dot(yCenter - y - d, yCenter - x, mode);

      if (tSwitch < 0) 
         tSwitch += (4 * x + 6);
      else 
       { tSwitch += (4 * (x - y) + 10);
         y--;
       }
      x++;
    }
 }


void lcd_rect(const uint8_t x, const uint8_t y, uint8_t width, uint8_t height, const uint8_t mode) 
 { width--;
   height--;
   lcd_line(x, y, x+width, y, mode);	// top
   lcd_line(x, y, x, y+height, mode);	// left
   lcd_line(x, y+height, x+width, y+height, mode);	// bottom
   lcd_line(x+width, y, x+width, y+height, mode);		// right
 }

void lcd_box(const uint8_t x, const uint8_t y, uint8_t width, const uint8_t height, const uint8_t mode) 
 { uint8_t i;
   if (!width) return; 

   width--;
	
   for (i=y;i<y+height;i++) 
      lcd_line(x, i, x+width, i, mode);
 }


/*
 Writes a glyph("letter") to the display at location x,y
 (adapted function from the MJK-code)
 Arguments are:
    column    - x corrdinate of the left part of glyph          
    row       - y coordinate of the top part of glyph       
    width     - size in pixels of the width of the glyph    
    height    - size in pixels of the height of the glyph   
    glyph     - an unsigned char pointer to the glyph pixels 
                to write assumed to be of length "width"
*/

void lcd_glyph(uint8_t left, uint8_t top, uint8_t width, uint8_t height, uint8_t *glyph_ptr, uint8_t store_width)
 { uint8_t bit_pos;
   uint8_t byte_offset;
   uint8_t y_bits;
   uint8_t remaining_bits;
   uint8_t mask;
   uint8_t char_mask;
   uint8_t x;
   uint8_t *glyph_scan;
   uint8_t glyph_offset;

   bit_pos = top & 0x07;		/* get the bit offset into a byte */
   glyph_offset = 0;			/* start at left side of the glyph rasters */
   char_mask = 0x80;			/* initial character glyph mask */

   for (x = left; x < (left + width); x++)
    { byte_offset = top >> 3;        	/* get the byte offset into y direction */
      y_bits = height;		/* get length in y direction to write */
      remaining_bits = 8 - bit_pos;	/* number of bits left in byte */
      mask = l_mask_array[bit_pos];	/* get mask for this bit */
      glyph_scan = glyph_ptr + glyph_offset;	 /* point to base of the glyph */
      /* boundary checking here to account for the possibility of  */
      /* write past the bottom of the screen.                        */
      while((y_bits) && (byte_offset < LCD_Y_BYTES)) /* while there are bits still to write */
       { /* check if the character pixel is set or not */
         //if(*glyph_scan & char_mask)
         if(pgm_read_byte(glyph_scan) & char_mask)
            l_display_array[byte_offset][x] |= mask;	/* set image pixel */
         else
            l_display_array[byte_offset][x] &= ~mask;	/* clear the image pixel */
   
         if(l_mask_array[0] & 0x80)
            mask >>= 1;
         else
            mask <<= 1;
			
         y_bits--;
         remaining_bits--;
         if(remaining_bits == 0)
          { /* just crossed over a byte boundry, reset byte counts */
            remaining_bits = 8;
            byte_offset++;
            mask = l_mask_array[0];
          }
         /* bump the glyph scan to next raster */
         glyph_scan += store_width;
       }

      /* shift over to next glyph bit */
      char_mask >>= 1;
      if(char_mask == 0)				/* reset for next byte in raster */
       { char_mask = 0x80;
         glyph_offset++;
       }
    }
 }


/*
 Prints the given string at location x,y in the specified font.
 Prints each character given via calls to lcd_glyph. The entry string
 is null terminated. (adapted function from the MJK-code)
 Arguments are:
	left       coordinate of left start of string.
	top        coordinate of top of string.
	font       font number to use for display (see fonts.h)
	str        text string to display (null-terminated)
*/
#if 0
static void lcd_text_intern(uint8_t left, uint8_t top, uint8_t font, const char *str, uint8_t inprogmem)
 { uint8_t x = left;
   uint8_t glyph;
   uint8_t width;
   uint8_t height, defaultheight;
   uint8_t store_width;
   uint8_t *glyph_ptr;
   uint8_t *width_table_ptr;
   uint8_t *glyph_table_ptr;
   uint8_t glyph_beg, glyph_end;
   uint8_t fixedwidth;

   defaultheight = pgm_read_byte ( &(fonts[font].glyph_height) );
   store_width = pgm_read_byte ( &(fonts[font].store_width) );
   width_table_ptr = (uint8_t*) pgm_read_word( &(fonts[font].width_table) );
   glyph_table_ptr = (uint8_t*) pgm_read_word( &(fonts[font].glyph_table) );
   glyph_beg  = pgm_read_byte( &(fonts[font].glyph_beg) );
   glyph_end  = pgm_read_byte( &(fonts[font].glyph_end) );
   fixedwidth = pgm_read_byte( &(fonts[font].fixed_width) );

   if (inprogmem) 
      glyph = pgm_read_byte(str);
    
   else 
      glyph = (uint8_t)*str;
	
   while(glyph != 0x00) // while(*str != 0x00)
    { /* check to make sure the symbol is a legal one */
      /* if not then just replace it with the default character */
      if((glyph < glyph_beg) || (glyph > glyph_end))
         glyph = pgm_read_byte( &(fonts[font].glyph_def) ) ;

      /* make zero based index into the font data arrays */
      glyph -= glyph_beg;
      if(fixedwidth == 0)
         // width=fonts[font].width_table[glyph];	/* get the variable width instead */
         width=pgm_read_byte(width_table_ptr+glyph);
      else 
         width = fixedwidth;
		
      height = defaultheight;
      //glyph_ptr = fonts[font].glyph_table + ((unsigned int)glyph * (unsigned int)store_width * (unsigned int)height);
      glyph_ptr = glyph_table_ptr + ((unsigned int)glyph * (unsigned int)store_width * (unsigned int)height) ;

      /* range check / limit things here */
      if(x > SCRN_RIGHT)
         x = SCRN_RIGHT;
       
      if((x + width) > SCRN_RIGHT+1)
         width = SCRN_RIGHT - x + 1;
       
      if(top > SCRN_BOTTOM)
         top = SCRN_BOTTOM;
       
      if((top + height) > SCRN_BOTTOM+1)
         height = SCRN_BOTTOM - top + 1;
       
      lcd_glyph(x,top,width,height,glyph_ptr,store_width);  /* plug symbol into buffer */

      x += width;		/* move right for next character */
      str++;			/* point to next character in string */
      if (inprogmem) 
         glyph = pgm_read_byte(str);
      else 
         glyph = (uint8_t)*str;
       
    }
 }

/* draw string from RAM */
void lcd_text(uint8_t left, uint8_t top, uint8_t font, const char *str)
 { lcd_text_intern(left, top, font, str, 0);
 }

/* draw string from PROGMEM */
void lcd_text_p(uint8_t left, uint8_t top, uint8_t font, const char *str)
 { lcd_text_intern(left, top, font, str, 1);
 }

#endif



void lcd_bitmap(const uint8_t left, const uint8_t top, const struct IMG_DEF *img_ptr, const uint8_t mode)
{ 
	uint8_t width, heigth, h, w, pattern, mask;
	uint8_t* ptable;

	uint8_t bitnum,bitmask;
	uint8_t page,col,vdata;

	width  = img_ptr->width_in_pixels;
	heigth = img_ptr->height_in_pixels;
	ptable  =(uint8_t*)(img_ptr->char_table); 


	mask=0x80;
	pattern = *ptable;
	
	for ( h=0; h<heigth; h++ ){   /**/
		page=(h+top)>>3;
		bitnum=(h+top)&0x07;
		bitmask=(1<<bitnum);
		for ( w=0; w<width; w++ ) { 
			col=left+w;
			vdata=l_display_array[page][col];
			switch(mode){
				case LCD_MODE_SET:		/*不管原来的数据，直接设置为pattern的值*/
					if (pattern&mask)	
						vdata|=bitmask;
					else
						vdata&=~bitmask;
					break;
				case LCD_MODE_CLEAR:	/*不管原来的数据，清除原来的值=>0*/
					vdata&=~bitmask;	
					break;
				case LCD_MODE_XOR:		/*原来的数据，直接设置为pattern的值*/
					if(vdata&bitmask){
						if(pattern&mask)	vdata&=~bitmask;
						else vdata|=bitmask;
					}else{
						if(pattern&mask)	vdata|=bitmask;
						else vdata&=~bitmask;
					}
					break;
				case LCD_MODE_INVERT:	/*不管原来的数据，直接设置为pattern的值*/
					if (pattern&mask)	
						vdata&=~bitmask;
					else
						vdata|=bitmask;
					break;	 
						
			}
			l_display_array[page][col]=vdata;
			mask >>= 1;
			if ( mask == 0 ){ 
				mask = 0x80;
				ptable++;
				pattern = *ptable;
				
			}
		}
		if(mask!=0x80){		/*一行中的列已处理完*/
			mask = 0x80;
			ptable++;
			pattern = *ptable;
		}	
	}
}


/*
绘制12点阵的字符，包括中文和英文


*/
void lcd_text12(char left,char top ,char *p,char len,const char mode)
{
	int charnum=len;
	int i;
	char msb,lsb;
	
	int addr;
	unsigned char start_col=left;
	unsigned int  val_old, val_new, val_mask;

	unsigned int glyph[12];   /*保存一个字符的点阵信息，以逐列式*/

	
	while( charnum )
	{
		for( i = 0; i < 12; i++ )
		{
			glyph[i] = 0;
		}
		msb = *p++;
		charnum--;
		if( msb <= 0x80 ) //ascii字符 0612
		{
			addr = ( msb - 0x20 ) * 12 + FONT_ASC0612_ADDR;
			for( i = 0; i < 3; i++ )
			{
				val_new				= *(__IO uint32_t*)addr;
				glyph[i * 2 + 0]	= ( val_new & 0xffff );
				glyph[i * 2 + 1]	= ( val_new & 0xffff0000 )>>16;
				addr						+= 4;
			}
			
			val_mask=((0xfff)<<top);/*12bit*/

			/*加上top的偏移*/
			for( i = 0; i < 6; i++ )
			{
				glyph[i]<<=top;

				val_old=l_display_array[0][start_col]|(l_display_array[1][start_col]<<8)|(l_display_array[2][start_col]<<16)|(l_display_array[3][start_col]<<24);
				if(mode==LCD_MODE_SET)
				{
					val_new=val_old&(~val_mask)|glyph[i];
				}
				else if(mode==LCD_MODE_INVERT)
				{
					val_new=(val_old|val_mask)&(~glyph[i]);
				}
				l_display_array[0][start_col]=val_new&0xff;
				l_display_array[1][start_col]=(val_new&0xff00)>>8;
				l_display_array[2][start_col]=(val_new&0xff0000)>>16;
				l_display_array[3][start_col]=(val_new&0xff000000)>>24;
				start_col++;
			}
		}
		else
		{
			lsb = *p++;
			charnum--;
			if( ( msb >= 0xa1 ) && ( msb <= 0xa3 ) && ( lsb >= 0xa1 ) )
			{
				addr = FONT_HZ1212_ADDR + ( ( ( (unsigned long)msb ) - 0xa1 ) * 94 + ( ( (unsigned long)lsb ) - 0xa1 ) ) * 24;
			}else if( ( msb >= 0xb0 ) && ( msb <= 0xf7 ) && ( lsb >= 0xa1 ) )
			{
				addr = FONT_HZ1212_ADDR + ( ( ( (unsigned long)msb ) - 0xb0 ) * 94 + ( ( (unsigned long)lsb ) - 0xa1 ) ) * 24 + 282 * 24;
			}
			for( i = 0; i < 6; i++ )
			{
				val_new				= *(__IO uint32_t*)addr;
				glyph[i * 2 + 0]	= ( val_new & 0xffff );
				glyph[i * 2 + 1]	= ( val_new & 0xffff0000 )>>16;
				addr				+= 4;
			}
			val_mask=((0xfff)<<top);/*12bit*/

			/*加上top的偏移*/
			for( i = 0; i < 12; i++ )
			{
				glyph[i]<<=top;
				/*通过start_col映射到I_display_array中，注意mask*/
				val_old=l_display_array[0][start_col]|(l_display_array[1][start_col]<<8)|(l_display_array[2][start_col]<<16)|(l_display_array[3][start_col]<<24);
				if(mode==LCD_MODE_SET)
				{
					val_new=val_old&(~val_mask)|glyph[i];
				}
				else if(mode==LCD_MODE_INVERT)
				{
					val_new=(val_old|val_mask)&(~glyph[i]);
				}
				l_display_array[0][start_col]=val_new&0xff;
				l_display_array[1][start_col]=(val_new&0xff00)>>8;
				l_display_array[2][start_col]=(val_new&0xff0000)>>16;
				l_display_array[3][start_col]=(val_new&0xff000000)>>24;
				start_col++;
			}

		}
	}




}



