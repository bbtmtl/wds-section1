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
#include <stdio.h>
#include "s3c24xx.h"
#include "lcddrv.h"


#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include "s3c2410fb.h"

//���3.2Һ�����ƼĴ�����˵������
/*
3.2 ����LCD�ֲ�����LCD����������vclkƵ�ʵȡ�
�����ֲ�LCD CONTROLLER 396ҳ�����ݴ�ͷ�赽β
�������ݼ�422ҳ LCD CONTROL REGISTOR
���ڼĴ�������̫�࣬��˶�������������ȽϷ���
ע�����Ǹ��Ĵ���֮�����4���ֽ�
_____��Ծ̫��Ļ����м��һЩ�����ֽ�
ȡ����3th ��lcd.c, ������2410��Ҫ���ֲ�Ƚ�һ�¿��Ƿ��б仯
������Ĵ������ϣ�2410��ȫ��2440��ͬ��
*/
//����LCD���ƼĴ������顡
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
		unsigned long	reserved[9]; // bluelut��dithmode��ַ�ֱ�Ϊ28��4c,���0x36,�����9�����Ա���9�ֽ�
		unsigned long	dithmode;
		unsigned long	tpal;
		unsigned long	lcdintpnd;
		unsigned long	lcdsrcpnd;
		unsigned long	lcdintmsk;
		unsigned long	lpcsel;
	};





//������atmel_lcdfb.c(drivers\video)
static struct fb_ops s3c_lcdfb_ops = {  //���Դ�Ĳ��� 
	.owner		= THIS_MODULE,
	//.fb_setcolreg	= atmel_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,    //������
	.fb_copyarea	= cfb_copyarea, //��������
	.fb_imageblit	= cfb_imageblit,
};


// Step1 �ڿ�����ú󣬿�ʼ���ò���
static struct fb_info *s3c_lcd;
static volatile unsigned long *gpbcon; //�����Ĵ����ṹ��ָ��
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;
static volatile struct lcd_regs* lcd_regs; //ָ�������أ���3.2��ioremap

