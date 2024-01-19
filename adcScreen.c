#include<p30fxxxx.h>

#include "adcScreen.h"




void ConfigureADCPins(void)
{
	//ADPCFGbits.PCFG6=0;
	ADPCFGbits.PCFG8=0;
	ADPCFGbits.PCFG9=0;
	
//	TRISBbits.TRISB6=1;
	TRISBbits.TRISB8=1;
	TRISBbits.TRISB9=1;
}




