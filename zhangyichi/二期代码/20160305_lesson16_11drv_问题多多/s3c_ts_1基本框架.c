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


