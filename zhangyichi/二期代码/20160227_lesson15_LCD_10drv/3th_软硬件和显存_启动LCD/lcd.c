//lcd.c 20160227

//Í·ÎÄ¼şÀ´×ÔÓÚs3c2410fb.c(driers\vidoe)

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>
#include <stdio.h>
#include "s3c24xx.h"
#include "lcddrv.h"


#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include "s3c2410fb.h"

//Õë¶Ô3.2Òº¾§¿ØÖÆ¼Ä´æÆ÷µÄËµÃ÷ÈçÏÂ
/*
3.2 ¸ù¾İLCDÊÖ²áÉèÖÃLCD¿ØÖÆÆ÷£¬ÈçvclkÆµÂÊµÈ¡£
¸ù¾İÊÖ²áLCD CONTROLLER 396Ò³µÄÄÚÈİ´ÓÍ·Éèµ½Î²
¾ßÌåÄÚÈİ¼û422Ò³ LCD CONTROL REGISTOR
ÓÉÓÚ¼Ä´æÆ÷ÊıÁ¿Ì«¶à£¬Òò´Ë¶¼·ÅÔÚÊı×éÀïÃæ±È½Ï·½±ã
×¢ÒâËûÃÇ¸÷¼Ä´æÆ÷Ö®¼äÏà²î4¸ö×Ö½Ú
_____ÌøÔ¾Ì«´óµÄ»°£¬ÖĞ¼ä¼ÓÒ»Ğ©±£Áô×Ö½Ú
È¡×ÔÓÚ3th µÄlcd.c, ÊÊÓÃÓÚ2410£¬ÒªÓëÊÖ²á±È½ÏÒ»ÏÂ¿´ÊÇ·ñÓĞ±ä»¯
ÔÚÕâ¸ö¼Ä´æÆ÷×éÉÏ£¬2410ÍêÈ«ºÍ2440ÏàÍ¬¡£
*/
//¹¹½¨LCD¿ØÖÆ¼Ä´æÆ÷Êı×é¡¡
struct lcd_regs {  //
		unsigned long	lcdcon1;
		unsigned long	lcdcon2;
		unsigned long	lcdcon3;
		unsigned long	lcdcon4;
		unsigned long	lcdcon5;
		unsigned long	lcdsaddr1;
		unsigned long	lcdsaddr2;
		unsigned long	lcdsaddr3;
		unsigned long	redlut;
		unsigned long	greenlut;
		unsigned long	bluelut;
		unsigned long	reserved[9]; // bluelutµ½dithmodeµØÖ··Ö±ğÎª28ºÍ4c,Ïà²î0x36,¼´Ïà²î9£¬ËùÒÔ±£Áô9×Ö½Ú
		unsigned long	dithmode;
		unsigned long	tpal;
		unsigned long	lcdintpnd;
		unsigned long	lcdsrcpnd;
		unsigned long	lcdintmsk;
		unsigned long	lpcsel;
	};





//À´×ÔÓÚatmel_lcdfb.c(drivers\video)
static struct fb_ops s3c_lcdfb_ops = {  //¶ÔÏÔ´æµÄ²Ù×÷ 
	.owner		= THIS_MODULE,
	//.fb_setcolreg	= atmel_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,    //Ìî³ä¾ØĞÎ
	.fb_copyarea	= cfb_copyarea, //¿½±´ÇøÓò
	.fb_imageblit	= cfb_imageblit,
};


// Step1 ÔÚ¿ò¼Ü×öºÃºó£¬¿ªÊ¼ÉèÖÃ²ÎÊı
static struct fb_info *s3c_lcd;
static volatile unsigned long *gpbcon; //¹¹½¨¼Ä´æÆ÷½á¹¹ÌåÖ¸Õë
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;
static volatile struct lcd_regs* lcd_regs; //Ö¸ÏòÄÄÀïÄØ£¬ÔÚ3.2Àïioremap

