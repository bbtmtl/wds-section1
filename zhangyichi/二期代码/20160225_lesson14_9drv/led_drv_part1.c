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

//6 写.probe函数 源码static int _devinit gpio_keys_probe(struct platform_device *pdev)
static int led_probe(struct platform_device *pdev)
{
	//根据platform_device的资源进行ioremap,如设置引脚等
	//注册字符设备驱动程序
	printk("led_probe, found led\n");
	return 0;

}

//7 卸载
static int led_remove(struct platform_device *pdev)
{
	
	//卸载字符设备驱动程序iounmap
	printk("led_remove, remove led\n");
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

