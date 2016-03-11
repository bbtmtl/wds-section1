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


static struct class *sixdrv_class;
static struct class_device	*sixdrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，six_drv_read将它清0 */
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


static DECLARE_MUTEX(button_lock);  //定义互斥锁


/*
  * 确定按键值
  */
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
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

    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */

	kill_fasync (&button_async, SIGIO, POLL_IN);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int six_drv_open(struct inode *inode, struct file *file)

{
	//获取信号量

	down(&button_lock);

		
	/* 配置GPF0,2为输入引脚 */
	/* 配置GPG3,11为输入引脚 */
	request_irq(IRQ_EINT0,  buttons_irq, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	request_irq(IRQ_EINT2,  buttons_irq, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5", &pins_desc[3]);	

	return 0;
}

 


	 
ssize_t six_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	/* 如果没有按键动作, 休眠 */
	wait_event_interruptible(button_waitq, ev_press);

	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;  }


int six_drv_close(struct inode *inode, struct file *file)
{
	// atomic_inc(&canopen);   //关闭设备时加1
	
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
	up(&button_lock);   //释放掉信号量
	return 0;
}

static unsigned six_drv_poll(struct file *file, poll_table *wait)
{
	unsigned  int mask = 0;
	poll_wait(file, &button_waitq, wait);   //不会立即休眠

	if (ev_press)
		mask |= POLLIN  |   POLLRDNORM;
	

	return mask;
}

static int six_drv_fasync(int fd, struct file *filp, int on)
{
	printk("driver: six_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}


static struct file_operations six_drv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  six_drv_open,     
	.read	 =	six_drv_read,	   
	.release =  six_drv_close,	   
	.poll    =    six_drv_poll,
	.fasync =   six_drv_fasync,
};




int major;
static int six_drv_init(void)
{
	major = register_chrdev(0, "six_drv", &six_drv_fops);

	sixdrv_class = class_create(THIS_MODULE, "six_drv");

	sixdrv_class_dev = class_device_create(sixdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void six_drv_exit(void)
{
	unregister_chrdev(major, "six_drv");
	class_device_unregister(sixdrv_class_dev);
	class_destroy(sixdrv_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}


module_init(six_drv_init);
module_exit(six_drv_exit);

MODULE_LICENSE("GPL");

