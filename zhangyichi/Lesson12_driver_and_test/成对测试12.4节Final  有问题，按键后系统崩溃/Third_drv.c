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
#include <asm/mach-types.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/interrupt.h>








static struct class *Thirddrv_class;
static struct class_device	*Thirddrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

//�ж��¼���־���жϷ����������1��Third_drv_read������0
static volatile int ev_press = 0;



struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

//��ֵ  �� ����ʱ�� 0x01, 0x02  0x03   0x04
//��ֵ  �� �ɿ�ʱ�� 0x81, 0x82  0x83   0x84

static unsigned char key_val;
struct pin_desc pins_desc[4]={
		{S3C2410_GPF0,0X01},
		{S3C2410_GPF2,0X02},
		{S3C2410_GPG3,0X03},
		{S3C2410_GPG11,0X04},
};

//ȷ������ֵ

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = s3c2410_gpio_getpin(pindesc->pin);	//��ȡ��ֵ

	if (pinval)
		{
			//�ɿ�
			key_val = 0x80 |pindesc ->key_val;
		}
	else{
			//����
			key_val = pindesc ->key_val;
		}

	ev_press = 1;             //��ʾ�жϷ����ˡ�
	wake_up_interruptible(&button_waitq);   //�������ߵĽ���

	
	return IRQ_RETVAL(IRQ_HANDLED);
}


static int Third_drv_open(struct inode *inode, struct file *file)
{
	/* ����GPF0,2Ϊ�������� */
	
	/* ����GPG3,11Ϊ�������� */
	request_irq(IRQ_EINT0,buttons_irq,IRQT_BOTHEDGE, "s2",1);
	request_irq(IRQ_EINT2,buttons_irq,IRQT_BOTHEDGE, "s3",1);
	request_irq(IRQ_EINT11,buttons_irq,IRQT_BOTHEDGE, "s4",1);	
	request_irq(IRQ_EINT19,buttons_irq,IRQT_BOTHEDGE, "s5",1);

	return 0;
}

ssize_t Third_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	
	if (size != 1)
		return -EINVAL;

	//���û�а���������������������ߣ���ζ���ó�CPU.
	wait_event_interruptible(button_waitq, ev_press);
	
	//����а����������ͷ��ؼ�ֵ
	copy_to_user(buf,&key_val,1);
	ev_press = 1;
	
	return 1;
}

int Third_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0, 1);
	free_irq(IRQ_EINT2, 1);
	free_irq(IRQ_EINT11, 1);
	free_irq(IRQ_EINT19, 1);

	return 0;
}
	
static struct file_operations Third_drv_fops = {
    .owner     =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open       =   Third_drv_open,     
    .read	     =   Third_drv_read,	
    .release   =   Third_drv_close,
};


int major;
static int Third_drv_init(void)
{
	major = register_chrdev(0, "Third_drv", &Third_drv_fops);

	Thirddrv_class = class_create(THIS_MODULE, "Third_drv");

	Thirddrv_class_dev = class_device_create(Thirddrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void Third_drv_exit(void)
{
	unregister_chrdev(major, "Third_drv");
	class_device_unregister(Thirddrv_class_dev);
	class_destroy(Thirddrv_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}


module_init(Third_drv_init);

module_exit(Third_drv_exit);

MODULE_LICENSE("GPL");

