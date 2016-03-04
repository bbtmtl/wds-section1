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
#include <asm/uaccess.h>
#include <asm/io.h>





static int major;




//6 д.probe���� Դ��static int _devinit gpio_keys_probe(struct platform_device *pdev)
//9 ����class_create ������  ioremap
//10 �����Դ resource
static struct class *cls;
static volatile unsigned long *gpio_con;  // step 10
static volatile unsigned long *gpio_dat;
static int pin;   //����

//11 led_open
static int led_open(struct inode *inode, struct file *file)

{
	
	// ����Ϊ�������
	*gpio_con &= ~(0x3 <<(pin*2));
	*gpio_dat |= (0x1 <<(pin*2));  //�������
	return 0;
}


//12 led_write  �ο�first_drv.c
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

//8 ��һ��ϸ��,����file_operations�ṹ
static struct file_operations led_fops ={
	.owner   =  THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
   	 .open    =  led_open,     
	.write    =  led_write,	   

};




static int led_probe(struct platform_device *pdev)
{
	struct resource *res;

	//����platform_device����Դ����ioremap,���������ŵ�

	res =platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio_con = ioremap(res->start, res->end - res->start +1);  //������Ϊioremap�Ĳ���������ʼ��ַ���ߴ��С�
	gpio_dat = gpio_con + 1;   //ָ��+1�൱�ڼ�4�� ָ����dat�Ĵ���

	res =platform_get_resource(pdev, IORESOURCE_IRQ, 0);  
	pin = res->start;
	
	
	//ע���ַ��豸��������
	printk("led_probe, found led\n");
	major = 	register_chrdev(0, "myled", &led_fops);
	
	cls = class_create(THIS_MODULE, "myled");

	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "led");  /* /dev/led ϵͳ�Զ������豸�ڵ�*/

	
	return 0;

}







//7 ж��
//13 ��ǰ��������Ҫж��
static int led_remove(struct platform_device *pdev)
{
	
	//ж���ַ��豸��������iounmap
	printk("led_remove, remove led\n");
	
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "myled");
	iounmap(gpio_con);
	
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

