//This is my first drver update.  20160204

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






static int first_drv_open(struct inode *inode, struct file *file)

{
	printk("first_drv_open\n");
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	printk("first_drv_write\n");
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
	if (IS_ERR(firstdrv_class))  //判断
		return PTR_ERR(firstdrv_class);

	firstdrv_class_devs[0] = class_device_create(leds_class, NULL, MKDEV(major, 0), NULL, "zyz");
	if (unlikely(IS_ERR(firstdrv_class_devs[minor])))
			return PTR_ERR(firstdrv_class_devs[minor]);
	return ;

}

static void  first_drv_exit(void)  //exit
{
	unregister_chrdev(major, "first_drv");  //卸载
	class_device_unregister(firstdrv_class_dev;
	class_destroy(firstdrv_class)
	return 0;

}

module_init(first_drv_init);
module_exit(first_drv_exit);
