// 第一步  拷贝s3c2410_ts.c 头文件
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/plat-s3c24xx/ts.h>

#include <asm/arch/regs-adc.h>
#include <asm/arch/regs-gpio.h>

//第二步  开始写入口函数 
static int s3c_ts_init(void)
{
//第五步 细化，把要做的东西写下来，以下框架为所有触摸屏程序的框架
 	// 1.分配一个input_dev结构体
 	s3c_ts_dev = input_alloc_device();

 	// 2.设置，分两大类 参考8th Driver buttons.c
	// 2.1 能产生哪类事件
	set_bit(EV_KEY, s3c_ts_dev->evbit);  //EV_KEY表示会产生按键类事件，buttons_dev->evbit表示设置数组总的某一位可以产生案件类事件。
	set_bit(EV_ABS, s3c_ts_dev->evbit); //绝对位移 
	
	// 2.2 能产生这类操作中的哪些事件  
	set_bit_bit(BTN_TOUCH,  s3c_ts_dev->keybit); //能够产生按键类事件中的触摸屏事件

	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);  //最大尺寸为0x3ff.
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);// 笔压力仅设置为两种 0和1.
	
	
 	// 3.注册
 	input_register_device (s3c_ts_dev);
 	// 4.
	
	

	return 0;
}

//第三步  开始写出口函数
statci int s3c_ts_exit(void)
{
	return 0;
}

//第四步  修饰 协议
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");


