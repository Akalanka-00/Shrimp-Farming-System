#define F_CPU 8000000
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "LCDI2C.h"

const uint8_t kaypad[4][4]={{7,8,9,10},{4,5,6,11},{1,2,3,12},{13,0,14,15}};
uint8_t getkeyNum();
volatile uint16_t TimerCal=0;// variable for collect echo data
uint16_t ultraINT0=0;

char lcddata[20];
int main(void)
{
	LcdInit(0x27);
	/*Ultrasonic Timer Part*/
	DDRD|=(1<<3); //D4 as output
	TCCR0|=(1<<WGM01);//Enable Compare match mode
	TCCR0|=(1<<CS11);//Start timer  prescaler =8
	TCNT0=0;
	OCR0=10;
	/*register value= time*(clock speed/prescale)
	register value=0.000001*(8000000/1)
	register value=10*/
	TIMSK|=(1<<OCIE0);//enable timer Compare inturrept
	sei();	
	_delay_ms(500);
	PORTD&=~(1<<3);//TRIG pin low
	_delay_us(50);//wait 50 micro sec
	PORTD|=(1<<3);//TRIG pin high
	_delay_us(50);//wait 50 micro sec
	PORTD&=~(1<<3);////TRIG pin low
	while(!(PIND&(1<<2)))//wait for pulse
	TimerCal=0;//rest timer
	while((PIND&(1<<2)))////wait for pulse down
	ultraINT0=TimerCal/4.12;//copy timer value
	
	sprintf(lcddata,"%02u",ultraINT0);
	LcdSetCursor(0,0,lcddata);
	DDRD|=(1<<4);
	
	for(uint8_t j=0;j<100;j++){
		PORTD|=(1<<4);
		for(uint8_t i=0;i<10;i++){
			_delay_us(100);
		}
		PORTD&=~(1<<4);
		
		uint8_t ser=200-10;
		
		for(uint8_t i=0;i<ser;i++){
			_delay_us(100);
		}
	}
	_delay_ms(500);
	for(uint8_t j=0;j<100;j++){
		
		PORTD|=(1<<4);
		for(uint8_t i=0;i<15;i++){
			_delay_us(100);
		}
		PORTD&=~(1<<4);
		
		uint8_t ser=200-15;
		
		for(uint8_t i=0;i<15;i++){
			_delay_us(100);
		}
	}_delay_ms(500);
	
	while (1)
	{	uint8_t key=getkeyNum();
		sprintf(lcddata,"%02u",key);
		LcdSetCursor(0,1,lcddata);
		if (key>5)
		{
		}
		else
		{
		}
	}
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

ISR(TIMER0_COMP_vect){//ultrasonic
	TimerCal++;
	TCNT0=0;
	
}