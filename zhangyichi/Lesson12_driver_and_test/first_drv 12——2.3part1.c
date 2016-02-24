//This is my first drver update.  20160204_12(2.3)

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


static struct class *firstdrv_class;
static struct class_device	*firstdrv_class_devs[4];
volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;







static int first_drv_open(struct inode *inode, struct file *file)

{
	//printk("first_drv_open\n");
	*gpfcon &= ~((0x3 <<(4*2))| (0x3<<(5*2))|(0x3<<(6*2)));
	*gpfdat |= (0x1 <<(4*2))| (0x1<<(5*2))|(0x1<<(6*2));
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	copy_from_user(&val,buf,count);
	if (val ==1)
		{
			*gpfdat |=(1<<4)|(1<<5)|(1<<6);
		}
	else
		{
			*gpfdat &= ~((1<<4)|(1<<5)|(1<<6));
		}
	// printk("first_drv_write\n");
	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =    first_drv_open,     
	.write	=	 first_drv_write,	


};

int major;
static  int first_drv_init(void)  //entrance 
{
	major = register_chrdev(0, "first_drv",&first_drv_fops);

	firstdrv_class = class_create(THIS_MODULE, "firstdrv");  //生成系统信息,建立一个类,在这个类下面生成设备MKV
	
	firstdrv_class_devs[0] = class_device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "zyz");
	gpfcon = (volatile unsigned long *) ioremap (0x5000050,16);
	gpfdat = gpfcon + 1;
	
	return ;

}

static void  first_drv_exit(void)  //exit
{
	unregister_chrdev(major, "first_drv");  //卸载
	class_device_unregister(firstdrv_class_devs);
	class_destroy(firstdrv_class);
	iounmap(gpfcon);
	return ;

}

module_init(first_drv_init);
module_exit(first_drv_exit);

MODULE_LICENSE("GPL");

