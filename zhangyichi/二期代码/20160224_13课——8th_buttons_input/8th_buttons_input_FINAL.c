
// 8th driver 20160224
#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/gpio_keys.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>




//step4 part2  4 硬件相关的操作，参考seventh_drv的代码 部分继续
struct pin_desc             // 参考第七个驱动buttons.c
	{ 
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};

//step4 part3   4 硬件相关的操作，参考seventh_drv的代码 部分继续
//参考第七个驱动buttons.c  终端号，哪个引脚，名字，按键值

struct pin_desc pins_desc[4] = {  
	{IRQ_EINT0,  "S2", S3C2410_GPF0, KEY_L},// {S3C2410_GPF0, 0x01},
	{IRQ_EINT2,  "S3", S3C2410_GPF2, KEY_S},// {S3C2410_GPF2, 0x02},
	{IRQ_EINT11, "S4", S3C2410_GPG3, KEY_ENTER},// {S3C2410_GPG3, 0x03},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},// {S3C2410_GPG11, 0x04},
};

static struct input_dev *buttons_dev;
static struct pin_desc *irq_pd;
static struct timer_list buttons_timer;


// Step 4 part2  启动定时器
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	// 10ms后启动定时器
	irq_pd = (struct pin_desc  *)dev_id;
	mod_timer(&buttons_timer,jiffies+HZ/100);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

// Step 4 part3  上报 input_event
static void buttons_timer_function(unsigned long data)
	{
		/*原崩溃程序 struct pin_desc * pindesc = (struct pin_desc *) irq_pd; */
			struct pin_desc * pindesc =  irq_pd;
			unsigned int pinval;
	
			if (!pindesc)
				return ;
			
			pinval = s3c2410_gpio_getpin(pindesc->pin);  //读引脚值
		
			if (pinval)
			{
				/* 松开 最后一个参数 0 表示松开，1表示按下*/
				input_event(buttons_dev, EV_KEY,  pindesc->key_val, 0);   //上报事件
				input_sync(buttons_dev);  //上报一个同步事件
			}
			else
			{
				/* 按下 */
				input_event(buttons_dev,EV_KEY, pindesc->key_val, 1);
				input_sync(buttons_dev);
	    	       }
		
	
}


//1step 写入口函数
static int buttons_init(void)
	
{
	int i;
	
	/*step4 */
	// 1 分配一个input_device结构体
	buttons_dev =  input_allocate_device();; /*参考gpio中的枚举gpio_keys_probe,本例中未判断返回值 */

	// 2 设置
	
	// 2.1 能产生哪类事件
	set_bit(EV_KEY, buttons_dev->evbit);     //EV_KEY表示会产生按键类事件，buttons_dev->evbit表示设置数组总的某一位可以产生案件类事件。
	set_bit(EV_REP, buttons_dev->evbit);    //产生重复的事件，即按下按键时间长会产生很多个重复值

	//2.2 能产生这类事件里的哪些事件呢，因为按键类事件有好多种，需要确认 L S ENTER LEFTSHIFT    
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER,buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);
	
	// 3 注册
	input_register_device(buttons_dev);

	
	// 4 硬件相关的操作，注册中断，参考seventh_drv的代码

	init_timer(&buttons_timer);  //timer初始化
	buttons_timer.function  =  buttons_timer_function;
	//崩溃前代码add_timer;
	add_timer(&buttons_timer);

	for (i=0; i<4; i++)   //崩溃前i=1;
		{                                 //终端号，      中断处理函数，        名字
			request_irq(pins_desc[i].irq, buttons_irq, IRQT_BOTHEDGE, pins_desc[i].name,  &pins_desc[i]);
		}   //崩溃前上一行pin_dest[i].name 外面有引号， 最后pin_desc[0]
	
	return 0;
}

//2step 写出口函数
//5step 补充出口函数内容
static void buttons_exit(void)
{
	int i;
	for (i = 0; i<4; i++)      //释放
		{
			free_irq(pins_desc[i].irq,  &pins_desc[i]);
		}
	
	del_timer(&buttons_timer);   //删除錞imer
	input_unregister_device(buttons_dev);  //注册的反向
	input_free_device(buttons_dev);   //分配的空间释放掉
	
	//崩溃前有return ;
}



// 3step 入口函数出口函数只是普通函数，需要修饰一下
module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");























