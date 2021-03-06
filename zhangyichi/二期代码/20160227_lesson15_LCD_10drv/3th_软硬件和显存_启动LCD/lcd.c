//lcd.c 20160227

//头文件来自于s3c2410fb.c(driers\vidoe)

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
#include <stdio.h>
#include "s3c24xx.h"
#include "lcddrv.h"


#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include "s3c2410fb.h"

//针对3.2液晶控制寄存器的说明如下
/*
3.2 根据LCD手册设置LCD控制器，如vclk频率等。
根据手册LCD CONTROLLER 396页的内容从头设到尾
具体内容见422页 LCD CONTROL REGISTOR
由于寄存器数量太多，因此都放在数组里面比较方便
注意他们各寄存器之间相差4个字节
_____跳跃太大的话，中间加一些保留字节
取自于3th 的lcd.c, 适用于2410，要与手册比较一下看是否有变化
在这个寄存器组上，2410完全和2440相同。
*/
//构建LCD控制寄存器数组　
struct lcd_regs {  //
		unsigned long	lcdcon1;
		unsigned long	lcdcon2;
		unsigned long	lcdcon3;
		unsigned long	lcdcon4;
		unsigned long	lcdcon5;
		unsigned long	lcdsaddr1;
		unsigned long	lcdsaddr2;
		unsigned long	lcdsaddr3;
		unsigned long	redlut;
		unsigned long	greenlut;
		unsigned long	bluelut;
		unsigned long	reserved[9]; // bluelut到dithmode地址分别为28和4c,相差0x36,即相差9，所以保留9字节
		unsigned long	dithmode;
		unsigned long	tpal;
		unsigned long	lcdintpnd;
		unsigned long	lcdsrcpnd;
		unsigned long	lcdintmsk;
		unsigned long	lpcsel;
	};





//来自于atmel_lcdfb.c(drivers\video)
static struct fb_ops s3c_lcdfb_ops = {  //对显存的操作 
	.owner		= THIS_MODULE,
	//.fb_setcolreg	= atmel_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,    //填充矩形
	.fb_copyarea	= cfb_copyarea, //拷贝区域
	.fb_imageblit	= cfb_imageblit,
};


// Step1 在框架做好后，开始设置参数
static struct fb_info *s3c_lcd;
static volatile unsigned long *gpbcon; //构建寄存器结构体指针
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;
static volatile struct lcd_regs* lcd_regs; //指向哪里呢，在3.2里ioremap