static int lcd_init(void)
{
	//1. ����һ��fb_info�ṹ��ο�s3c2410fb.c
	s3c_lcd = framebuffer_alloc(0�� NULL); //������˽�����ݵĶ���ռ䣬����������Ҫ
						//�ڴ��п��ܲ�������Ҫ�жϣ�����������ʱ���ж�

	//2. ����  �ο�fb.h***********************
	//2.1 ���ù̶��Ĳ���
	strcpy(s3c_lcd->fix.id, "mylcd");
	s3c_lcd->fix.smem_len = 240*320*16/8;   //ÿ������16λ��ռ2���ֽ�
	s3c_lcd->fix.type         = FB_TYPE_PACKED_PIXELS;
	s3c_lcd->fix.visual       = FB_VISUAL_TRUECOLOR;  //���ɫ
	s3c_lcd->fix.line_length= 
							
	//2.2 ���ÿɱ�Ĳ���
	s3c_lcd->var.xres           =       240;
	s3c_lcd->var.yres           =       320;
	s3c_lcd->var.xres_virtual =       240;
	s3c_lcd->var.yres_virtual =       320;
	s3c_lcd->bits_per_pixel   =       16;    //16λ 
	

	// RGB:565
	s3c_lcd->var.red.offset   = 11;
	s3c_lcd->var.red.length   =   5;
	
	s3c_lcd->var.red.offset   =   5;
	s3c_lcd->var.red.length   =   6;      

	s3c_lcd->var.red.offset   = 	  0;
	s3c_lcd->var.red.length  =	  5;

	s3c_lcd->var.activate     =  FB_ACTIVATE_NOW  //Ĭ��ֵ 

	//2.3 ���ò������������������涨����static struct fb_ops s3c_lcdfb_ops 
	s3c_lcd->fbops             =  &s3c_lcdfb_ops
	
	
	//2.4 ����������
	// s3c_lcd->pseudo_palette  =   ; //�ٵĵ�ɫ��� 
	//s3c_lcd->screen_base      =;   //�Դ�������ַ
	s3c_lcd->screen_size       = 240*320*16/8;  //�ִ�Ĵ�С

	/*
	�����ֲ�LCD CONTROLLER 396ҳ�����ݴ�ͷ�赽β
	�������ݼ�422ҳ LCD CONTROL REGISTOR
	���ڼĴ�������̫�࣬��˶�������������ȽϷ���
	*/
	
	//3. Ӳ����صĲ���*********************
	//3.1 ����GPIO����LCD
	gpbcon = ioremap(0x56000010, 8); 
	gpbdat = gpbcon+1;
	gpccon = ioremap(0x56000020, 4); //ϵͳӳ������ҳΪ��λ����1024�ֽڣ�������1024��4 ��û��ϵ
	gpdcon = ioremap(0x56000030, 4);
	gpgcon= ioremap(0x56000060, 4);

	*gpccon   = 0xaaaaaaaa;   //������lcddrv.c GPIO�ܽ�����VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 
	*gpdcon   = 0xaaaaaaaa;	// GPIO�ܽ�����VD[23:8]

	*gpbcon &= ~(3); //GPIO����Ϊ�������
	*gpbcon |=1;   //����͵�ƽ�����Ȳ���
	*gpbdat &= ~1;  //����͵�ƽ���Ȳ��ñ��⿪��

	//KEYBOARD�Ǳ����Դ��LCD_PWREN�ǵ�Դ��������
	//LCD_PWREN����GPG4����,bit[8,9]����Ϊ11
	*gpgcon |=(3<<8); //GPG4����LCD��ԴLCD_PWRENʹ������
	

	
	//3.2 ����LCD�ֲ�����LCD����������vclkƵ�ʵȡ�
	lcd_regs ==ioremap(0x4D000000,sizeof(struct lcd_regs)); //����ַ�ʹ�С
	//��ʼ���о������ã�Ҫһλһλ�����ã�����׼ȷ �ֲ�422ҳ

	// LCD������1  lcdcon1
	
	/*  [17:8]      ����CLKVAL
		CLKVAL [17:8],��Ҫ���ʵ�ʱ�Ӳ���
	  ��ʽ VCLK = HCLK / [(CLKVAL +1) *2] LCD�ֲ�P14
	  �ں˵������Ϣ������dmsg��������
	  HCLK= 100 MHz (��dmsg����鿴�ں˵���)
	   VCLKȡ���٣�Ҫ��LCD�ֲᣬ�ο�Һ����.pdf14ҳ clock cycle time ��С100ns
	 	15ҳDCLK ͼƬ 83ns+�����أ�������Сȡ100ns
	  10MHz(100nsƵ�ʼ�10MHz) = 100MHz / [(CLKVAL+1) x 2]
	 
	 *	���Եõ�:		  CLKVAL = 4

	* MMODE  ��֪����ʲô������

	* [16:5]    ����PNRMODE �����õ���TFT LCD panel��������0b11
	* [4:1]  ����BPMODE,����BPP,��λ���ر���λ��
	         0b1100, 16 bpp for TFT
	* [0]    ʹ�� 
	      0 = Disable the video output and the LCD control signal         
	*/  

	lcd_regs->lcdcon1  =  (4<<8) | (3<<5) | (0x0c<<1);

	//LCD������2 lcdcon2  add 0x4D00004
	/* 
	/* ��ֱ�����ʱ�����  �������μ��ʼ�
	
	 * bit[31:24]: VBPD, VSYNC֮���ٹ��೤ʱ����ܷ�����1������
	 *			   LCD�ֲ� T0-T2-T1=4  
		 *			   VBPD=3
		 * bit[23:14]: ������, 320��, ����LINEVAL=320-1=319
		 * bit[13:6] : VFPD, �������һ������֮���ٹ��೤ʱ��ŷ���VSYNC
		 *			   LCD�ֲ�T2-T5=322-320=2, ����VFPD=2-1=1
		 * bit[5:0]  : VSPW, VSYNC�źŵ�������, LCD�ֲ�T1=1, ����VSPW=1-1=0
		 */
		lcd_regs->lcdcon2  = (3<<24) | (319<<14) | (1<<6) | (0<<0);

	
	
	/* ˮƽ�����ʱ�����

	LCD������3 lcdcon3  0x4D00008 
		 * bit[25:19]: HBPD, VSYNC֮���ٹ��೤ʱ����ܷ�����1������
		 *			   LCD�ֲ� T6-T7-T8=17
		 *			   HBPD=16
		 * bit[18:8]: ������, 240, ����HOZVAL=240-1=239
		 * bit[7:0] : HFPD, �������һ�������һ����������֮���ٹ��೤ʱ��ŷ���HSYNC
		 *			   LCD�ֲ�T8-T11=251-240=11, ����HFPD=11-1=10
		 */
		lcd_regs->lcdcon3 = (16<<19) | (239<<8) | (10<<0);
	
	/* 
	LCD�źſ�����4 add 0x4D0000c
		ˮƽ�����ͬ���ź�lcdcon4  
		 * bit[7:0] : HSPW, HSYNC�źŵ�������, LCD�ֲ�T7=5, ����HSPW=5-1=4
		 */ 
		lcd_regs->lcdcon4 = 4;
	
	/* 
	LCD�źſ�����5  �źŵļ���  0x4D000010
	 * bit[11]: 1=565 format   FRM565
	 * bit[10]: 0 = The video data is fetched at VCLK falling edge INVVCLK
	 				�½���ȡ���飬����Ҫ��ת
	 * bit[9] : 1 = HSYNC�ź�Ҫ��ת,���͵�ƽ��Ч  INVVLINE 2440 �ֲ���Կ������ֱ�Ϊ
	 				�͵�ƽ�͸ߵ�ƽ��Ч�����Ҫ��ת�ų�
	 * bit[8] : 1 = VSYNC�ź�Ҫ��ת,���͵�ƽ��Ч  INVVFRAME ֡ͬ���ź�Ҫ��ת
	 * bit[7] : 0 = INVVD video�����źŲ�Ҫ��ת
	 * bit[6] : 0 = VDEN���÷�ת
	 * bit[3] : 0 = PWREN���0  �Ƿ�ʹ������͵�ƽ
	 * bit[1] : 0 = BSWP  �ֽڽ���
	 * bit[0] : 1 = HWSWP 2440�ֲ�P413
	 bwsp =0 hwswp=0, [31:16][15:0]  ����Ϊp2p1 p4p3,�ܱ�Ť
	 bwsp =0 hwswp=1, [31:16][15:0]  ����Ϊp1p2 p3p4,��˳����
	 
	 */
	lcd_regs->lcdcon5 = (1<<11) | (0<<10) | (1<<9) | (1<<8) | (1<<0);
	

			
	//3.3 �����Դ�(framebuffer),���ѵ�ַ����LCD������
	
	
	// �ο�s3c2410fb.c
	//dma_alloc_writecombine(fbi->dev, fbi->map_size, &fbi->map_dma, GFP_KERNEL);
	//fbi->map_size,��СΪs3c_lcd->fix.smem_len = 240*320*16/8;ÿ������16λ��ռ2���ֽ�
	//&fbi->map_dma, DMA���ͣ������ַs3c_lcd->smem_start
	//����ֵΪ�ڴ�������ַ 2.4��������

	s3c_lcd->screen_base  = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->smem_start,  GFP_KERNEL);


	//FRAME BUFFER START ADDRESS�Ĵ��� �ֲ�428ҳ ��3���Ĵ���LCDSADDR1-3

	//��һ���Ĵ��� ���2λ���� �  ��������λ
	//�ڶ����Ĵ��� ������ַA[21:1] ����һλ
	//�������Ĵ��� �������ʵ��Ļ��ƫ��ֵ������OFFSIZEΪ0 ֻ������PAGEWIDTH,һ�еĴ�С
	//s3c_lcd->fix.smem_start = xxx;  /* �Դ�������ַ */lcd_regs->lcdsaddr1  = (s3c_lcd->fix.smem_start >> 1) & ~(3<<30); 
	lcd_regs->lcdsaddr2  = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len) >> 1) & 0x1fffff;
	lcd_regs->lcdsaddr3  = (240*16/16);  /* һ�еĳ���(��λ�ǰ��ּ�2�ֽ�) */	
		
	
	
	/* ����LCD
	�򿪱���
	�ṩLCD��Դ
	�ṩһЩ�ӿڣ��ñ��˴�

	*/
	
	
	lcd_regs->lcdcon1 |= (1<<0); /* ʹ��LCD������ */
	lcd_regs->lcdcon5 |= (1<<3); /* ʹ��LCD���� */
	
	*gpbdat |= 1;	  /* ����ߵ�ƽ, ʹ�ܱ��� ԭ������*gpbdat &= ~1;����͵�ƽ���Ȳ��ñ��⿪��*/		
	
	
	
	//4. ע��_���ú�����ע�� 
	
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
