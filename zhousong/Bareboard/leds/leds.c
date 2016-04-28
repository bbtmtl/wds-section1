/**
 *************************************************
 * @file leds.c
 * @brief BareBoard drv for Mini2440
 *  Turn on LEDs from LED1 to LED4 and repeatly
 *  Code wrote based on WDS code for JZ2440
 **************************************************
**/

/*For LEDs, LED1~LED4 belongs to GPB-Group*/
#define GPBCON	(*(volatile unsigned long *) 0x56000010)
#define GPBDAT  (*(volatile unsigned long *) 0x56000014)

/*LED1~4 respected to GPB5, GPB6, GPB7 and GPB8*/
#define GPB5_OUT  		(1<<(5*2))
#define GPB6_OUT  		(1<<(6*2))
#define GPB7_OUT  		(1<<(7*2))
#define GPB8_OUT		(1<<(8*2))

void wait_here (volatile unsigned long delay)
{
	for (; delay > 0; delay--);
}

int main () 
{
	unsigned long start = 0;

	// LED1~4 respected pins be set as output (01)
    GPBCON &=~( GPB5_OUT | GPB6_OUT |GPB7_OUT | GPB8_OUT);
	GPBCON = GPB5_OUT | GPB6_OUT |GPB7_OUT | GPB8_OUT;

	while (1)
		{
		wait_here (30000);
//		GPBDAT &= ~(start<<5); //LEDs turned ON according to value of start
		//what is the difference with: GPBDAT = ~(start<<5);
		GPBDAT = ~(start<<5);
		if (++start == 16)
			start = 0;
		}
	return 0;
}

