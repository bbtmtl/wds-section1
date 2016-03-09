//·ÖÅä/ÉèÖÃ/×¢²áÒ»¸öplatform_driver

/* led_drv.c */

//Í·ÎÄ¼şÀ´×ÔÓÚgpio_keys.c
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
#include <asm/uaccess.h>
#include <asm/io.h>





static int major;




//6 Ğ´.probeº¯Êı Ô´Âëstatic int _devinit gpio_keys_probe(struct platform_device *pdev)
//9 Ôö¼Óclass_create ´´½¨Àà  ioremap
//10 »ñµÃ×ÊÔ´ resource
static struct class *cls;
static volatile unsigned long *gpio_con;  // step 10
static volatile unsigned long *gpio_dat;
static int pin;   //Òı½Å

//11 led_open
static int led_open(struct inode *inode, struct file *file)

{
	
	// ÅäÖÃÎªÊä³öÒı½Å
	*gpio_con &= ~(0x3 <<(pin*2));
	*gpio_dat |= (0x1 <<(pin*2));  //Êä³öÒı½Å
	return 0;
}


//12 led_write  ²Î¿¼first_drv.c
static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	copy_from_user(&val,buf,count);
	if (val ==1)
		{
			*gpio_dat &= ~(1<<pin);
		}
	else
		{
			*gpio_dat  |=(1<< pin);
		}
	// printk("first_drv_write\n");
	return 0;
}

//8 ½øÒ»²½Ï¸»¯,´´½¨file_operations½á¹¹
static struct file_operations led_fops ={
	.owner   =  THIS_MODULE,    /* ÕâÊÇÒ»¸öºê£¬ÍÆÏò±àÒëÄ£¿éÊ±×Ô¶¯´´½¨µÄ__this_module±äÁ¿ */
   	 .open    =  led_open,     
	.write    =  led_write,	   

};




static int led_probe(struct platform_device *pdev)
{
	struct resource *res;

	//¸ù¾İplatform_deviceµÄ×ÊÔ´½øĞĞioremap,ÈçÉèÖÃÒı½ÅµÈ

	res =platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio_con = ioremap(res->start, res->end - res->start +1);  //À¨ºÅÖĞÎªioremapµÄ²ÎÊı°üÀ¨¿ªÊ¼µØÖ·£¬³ß´ç´óĞ¡£
	gpio_dat = gpio_con + 1;   //Ö¸Õë+1Ïàµ±ÓÚ¼Ó4£¬ Ö¸ÏòÁËdat¼Ä´æÆ÷

	res =platform_get_resource(pdev, IORESOURCE_IRQ, 0);  
	pin = res->start;
	
	
	//×¢²á×Ö·ûÉè±¸Çı¶¯³ÌĞò
	printk("led_probe, found led\n");
	major = 	register_chrdev(0, "myled", &led_fops);
	
	cls = class_create(THIS_MODULE, "myled");

	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "led");  /* /dev/led ÏµÍ³×Ô¶¯´´½¨Éè±¸½Úµã*/

	
	return 0;

}







//7 Ğ¶ÔØ
//13 ÒÔÇ°´´½¨µÄÀàÒªĞ¶ÔØ
static int led_remove(struct platform_device *pdev)
{
	
	//Ğ¶ÔØ×Ö·ûÉè±¸Çı¶¯³ÌĞòiounmap
	printk("led_remove, remove led\n");
	
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "myled");
	iounmap(gpio_con);
	
	return 0;

}




//5 ¶¨ÒåÒ»¸öÆ½Ì¨driver  Ô´Âëgpio_keys.c struct platform_driver gpio_keys_device_driver 
//********&&&&&& Èç¹ûÃû×ÖÏàÍ¬£¬driver¾ÍÈÏÎª¿ÉÒÔÖ§³ÖÕâ¸ödevice.²Å»áµ÷ÓÃÀïÃæµÄ.probeº¯Êı
struct platform_driver led_drv= {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",     //Ö»ÓĞÍ¬ÃûµÄ,Èç¹ûÃû×ÖÏàÍ¬£¬driver¾ÍÈÏÎª¿ÉÒÔÖ§³ÖÕâ¸ödevice.²Å»áµ÷ÓÃÀïÃæµÄ.probeº¯Êı
	}
};





//step1 Èë¿Úº¯Êı
//8 ³äÊµÈë¿Úº¯Êı    ×¢²áÒ»¸öÆ½Ì¨driver
static int led_drv_init(void)
{
	platform_driver_register(&led_drv);   //×¢²áÒ»¸öÆ½Ì¨driver
	return 0;


}

//step2  ³ö¿Úº¯Êı 
// 9 ³äÊµ³ö¿Úº¯Êı  Ğ¶ÔØÒ»¸öÆ½Ì¨Çı¶¯
static int led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);  //Ğ¶ÔØÒ»¸öÆ½Ì¨Driver
	return 0;

}

//step3 ĞŞÊÎ
module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");

