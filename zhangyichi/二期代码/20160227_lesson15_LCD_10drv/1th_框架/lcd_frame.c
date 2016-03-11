//lcd.c 20160227

//ͷ�ļ�������s3c2410fb.c(driers\vidoe)

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include "s3c2410fb.h"

static struct fb_info *s3c_lcd;
static int lcd_init(void)
{
	//1. ����һ��fb_info�ṹ��ο�s3c2410fb.c
	s3c_lcd = framebuffer_alloc(0�� NULL); //������˽�����ݵĶ���ռ䣬����������Ҫ
						//�ڴ��п��ܲ�������Ҫ�жϣ�����������ʱ���ж�

	//2. ����  �ο�fb.h
	//2.1 ���ù̶��Ĳ���
	//2.2 ���ÿɱ�Ĳ���
	//2.3 ���ò�������
	//2.4 ����������
	
	
	//3. Ӳ����صĲ���
	//3.1 ����GPIO����LCD
	//3.2 ����LCD�ֲ�����LCD����������vclkƵ�ʵȡ�
	//3.3 �����Դ�(framebuffer),���ѵ�ַ����LCD������
	//4. ע��_���ú�����ע��
	register_framebuffer(s3c_lcd);

	
	return 0;

}

static int lcd_exit(void)
{
	return 0 ;

	
}


module_init(lcd_init);
modul_exit(lcd_exit);

MODULE_LICENSE("GPL");
