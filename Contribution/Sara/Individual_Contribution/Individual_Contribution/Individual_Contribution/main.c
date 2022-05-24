
#define F_CPU 8000000
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "LCDI2C.h"


volatile uint16_t TimerCal=0;// variable for collect echo data
uint16_t ultraINT0=0;

char lcddata[20];
int main(void)
{
	LcdInit(0x27);
	/*Ultrasonic Timer Part*/
	DDRD|=(1<<3); //D3 as output
	TCCR0|=(1<<WGM01);//Enable Compare match mode
	TCCR0|=(1<<CS11);//Start timer  prescaler =8
	TCNT0=0;
	OCR0=10;
	/*register value= time*(clock speed/prescale)
	register value=0.000001*(8000000/1)
	register value=10*/
	TIMSK|=(1<<OCIE0);//enable timer Compare inturrept
	sei();	
	
	
	
	DDRD|=(1<<4);
	
	for(uint8_t j=0;j<100;j++){
		PORTD|=(1<<4);
		for(uint8_t i=0;i<9;i++){
			_delay_us(100);
		}
		PORTD&=~(1<<4);
		
		
		for(uint8_t i=0;i<9;i++){
			_delay_us(100);
		}
	}
	
	_delay_ms(500);
	
	for(uint8_t j=0;j<100;j++){
		PORTD|=(1<<4);
		for(uint8_t i=0;i<20;i++){
			_delay_us(100);
		}
		PORTD&=~(1<<4);
		
		
		for(uint8_t i=0;i<20;i++){
			_delay_us(100);
		}
	}
	
	_delay_ms(1000);
	for(uint8_t j=0;j<100;j++){
		PORTD|=(1<<4);
		for(uint8_t i=0;i<9;i++){
			_delay_us(100);
		}
		PORTD&=~(1<<4);
		
		
		for(uint8_t i=0;i<9;i++){
			_delay_us(100);
		}
	}
	
	while (1)
	{	
		_delay_ms(500);
		PORTD&=~(1<<3);//TRIG pin low
		_delay_us(50);//wait 50 micro sec
		PORTD|=(1<<3);//TRIG pin high
		_delay_us(50);//wait 50 micro sec
		PORTD&=~(1<<3);////TRIG pin low
		while(!(PIND&(1<<2)))//wait for pulse
		TimerCal=0;//rest timer
		while((PIND&(1<<2)))////wait for pulse down
		ultraINT0=TimerCal/4.125;//copy timer value
		
		if (ultraINT0<50)
		{
			sprintf(lcddata,"Water level Low");
		}
		if (ultraINT0>80)
		{
			sprintf(lcddata,"Water level High");
		}
		if (ultraINT0>50 && ultraINT0<80 )
		{
			sprintf(lcddata,"Water level fine");
		}
		
		//sprintf(lcddata,"%02u",ultraINT0);
		LcdSetCursor(0,0,lcddata);
		
		
		if (PINA&(1<<0))
		{
			LcdSetCursor(0,1,"HIGH");
		} 
		else
		{	LcdSetCursor(0,1,"LOW ");
		}
	}
}



ISR(TIMER0_COMP_vect){//ultrasonic
	TimerCal++;
	TCNT0=0;
	
}