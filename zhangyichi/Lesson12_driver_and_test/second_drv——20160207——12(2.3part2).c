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



static struct class *seconddrv_class;
static struct class_device	*seconddrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static int second_drv_open(struct inode *inode, struct file *file)

{
	/* 配置 GPF0 2 为输入引脚 */
	*gpfcon &= ~((0x3 <<(0*2))| (0x3<<(2*2)));
	
	
	/* 配置 GPG3 11 为输入引脚 */
	*gpgcon &= ~((0x3 <<(0*2))| (0x3<<(2*2));
	
	
	return 0;
}

 ssize_t second_drv_read(struct file *file,  char __user *buf, size_t size, loff_t * ppos)
{
	/*返回4个引脚的电平 */
	unsigned char key_vals[4];
	int regval;

	if  (size != sizeof(key_vals))
		return -EINVAL;
	/*读GPF0 2 */
	regval =  *gpfdat;
	key_vals[0] = (regval & (1 << 0))? 1 :0;
	key_vals[1] = (regval & (1 << 2))? 1 :0;

	/*读GPG3 11 */
	regval =  *gpgdat;
	key_vals[2] = (regval & (1 << 0))? 1 :0;
	key_vals[3] = (regval & (1 << 2))? 1 :0;

	copy_to_user(buf, key_vals, sizeof(key_vals));
	return sizeof(key_vals);
}



/* first step */
static struct file_operations second_drv_fops = {
    .owner  =   THIS_MODULE,  
    .open   =    second_drv_open,     
    .read	=	 second_drv_read,	
};


/* register file operation */
	int major;
	static	int second_drv_init(void)  //entrance 
	{
		major = register_chrdev(0, "second_drv",&second_drv_fops);
		
		seconddrv_class = class_create(THIS_MODULE, "seconddrv");  
	
	      seconddrv_class_dev = class_device_create(seconddrv_class, NULL, MKDEV(major,0), NULL,  "buttons");  /* create /dev/buttons device */

		gpfcon = (volatile unsigned long *) ioremap (0x56000050,16);
		gpfdat = gpfcon + 1; 	

		gpgcon = (volatile unsigned long *) ioremap (0x56000060,16);
		gpgdat = gpgcon + 1; 	

		return ;	
	
	}


static	int second_drv_exit(void)  //exit 
	{
		unregister_chrdev(major, "second_drv");
		class_device_unregister(seconddrv_class_dev);
		class_destroy(seconddrv_class);
		iounmap(gpfcon);
		iounmap(gpgcon);
		
		return ;		
	}

module_init(second_drv_init);
module_exit(second_drv_exit);

MODULE_LICENSE("GPL");