static int lcd_init(void)
{
	//1. 分配一个fb_info结构体参考s3c2410fb.c
	s3c_lcd = framebuffer_alloc(0， NULL); //可以有私有数据的额外空间，本驱动不需要
						//内存有可能不够，需要判断，但本驱动暂时不判断

	//2. 设置  参考fb.h***********************
	//2.1 设置固定的参数
	strcpy(s3c_lcd->fix.id, "mylcd");
	s3c_lcd->fix.smem_len = 240*320*16/8;   //每个像素16位，占2个字节
	s3c_lcd->fix.type         = FB_TYPE_PACKED_PIXELS;
	s3c_lcd->fix.visual       = FB_VISUAL_TRUECOLOR;  //真彩色
	s3c_lcd->fix.line_length= 
							
	//2.2 设置可变的参数
	s3c_lcd->var.xres           =       240;
	s3c_lcd->var.yres           =       320;
	s3c_lcd->var.xres_virtual =       240;
	s3c_lcd->var.yres_virtual =       320;
	s3c_lcd->bits_per_pixel   =       16;    //16位 
	

	// RGB:565
	s3c_lcd->var.red.offset   = 11;
	s3c_lcd->var.red.length   =   5;
	
	s3c_lcd->var.red.offset   =   5;
	s3c_lcd->var.red.length   =   6;      

	s3c_lcd->var.red.offset   = 	  0;
	s3c_lcd->var.red.length  =	  5;

	s3c_lcd->var.activate     =  FB_ACTIVATE_NOW  //默认值 

	//2.3 设置操作参数，数组在上面定义了static struct fb_ops s3c_lcdfb_ops 
	s3c_lcd->fbops             =  &s3c_lcdfb_ops
	
	
	//2.4 其他的设置
	// s3c_lcd->pseudo_palette  =   ; //假的调色板� 
	//s3c_lcd->screen_base      =;   //显存的虚拟地址
	s3c_lcd->screen_size       = 240*320*16/8;  //现存的大小

	/*
	根据手册LCD CONTROLLER 396页的内容从头设到尾
	具体内容见422页 LCD CONTROL REGISTOR
	由于寄存器数量太多，因此都放在数组里面比较方便
	*/
	
	//3. 硬件相关的操作*********************
	//3.1 配置GPIO用于LCD
	gpbcon = ioremap(0x56000010, 8); 
	gpbdat = gpbcon+1;
	gpccon = ioremap(0x56000020, 4); //系统映射是以页为单位，是1024字节，所以填1024或4 都没关系
	gpdcon = ioremap(0x56000030, 4);
	gpgcon= ioremap(0x56000060, 4);

	*gpccon   = 0xaaaaaaaa;   //拷贝自lcddrv.c GPIO管脚用于VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 
	*gpdcon   = 0xaaaaaaaa;	// GPIO管脚用于VD[23:8]

	*gpbcon &= ~(3); //GPIO设置为输出引脚
	*gpbcon |=1;   //输出低电平，但先不用
	*gpbdat &= ~1;  //输出低电平，先不让背光开启

	//KEYBOARD是背光电源，LCD_PWREN是电源，两回事
	//LCD_PWREN是由GPG4控制,bit[8,9]设置为11
	*gpgcon |=(3<<8); //GPG4用作LCD电源LCD_PWREN使能引脚
	

	
	//3.2 根据LCD手册设置LCD控制器，如vclk频率等。
	lcd_regs ==ioremap(0x4D000000,sizeof(struct lcd_regs)); //基地址和大小
	//开始进行具体设置，要一位一位的设置，才能准确 手册422页

	// LCD控制器1  lcdcon1
	
	/*  [17:8]      设置CLKVAL
		CLKVAL [17:8],需要合适的时钟才行
	  公式 VCLK = HCLK / [(CLKVAL +1) *2] LCD手册P14
	  内核的输出信息可以用dmsg命令来看
	  HCLK= 100 MHz (用dmsg命令查看内核得来)
	   VCLK取多少，要看LCD手册，参考液晶屏.pdf14页 clock cycle time 最小100ns
	 	15页DCLK 图片 83ns+上升沿，所以最小取100ns
	  10MHz(100ns频率即10MHz) = 100MHz / [(CLKVAL+1) x 2]
	 
	 *	所以得到:		  CLKVAL = 4

	* MMODE  不知道是什么，不管

	* [16:5]    设置PNRMODE 我们用的是TFT LCD panel，所以是0b11
	* [4:1]  设置BPMODE,设置BPP,单位像素比特位，
	         0b1100, 16 bpp for TFT
	* [0]    使能 
	      0 = Disable the video output and the LCD control signal         
	*/  

	lcd_regs->lcdcon1  =  (4<<8) | (3<<5) | (0x0c<<1);

	//LCD控制器2 lcdcon2  add 0x4D00004
	/* 
	/* 垂直方向的时间参数  具体计算参见笔记
	
	 * bit[31:24]: VBPD, VSYNC之后再过多长时间才能发出第1行数据
	 *			   LCD手册 T0-T2-T1=4  
		 *			   VBPD=3
		 * bit[23:14]: 多少行, 320行, 所以LINEVAL=320-1=319
		 * bit[13:6] : VFPD, 发出最后一行数据之后，再过多长时间才发出VSYNC
		 *			   LCD手册T2-T5=322-320=2, 所以VFPD=2-1=1
		 * bit[5:0]  : VSPW, VSYNC信号的脉冲宽度, LCD手册T1=1, 所以VSPW=1-1=0
		 */
		lcd_regs->lcdcon2  = (3<<24) | (319<<14) | (1<<6) | (0<<0);

	
	
	/* 水平方向的时间参数

	LCD控制器3 lcdcon3  0x4D00008 
		 * bit[25:19]: HBPD, VSYNC之后再过多长时间才能发出第1行数据
		 *			   LCD手册 T6-T7-T8=17
		 *			   HBPD=16
		 * bit[18:8]: 多少列, 240, 所以HOZVAL=240-1=239
		 * bit[7:0] : HFPD, 发出最后一行里最后一个象素数据之后，再过多长时间才发出HSYNC
		 *			   LCD手册T8-T11=251-240=11, 所以HFPD=11-1=10
		 */
		lcd_regs->lcdcon3 = (16<<19) | (239<<8) | (10<<0);
	
	/* 
	LCD信号控制器4 add 0x4D0000c
		水平方向的同步信号lcdcon4  
		 * bit[7:0] : HSPW, HSYNC信号的脉冲宽度, LCD手册T7=5, 所以HSPW=5-1=4
		 */ 
		lcd_regs->lcdcon4 = 4;
	
	/* 
	LCD信号控制器5  信号的极性  0x4D000010
	 * bit[11]: 1=565 format   FRM565
	 * bit[10]: 0 = The video data is fetched at VCLK falling edge INVVCLK
	 				下降沿取数组，不需要翻转
	 * bit[9] : 1 = HSYNC信号要反转,即低电平有效  INVVLINE 2440 手册可以看出，分别为
	 				低电平和高电平有效，因此要翻转才成
	 * bit[8] : 1 = VSYNC信号要反转,即低电平有效  INVVFRAME 帧同步信号要翻转
	 * bit[7] : 0 = INVVD video数据信号不要反转
	 * bit[6] : 0 = VDEN不用反转
	 * bit[3] : 0 = PWREN输出0  是否使能输出低电平
	 * bit[1] : 0 = BSWP  字节交换
	 * bit[0] : 1 = HWSWP 2440手册P413
	 bwsp =0 hwswp=0, [31:16][15:0]  排列为p2p1 p4p3,很别扭
	 bwsp =0 hwswp=1, [31:16][15:0]  排列为p1p2 p3p4,就顺当了
	 
	 */
	lcd_regs->lcdcon5 = (1<<11) | (0<<10) | (1<<9) | (1<<8) | (1<<0);
	

			
	//3.3 分配显存(framebuffer),并把地址告诉LCD控制器
	
	
	// 参考s3c2410fb.c
	//dma_alloc_writecombine(fbi->dev, fbi->map_size, &fbi->map_dma, GFP_KERNEL);
	//fbi->map_size,大小为s3c_lcd->fix.smem_len = 240*320*16/8;每个像素16位，占2个字节
	//&fbi->map_dma, DMA类型，物理地址s3c_lcd->smem_start
	//返回值为内存的虚拟地址 2.4里面设置

	s3c_lcd->screen_base  = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->smem_start,  GFP_KERNEL);


	//FRAME BUFFER START ADDRESS寄存器 手册428页 有3个寄存器LCDSADDR1-3

	//第一个寄存器 最高2位清零 �  清掉最高两位
	//第二个寄存器 结束地址A[21:1] 右移一位
	//第三个寄存器 虚拟和真实屏幕的偏移值，本例OFFSIZE为0 只需设置PAGEWIDTH,一行的大小
	//s3c_lcd->fix.smem_start = xxx;  /* 显存的物理地址 */lcd_regs->lcdsaddr1  = (s3c_lcd->fix.smem_start >> 1) & ~(3<<30); 
	lcd_regs->lcdsaddr2  = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len) >> 1) & 0x1fffff;
	lcd_regs->lcdsaddr3  = (240*16/16);  /* 一行的长度(单位是半字即2字节) */	
		
	
	
	/* 启动LCD
	打开背光
	提供LCD电源
	提供一些接口，让别人打开

	*/
	
	
	lcd_regs->lcdcon1 |= (1<<0); /* 使能LCD控制器 */
	lcd_regs->lcdcon5 |= (1<<3); /* 使能LCD本身 */
	
	*gpbdat |= 1;	  /* 输出高电平, 使能背光 原来设置*gpbdat &= ~1;输出低电平，先不让背光开启*/		
	
	
	
	//4. 注册_设置好了再注册 
	
	register_framebuffer(s3c_lcd);

	
	return 0;

}

5static int lcd_exit(void)
{
	return 0 ;

	
}


module_init(lcd_init);
modul_exit(lcd_exit);

MODULE_LICENSE("GPL");
