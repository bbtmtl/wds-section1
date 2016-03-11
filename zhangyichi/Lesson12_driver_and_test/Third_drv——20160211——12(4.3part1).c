#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/mach-types.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/interrupt.h>








static struct class *Thirddrv_class;
static struct class_device	*Thirddrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	printk("irq = %d\n" , irq);
	return IRQ_HANDLED;
}


static int Third_drv_open(struct inode *inode, struct file *file)
{
	/* 配置GPF0,2为输入引脚 */
	
	/* 配置GPG3,11为输入引脚 */
	request_irq(IRQ_EINT0,buttons_irq,IRQT_BOTHEDGE, "s2",1);
	request_irq(IRQ_EINT2,buttons_irq,IRQT_BOTHEDGE, "s3",1);
	request_irq(IRQ_EINT11,buttons_irq,IRQT_BOTHEDGE, "s4",1);	
	request_irq(IRQ_EINT19,buttons_irq,IRQT_BOTHEDGE, "s5",1);

	return 0;
}

ssize_t Third_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	/* 返回4个引脚的电平 */
	unsigned char key_vals[4];
	int regval;

	if (size != sizeof(key_vals))
		return -EINVAL;

	/* 读GPF0,2 */
	regval = *gpfdat;
	key_vals[0] = (regval & (1<<0)) ? 1 : 0;
	key_vals[1] = (regval & (1<<2)) ? 1 : 0;
	

	/* 读GPG3,11 */
	regval = *gpgdat;
	key_vals[2] = (regval & (1<<3)) ? 1 : 0;
	key_vals[3] = (regval & (1<<11)) ? 1 : 0;

	copy_to_user(buf, key_vals, sizeof(key_vals));
	
	return sizeof(key_vals);
}

int Third_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0, 1);
	free_irq(IRQ_EINT2, 1);
	free_irq(IRQ_EINT11, 1);
	free_irq(IRQ_EINT19, 1);

	return 0;
}
	
static struct file_operations Third_drv_fops = {
    .owner     =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open       =   Third_drv_open,     
    .read	     =   Third_drv_read,	
    .release   =   Third_drv_close,
};


int major;
static int Third_drv_init(void)
{
	major = register_chrdev(0, "Third_drv", &Third_drv_fops);

	Thirddrv_class = class_create(THIS_MODULE, "Third_drv");

	Thirddrv_class_dev = class_device_create(Thirddrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void Third_drv_exit(void)
{
	unregister_chrdev(major, "Third_drv");
	class_device_unregister(Thirddrv_class_dev);
	class_destroy(Thirddrv_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}


module_init(Third_drv_init);

module_exit(Third_drv_exit);

MODULE_LICENSE("GPL");

