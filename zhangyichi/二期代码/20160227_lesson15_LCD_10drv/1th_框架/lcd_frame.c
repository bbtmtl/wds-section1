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

static struct fb_info *s3c_lcd;
static int lcd_init(void)
{
	//1. 分配一个fb_info结构体参考s3c2410fb.c
	s3c_lcd = framebuffer_alloc(0， NULL); //可以有私有数据的额外空间，本驱动不需要
						//内存有可能不够，需要判断，但本驱动暂时不判断

	//2. 设置  参考fb.h
	//2.1 设置固定的参数
	//2.2 设置可变的参数
	//2.3 设置操作参数
	//2.4 其他的设置
	
	
	//3. 硬件相关的操作
	//3.1 配置GPIO用于LCD
	//3.2 根据LCD手册设置LCD控制器，如vclk频率等。
	//3.3 分配显存(framebuffer),并把地址告诉LCD控制器
	//4. 注册_设置好了再注册
	register_framebuffer(s3c_lcd);

	
	return 0;

}

static int lcd_exit(void)
{
	return 0 ;

	
}


module_init(lcd_init);
modul_exit(lcd_exit);

MODULE_LICENSE("GPL");
