//  5th  ��2���Ż�   �������Ż���ʩ3��ʼ
// �Ż���ʩ3����˼·˵��
/*
�������̣�һ��ʼ����init�еĵȴ��ж�ģʽenter_wait_pen_down_mode��
�����ʰ���set_bit(EV_KEY, s3c_ts_dev->evbit); set_bit(EV_ABS, s3c_ts_dev->evbit)��
����pen_down_up_irq�жϣ��ú���������
	����������£�����ú����еĲ���xyģʽ enter_measure_xy_mode()������ADC start_adc()
ADC�ɹ��󣬽����ж�adc_irq���������ڸú�����ȷ��adcdat0 = s3c_ts_regs->adcdat0��adcdat1 = s3c_ts_regs->adcdat1��ֵ
	�ú����ڣ�����ɿ������ٴν���ȴ������ʽ���ģʽenter_wait_pen_down_mode()
	�����Լ������µĻ����򱣴�����ֵx[cnt] = s3c_ts_regs->adcdat0 & 0x3ff��y[cnt] = s3c_ts_regs->adcdat1 & 0x3ff;
		�ۼ�++cnt�� ��4�εĻ�����ӡ������Ȼ���ٴν��봥�����ɿ�ģʽenter_wait_pen_up_mode()
		����Ļ����ٴν������xyģʽenter_measure_xy_mode()������ADC start_adc().


*/


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

// 3th �������� 1 �������м���̲��ܴ���ֵ������ȷ 2 ��ѹ�仯�ϴ󣬻����Ż�
// ��������1�� ���Ӽ�ʱ��  �μ�4.2֮�Ż���ʩ
// �Ż���ʩ2 ��static irqreturn_t adc_irq�У���������ȷ�Ľ��

//���� 4.2 ����ioremap�������Ĵ������Ĵ������Ʋμ��ֲ�68ҳ
struct s3c_ts_regs{
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};
	
static struct input_dev *s3c_ts_dev;
static  volatile struct s3c_ts_regs *s3c_ts_regs;  //����ָ�룬ָ��ioremap.��4.2��Ӧ

static void enter_wait_pen_down_mode(void)  // �ȴ������ʷ���ģʽ
{
	s3c_ts_regs->adctsc = 0xd3;  //�μ��ֲ�442ҳ�������ֵȴ��ж�ģʽ����ADCTSC = 0Xd3  0b11010011
}

static void enter_wait_pen_up_mode(void)
{
	s3c_ts_regs->adctsc = 0x1d3;  //�μ��ֲ�442ҳ�������ֵȴ��ж�ģʽ����ADCTSC = 0Xd3
}

// ��else�����Ѿ��������Զ�����ģʽ�����Խ����ģʽ��������ģʽ�ĺ�����Ҫע�ᡣ
//������Ҫʱ�䣬��������Ҫ�ж�����֪�����Ի���Ҫ�� s3c_ts_init������һ���жϡ�
static void enter_measure_xy_mode(void)
{
 	// �������������Զ�����xy����ģʽ�� �ֲ�445ҳ NOTES2
 	// AUTO_PST bit should be set '1' only in Automatic & Sequential X/Y Position conversion.
 	// bit3 should be disable .����Ϊ1.
 	s3c_ts_regs->adctsc =(1<<3) |(1<<2);   
}

static void start_adc(void)  // �����������ADCCON�Ĵ���  ����bit[0]Ϊ1����������
{
	s3c_ts_regs->adccon  |=(1<<0);
}
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)

{
	if (s3c_ts_regs->adcdat0 & (1<<15))
	{
		printk("pen up\n");  //�ɿ����ܵȴ���һ�εȴ�ģʽ
		enter_wait_pen_down_mode();
	}
	else
	{
	// 3th ��ʼ����������ADC��ת��x,y����
	//printk("pen down\n"); //���½���ȴ��ɿ�ģʽ
	//enter_wait_pen_up_mode();
	// ��������xy�ֱ����ģʽ���Զ�����ģʽ������ȡ��һ���Զ�����ģʽ�����ּ򵥡�
	enter_measure_xy_mode();
	start_adc();
	}
	return IRQ_HANDLED;
}

