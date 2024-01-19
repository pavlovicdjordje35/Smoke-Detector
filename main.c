#include <stdio.h>
#include <stdlib.h>
#include <p30fxxxx.h>
#include "adc.h"
#include "driverGLCD.h"
#include "pocetak.h"
#include "tajmeri.h"
#include "timer1.h"
#include "DISPLAY_MBP.h"
#include <stdbool.h>



#define xtal 25000000





//_FOSC(CSW_FSCM_OFF & XT_PLL4);//instruction takt je isti kao i kristal 10MHz -- RAZLIKA!
_FWDT(WDT_OFF);
_FOSC(CSW_FSCM_OFF & HS3_PLL4);
_FGS(CODE_PROT_OFF);
unsigned int X, Y,x_vrednost, y_vrednost;
int sirovi0, sirovi1, sirovi2, sirovi3, temp0, temp1;
const unsigned int AD_Xmin =220;
const unsigned int AD_Xmax =3642;
const unsigned int AD_Ymin =520;
const unsigned int AD_Ymax =3450;

int pomocna;
int i=0;
bool state1,state2 = false;

#define DRIVE_A PORTCbits.RC13
#define DRIVE_B PORTCbits.RC14


unsigned int pauza;
unsigned int stanje, taster;
unsigned int match_value;


unsigned int brojac_ms,stoperica,ms,sekund;
void ConfigureTSPins(void)
{
	TRISCbits.TRISC13=0;
    TRISCbits.TRISC14=0;
	
}

void delay_X(unsigned int vrednost)
{
    unsigned int i;
    for(i = 0; i < vrednost; i++);
}

void Delay(unsigned int N)
{
	unsigned int i;
	for(i=0;i<N;i++);
}




void Write_GLCD(unsigned int data)
{
unsigned char temp;

temp=data/1000;
Glcd_PutChar(temp+'0');
data=data-temp*1000;
temp=data/100;
Glcd_PutChar(temp+'0');
data=data-temp*100;
temp=data/10;
Glcd_PutChar(temp+'0');
data=data-temp*10;
Glcd_PutChar(data+'0');
}




//********SERIJSKA KOMUNIKACIJA***********

void initUART1(void)
{
U1BRG=0x0015;//baud rate 9600
U1MODEbits.ALTIO = 0;
IEC0bits.U1RXIE = 1;
U1STA&=0xfffc;
U1MODEbits.UARTEN=1;
U1STAbits.UTXEN=1;
}

void WriteUART1(unsigned int data)
{


    while (U1STAbits.TRMT==0);
    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}


void WriteUART1dec2string(unsigned int data)
{
    unsigned char temp;

    temp=data/1000;
    WriteUART1(temp+'0');
    data=data-temp*1000;
    temp=data/100;
    WriteUART1(temp+'0');
    data=data-temp*100;
    temp=data/10;
    WriteUART1(temp+'0');
    data=data-temp*10;
    WriteUART1(data+'0');
}

void writeStringUART1(char* data)//funkcija koja pise ceo string jedan po jedan karakter
{
    int j = 0;//brojac za prolaz kroz ceo niz/string

    while(data[j] != '\0')//idem kroz ceo string
    {
        WriteUART1(data[j]);//ispisujem jedan karakter(jedan po jedan )
        j++;//povecavam brojac da bih presao na sledeci karakter
    }

    WriteUART1(13);
}






void ispisiLCD()
{
    
    GoToXY(0,0);		
    GLCD_Printf("POZAR");
    
    GoToXY(25, 5);
    GLCD_Printf("1");
    
    GoToXY(85, 5);
    GLCD_Printf("2");
   
    GLCD_Rectangle(5,35,60,60);
	GLCD_Rectangle(65,35,120,60);
}

//**************TIMER************
void __attribute__ ((__interrupt__)) _T2Interrupt(void) // svakih 1ms
{

		TMR2 =0;
     ms=1;//fleg za milisekundu ili prekid;potrebno ga je samo resetovati u funkciji

	brojac_ms++;//brojac milisekundi
    stoperica++;//brojac za funkciju Delay_ms

    if (brojac_ms==1000)//sek
        {
          brojac_ms=0;
          sekund=1;//fleg za sekundu
		 } 
	IFS0bits.T2IF = 0; 
       
}
void __attribute__((__interrupt__)) _T1Interrupt(void) // 
{

   	TMR1 =0;
    IFS0bits.T1IF = 0;   

}
void Delay_ms (int vreme)//funkcija za kasnjenje u milisekundama
	{
		stoperica = 0;
		while(stoperica < vreme);
	}

//const unsigned int ADC_THRESHOLD = 900; 



