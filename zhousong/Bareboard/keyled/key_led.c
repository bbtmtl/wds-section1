/**
 *************************************************
 * @file keyled.c
 * @brief BareBoard drv for Mini2440
 *  Push the key (button) will turn on the LED
 *  Code wrote based on WDS code for JZ2440
 **************************************************
**/

/*For LED*/
#define GPBCON	(*(volatile unsigned long *) 0x56000010)
#define GPBDAT  (*(volatile unsigned long *) 0x56000014)
/*For Key*/
#define GPGCON  (*(volatile unsigned long *) 0x56000060)
#define GPGDAT  (*(volatile unsigned long *) 0x56000064)

/*Using Gray Code for LED notation*/
/* K1-->EINT8  -->GPG0	 (0001) GPB5 (LED 1)
 * K2-->EINT11 -->GPG3   (0010) GPB6 (LED 2)
 * K3-->EINT13 -->GPG5   (0100) GPB7 (LED 3)
 * K4-->EINT14 -->GPG6   (1000) GPB8 (LED 4)
 * K5-->EINT15 -->GPG7   (1001)    (LED 1+4)
 * K6-->EINT16 -->GPG11  (1010)    (LED 2+4)
 */

/*LED1~4 respected to GPB5, GPB6, GPB7 and GPB8*/
#define GPB5_OUT  		(1<<(5*2))
#define GPB6_OUT  		(1<<(6*2))
#define GPB7_OUT  		(1<<(7*2))
#define GPB8_OUT		(1<<(8*2))

/*Key1~6 respected to GPG0, GPG3, GPG5, GPG6, GPG7, GPG11*/
#define GPG0_IN			(1<<(0*2))
#define GPG3_IN         (1<<(3*2))
#define GPG5_IN         (1<<(5*2))
#define GPG6_IN         (1<<(6*2))
#define GPG7_IN         (1<<(7*2))
#define GPG11_IN        (1<<(11*2))


int main () 
{
	unsigned long dwDat;
	// LED1~4 respected pins be set as output (01)
        GPBCON &=~( GPB5_OUT | GPB6_OUT |GPB7_OUT | GPB8_OUT);
	GPBCON = GPB5_OUT | GPB6_OUT |GPB7_OUT | GPB8_OUT;

	//K1~6 respected pins be set as input (00)
	GPGCON = GPG0_IN & GPG3_IN & GPG5_IN & GPG6_IN & GPG7_IN & GPG11_IN;

	while (1)
		{
		// If Kn=0, button was pushed,and LEDn=0 (turned on)
		dwDat = GPGDAT; //Read GPG pins status

		if (dwDat & (1<<0)) //K1 unpushed
			GPBDAT |= (1<<5); //LED1 OFF
		else GPBDAT &= ~(1<<5); //LED1 ON


        if (dwDat & (1<<3)) //K2 unpushed
             GPBDAT |= (1<<6); //LED2 OFF
        else GPBDAT &= ~(1<<6); //LED2 ON


        if (dwDat & (1<<5)) //K3 unpushed
             GPBDAT |= (1<<7); //LED3 OFF
        else GPBDAT &= ~(1<<7); //LED3 ON


        if (dwDat & (1<<6)) //K4 unpushed
             GPBDAT |= (1<<8); //LED4 OFF
        else GPBDAT &= ~(1<<8); //LED4 ON


        if (dwDat & (1<<7)) //K5 unpushed
             GPBDAT |= ((1<<5) | (1<<8)); //LED1+4 OFF
        else GPBDAT &= ~((1<<5) | (1<<8)); //LED1+4 ON


        if (dwDat & (1<<11)) //K6 unpushed
             GPBDAT |=((1<<6) | (1<<8)); //LED2+4 OFF
        else GPBDAT &= ~((1<<6) | (1<<8)); //LED2+4 ON
		
	}
	return 0;
}