// 3th������Զ�����ģʽ��������Ҫ���жϺ���ע�ᣬ���庯��������4.2����
static irqreturn_t adc_irq(int irq, void *dev_id)
{

	static int cnt = 0;
	static int x[4],   y[4]; // �Ż���ʩ3  ��������4�Σ�Ȼ����ƽ��ֵ
	int adcdat0, adcdat1;  // �Ĵ�����bit[15]���ж��ɿ����ǰ��µ�  �ֲ�447

	// X,Y��ֵ����ADCDAT0 AND ADCDAT1�Ĵ�������  �ֲ�442  part3
	// ���10λΪx����ֵ���������ֵ�ǵ�ѹֵ���ѣ�������ֵû�й�ϵ
		
	//  &&&&&&&&&&&&***********  ���ADC�������ʱ�����ִ������Ѿ��ɿ��������ֵ�Ѿ���׼ȷ�������˴ν����
		
	adcdat0 = s3c_ts_regs->adcdat0 ;
	adcdat1 = s3c_ts_regs->adcdat1 ;

	if (s3c_ts_regs->adcdat0 & (1<<15))
	{
		// ����������Ѿ��ɿ�
		cnt = 0;  // �Ż�3 ��Ҫÿ4����һ��ƽ��ֵ
		enter_wait_pen_down_mode();
	}
	else
	{
		//printk ("adc_irq cnt = %d, x = %d,	y = %d\n", ++cnt, s3c_ts_regs->adcdat0 & 0x3ff, adcdat1 & 0x3ff);
		// *******���е�������û�еȴ��ɿ��������򿪷���ֻ����һ�β������û�з�ӳ�ˣ�����������Ҫ����������䡣

		// �Ż���ʩ3 ��β�����ƽ��ֵ
		x[cnt] = s3c_ts_regs->adcdat0 & 0x3ff;
		y[cnt] = s3c_ts_regs->adcdat1 & 0x3ff;
		++cnt;
		if (cnt == 4)
		{
			//printk ("adc_irq cnt = %d, x = %d,	y = %d\n", ++cnt, (x[0]+x[1]+x[2]+x[3])/4,  (y[0]+y[1]+y[2]+y[3])/4);
			// �������д�����룬������ ++cnt.
			printk (" x = %d,	y = %d\n", (x[0]+x[1]+x[2]+x[3])/4,  (y[0]+y[1]+y[2]+y[3])/4);
			cnt = 0;
			enter_wait_pen_up_mode();			
		}
		else
		{
			enter_measure_xy_mode();
			start_adc();	
		}

		enter_wait_pen_up_mode();  // �������Ҫ�ȴ��������ɿ�

	}

	
	return IRQ_HANDLED;
}



//�ڶ���  ��ʼд��ں��� 
static int s3c_ts_init(void)
{
	struct clk* clk; 
	
//���岽 ϸ������Ҫ���Ķ���д���������¿��Ϊ���д���������Ŀ��
		

	// 1.����һ��input_dev�ṹ��
 	s3c_ts_dev = input_allocate_device();

 	// 2.���ã��������� �ο�8th Driver buttons.c
	// 2.1 �ܲ��������¼�
	set_bit(EV_KEY, s3c_ts_dev->evbit);  //EV_KEY��ʾ������������¼���buttons_dev->evbit��ʾ���������ܵ�ĳһλ���Բ����������¼���
	set_bit(EV_ABS, s3c_ts_dev->evbit); //����λ�� 
	
	// 2.2 �ܲ�����������е���Щ�¼�  
	set_bit(BTN_TOUCH,  s3c_ts_dev->keybit); //�ܹ������������¼��еĴ������¼�

	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);  //���ߴ�Ϊ0x3ff.
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);// ��ѹ��������Ϊ���� 0��1.
	
	
 	// 3.ע��
 	input_register_device (s3c_ts_dev);
	
 	// 4. Ӳ����صĲ��� Ӳ���ֲ�16�� 
 	// 4.1 ʹ��ʱ��(Ҳ��������CLKCON��bit[15])
 	clk = clk_get(NULL, "abc");
	clk_enable(clk);
	

	// 4.2 ����s3c2440��ADC (TS�Ĵ���)
	// ����TS�Ĵ�������Ҫ����ioremap. ����һ�����Ľ���ioremap���鷳�� 
	// дһ���ṹ��ͱȽϼ�struct s3c_ts_regs.
	s3c_ts_regs = ioremap(0x58000000, sizeof(struct s3c_ts_regs)); //�����ַ
	// ���²������ò����ֲ� 444ҳ
	// bit[14]    1-A/D converter prescaler enable ����Ƶʹ��
	// bit[13:6]  1-A/D converter prescaler value ����Ƶϵ�� 
	//            ��Ϊ49   ADCCLK = PCLK/(49+1) = 50MHz/(49+1)= 1MHz.0
	// bit[5:3]   ģ������Ƶ��ѡ����ʱ������
	// bit[2]     STDBM
	// bit[0]     A/D conversion starts by enable  ����Ϊ0��
	
	s3c_ts_regs->adccon = (1<<14) | (49<<6);

	request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL); //������һ���жϣ����������Ƿ��ж������ο��ʼ� 
	
	// 3th���ӵ��Զ��������ģʽ����Ҫע����������ж�
	request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);
	

	//&&&&&&&&&&%%%%%%% �Ż���ʩ1
	// �ֲ�446 ��������һ��ADCDLY�Ĵ���������Ϊ���ֵ����ʹ�õ�ѹ�ȶ����ٷ����ж�
	s3c_ts_regs->adcdly = 0xffff;
	

	enter_wait_pen_down_mode(); //���Ӹú�����������ǰ���������ȴ������ʰ���ģʽ

	return 0;
}

//������  ��ʼд���ں���
static int s3c_ts_exit(void)
{
	free_irq(IRQ_TC, NULL);
	free_irq(IRQ_ADC, NULL); // ���������������ͷ��жϡ�
	iounmap(s3c_ts_regs);
	input_unregister_device(s3c_ts_dev);
	input_free_device(s3c_ts_dev);	
}

//���Ĳ�  ���� Э��
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");


