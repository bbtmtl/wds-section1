
// 8th driver 20160224
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
#include <linux/gpio_keys.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>




//step4 part2  4 Ӳ����صĲ������ο�seventh_drv�Ĵ��� ���ּ���
struct pin_desc             // �ο����߸�����buttons.c
	{ 
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};

//step4 part3   4 Ӳ����صĲ������ο�seventh_drv�Ĵ��� ���ּ���
//�ο����߸�����buttons.c  �ն˺ţ��ĸ����ţ����֣�����ֵ

struct pin_desc pins_desc[4] = {  
	{IRQ_EINT0,  "S2", S3C2410_GPF0, KEY_L},// {S3C2410_GPF0, 0x01},
	{IRQ_EINT2,  "S3", S3C2410_GPF2, KEY_S},// {S3C2410_GPF2, 0x02},
	{IRQ_EINT11, "S4", S3C2410_GPG3, KEY_ENTER},// {S3C2410_GPG3, 0x03},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},// {S3C2410_GPG11, 0x04},
};

static struct input_dev *buttons_dev;
static struct pin_desc *irq_pd;
static struct timer_list buttons_timer;


// Step 4 part2  ������ʱ��
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	// 10ms��������ʱ��
	irq_pd = (struct pin_desc  *)dev_id;
	mod_timer(&buttons_timer,jiffies+HZ/100);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

// Step 4 part3  �ϱ� input_event
static void buttons_timer_function(unsigned long data)
	{
		/*ԭ�������� struct pin_desc * pindesc = (struct pin_desc *) irq_pd; */
			struct pin_desc * pindesc =  irq_pd;
			unsigned int pinval;
	
			if (!pindesc)
				return ;
			
			pinval = s3c2410_gpio_getpin(pindesc->pin);  //������ֵ
		
			if (pinval)
			{
				/* �ɿ� ���һ������ 0 ��ʾ�ɿ���1��ʾ����*/
				input_event(buttons_dev, EV_KEY,  pindesc->key_val, 0);   //�ϱ��¼�
				input_sync(buttons_dev);  //�ϱ�һ��ͬ���¼�
			}
			else
			{
				/* ���� */
				input_event(buttons_dev,EV_KEY, pindesc->key_val, 1);
				input_sync(buttons_dev);
	    	       }
		
	
}


//1step д��ں���
static int buttons_init(void)
	
{
	int i;
	
	/*step4 */
	// 1 ����һ��input_device�ṹ��
	buttons_dev =  input_allocate_device();; /*�ο�gpio�е�ö��gpio_keys_probe,������δ�жϷ���ֵ */

	// 2 ����
	
	// 2.1 �ܲ��������¼�
	set_bit(EV_KEY, buttons_dev->evbit);     //EV_KEY��ʾ������������¼���buttons_dev->evbit��ʾ���������ܵ�ĳһλ���Բ����������¼���
	set_bit(EV_REP, buttons_dev->evbit);    //�����ظ����¼��������°���ʱ�䳤������ܶ���ظ�ֵ

	//2.2 �ܲ��������¼������Щ�¼��أ���Ϊ�������¼��кö��֣���Ҫȷ�� L S ENTER LEFTSHIFT    
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER,buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);
	
	// 3 ע��
	input_register_device(buttons_dev);

	
	// 4 Ӳ����صĲ�����ע���жϣ��ο�seventh_drv�Ĵ���

	init_timer(&buttons_timer);  //timer��ʼ��
	buttons_timer.function  =  buttons_timer_function;
	//����ǰ����add_timer;
	add_timer(&buttons_timer);

	for (i=0; i<4; i++)   //����ǰi=1;
		{                                 //�ն˺ţ�      �жϴ�������        ����
			request_irq(pins_desc[i].irq, buttons_irq, IRQT_BOTHEDGE, pins_desc[i].name,  &pins_desc[i]);
		}   //����ǰ��һ��pin_dest[i].name ���������ţ� ���pin_desc[0]
	
	return 0;
}

//2step д���ں���
//5step ������ں�������
static void buttons_exit(void)
{
	int i;
	for (i = 0; i<4; i++)      //�ͷ�
		{
			free_irq(pins_desc[i].irq,  &pins_desc[i]);
		}
	
	del_timer(&buttons_timer);   //ɾ���Timer
	input_unregister_device(buttons_dev);  //ע��ķ���
	input_free_device(buttons_dev);   //����Ŀռ��ͷŵ�
	
	//����ǰ��return ;
}



// 3step ��ں������ں���ֻ����ͨ��������Ҫ����һ��
module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");























