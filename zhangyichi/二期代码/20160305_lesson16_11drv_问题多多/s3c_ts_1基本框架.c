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


