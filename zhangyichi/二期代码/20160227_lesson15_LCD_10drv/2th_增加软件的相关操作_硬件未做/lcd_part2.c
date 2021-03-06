//lcd.c 20160227

//头文件来自于s3c2410fb.c(driers\vidoe)

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

//来自于atmel_lcdfb.c(drivers\video)
static struct fb_ops s3c_lcdfb_ops = {  //对显存的操作 
	.owner		= THIS_MODULE,
	//.fb_setcolreg	= atmel_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,    //填充矩形
	.fb_copyarea	= cfb_copyarea, //拷贝区域
	.fb_imageblit	= cfb_imageblit,
};


// Step1 在框架做好后，开始设置参数
static struct fb_info *s3c_lcd;
static int lcd_init(void)
{
	//1. 分配一个fb_info结构体参考s3c2410fb.c
	s3c_lcd = framebuffer_alloc(0， NULL); //可以有私有数据的额外空间，本驱动不需要
						//内存有可能不够，需要判断，但本驱动暂时不判断

	//2. 设置  参考fb.h
	//2.1 设置固定的参数
	strcpy(s3c_lcd->fix.id, "mylcd");
	s3c_lcd->fix.smem_len = 240*320*16/8;   //每个像素16位，占2个字节
	s3c_lcd->fix.type         = FB_TYPE_PACKED_PIXELS;
	s3c_lcd->fix.visual       = FB_VISUAL_TRUECOLOR;  //真彩色
	s3c_lcd->fix.line_length= 
							
	//2.2 设置可变的参数
	s3c_lcd->var.xres           =       240;
	s3c_lcd->var.yres           =       320;
	s3c_lcd->var.xres_virtual =       240;
	s3c_lcd->var.yres_virtual =       320;
	s3c_lcd->bits_per_pixel   =       16;    //16位 
	

	// RGB:565
	s3c_lcd->var.red.offset   = 11;
	s3c_lcd->var.red.length   =   5;
	
	s3c_lcd->var.red.offset   =   5;
	s3c_lcd->var.red.length   =   6;      

	s3c_lcd->var.red.offset   = 	  0;
	s3c_lcd->var.red.length  =	  5;

	s3c_lcd->var.activate     =  FB_ACTIVATE_NOW  //默认值 

	//2.3 设置操作参数，数组在上面定义了static struct fb_ops s3c_lcdfb_ops 
	s3c_lcd->fbops             =  &s3c_lcdfb_ops
	
	
	//2.4 其他的设置
	// s3c_lcd->pseudo_palette  =   ; //假的调色板� 
	//s3c_lcd->screen_base      =;   //显存的虚拟地址
	s3c_lcd->screen_size       = 240*320*16/8;  //现存的大小
	
	//3. 硬件相关的操作
	//3.1 配置GPIO用于LCD
	//3.2 根据LCD手册设置LCD控制器，如vclk频率等。
	//3.3 分配显存(framebuffer),并把地址告诉LCD控制器
	s3c_lcd->smem_start = xxx; //显存的物理地址
	
	//4. 注册_设置好了再注册
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