//*********AD-KONVERZIJA****************
void __attribute__((__interrupt__)) _ADCInterrupt(void) // RAZLIKA!
{   sirovi0=ADCBUF0;//0
	sirovi1=ADCBUF1;//1
    sirovi2=ADCBUF2;
    sirovi3=ADCBUF3;

	temp0=sirovi0;
	temp1=sirovi1;

    IFS0bits.ADIF = 0;
}

void Touch_Panel (void)
{
// vode horizontalni tranzistori
	DRIVE_A = 1;  
	DRIVE_B = 0;
    
     LATCbits.LATC13=1;
     LATCbits.LATC14=0;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
				
	// ocitavamo x	
	x_vrednost = temp0;//temp0 je vrednost koji nam daje AD konvertor na BOTTOM pinu		

	// vode vertikalni tranzistori
     LATCbits.LATC13=0;
     LATCbits.LATC14=1;
	DRIVE_A = 0;  
	DRIVE_B = 1;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
	
	// ocitavamo y	
	y_vrednost = temp1;// temp1 je vrednost koji nam daje AD konvertor na LEFT pinu	
	
//Ako ?elimo da nam X i Y koordinate budu kao rezolucija ekrana 128x64 treba skalirati vrednosti x_vrednost i y_vrednost tako da budu u opsegu od 0-128 odnosno 0-64
//skaliranje x-koordinate

    X=(x_vrednost-161)*0.03629;



//X= ((x_vrednost-AD_Xmin)/(AD_Xmax-AD_Xmin))*128;	
//vrednosti AD_Xmin i AD_Xmax su minimalne i maksimalne vrednosti koje daje AD konvertor za touch panel.


//Skaliranje Y-koordinate
	Y= ((y_vrednost-500)*0.020725);

//	Y= ((y_vrednost-AD_Ymin)/(AD_Ymax-AD_Ymin))*64;
}



void __attribute__((__interrupt__)) _U1RXInterrupt(void) // INTERAPT U MAINU A NE U POCETAK
{
     IFS0bits.U1RXIF = 0;
    
    /*niz[i] = U1RXREG;
    if(i < 5)
        i++;
    else
        i = 0;*/

} 
void iskljuci_alarm(){
 LATAbits.LATA11 = 0;

}

void petlja()
{  
    

     if(  sirovi3 <= 580 )
        {   
            LATAbits.LATA11 = 0;
            state1 = false;
            state2 = false;
        }
        else if( sirovi3 >= 581 && sirovi3 <= 780) 
        {
            //GLCD_DisplayPicture(Slika_bmp);
            

            if(PORTFbits.RF6){
                
                switch(i)
				{
                    case 0:
				LATDbits.LATD8=1;
                Delay_ms (10);
                LATDbits.LATD8=0;
                Delay_ms (190);
                i=1;
                Delay_ms(1000);
                break;
                
                    case 1:
                LATDbits.LATD8=1;
                Delay_ms (15);
                LATDbits.LATD8=0;
                Delay_ms (185);
                i=2;
                Delay_ms(1000);
                break;
                
                case 2:
                LATDbits.LATD8=1;
                Delay_ms (20);
                LATDbits.LATD8=0;
                Delay_ms (180);
                i=0;
                Delay_ms(1000);
                break;
                
           
				} 
            }

            LATAbits.LATA11 = 1;
            Delay_ms(70);
            LATAbits.LATA11 = 0;
            Delay_ms(600);
           // writeStringUART1("Pozar!");
            
        }
     else if(sirovi3 > 780 &&  sirovi3 <2512)
        {
            //GLCD_DisplayPicture(Slika_bmp);
            state2 = false;
            if(!state1){
                //for (pomocna = 0; pomocna < 50; pomocna++) {
            Delay_ms(1000);
            LATDbits.LATD8=1;
            Delay_ms (15);
            LATDbits.LATD8=0;
            Delay_ms (185);
            LATDbits.LATD8=1;
            Delay_ms (15);
            LATDbits.LATD8=0;
            Delay_ms (185);
            LATDbits.LATD8=1;
            Delay_ms (15);
            LATDbits.LATD8=0;
            Delay_ms (185);
            LATDbits.LATD8=1;
            Delay_ms (15);
            LATDbits.LATD8=0;
            Delay_ms (185);
            LATDbits.LATD8=1;
            Delay_ms (15);
            LATDbits.LATD8=0;
            Delay_ms (185);
            Delay_ms(1000);
                //}
            state1 = true;
            }
            
            LATAbits.LATA11 = 1;
            Delay_ms(60);
            LATAbits.LATA11 = 0;
            Delay_ms(200); 
            //writeStringUART1("Saljite pomoc!");
        }
     else if(sirovi3 > 2511)
        {
            //GLCD_DisplayPicture(Slika_bmp);
            state1 = false;
            if(!state2){
                
            Delay_ms(1000);
            LATDbits.LATD8=1;
            Delay_ms (20);
            LATDbits.LATD8=0;
            Delay_ms (180);
            LATDbits.LATD8=1;
            Delay_ms (20);
            LATDbits.LATD8=0;
            Delay_ms (180);
            LATDbits.LATD8=1;
            Delay_ms (20);
            LATDbits.LATD8=0;
            Delay_ms (180);
            LATDbits.LATD8=1;
            Delay_ms (20);
            LATDbits.LATD8=0;
            Delay_ms (180);
            LATDbits.LATD8=1;
            Delay_ms (20);
            LATDbits.LATD8=0;
            Delay_ms (180);
            Delay_ms(1000);
            state2 = true;
            }
            
            LATAbits.LATA11 = 1;
            Delay_ms(60);
            LATAbits.LATA11 = 0;
            Delay_ms(100); 
           // writeStringUART1("Hitnoo! Saljite jos jedno vozilo!");
        }

}
void ocitajFotootopornik()
{
    if(sirovi2 > 2000)
    {
        LATFbits.LATF4 = 1;
    }
    else
    {
        LATFbits.LATF4 = 0;
    }
      
}

