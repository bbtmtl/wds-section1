/*sam_led.c*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
//#include <asm/irq.h> //no irq involved here
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

#define DEVICE_NAME	"sam_led"	/* may see device by cat /proc/devices after insmod*/
#define LED_MAJOR	168	/* device major #*/

static char kernel_buff[2];
static void __iomem *base_addr0;
//static unsigned long base_addr0;

#define S3C2440_GPB (0x56000010)
#define gpbcon	(*(volatile unsigned long *)(base_addr0 + 0x00))
#define gpbdat	(*(volatile unsigned long *)(base_addr0 + 0x04))


static int sam_led_open(struct inode *inode, struct file *filp){
	printk("[Sam] sam_led_open!\n");
gpbcon	 &= ~((0x03 << (2*5)) | (0x03 << (2*6)) | (0x03 << (2*7)) | (0x03 << (2*8)));
gpbcon	 |=  ((0x01 << (2*5)) | (0x01 << (2*6)) | (0x01 << (2*7)) | (0x01 << (2*8)));
	return 0;
}

static int sam_led_write(struct file *filp,const char __user *user_buff,size_t count,loff_t *f_ops){
	int ret;
	ret = copy_from_user(kernel_buff,user_buff,count);{
	printk("[Sam] sam_led_write!LED_# = %c\n", kernel_buff[0]);//%c is char
		switch(kernel_buff[0]){
		case '1':{
			if	(kernel_buff[1]=='0')
				{printk("LED1 turns off");
				gpbdat	|=  (0x01 << 5);//LED1 turn off
			   	} 
			else
			   	{printk("LED1 turns on");
				gpbdat  &= ~(0x01 << 5);//LED1 turn on
			   	} 
			}
		break;


		case '2':{
			if	(kernel_buff[1]=='0')
                           	{printk("LED2 turns off");
				gpbdat |=  (0x01 << 6);//LED2 turn off
			   	}
			else
                        	{printk("LED2 turns on");
				gpbdat &= ~(0x01 << 6);//LED2 turn on
			   	}
			}
		break;

		case '3':{
			if	(kernel_buff[1]=='0')
				{printk("LED3 turns off");
				gpbdat |=  (0x01 << 7);//LED3 turn off
			   	}
			else
				{printk("LED3 turns on");
				gpbdat  &= ~(0x01 << 7);//LED3 turn on
			   	}	
			}
		break;

		default:{
			if	(kernel_buff[1]=='0')
				{printk("LED4 turns off");
				gpbdat  |=  (0x01 << 8);//LED4 turn     off
                             	}
			else
                        	{printk("LED4 turns on");
                                gpbdat  &= ~(0x01 << 8);//LED4 turn     on
                             	}
                         }
                 break;

		}
	}
    return count;
} 


static struct file_operations sam_led_fops = {
    .owner   =   THIS_MODULE,
    .open    =   sam_led_open,     
    .write   =   sam_led_write,	   
};


int sam_led_init(void){
	int ret;
	base_addr0 = ioremap(S3C2440_GPB, 0x10);
	gpbcon	 &= ~((0x03 << (2*5)) | (0x03 << (2*6)) | (0x03 << (2*7)) | (0x03 << (2*8)));
	gpbcon	 |=  ((0x01 << (2*5)) | (0x01 << (2*6)) | (0x01 << (2*7)) | (0x01 << (2*8)));
	gpbdat	 |= ((0x01 << 5) | (0x01 << 6) | (0x01 << 7) | (0x01 << 8));//all LEDs off
	ret = register_chrdev(LED_MAJOR, DEVICE_NAME, &sam_led_fops);
    printk("[Sam] sam_led_init!\n");
	
    return ret;
}

void sam_led_exit(void){
	gpbcon   &= ~((0x03 << (2*5)) | (0x03 << (2*6)) | (0x03 << (2*7)) | (0x03 << (2*8)));
        gpbcon   |=  ((0x01 << (2*5)) | (0x01 << (2*6)) | (0x01 << (2*7)) | (0x01 << (2*8)));
        gpbdat   &= ~((0x01 << 5) | (0x01 << 6) | (0x01 << 7) | (0x01 << 8));//all LEDs on
	gpbdat   |= ((0x01 << 5) | (0x01 << 6) | (0x01 << 7) | (0x01 << 8));//all LEDs off
	unregister_chrdev(LED_MAJOR, DEVICE_NAME);
    	printk("[Sam] sam_led_exit!\n");
	
}

module_init(sam_led_init);
module_exit(sam_led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sam Zhou");