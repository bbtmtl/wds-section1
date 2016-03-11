// ��һ��  ����s3c2410_ts.c ͷ�ļ�
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/plat-s3c24xx/ts.h>

#include <asm/arch/regs-adc.h>
#include <asm/arch/regs-gpio.h>

//�ڶ���  ��ʼд��ں��� 
static int s3c_ts_init(void)
{
//���岽 ϸ������Ҫ���Ķ���д���������¿��Ϊ���д���������Ŀ��
 	// 1.����һ��input_dev�ṹ��
 	s3c_ts_dev = input_alloc_device();

 	// 2.���ã��������� �ο�8th Driver buttons.c
	// 2.1 �ܲ��������¼�
	set_bit(EV_KEY, s3c_ts_dev->evbit);  //EV_KEY��ʾ������������¼���buttons_dev->evbit��ʾ���������ܵ�ĳһλ���Բ����������¼���
	set_bit(EV_ABS, s3c_ts_dev->evbit); //����λ�� 
	
	// 2.2 �ܲ�����������е���Щ�¼�  
	set_bit_bit(BTN_TOUCH,  s3c_ts_dev->keybit); //�ܹ������������¼��еĴ������¼�

	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);  //���ߴ�Ϊ0x3ff.
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);// ��ѹ��������Ϊ���� 0��1.
	
	
 	// 3.ע��
 	input_register_device (s3c_ts_dev);
 	// 4.
	
	

	return 0;
}

//������  ��ʼд���ں���
statci int s3c_ts_exit(void)
{
	return 0;
}

//���Ĳ�  ���� Э��
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");


