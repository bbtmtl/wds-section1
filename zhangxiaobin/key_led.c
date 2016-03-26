
#define	GPGCON		(*(volatile unsigned long *)0x56000060)
#define	GPGDAT		(*(volatile unsigned long *)0x56000064)
#define	GPBCON		(*(volatile unsigned long *)0x56000010)
#define	GPBDAT		(*(volatile unsigned long *)0x56000014)

#define	GPB5_out	(1<<(5*2))
#define	GPB6_out	(1<<(6*2))
#define	GPB7_out	(1<<(7*2))
#define	GPB8_out	(1<<(8*2))
/*
 * LED1,LED2,led3,LED4¶ÔÓ¦GPF4¡¢GPF5¡¢GPF6
 */


#define	GPB5_msk	(3<<(5*2))
#define	GPB6_msk	(3<<(6*2))
#define	GPB7_msk	(3<<(7*2))
#define	GPB8_msk	(3<<(8*2))

/*
 * K1-K6:GPG0,GPG3,GPG5,GPG6,GPG,7,GPG11
		 EINT8,EINT11,EINT13,EINT14,EINT15,EINT19
 */
#define GPG0_in     (0<<(0*2))
#define GPG3_in     (0<<(3*2))
#define GPG5_in     (0<<(5*2))
#define GPG6_in     (0<<(6*2))
#define GPG7_in     (0<<(7*2))
#define GPG11_in     (0<<(11*2))


#define GPG0_msk     (3<<(0*2))
#define GPG3_msk     (3<<(3*2))
#define GPG5_msk     (3<<(5*2))
#define GPG6_msk     (3<<(6*2))
#define GPG7_msk     (3<<(7*2))
#define GPG11_msk    (3<<(11*2))

int main()
{
        unsigned long dwDat;
        // LED1,LED2,LED4:clear as 0 & set as output
        GPBCON &= ~(GPB5_msk | GPB6_msk | GPB7_msk | GPB8_msk);
        GPBCON |= (GPB5_out |GPB6_out |GPB7_out | GPB8_out);
        
        // K1-K6:GPG0,GPG3,GPG5,GPG6,GPG7,GPG11,
       GPGCON &= ~(GPG0_msk | GPG3_msk | GPG5_msk | GPG6_msk | GPG7_msk | GPG11_msk);
  //     GPGCON |= (GPG0_in | GPG3_in | GPG5_in | GPG6_in | GPG7_in | GPG11_in);

     
        while(1){
            
            dwDat =GPGDAT;             
        
            if (dwDat & (1<<0))        //K1 NOT PRESSED
               GPBDAT |= (1<<5);       // LED1 OFF
            else    
               GPBDAT &= ~(1<<5);      // LED1 ON
                
            if (dwDat & (1<<3))         // K2 NOT PRESSED
               GPBDAT |= (1<<6);       // LED2 OFF
            else    
               GPBDAT &= ~(1<<6);      // LED2 ON
    
                  
            if (dwDat & (1<<5))         // K3 NOT PRESSED
               GPBDAT |= (1<<7);       // LED3 OFF
            else    
               GPBDAT &= ~(1<<7);      // LED3 ON

       	    if (dwDat & (1<<6))         // K4 NOT PRESSED
               GPBDAT |= (1<<8);       // LED4 OFF
            else    
               GPBDAT &= ~(1<<8);      // LED4 ON    

            if (dwDat & (1<<7))         // K5 NOT PRESSED
               GPBDAT |= (1<<5 | 1<<6 |1<<7 | 1<<8);       //  OFF
            else     			// LED1,LED2,LED3,LED4 ON
               GPBDAT &= ~(1<<5 | 1<<6 |1<<7 | 1<<8);   
 
    }

    return 0;
}
