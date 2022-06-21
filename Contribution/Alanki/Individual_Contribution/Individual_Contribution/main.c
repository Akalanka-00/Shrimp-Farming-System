#define F_CPU 8000000
#include <avr/io.h>
#include <stdio.h>
#include "LCDI2C.h"

const uint8_t kaypad[4][4]={{7,8,9,10},{4,5,6,11},{1,2,3,12},{13,0,14,15}};
uint8_t getkeyNum();

uint16_t ReadADC(uint8_t ADCchannel);
float ph=0.0;
char lcddata[20];
int main(void)
{
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));   // prescaler 128
	ADMUX |= (1<<REFS0);					//external vcc
	ADCSRA |= (1<<ADEN);                            // Turn on ADC
	LcdInit(0x27);
	LcdSetCursor(0,0,"Wait . . .");
	_delay_ms(1000);
	LcdCommand(LCD_CLEARDISPLAY);
	ADMUX = (1<<REFS0);		
	ph=ReadADC(0)*5.0/1024;
	ph= ph*3.5;
	sprintf(lcddata,"%0.1f",ph);
	LcdSetCursor(0,0,lcddata);
	DDRD=0b1;
	while (1)
	{	uint8_t key=getkeyNum();
		sprintf(lcddata,"%02u",key);
		LcdSetCursor(0,1,lcddata);
		if (key>5)
		{PORTD=0b1;
		}
		else
		{PORTD=0b0;
		}
	}
}

uint16_t ReadADC(uint8_t ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while( ADCSRA & (1<<ADSC) );
	return ADCW;
}

uint8_t getkeyNum(){
	DDRB=0b00001111;
	PORTB=0b11110000;
	uint8_t getx;
	uint8_t gety;
	while(1){
		
		uint8_t pin=(~(PINB|0x0F));
		if (pin)
		{
			
			
			switch(pin){
				
				
				case (1<<4) :getx=0; break;
				case (1<<5) :getx=1;break;
				case (1<<6) :getx=2;break;
				case (1<<7) :getx=3;break;
				
				
			} //switch
			DDRB=0b0;
			PORTB=0b0;
			
			DDRB=0b11110000;
			PORTB=0b00001111;
			
			pin=~(PINB|0xF0);
			
			switch(pin){
				case (1<<0) :gety=0;break;
				case (1<<1) :gety=1;break;
				case (1<<2) :gety=2;break;
				case (1<<3) :gety=3;break;
				
			} //switch
			
			DDRB=0b00001111;
			PORTB=0b11110000;
			
			
			return kaypad[gety][getx];
			
		}//if
		
		
		
	}//while 1
}
