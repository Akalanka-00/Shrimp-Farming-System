/*
 * Indivitual_Contribution.c
 *
 * Created: 5/11/2022 6:06:15 PM
 * Author : Lucifer
 */ 

#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "headers/LCDI2C.h"
#include "headers/USART.h"

void setTime(uint8_t hSet,uint8_t mSet,uint8_t sSet /*,uint8_t dSet*/ ,uint8_t dateSet,uint8_t monthSet, uint8_t yearSet);
void Readtime();

uint8_t dayC;
uint8_t hourC;
uint8_t minC;
uint8_t secC;
uint8_t dateC;
uint8_t monthC;
uint8_t YearC;

char lcddata[20];

int main(void){
	_delay_ms(500);
	LcdInit(0x27);
	USART_Init(9600);
	_delay_ms(500);
	USART_TxStringln("AT");
	_delay_ms(500);
	USART_TxStringln("AT");
	_delay_ms(500);
	USART_TxStringln("AT+CMGF=1");
	_delay_ms(500);
	USART_TxStringln("Hello World");
	
	setTime(22,9,33,23,12,21); //hour , minute, second, date, month, year
	
	while (1)
	{Readtime();
		sprintf(lcddata,"%02u:%02u:%02u",hourC,minC,secC);
		LcdSetCursor(4,0,lcddata);
		sprintf(lcddata,"%02u %02u 20%02u",dateC,monthC,YearC);
		LcdSetCursor(3,1,lcddata);
	}
}

void setTime(uint8_t hSet,uint8_t mSet,uint8_t sSet /*,uint8_t dSet*/ ,uint8_t dateSet,uint8_t monthSet, uint8_t yearSet){
	TWIStart();
	TWIWriteAddrs(104,0);
	TWIWriteData(0x02);
	TWIWriteData(((hSet/10)<<4)|(hSet%10));
	TWIStop();
	
	TWIStart();
	TWIWriteAddrs(104,0);
	TWIWriteData(0x01);
	TWIWriteData(((mSet/10)<<4)|(mSet%10));
	TWIStop();
	
	TWIStart();
	TWIWriteAddrs(104,0);
	TWIWriteData(0x00);
	TWIWriteData(((sSet/10)<<4)|(sSet%10));
	TWIStop();
	
	/*TWIStart();
	TWIWriteAddrs(104,0);
	TWIWriteData(0x3);
	TWIWriteData(dSet);
	TWIStop();*/
	
	TWIStart();
	TWIWriteAddrs(104,0);
	TWIWriteData(0x04);
	TWIWriteData(((dateSet/10)<<4)|(dateSet%10));
	TWIStop();
	
	TWIStart();
	TWIWriteAddrs(104,0);
	TWIWriteData(0x05);
	TWIWriteData(((monthSet/10)<<4)|(monthSet%10));
	TWIStop();
	
	TWIStart();
	TWIWriteAddrs(104,0);
	TWIWriteData(0x06);
	TWIWriteData(((yearSet/10)<<4)|(yearSet%10));
	TWIStop();
	
}


void Readtime(){
	uint8_t bit[7];
	TWIStart();/* Start I2C with device write address */
	TWIWriteAddrs(104,0); //Return 24 mean device found, return mean no device
	TWIWriteData(0);  //Register address
	
	TWIStart();
	TWIWriteAddrs(104,1); //Write bit
	bit[0]=TWIReadACK();
	bit[1]=TWIReadACK();
	bit[2]=TWIReadACK();
	bit[3]=TWIReadACK();
	bit[4]=TWIReadACK();
	bit[5]=TWIReadACK();
	bit[6]=TWIReadNACK();
		
	//h|=TWIReadACK()<<8;
	//h|=TWIReadNACK()<<16;
	TWIStop();
	secC=(((bit[0]&0xF0)>>4)*10)+(bit[0]&0xF);
	minC=(((bit[1]&0xF0)>>4)*10)+(bit[1]&0xF);
	hourC=(((bit[2]&0b00110000)>>4)*10)+(bit[2]&0xF);
	dayC=bit[3]&0b111;
	dateC=(((bit[4]&0xF0)>>4)*10)+(bit[4]&0xF);
	monthC=(((bit[5]&0xF0)>>4)*10)+(bit[5]&0xF);
	YearC=(((bit[6]&0xF0)>>4)*10)+(bit[6]&0xF);
}
