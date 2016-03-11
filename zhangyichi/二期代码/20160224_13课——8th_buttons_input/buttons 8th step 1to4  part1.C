
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

//step4 part2   4 硬件相关的操作，参考seventh_drv的代码 部分继续
struct pin_desc{    // 参考第七个驱动buttons.c
	int irq;
	unsigned int pin;
	unsigned int _key_val;
	char *name;
		
};

//step4 part3   4 硬件相关的操作，参考seventh_drv的代码 部分继续

struct pin_desc pins_desc[4] = {  //参考第七个驱动buttons.c  终端号，哪个引脚，名字，按键值
	{IRQ_EINT0,  "S2", S3C2410_GPF0, KEY_L},// {S3C2410_GPF0, 0x01},
	{IRQ_EINT2,  "S3", S3C2410_GPF2, KEY_S},// {S3C2410_GPF2, 0x02},
	{IRQ_EINT11, "S4", S3C2410_GPG3, KEY_ENTER},// {S3C2410_GPG3, 0x03},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},// {S3C2410_GPG11, 0x04},
};





static struct input_dev *buttons_dev;
static strcut pin_desc *irq_pd;
static struct timer_list buttons_timer;


// Step 4 part2  启动定时器
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	//10ms后启动定时器
	irq_pd = (struct pin_desc  *)dev_id;
	mod_timer(&buttons_timer,jiffies+HZ/100);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

// Step 4 part3  上报 input_event
static void buttons_timer_function(unsigned long data)
{
	


}


//1step 写入口函数
static int buttons_init(void)
	int i;
{
	/*step4 */
	// 1 分配一个input_device结构体
	buttons_dev =  input_allocate_device();  /*参考gpio中的枚举gpio_keys_probe,本例中未判断返回值 */

	// 2 设置
	// 2.1 能产生哪类事件
	set_bit(EV_KEY, buttons_dev->evbit);     //EV_KEY表示会产生按键类事件，buttons_dev->evbit表示设置数组总的某一位可以产生案件类事件。

	//2.2 能产生这类事件里的哪些事件呢，因为按键类事件有好多种，需要确认 L S ENTER LEFTSHIFT    
	set_bit_bit(KEY_L, buttons_dev->keybit);
	set_bit_bit(KEY_S, buttons_dev->keybit);
	set_bit_bit(KEY_ENTER buttons_dev->keybit);
	set_bit_bit(KEY_LEFTSHIFT, buttons_dev->keybit);
	
	// 3 注册
	input_register_device(buttons_dev);

	
	// 4 硬件相关的操作，注册中断，参考seventh_drv的代码

	init_timer(&buttons_timer);  //timer初始化
	buttons_timer.function  =  buttons_timer_function;
	add_timer;

	for (i=1; i<4; i++)
		{                                 //终端号，      中断处理函数，        名字
			request_irq(pins_desc[i].irq, buttons_irq, IRQT_BOTHEDGE, "pins_desc[i].name",  &pins_desc[0]);
		}
	
	
	
	
	

	return 0;
}

//2step 写出口函数

static void buttons_exit(void)
{
	return 0;
}



// 3step 入口函数出口函数只是普通函数，需要修饰一下
module_init(buttons_init);
module_init(buttons_exit);

MODULE_LECENSE("GPL")






















#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/poll.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>


static struct class *sixthdrv_class;
static struct class_device	*sixthdrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static struct timer_list buttons_timer;    //定义一个结构体


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，sixth_drv_read将它清0 */
static volatile int ev_press = 0;

static struct fasync_struct *button_async;



struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};


/* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */
/* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

struct pin_desc pins_desc[4] = {
	{S3C2410_GPF0, 0x01},
	{S3C2410_GPF2, 0x02},
	{S3C2410_GPG3, 0x03},
	{S3C2410_GPG11, 0x04},
};


static struct pin_desc *irq_pd;
static DECLARE_MUTEX(button_lock);  //定义互斥锁


/*
  * 确定按键值
  */
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	//10ms后启动定时器
	irq_pd = (struct pin_desc  *)dev_id;
	mod_timer(&buttons_timer,jiffies+HZ/100);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int sixth_drv_open(struct inode *inode, struct file *file)

{
	if (file->f_flags & O_NONBLOCK)
		{
			if(down_trylock(&button_lock))
			return -EBUSY;	
		}
	else
		{
			//获取信号量

			down(&button_lock);
		}

	

		
	/* 配置GPF0,2为输入引脚 */
	/* 配置GPG3,11为输入引脚 */
	request_irq(IRQ_EINT0,  buttons_irq, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	request_irq(IRQ_EINT2,  buttons_irq, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5", &pins_desc[3]);	

	return 0;
}

 


	 
ssize_t sixth_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	if (file->f_flags & O_NONBLOCK)
		{
			if(!ev_press)
			return -EAGAIN;	
		}
	else
		{
			/* 如果没有按键动作, 休眠 */
			wait_event_interruptible(button_waitq, ev_press);
		}

	

	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;  }


int sixth_drv_close(struct inode *inode, struct file *file)
{
	// atomic_inc(&canopen);   //关闭设备时加1
	
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
	up(&button_lock);   //释放掉信号量
	return 0;
}

static unsigned sixth_drv_poll(struct file *file, poll_table *wait)
{
	unsigned  int mask = 0;
	poll_wait(file, &button_waitq, wait);   //不会立即休眠

	if (ev_press)
		mask |= POLLIN  |   POLLRDNORM;
	

	return mask;
}

static int sixth_drv_fasync(int fd, struct file *filp, int on)
{
	printk("driver: sixth_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}


static struct file_operations sixth_drv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  sixth_drv_open,     
	.read	 =	sixth_drv_read,	   
	.release =  sixth_drv_close,	   
	.poll    =    sixth_drv_poll,
	.fasync =   sixth_drv_fasync,
};




int major;

static void buttons_timer_function (unsigned long data)
{
	struct pin_desc * pindesc = (struct pin_desc *) irq_pd;
		unsigned int pinval;

		if (!pindesc)
			return ;
		
		pinval = s3c2410_gpio_getpin(pindesc->pin);
	
		if (pinval)
		{
			/* 松开 */
			key_val = 0x80 | pindesc->key_val;
		}
		else
		{
			/* 按下 */
			key_val = pindesc->key_val;
		}
	
		ev_press = 1;				   /* 表示中断发生了 */
		wake_up_interruptible(&button_waitq);	/* 唤醒休眠的进程 */
	
		kill_fasync (&button_async, SIGIO, POLL_IN);



}
static int sixth_drv_init(void)
{
	init_timer (&buttons_timer);  //初始化定时器
	// buttons_timer.data         =    (unsigned long ) SCpnt;
	//buttons_timer.expires      =   jiffies + 100*HZ     /*10S  中断里面，暂时不用*/
	buttons_timer.function     =  buttons_timer_function; //设置处理函数 
	add_timer(&buttons_timer);    //把定时器告诉了内核

	major = register_chrdev(0, "sixth_drv", &sixth_drv_fops);

	sixthdrv_class = class_create(THIS_MODULE, "sixth_drv");

	sixthdrv_class_dev = class_device_create(sixthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void sixth_drv_exit(void)
{
	unregister_chrdev(major, "sixth_drv");
	class_device_unregister(sixthdrv_class_dev);
	class_destroy(sixthdrv_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}


module_init(sixth_drv_init);
module_exit(sixth_drv_exit);

MODULE_LICENSE("GPL");