static int lcd_init(void)
{
	//1. ·ÖÅäÒ»¸öfb_info½á¹¹Ìå²Î¿¼s3c2410fb.c
	s3c_lcd = framebuffer_alloc(0£¬ NULL); //¿ÉÒÔÓĞË½ÓĞÊı¾İµÄ¶îÍâ¿Õ¼ä£¬±¾Çı¶¯²»ĞèÒª
						//ÄÚ´æÓĞ¿ÉÄÜ²»¹»£¬ĞèÒªÅĞ¶Ï£¬µ«±¾Çı¶¯ÔİÊ±²»ÅĞ¶Ï

	//2. ÉèÖÃ  ²Î¿¼fb.h***********************
	//2.1 ÉèÖÃ¹Ì¶¨µÄ²ÎÊı
	strcpy(s3c_lcd->fix.id, "mylcd");
	s3c_lcd->fix.smem_len = 240*320*16/8;   //Ã¿¸öÏñËØ16Î»£¬Õ¼2¸ö×Ö½Ú
	s3c_lcd->fix.type         = FB_TYPE_PACKED_PIXELS;
	s3c_lcd->fix.visual       = FB_VISUAL_TRUECOLOR;  //Õæ²ÊÉ«
	s3c_lcd->fix.line_length= 
							
	//2.2 ÉèÖÃ¿É±äµÄ²ÎÊı
	s3c_lcd->var.xres           =       240;
	s3c_lcd->var.yres           =       320;
	s3c_lcd->var.xres_virtual =       240;
	s3c_lcd->var.yres_virtual =       320;
	s3c_lcd->bits_per_pixel   =       16;    //16Î» 
	

	// RGB:565
	s3c_lcd->var.red.offset   = 11;
	s3c_lcd->var.red.length   =   5;
	
	s3c_lcd->var.red.offset   =   5;
	s3c_lcd->var.red.length   =   6;      

	s3c_lcd->var.red.offset   = 	  0;
	s3c_lcd->var.red.length  =	  5;

	s3c_lcd->var.activate     =  FB_ACTIVATE_NOW  //Ä¬ÈÏÖµ 

	//2.3 ÉèÖÃ²Ù×÷²ÎÊı£¬Êı×éÔÚÉÏÃæ¶¨ÒåÁËstatic struct fb_ops s3c_lcdfb_ops 
	s3c_lcd->fbops             =  &s3c_lcdfb_ops
	
	
	//2.4 ÆäËûµÄÉèÖÃ
	// s3c_lcd->pseudo_palette  =   ; //¼ÙµÄµ÷É«°åå 
	//s3c_lcd->screen_base      =;   //ÏÔ´æµÄĞéÄâµØÖ·
	s3c_lcd->screen_size       = 240*320*16/8;  //ÏÖ´æµÄ´óĞ¡

	/*
	¸ù¾İÊÖ²áLCD CONTROLLER 396Ò³µÄÄÚÈİ´ÓÍ·Éèµ½Î²
	¾ßÌåÄÚÈİ¼û422Ò³ LCD CONTROL REGISTOR
	ÓÉÓÚ¼Ä´æÆ÷ÊıÁ¿Ì«¶à£¬Òò´Ë¶¼·ÅÔÚÊı×éÀïÃæ±È½Ï·½±ã
	*/
	
	//3. Ó²¼şÏà¹ØµÄ²Ù×÷*********************
	//3.1 ÅäÖÃGPIOÓÃÓÚLCD
	gpbcon = ioremap(0x56000010, 8); 
	gpbdat = gpbcon+1;
	gpccon = ioremap(0x56000020, 4); //ÏµÍ³Ó³ÉäÊÇÒÔÒ³Îªµ¥Î»£¬ÊÇ1024×Ö½Ú£¬ËùÒÔÌî1024»ò4 ¶¼Ã»¹ØÏµ
	gpdcon = ioremap(0x56000030, 4);
	gpgcon= ioremap(0x56000060, 4);

	*gpccon   = 0xaaaaaaaa;   //¿½±´×Ôlcddrv.c GPIO¹Ü½ÅÓÃÓÚVD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 
	*gpdcon   = 0xaaaaaaaa;	// GPIO¹Ü½ÅÓÃÓÚVD[23:8]

	*gpbcon &= ~(3); //GPIOÉèÖÃÎªÊä³öÒı½Å
	*gpbcon |=1;   //Êä³öµÍµçÆ½£¬µ«ÏÈ²»ÓÃ
	*gpbdat &= ~1;  //Êä³öµÍµçÆ½£¬ÏÈ²»ÈÃ±³¹â¿ªÆô

	//KEYBOARDÊÇ±³¹âµçÔ´£¬LCD_PWRENÊÇµçÔ´£¬Á½»ØÊÂ
	//LCD_PWRENÊÇÓÉGPG4¿ØÖÆ,bit[8,9]ÉèÖÃÎª11
	*gpgcon |=(3<<8); //GPG4ÓÃ×÷LCDµçÔ´LCD_PWRENÊ¹ÄÜÒı½Å
	

	
	//3.2 ¸ù¾İLCDÊÖ²áÉèÖÃLCD¿ØÖÆÆ÷£¬ÈçvclkÆµÂÊµÈ¡£
	lcd_regs ==ioremap(0x4D000000,sizeof(struct lcd_regs)); //»ùµØÖ·ºÍ´óĞ¡
	//¿ªÊ¼½øĞĞ¾ßÌåÉèÖÃ£¬ÒªÒ»Î»Ò»Î»µÄÉèÖÃ£¬²ÅÄÜ×¼È· ÊÖ²á422Ò³

	// LCD¿ØÖÆÆ÷1  lcdcon1
	
	/*  [17:8]      ÉèÖÃCLKVAL
		CLKVAL [17:8],ĞèÒªºÏÊÊµÄÊ±ÖÓ²ÅĞĞ
	  ¹«Ê½ VCLK = HCLK / [(CLKVAL +1) *2] LCDÊÖ²áP14
	  ÄÚºËµÄÊä³öĞÅÏ¢¿ÉÒÔÓÃdmsgÃüÁîÀ´¿´
	  HCLK= 100 MHz (ÓÃdmsgÃüÁî²é¿´ÄÚºËµÃÀ´)
	   VCLKÈ¡¶àÉÙ£¬Òª¿´LCDÊÖ²á£¬²Î¿¼Òº¾§ÆÁ.pdf14Ò³ clock cycle time ×îĞ¡100ns
	 	15Ò³DCLK Í¼Æ¬ 83ns+ÉÏÉıÑØ£¬ËùÒÔ×îĞ¡È¡100ns
	  10MHz(100nsÆµÂÊ¼´10MHz) = 100MHz / [(CLKVAL+1) x 2]
	 
	 *	ËùÒÔµÃµ½:		  CLKVAL = 4

	* MMODE  ²»ÖªµÀÊÇÊ²Ã´£¬²»¹Ü

	* [16:5]    ÉèÖÃPNRMODE ÎÒÃÇÓÃµÄÊÇTFT LCD panel£¬ËùÒÔÊÇ0b11
	* [4:1]  ÉèÖÃBPMODE,ÉèÖÃBPP,µ¥Î»ÏñËØ±ÈÌØÎ»£¬
	         0b1100, 16 bpp for TFT
	* [0]    Ê¹ÄÜ 
	      0 = Disable the video output and the LCD control signal         
	*/  

	lcd_regs->lcdcon1  =  (4<<8) | (3<<5) | (0x0c<<1);

	//LCD¿ØÖÆÆ÷2 lcdcon2  add 0x4D00004
	/* 
	/* ´¹Ö±·½ÏòµÄÊ±¼ä²ÎÊı  ¾ßÌå¼ÆËã²Î¼û±Ê¼Ç
	
	 * bit[31:24]: VBPD, VSYNCÖ®ºóÔÙ¹ı¶à³¤Ê±¼ä²ÅÄÜ·¢³öµÚ1ĞĞÊı¾İ
	 *			   LCDÊÖ²á T0-T2-T1=4  
		 *			   VBPD=3
		 * bit[23:14]: ¶àÉÙĞĞ, 320ĞĞ, ËùÒÔLINEVAL=320-1=319
		 * bit[13:6] : VFPD, ·¢³ö×îºóÒ»ĞĞÊı¾İÖ®ºó£¬ÔÙ¹ı¶à³¤Ê±¼ä²Å·¢³öVSYNC
		 *			   LCDÊÖ²áT2-T5=322-320=2, ËùÒÔVFPD=2-1=1
		 * bit[5:0]  : VSPW, VSYNCĞÅºÅµÄÂö³å¿í¶È, LCDÊÖ²áT1=1, ËùÒÔVSPW=1-1=0
		 */
		lcd_regs->lcdcon2  = (3<<24) | (319<<14) | (1<<6) | (0<<0);

	
	
	/* Ë®Æ½·½ÏòµÄÊ±¼ä²ÎÊı

	LCD¿ØÖÆÆ÷3 lcdcon3  0x4D00008 
		 * bit[25:19]: HBPD, VSYNCÖ®ºóÔÙ¹ı¶à³¤Ê±¼ä²ÅÄÜ·¢³öµÚ1ĞĞÊı¾İ
		 *			   LCDÊÖ²á T6-T7-T8=17
		 *			   HBPD=16
		 * bit[18:8]: ¶àÉÙÁĞ, 240, ËùÒÔHOZVAL=240-1=239
		 * bit[7:0] : HFPD, ·¢³ö×îºóÒ»ĞĞÀï×îºóÒ»¸öÏóËØÊı¾İÖ®ºó£¬ÔÙ¹ı¶à³¤Ê±¼ä²Å·¢³öHSYNC
		 *			   LCDÊÖ²áT8-T11=251-240=11, ËùÒÔHFPD=11-1=10
		 */
		lcd_regs->lcdcon3 = (16<<19) | (239<<8) | (10<<0);
	
	/* 
	LCDĞÅºÅ¿ØÖÆÆ÷4 add 0x4D0000c
		Ë®Æ½·½ÏòµÄÍ¬²½ĞÅºÅlcdcon4  
		 * bit[7:0] : HSPW, HSYNCĞÅºÅµÄÂö³å¿í¶È, LCDÊÖ²áT7=5, ËùÒÔHSPW=5-1=4
		 */ 
		lcd_regs->lcdcon4 = 4;
	
	/* 
	LCDĞÅºÅ¿ØÖÆÆ÷5  ĞÅºÅµÄ¼«ĞÔ  0x4D000010
	 * bit[11]: 1=565 format   FRM565
	 * bit[10]: 0 = The video data is fetched at VCLK falling edge INVVCLK
	 				ÏÂ½µÑØÈ¡Êı×é£¬²»ĞèÒª·­×ª
	 * bit[9] : 1 = HSYNCĞÅºÅÒª·´×ª,¼´µÍµçÆ½ÓĞĞ§  INVVLINE 2440 ÊÖ²á¿ÉÒÔ¿´³ö£¬·Ö±ğÎª
	 				µÍµçÆ½ºÍ¸ßµçÆ½ÓĞĞ§£¬Òò´ËÒª·­×ª²Å³É
	 * bit[8] : 1 = VSYNCĞÅºÅÒª·´×ª,¼´µÍµçÆ½ÓĞĞ§  INVVFRAME Ö¡Í¬²½ĞÅºÅÒª·­×ª
	 * bit[7] : 0 = INVVD videoÊı¾İĞÅºÅ²»Òª·´×ª
	 * bit[6] : 0 = VDEN²»ÓÃ·´×ª
	 * bit[3] : 0 = PWRENÊä³ö0  ÊÇ·ñÊ¹ÄÜÊä³öµÍµçÆ½
	 * bit[1] : 0 = BSWP  ×Ö½Ú½»»»
	 * bit[0] : 1 = HWSWP 2440ÊÖ²áP413
	 bwsp =0 hwswp=0, [31:16][15:0]  ÅÅÁĞÎªp2p1 p4p3,ºÜ±ğÅ¤
	 bwsp =0 hwswp=1, [31:16][15:0]  ÅÅÁĞÎªp1p2 p3p4,¾ÍË³µ±ÁË
	 
	 */
	lcd_regs->lcdcon5 = (1<<11) | (0<<10) | (1<<9) | (1<<8) | (1<<0);
	

			
	//3.3 ·ÖÅäÏÔ´æ(framebuffer),²¢°ÑµØÖ·¸æËßLCD¿ØÖÆÆ÷
	
	
	// ²Î¿¼s3c2410fb.c
	//dma_alloc_writecombine(fbi->dev, fbi->map_size, &fbi->map_dma, GFP_KERNEL);
	//fbi->map_size,´óĞ¡Îªs3c_lcd->fix.smem_len = 240*320*16/8;Ã¿¸öÏñËØ16Î»£¬Õ¼2¸ö×Ö½Ú
	//&fbi->map_dma, DMAÀàĞÍ£¬ÎïÀíµØÖ·s3c_lcd->smem_start
	//·µ»ØÖµÎªÄÚ´æµÄĞéÄâµØÖ· 2.4ÀïÃæÉèÖÃ

	s3c_lcd->screen_base  = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->smem_start,  GFP_KERNEL);


	//FRAME BUFFER START ADDRESS¼Ä´æÆ÷ ÊÖ²á428Ò³ ÓĞ3¸ö¼Ä´æÆ÷LCDSADDR1-3

	//µÚÒ»¸ö¼Ä´æÆ÷ ×î¸ß2Î»ÇåÁã ¿  Çåµô×î¸ßÁ½Î»
	//µÚ¶ş¸ö¼Ä´æÆ÷ ½áÊøµØÖ·A[21:1] ÓÒÒÆÒ»Î»
	//µÚÈı¸ö¼Ä´æÆ÷ ĞéÄâºÍÕæÊµÆÁÄ»µÄÆ«ÒÆÖµ£¬±¾ÀıOFFSIZEÎª0 Ö»ĞèÉèÖÃPAGEWIDTH,Ò»ĞĞµÄ´óĞ¡
	//s3c_lcd->fix.smem_start = xxx;  /* ÏÔ´æµÄÎïÀíµØÖ· */lcd_regs->lcdsaddr1  = (s3c_lcd->fix.smem_start >> 1) & ~(3<<30); 
	lcd_regs->lcdsaddr2  = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len) >> 1) & 0x1fffff;
	lcd_regs->lcdsaddr3  = (240*16/16);  /* Ò»ĞĞµÄ³¤¶È(µ¥Î»ÊÇ°ë×Ö¼´2×Ö½Ú) */	
		
	
	
	/* Æô¶¯LCD
	´ò¿ª±³¹â
	Ìá¹©LCDµçÔ´
	Ìá¹©Ò»Ğ©½Ó¿Ú£¬ÈÃ±ğÈË´ò¿ª

	*/
	
	
	lcd_regs->lcdcon1 |= (1<<0); /* Ê¹ÄÜLCD¿ØÖÆÆ÷ */
	lcd_regs->lcdcon5 |= (1<<3); /* Ê¹ÄÜLCD±¾Éí */
	
	*gpbdat |= 1;	  /* Êä³ö¸ßµçÆ½, Ê¹ÄÜ±³¹â Ô­À´ÉèÖÃ*gpbdat &= ~1;Êä³öµÍµçÆ½£¬ÏÈ²»ÈÃ±³¹â¿ªÆô*/		
	
	
	
	//4. ×¢²á_ÉèÖÃºÃÁËÔÙ×¢²á 
	
	register_framebuffer(s3c_lcd);

	
	return 0;

}

5static int lcd_exit(void)
{
	return 0 ;

	
}


module_init(lcd_init);
modul_exit(lcd_exit);

MODULE_LICENSE("GPL");
