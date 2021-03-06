//分配/设置/注册一个platform_driver

/* led_drv.c */

//头文件来自于gpio_keys.c
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




//6 写.probe函数 源码static int _devinit gpio_keys_probe(struct platform_device *pdev)
//9 增加class_create 创建类  ioremap
//10 获得资源 resource
static struct class *cls;
static volatile unsigned long *gpio_con;  // step 10
static volatile unsigned long *gpio_dat;
static int pin;   //引脚

//11 led_open
static int led_open(struct inode *inode, struct file *file)

{
	
	// 配置为输出引脚
	*gpio_con &= ~(0x3 <<(pin*2));
	*gpio_dat |= (0x1 <<(pin*2));  //输出引脚
	return 0;
}


//12 led_write  参考first_drv.c
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

//8 进一步细化,创建file_operations结构
static struct file_operations led_fops ={
	.owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
   	 .open    =  led_open,     
	.write    =  led_write,	   

};




static int led_probe(struct platform_device *pdev)
{
	struct resource *res;

	//根据platform_device的资源进行ioremap,如设置引脚等

	res =platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio_con = ioremap(res->start, res->end - res->start +1);  //括号中为ioremap的参数包括开始地址，尺寸大小�
	gpio_dat = gpio_con + 1;   //指针+1相当于加4， 指向了dat寄存器

	res =platform_get_resource(pdev, IORESOURCE_IRQ, 0);  
	pin = res->start;
	
	
	//注册字符设备驱动程序
	printk("led_probe, found led\n");
	major = 	register_chrdev(0, "myled", &led_fops);
	
	cls = class_create(THIS_MODULE, "myled");

	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "led");  /* /dev/led 系统自动创建设备节点*/

	
	return 0;

}







//7 卸载
//13 以前创建的类要卸载
static int led_remove(struct platform_device *pdev)
{
	
	//卸载字符设备驱动程序iounmap
	printk("led_remove, remove led\n");
	
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "myled");
	iounmap(gpio_con);
	
	return 0;

}




//5 定义一个平台driver  源码gpio_keys.c struct platform_driver gpio_keys_device_driver 
//********&&&&&& 如果名字相同，driver就认为可以支持这个device.才会调用里面的.probe函数
struct platform_driver led_drv= {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",     //只有同名的,如果名字相同，driver就认为可以支持这个device.才会调用里面的.probe函数
	}
};





//step1 入口函数
//8 充实入口函数    注册一个平台driver
static int led_drv_init(void)
{
	platform_driver_register(&led_drv);   //注册一个平台driver
	return 0;


}

//step2  出口函数 
// 9 充实出口函数  卸载一个平台驱动
static int led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);  //卸载一个平台Driver
	return 0;

}

//step3 修饰
module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");

