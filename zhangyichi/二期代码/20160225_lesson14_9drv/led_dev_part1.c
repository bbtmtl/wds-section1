//led_dev.c

//分配/设置/注册一个platform_device
//头文件来自于mach-smdk2440.c 
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



//step2  参考mach-smdk2440.c (arch\arm\mach-s3c2440的static struct resource s3c2410_sdi_resource[] = {  )
/* SDI  以后针对哪盏灯的亮灭，只要改动下面的寄存器地址和相应引脚即可*/
static struct resource led_resource[] = {
    [0] = {   //寄存器地址
        .start = 0x56000050,   //参考錰irst_drv
        .end   = 0x56000050 +8 - 1,  //结束地址  -1是原程序要求的
        .flags = IORESOURCE_MEM,   //flags是表明属于哪些资源，这里借用IRQ
    },
    [1] = {   //哪个引脚
        .start = 4,     //哪个引脚，这里参考錰irst_drv是4
        .end   = 4,
        .flags = IORESOURCE_IRQ,
    }

};

// 5 release 函数
static void led_release (struct device * dev)
{

}



//step1:参考mach-smdk2440.c (arch\arm\mach-s3c2440的static struct platform_device s3c2410_device_sdi = {  )
static struct platform_device led_dev = {    
    .name         = "myled",
    .id       = -1,
    .num_resources    = ARRAY_SIZE(led_resource),
    .resource     = led_resource,
    .dev = {
    			.release = led_release,
					},  //release 函数是系统要求的，否则会产生错误。
};

//step3 入口函数
static int led_dev_init(void)
{
	platform_device_register(&led_dev);  //注册一个平台设备
	return 0;
}


// step4 出口函数
static int led_dev_exit(void)
{
	platform_device_unregister(&led_dev);  //卸载一个平台设备
	return 0;


}


module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");
