//����/����/ע��һ��platform_driver

/* led_drv.c */

//ͷ�ļ�������gpio_keys.c
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

//6 д.probe���� Դ��static int _devinit gpio_keys_probe(struct platform_device *pdev)
static int led_probe(struct platform_device *pdev)
{
	//����platform_device����Դ����ioremap,���������ŵ�
	//ע���ַ��豸��������
	printk("led_probe, found led\n");
	return 0;

}

//7 ж��
static int led_remove(struct platform_device *pdev)
{
	
	//ж���ַ��豸��������iounmap
	printk("led_remove, remove led\n");
	return 0;

}




//5 ����һ��ƽ̨driver  Դ��gpio_keys.c struct platform_driver gpio_keys_device_driver 
//********&&&&&& ���������ͬ��driver����Ϊ����֧�����device.�Ż���������.probe����
struct platform_driver led_drv= {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",     //ֻ��ͬ����,���������ͬ��driver����Ϊ����֧�����device.�Ż���������.probe����
	}
};





//step1 ��ں���
//8 ��ʵ��ں���    ע��һ��ƽ̨driver
static int led_drv_init(void)
{
	platform_driver_register(&led_drv);   //ע��һ��ƽ̨driver
	return 0;


}

//step2  ���ں��� 
// 9 ��ʵ���ں���  ж��һ��ƽ̨����
static int led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);  //ж��һ��ƽ̨Driver
	return 0;

}

//step3 ����
module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");

