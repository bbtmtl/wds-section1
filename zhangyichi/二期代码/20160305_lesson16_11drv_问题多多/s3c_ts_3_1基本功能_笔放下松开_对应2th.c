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

//用于 4.2 用于ioremap来操作寄存器，寄存器名称参见手册68页
struct s3c_ts_regs{
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};
	
static struct input_dev *s3c_ts_dev;
static  volatile struct s3c_ts_regs *s3c_ts_regs;  //定义指针，指向ioremap.与4.2呼应

static void enter_wait_pen_down_mode(void)
{
	s3c_ts_regs->adctsc = 0xd3;  //参见手册442页，第四种等待中断模式设置ADCTSC = 0Xd3  0b11010011
}

static void enter_wait_pen_up_mode(void)
{
	s3c_ts_regs->adctsc = 0x1d3;  //参见手册442页，第四种等待中断模式设置ADCTSC = 0Xd3
}

static irqreturn_t pen_down_up_irq(int irq, void *dev_id)

{
	if (s3c_ts_regs->adcdat0 & (1<<15))
	{
		printk("pen up\n");  //松开才能等待下一次等待模式
		enter_wait_pen_down_mode();
	}
	else
	{
		printk("pen down\n"); //按下进入等待松开模式
		enter_wait_pen_up_mode();
	}
	return IRQ_HANDLED;
}


//第二步  开始写入口函数 
static int s3c_ts_init(void)
{
	struct clk* clk; 
	
//第五步 细化，把要做的东西写下来，以下框架为所有触摸屏程序的框架
		

	// 1.分配一个input_dev结构体
 	s3c_ts_dev = input_allocate_device();

 	// 2.设置，分两大类 参考8th Driver buttons.c
	// 2.1 能产生哪类事件
	set_bit(EV_KEY, s3c_ts_dev->evbit);  //EV_KEY表示会产生按键类事件，buttons_dev->evbit表示设置数组总的某一位可以产生案件类事件。
	set_bit(EV_ABS, s3c_ts_dev->evbit); //绝对位移 
	
	// 2.2 能产生这类操作中的哪些事件  
	set_bit(BTN_TOUCH,  s3c_ts_dev->keybit); //能够产生按键类事件中的触摸屏事件

	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);  //最大尺寸为0x3ff.
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);// 笔压力仅设置为两种 0和1.
	
	
 	// 3.注册
 	input_register_device (s3c_ts_dev);
	
 	// 4. 硬件相关的操作 硬件手册16章 
 	// 4.1 使能时钟(也就是设置CLKCON的bit[15])
 	clk = clk_get(NULL, "abc");
	clk_enable(clk);
	

	// 4.2 设置s3c2440的ADC (TS寄存器)
	// 操作TS寄存器，需要进行ioremap. 这样一个个的进行ioremap多麻烦， 
	// 写一个结构体就比较简单struct s3c_ts_regs.
	s3c_ts_regs = ioremap(0x58000000, sizeof(struct s3c_ts_regs)); //物理地址
	// 以下参数设置参照手册 444页
	// bit[14]    1-A/D converter prescaler enable 欲分频使能
	// bit[13:6]  1-A/D converter prescaler value 欲分频系数 
	//            设为49   ADCCLK = PCLK/(49+1) = 50MHz/(49+1)= 1MHz.0
	// bit[5:3]   模拟输入频道选择，暂时不设置
	// bit[2]     STDBM
	// bit[0]     A/D conversion starts by enable  先设为0；
	
	s3c_ts_regs->adccon = (1<<14) | (49<<6);

	request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL); //先设置一个中断，看触摸屏是否有动作，参考笔记 

	enter_wait_pen_down_mode(); //增加该函数别忘了在前面声明，等待触摸笔按下模式

	return 0;
}

//第三步  开始写出口函数
static int s3c_ts_exit(void)
{
	free_irq(IRQ_TC, NULL);
	iounmap(s3c_ts_regs);
	input_unregister_device(s3c_ts_dev);
	input_free_device(s3c_ts_dev);	
}

//第四步  修饰 协议
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");


