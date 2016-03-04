//lcd.c 20160227

//ͷ�ļ�������s3c2410fb.c(driers\vidoe)

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

//������atmel_lcdfb.c(drivers\video)
static struct fb_ops s3c_lcdfb_ops = {  //���Դ�Ĳ��� 
	.owner		= THIS_MODULE,
	//.fb_setcolreg	= atmel_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,    //������
	.fb_copyarea	= cfb_copyarea, //��������
	.fb_imageblit	= cfb_imageblit,
};


// Step1 �ڿ�����ú󣬿�ʼ���ò���
static struct fb_info *s3c_lcd;
static int lcd_init(void)
{
	//1. ����һ��fb_info�ṹ��ο�s3c2410fb.c
	s3c_lcd = framebuffer_alloc(0�� NULL); //������˽�����ݵĶ���ռ䣬����������Ҫ
						//�ڴ��п��ܲ�������Ҫ�жϣ�����������ʱ���ж�

	//2. ����  �ο�fb.h
	//2.1 ���ù̶��Ĳ���
	strcpy(s3c_lcd->fix.id, "mylcd");
	s3c_lcd->fix.smem_len = 240*320*16/8;   //ÿ������16λ��ռ2���ֽ�
	s3c_lcd->fix.type         = FB_TYPE_PACKED_PIXELS;
	s3c_lcd->fix.visual       = FB_VISUAL_TRUECOLOR;  //���ɫ
	s3c_lcd->fix.line_length= 
							
	//2.2 ���ÿɱ�Ĳ���
	s3c_lcd->var.xres           =       240;
	s3c_lcd->var.yres           =       320;
	s3c_lcd->var.xres_virtual =       240;
	s3c_lcd->var.yres_virtual =       320;
	s3c_lcd->bits_per_pixel   =       16;    //16λ 
	

	// RGB:565
	s3c_lcd->var.red.offset   = 11;
	s3c_lcd->var.red.length   =   5;
	
	s3c_lcd->var.red.offset   =   5;
	s3c_lcd->var.red.length   =   6;      

	s3c_lcd->var.red.offset   = 	  0;
	s3c_lcd->var.red.length  =	  5;

	s3c_lcd->var.activate     =  FB_ACTIVATE_NOW  //Ĭ��ֵ 

	//2.3 ���ò������������������涨����static struct fb_ops s3c_lcdfb_ops 
	s3c_lcd->fbops             =  &s3c_lcdfb_ops
	
	
	//2.4 ����������
	// s3c_lcd->pseudo_palette  =   ; //�ٵĵ�ɫ��� 
	//s3c_lcd->screen_base      =;   //�Դ�������ַ
	s3c_lcd->screen_size       = 240*320*16/8;  //�ִ�Ĵ�С
	
	//3. Ӳ����صĲ���
	//3.1 ����GPIO����LCD
	//3.2 ����LCD�ֲ�����LCD����������vclkƵ�ʵȡ�
	//3.3 �����Դ�(framebuffer),���ѵ�ַ����LCD������
	s3c_lcd->smem_start = xxx; //�Դ�������ַ
	
	//4. ע��_���ú�����ע��
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
