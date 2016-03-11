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

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include "s3c2410fb.h"

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
static int lcd_init(void)
{
	//1. ·ÖÅäÒ»¸öfb_info½á¹¹Ìå²Î¿¼s3c2410fb.c
	s3c_lcd = framebuffer_alloc(0£¬ NULL); //¿ÉÒÔÓĞË½ÓĞÊı¾İµÄ¶îÍâ¿Õ¼ä£¬±¾Çı¶¯²»ĞèÒª
						//ÄÚ´æÓĞ¿ÉÄÜ²»¹»£¬ĞèÒªÅĞ¶Ï£¬µ«±¾Çı¶¯ÔİÊ±²»ÅĞ¶Ï

	//2. ÉèÖÃ  ²Î¿¼fb.h
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
	
	//3. Ó²¼şÏà¹ØµÄ²Ù×÷
	//3.1 ÅäÖÃGPIOÓÃÓÚLCD
	//3.2 ¸ù¾İLCDÊÖ²áÉèÖÃLCD¿ØÖÆÆ÷£¬ÈçvclkÆµÂÊµÈ¡£
	//3.3 ·ÖÅäÏÔ´æ(framebuffer),²¢°ÑµØÖ·¸æËßLCD¿ØÖÆÆ÷
	s3c_lcd->smem_start = xxx; //ÏÔ´æµÄÎïÀíµØÖ·
	
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
