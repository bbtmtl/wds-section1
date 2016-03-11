//  5th  第2次优化   从升级优化措施3开始
// 优化措施3总体思路说明
/*
遵照流程，一开始进入init中的等待中断模式enter_wait_pen_down_mode，
触摸笔按下set_bit(EV_KEY, s3c_ts_dev->evbit); set_bit(EV_ABS, s3c_ts_dev->evbit)后
进入pen_down_up_irq中断，该函数被调用
	如果继续按下，进入该函数中的测量xy模式 enter_measure_xy_mode()和启动ADC start_adc()
ADC成功后，进入中断adc_irq处理函数，在该函数里确定adcdat0 = s3c_ts_regs->adcdat0和adcdat1 = s3c_ts_regs->adcdat1的值
	该函数内，如果松开，则再次进入等待触摸笔进入模式enter_wait_pen_down_mode()
	否则，仍继续按下的话，则保存如下值x[cnt] = s3c_ts_regs->adcdat0 & 0x3ff和y[cnt] = s3c_ts_regs->adcdat1 & 0x3ff;
		累加++cnt， 够4次的话，打印出来，然后再次进入触摸笔松开模式enter_wait_pen_up_mode()
		否则的话，再次进入测量xy模式enter_measure_xy_mode()，启动ADC start_adc().


*/


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

// 3th 存在问题 1 长按是中间过程不能处理，值不够精确 2 电压变化较大，还需优化
// 处理问题1， 增加计时器  参见4.2之优化措施
// 优化措施2 在static irqreturn_t adc_irq中，丢弃不精确的结果

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

static void enter_wait_pen_down_mode(void)  // 等待触摸笔放下模式
{
	s3c_ts_regs->adctsc = 0xd3;  //参见手册442页，第四种等待中断模式设置ADCTSC = 0Xd3  0b11010011
}

static void enter_wait_pen_up_mode(void)
{
	s3c_ts_regs->adctsc = 0x1d3;  //参见手册442页，第四种等待中断模式设置ADCTSC = 0Xd3
}

// 在else里面已经启动了自动分离模式，所以进入该模式和启动该模式的函数需要注册。
//启动需要时间，启动后需要中断来告知，所以还需要在 s3c_ts_init里面做一个中断。
static void enter_measure_xy_mode(void)
{
 	// 怎样启动进入自动测量xy坐标模式呢 手册445页 NOTES2
 	// AUTO_PST bit should be set '1' only in Automatic & Sequential X/Y Position conversion.
 	// bit3 should be disable .设置为1.
 	s3c_ts_regs->adctsc =(1<<3) |(1<<2);   
}

static void start_adc(void)  // 如何启动，看ADCCON寄存器  设置bit[0]为1，即可启动
{
	s3c_ts_regs->adccon  |=(1<<0);
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
	// 3th 开始，进行启动ADC，转换x,y坐标
	//printk("pen down\n"); //按下进入等待松开模式
	//enter_wait_pen_up_mode();
	// 触摸屏有xy分别分离模式和自动分离模式，我们取后一种自动分离模式，这种简单。
	enter_measure_xy_mode();
	start_adc();
	}
	return IRQ_HANDLED;
}

// 3th里面的自动分离模式启动后需要的中断函数注册，具体函数内容在4.2里面
static irqreturn_t adc_irq(int irq, void *dev_id)
{

	static int cnt = 0;
	static int x[4],   y[4]; // 优化措施3  测量保存4次，然后求平均值
	int adcdat0, adcdat1;  // 寄存器的bit[15]是判断松开还是按下的  手册447

	// X,Y的值存在ADCDAT0 AND ADCDAT1寄存器里面  手册442  part3
	// 最低10位为x坐标值，不过这个值是电压值而已，和坐标值没有关系
		
	//  &&&&&&&&&&&&***********  如果ADC启动完成时，发现触摸笔已经松开，则测量值已经不准确，丢弃此次结果。
		
	adcdat0 = s3c_ts_regs->adcdat0 ;
	adcdat1 = s3c_ts_regs->adcdat1 ;

	if (s3c_ts_regs->adcdat0 & (1<<15))
	{
		// 如果触摸笔已经松开
		cnt = 0;  // 优化3 中要每4次求一次平均值
		enter_wait_pen_down_mode();
	}
	else
	{
		//printk ("adc_irq cnt = %d, x = %d,	y = %d\n", ++cnt, s3c_ts_regs->adcdat0 & 0x3ff, adcdat1 & 0x3ff);
		// *******运行到这里，如果没有等待松开操作，则开发板只进行一次测量后就没有反映了，所以在这里要加上如下语句。

		// 优化措施3 多次测量求平均值
		x[cnt] = s3c_ts_regs->adcdat0 & 0x3ff;
		y[cnt] = s3c_ts_regs->adcdat1 & 0x3ff;
		++cnt;
		if (cnt == 4)
		{
			//printk ("adc_irq cnt = %d, x = %d,	y = %d\n", ++cnt, (x[0]+x[1]+x[2]+x[3])/4,  (y[0]+y[1]+y[2]+y[3])/4);
			// 上面这行错误代码，错误在 ++cnt.
			printk (" x = %d,	y = %d\n", (x[0]+x[1]+x[2]+x[3])/4,  (y[0]+y[1]+y[2]+y[3])/4);
			cnt = 0;
			enter_wait_pen_up_mode();			
		}
		else
		{
			enter_measure_xy_mode();
			start_adc();	
		}

		enter_wait_pen_up_mode();  // 测量完毕要等待触摸笔松开

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
	
	// 3th增加的自动分离测量模式，需要注册启动后的中断
	request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);
	

	//&&&&&&&&&&%%%%%%% 优化措施1
	// 手册446 触摸屏有一个ADCDLY寄存器，设置为最大值，这使得电压稳定后再发出中断
	s3c_ts_regs->adcdly = 0xffff;
	

	enter_wait_pen_down_mode(); //增加该函数别忘了在前面声明，等待触摸笔按下模式

	return 0;
}

//第三步  开始写出口函数
static int s3c_ts_exit(void)
{
	free_irq(IRQ_TC, NULL);
	free_irq(IRQ_ADC, NULL); // 机器死机，忘记释放中断。
	iounmap(s3c_ts_regs);
	input_unregister_device(s3c_ts_dev);
	input_free_device(s3c_ts_dev);	
}

//第四步  修饰 协议
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");