/*void fotootpornik(){
    LATFbits.LATF4=1;


}*/






int main(int argc, char** argv) {
    
    ConfigureAllPins();
	ConfigureTSPins();

	GLCD_LcdInit();

	GLCD_ClrScr();
	
    Init_T2();
    
    TRISDbits.TRISD9 = 1;
    
    
   /* TRISDbits.TRISD9=0;*/
    TRISDbits.TRISD8=1; /*********pir*************/
    TRISAbits.TRISA11 = 0;//Buzzer izlaz
    
    
 
   
   // delay_X(10000);
	//TRISBbits.TRISB11=1;

//		delay_X(60000);


		initUART1();//inicijalizacija UART-a
        //initUART2();
 		ADCinit();//inicijalizacija AD konvertora
        ConfigureADCPins();

		ADCON1bits.ADON=1;//pocetak Ad konverzije
        TRISAbits.TRISA11 = 0;
        
        
        
        ADPCFGbits.PCFG7 = 1;
        
        TRISBbits.TRISB7 = 0;
        
    ADPCFGbits.PCFG1=1;//kazemo da nije analogni vec digitalni pin
	TRISDbits.TRISD8=0;//konfigurisemo kao izlaz
   /* TRISFbits.TRISF1=1;//konfigurisemo kao ulaz
	//TRISDbits.TRISD0=1;//konfigurisemo kao ulaz
    TRISFbits.TRISF2=1;
    TRISFbits.TRISF4=0;*/
    
    ADPCFGbits.PCFG6=0;//defini?emo da pin B4 nije analogni vec digitalni
	//ADPCFGbits.PCFG10=1;//defini?emo da pin B5 nije analogni vec digitalni

	TRISBbits.TRISB6=1;//konfigurisemo kao izlaz
	//TRISBbits.TRISB10=0;//konfigurisemo kao izlaz
    
    
    //------------------------------------------------------------------------------deo za init timera 1
	ConfigIntTimer1(T1_INT_PRIOR_1 & T1_INT_ON);
	WriteTimer1(0);
	//match_value = (xtal/20000)-11;
	OpenTimer1(T1_ON & T1_GATE_OFF & T1_IDLE_CON &
	T1_PS_1_1 & T1_SYNC_EXT_OFF &
	T1_SOURCE_INT, match_value);


	CORCONbits.PSV = 1;
	//deo mkoji erovatno nije potreban ali pisan je jer je oc pravio problem zbog pullupa
	IEC1bits.OC4IE = 0;// Disable the Interrupt bit in IEC Register 
	OC4CONbits.OCM = 0;//Turn off Output Compare 4 
	IFS1bits.OC4IF = 0;//Disable the Interrupt Flag bit in IFS Register 
    
    
 
for(pauza=0;pauza<40000;pauza++);
//ConfigureAllPins ();
for(pauza=0;pauza<40000;pauza++);

GLCD_LcdInit();


ispisiLCD();






        
	while(1)
	{
        
        
       petlja();
        
        
        
      Touch_Panel();
    
       if ((5<X)&&(X<63)&& (11<Y)&&(Y<52)){
           writeStringUART1("Hitnoo! Saljite POMOC!"); 
       if(sirovi2 < 2000)
    {
        LATFbits.LATF4 = 1;
    }
           
           delay_X(2000);
           
    }
        
        //dugme 2
       
    if ((64<X)&&(X<122)&& (11<Y)&&(Y<52)){
        LATFbits.LATF4 = 0;
    
        writeStringUART1("Lazna uzbuna! Zeni izgorio kolac :/");
        
        iskljuci_alarm();
        Delay(1000);
    }

        
    }
        

    return (EXIT_SUCCESS);
}


