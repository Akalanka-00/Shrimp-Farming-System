#define F_CPU 8000000 //Internal osilater speed 
#include <avr/io.h>
#include <util/delay.h> //Delay library

uint8_t hx711H=0; //Load Scale High Bits     
uint16_t hx711L=0;//Load Scale Low Bits       
float loadCellRead(); //define function
#define Load_data 2
#define Load_clk 3
#define MotorCCW 2
#define MotorCw 3

float loadCellRead();
void MotorControl(uint8_t dir);
float hx=0;

int main(void)
{
	DDRD|=(1<<Load_clk); //Load cell clock pin
	PORTD&=~(1<<Load_clk);//Clock pin low
	DDRC|=(1<<5)|(1<<MotorCw)|(1<<MotorCCW);//buzzer & motor
	
	while (1)
	{
		hx=loadCellRead();
		if (hx>15)
		{PORTC|=(1<<5);MotorControl(1);   //Port C 5 pin ON (buzzer)   //And give to MotorControl function = 1
		}
		else
		{PORTC&=~(1<<5);MotorControl(0);  ////Port C 5 pin OFF (buzzer)   //And give to MotorControl function = 0
		}
		_delay_ms(100);
	}
}

void MotorControl(uint8_t dir){     
	
	if (dir)  
	{PORTC|=(1<<MotorCCW);   
		PORTC&=~(1<<MotorCw); 
	}
	else
	{PORTC|=(1<<MotorCw); 
		PORTC&=~(1<<MotorCCW); 
	}
}

float loadCellRead(){
	hx711H=0;hx711L=0;  //clear variables
	for(uint8_t i=0;i<8;i++){  // Load cell data high 8 bits
		PORTD|=(1<<Load_clk); //Clock pin high
		_delay_us(10);
		if ((PIND&(1<<Load_data))>>Load_data)  //read data pin  
		{hx711H|=(1<<(7-i));//set hx 711 variable  
		}
		else
		{hx711H&=~(1<<(7-i));   //hx711H variable bit low
		}
		PORTD&=~(1<<Load_clk); //Clock pin low
		_delay_us(5);
	}
	
	
	for(uint8_t i=0;i<16;i++){ // Load cell data low 16 bits   
		PORTD|=(1<<Load_clk); //Clock pin high
		_delay_us(10);
		if ((PIND&(1<<Load_data))>>Load_data) //read data pin  
		{hx711L|=(1<<(15-i));
		}
		else
		{hx711L&=~(1<<(15-i));
		}
		PORTD&=~(1<<Load_clk); //Clock pin low
		_delay_us(5);
	}
	
	hx711L=hx711L>>1; //shift bits      
	
	if (hx711H&1)  //bit setup
	{hx711L|=(1<<15);
	}
	else
	{hx711L&=~(1<<15);
	}
	hx711H=hx711H>>1;
	
	return (hx711H*(65536/18029.6))+hx711L/18029.6; //load cell calibration
}
