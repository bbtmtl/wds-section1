//led_dev.c

//����/����/ע��һ��platform_device
//ͷ�ļ�������mach-smdk2440.c 
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <asm/arch/regs-serial.h>

#include <asm/plat-s3c24xx/devs.h>
#include <asm/plat-s3c24xx/cpu.h>

#include <asm/plat-s3c24xx/common-smdk.h>
#include <asm/arch/fb.h>



//step2  �ο�mach-smdk2440.c (arch\arm\mach-s3c2440��static struct resource s3c2410_sdi_resource[] = {  )
/* SDI  �Ժ������յ�Ƶ�����ֻҪ�Ķ�����ļĴ�����ַ����Ӧ���ż���*/
static struct resource led_resource[] = {
    [0] = {   //�Ĵ�����ַ
        .start = 0x56000050,   //�ο��first_drv
        .end   = 0x56000050 +8 - 1,  //������ַ  -1��ԭ����Ҫ���
        .flags = IORESOURCE_MEM,   //flags�Ǳ���������Щ��Դ���������IRQ
    },
    [1] = {   //�ĸ�����
        .start = 4,     //�ĸ����ţ�����ο��first_drv��4
        .end   = 4,
        .flags = IORESOURCE_IRQ,
    }

};

// 5 release ����
static void led_release (struct device * dev)
{

}



//step1:�ο�mach-smdk2440.c (arch\arm\mach-s3c2440��static struct platform_device s3c2410_device_sdi = {  )
static struct platform_device led_dev = {    
    .name         = "myled",
    .id       = -1,
    .num_resources    = ARRAY_SIZE(led_resource),
    .resource     = led_resource,
    .dev = {
    			.release = led_release,
					},  //release ������ϵͳҪ��ģ�������������
};

//step3 ��ں���
static int led_dev_init(void)
{
	platform_device_register(&led_dev);  //ע��һ��ƽ̨�豸
	return 0;
}


// step4 ���ں���
static int led_dev_exit(void)
{
	platform_device_unregister(&led_dev);  //ж��һ��ƽ̨�豸
	return 0;


}


module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");
