#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/device.h>     //class_device_create
//#include <asm-generic/iomap.h>
//#include <asm-generic/io.h>
//#include <asm-generic/gpio.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <mach/regs-gpio.h>
//#include <led_zxb_module.h>


#define DEVICE_NAME "led_zxb"
#define LED_MAJOR	118
//#define LED_MINOR	minor
#define led_in_out	1    //0 as input--read, 1 as output--write

//static struct class *leddrv_class;
//static struct class_device *leddrv_class_dev;

static int led_open(struct inode *inode, struct file *file);

ssize_t  led_read(struct file *file, char __user *buf, size_t count, loff_t *offp);

ssize_t led_write(struct file *file, char __user *buf, size_t count, loff_t *offp);

volatile unsigned long *gpbcon = NULL;
volatile unsigned long *gpbdat = NULL;

volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;

static struct file_operations led_fops = {
	.owner	=	THIS_MODULE,
	.open	=	led_open,
	.read	=	led_read,
	.write	=	led_write,
};


static int led_open(struct inode *inode, struct file *file)
{	
	//if(1) //1 as output--write
//	{	//clear the register as zero
//		printk(KERN_ALERT "enter the write led_open\n");

		*gpbcon &= ~(0x3<<(5*2) | 0x3<<(6*2) |0x3<<(7*2) |   0x3<<(8*2));
		//set as output--1
		*gpbcon |= 0x1<<(5*2) | 0x1<<(6*2) |0x1<<(7*2) |0x1<<(8*2);
//	}
//	else{	//clear the register as zero ---input--read
		*gpgcon &= ~(0x3<<(3*2) | 0x3<<(5*2) |0x3<<(6*2) |   0x3<<(7*2));
//		*gpbcon &= ~(0x3<<(5*2) | 0x3<<(6*2) |0x3<<(7*2) |   0x3<<(8*2));
//	}
	return 0;
}

ssize_t  led_read(struct file *file, char __user *buf, size_t count, loff_t *offp)
{	unsigned char key_val[4];
	int reg_val;
	
	if(count !=sizeof(key_val))
		return -EINVAL;

//read gpb5,gpb6,gpb7gpb8
	reg_val = *gpgdat;
	
	key_val[0] = (reg_val & (1<<7)) ? 1 : 0;
	key_val[1] = (reg_val & (1<<6)) ? 1 : 0;

	reg_val = *gpgdat;
	key_val[2] = (reg_val & (1<<5)) ? 1 : 0;
	key_val[3] = (reg_val & (1<<3)) ? 1 : 0;

	if(copy_to_user(buf,key_val,sizeof(key_val)))
	{
		printk(KERN_ERR"the driver can only give one key value 			once!\n");
		return -ENOMEM;
	}
	return sizeof(key_val);
}

ssize_t led_write(struct file *file, char __user *buf, size_t count, loff_t *offp)
{	int val;
	copy_from_user(&val, buf, count);
	
	switch(val){
		case 1111:
			printk("all light on\n");
			*gpbdat &= ~(1<<5 | 1<<6 |1<<7 |1<<8);
			break;
		case 0:
			printk("all light off\n");
			*gpbdat |= 1<<5 | 1<<6 |1<<7 |1<<8;
			break;
		case 1100:
			printk("light on led1 & led2\n");
			*gpbdat |= 1<<5 | 1<<6 |1<<7 |1<<8;
			*gpbdat &= ~(1<<5 | 1<<6);	
			break;
		case 0011:
			printk("light on led3 & led4\n");
			*gpbdat |= 1<<5 | 1<<6 |1<<7 |1<<8;
			*gpbdat &= ~(1<<7 | 1<<8);
			break;	
		case 1001:
			printk("light on led1 & led4\n");
			*gpbdat |= 1<<5 | 1<<6 |1<<7 |1<<8;
			*gpbdat &= ~(1<<5 | 1<<8);
			break;
		default:
			break;
	}
	return 0;
}

//int major;
static int __init led_init(void)
{	
	register_chrdev(118, "led_zxb", &led_fops);
	
	printk(KERN_ALERT "Initial the module\n");
//	leddrv_class = class_create(THIS_MODULE, "led_zxb");
//2.6 use device_create() instead of class_device_create()
//	leddrv_class_dev = device_create(leddrv_class, NULL,
//			MKDEV(major, 0), NULL, "led_zxb");

	gpbcon = (volatile unsigned long *)ioremap(0x56000010, 16);
	gpbdat = gpbcon + 1;	
	
	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;	
	return 0;
}

static void __exit led_exit(void)
{	
	unregister_chrdev(118, "led_zxb");

//2.4 	class_device_unregister(leddrv_class_dev);
	//2.6
//	device_destroy(leddrv_class_dev);
//	class_destroy(leddrv_class);

	iounmap(gpbcon);
	iounmap(gpgcon);
	printk(KERN_ALERT "exit the module\n");
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zxb");
