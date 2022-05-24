/*
 * Indivitual_Contribution.c
 *
 * Created: 5/13/2022 3:06:15 PM
 * Author : Lucifer
 */ 


#define F_CPU 8000000
#include <avr/io.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include "LCDI2C.h"
#include "USART.h"

const uint8_t kaypad[4][4]={{7,8,9,10},{4,5,6,11},{1,2,3,12},{13,0,14,15}};
uint8_t getkeyNum();
void AddWeight();
void Addtime(uint8_t val);

uint8_t hx711H=0; //Load Scale High Bits
uint16_t hx711L=0;//Load Scale Low Bits
float loadCellRead();
#define Load_data 4
#define Load_clk 5
float loadCellRead();
void servo(uint8_t val);
float hx=0;

uint16_t ReadADC(uint8_t ADCchannel);
float ph=0;
char lcddata[20];

void setTime(uint8_t hSet,uint8_t mSet,uint8_t sSet );
void Readtime();
void sendSMS(char*sms);
uint8_t hourC;
uint8_t minC;
uint8_t secC;

uint8_t feedingTimeCheckBit = 0;
uint16_t remainFood = 0;

/*#define list*/
uint8_t lampTimes[4]={8,30,8,31};//8.30-3.30pm
uint8_t tempuratureLimit[2]={10,40};
float phLimit[2]={5,9};
uint16_t foodLimit=2000;


/*eeprom address */
uint8_t feedingdataAdress[3]={0,2,4};
uint8_t phoneNoAddress[11] = {7,8,9,10,11,12,13,14,15,16,17};
char *phoneNo;
#define WeightDataAdress 6

void adddata();
void AddWeight();

uint8_t feedingTimes[3][2]={0};
uint16_t feedingWeight=0;

volatile uint16_t TimerCal=0;// variable for collect echo data
uint16_t ultraINT0=0;

int main(void)
{	/*ADC init part*/
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));   // prescaler 128
	ADMUX |= (1<<REFS0);					//external vcc
	ADCSRA |= (1<<ADEN);                            // Turn on ADC
	DDRC|=(1<<7);//buzzer
	DDRC|=(1<<6);//lamp
	DDRD|=(1<<Load_clk); //Load cell clock pin
	PORTD&=~(1<<Load_clk);//Clock pin low
	
	LcdInit();
	USART_Init(9600);
	
	
	LcdSetCursor(4,0,"Welcome");
	_delay_ms(1000);
	LcdCommand(LCD_CLEARDISPLAY);
	feedingWeight=eeprom_read_word((uint16_t*)WeightDataAdress);
	for(uint8_t i=0;i<3;i++){
		feedingTimes[i][0]=eeprom_read_byte((uint8_t*)feedingdataAdress[i]);
		feedingTimes[i][1]=eeprom_read_byte((uint8_t*)feedingdataAdress[i]+1);
	}
	
	//phoneNo = readPhoneNo();
	//phoneNo = eeprom_read_word((uint32_t*)PhoneNoAddress);
	
//	sprintf(lcddata,"%u:%u %u:%u ",feedingTimes[0][0],feedingTimes[0][1],feedingTimes[1][0],feedingTimes[1][1]);
//	LcdSetCursor(0,0,lcddata);
//	_delay_ms(3000);
	
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
	
	/*ph=ReadADC(0)*5.0/1024;
	ph=ph*3.5;
	sprintf(lcddata,"%0.2f",ph);
	LcdSetCursor(0,0,lcddata);*/
	
	
	
	while(1){
		LcdSetCursor(0,0,"Need data Enter?");
		LcdSetCursor(0,1,"Yes-1 No-2");
		
		uint8_t key=getkeyNum();
		if (key==1)
		{adddata();
			
		}
		
		if (key==2)
		{	LcdCommand(LCD_CLEARDISPLAY);
			setTime(11,20,59);
			break;
		}
	}
	DDRD|=(1<<6);
	PORTD|=(1<<6);
	
	
	
//#######################################################################################	
	while (1)
	{	
		Readtime();
		sprintf(lcddata,"%02u:%02u:%02u",hourC,minC,secC);
		LcdSetCursor(4,0,lcddata);
		
		ADMUX = (1<<REFS0);			//external vcc
		ph=ReadADC(0)*5.0/1024;
		ph=ph*3.5;
		
		ADMUX = (1<<REFS0)|(1<<REFS1);					//internal 2.56 v ref
		uint16_t TempReading=(ReadADC(1)*0.25024438); //calibrated number
		
		
		
		PORTD&=~(1<<3);//TRIG pin low
		_delay_us(50);//wait 50 micro sec
		PORTD|=(1<<3);//TRIG pin high
		_delay_us(50);//wait 50 micro sec
		PORTD&=~(1<<3);////TRIG pin low
		while(!(PIND&(1<<2)))//wait for pulse
		TimerCal=0;//rest timer
		while((PIND&(1<<2)))////wait for pulse down
		ultraINT0=TimerCal/4.148148148148148;//copy timer value
		
		
		float hx=loadCellRead();
		uint16_t weight=hx*1000;
		
		sprintf(lcddata,"%0.1f %02u %02u %05u",ph,TempReading,ultraINT0,weight);
		LcdSetCursor(0,1,lcddata);
		
		if ((TempReading<tempuratureLimit[0])||(TempReading>tempuratureLimit[1]))
		{
			sendSMS("Temperature's Limit Exceeded");
		}
		
		if ((ph<phLimit[0])||(ph>phLimit[1]))
		{
			sendSMS("Ph Limit Exceeded");
		}
		if (weight<foodLimit)
		{
			sendSMS("Fill the foods");
		}
		if (PINA&(0b11100))
		{
			sendSMS("Motion detected");
		}
		
		if (ultraINT0<50)
		{
			sendSMS("Water level low");
		}
		if (ultraINT0>80)
		{
			sendSMS("Water level High");
		}
		
		//sprintf(lcddata,"%02u:%02u:%02u:%02u:%02u:%02u",hourC,feedingTimes[0][0],minC,feedingTimes[0][1]);
		//LcdSetCursor(0,0,lcddata);
		
		if ((lampTimes[0]==hourC)&&(lampTimes[1]==minC))
		{PORTC|=(1<<6);//lamp on
		}
		
		if ((lampTimes[3]==hourC)&&(lampTimes[4]==minC))
		{PORTC&=~(1<<6);//lamp off
		}
		
		
		//Feeding
		if (((hourC==feedingTimes[0][0]&&minC==feedingTimes[0][1])||(hourC==feedingTimes[1][0]&&minC==feedingTimes[1][1])||(hourC==feedingTimes[2][0]&&minC==feedingTimes[2][1])) && (secC<2))
		{PORTD&=~(1<<6);
			LcdCommand(LCD_CLEARDISPLAY);
			LcdSetCursor(0,0,"Feeding");
			PORTC|=(1<<7);
			DDRD|=(1<<7);
			
			for(uint8_t j=0;j<100;j++){
				PORTD|=(1<<7);
				for(uint8_t i=0;i<9;i++){
					_delay_us(100);
				}
				PORTD&=~(1<<7);
				
				
				for(uint8_t i=0;i<9;i++){
					_delay_us(100);
				}
			}
			
			for(uint8_t j=0;j<100;j++){
				PORTD|=(1<<7);
				for(uint8_t i=0;i<20;i++){
					_delay_us(100);
				}
				PORTD&=~(1<<7);
				
				
				for(uint8_t i=0;i<20;i++){
					_delay_us(100);
				}
			}
			
			
			hx=loadCellRead();
			uint16_t weight=hx*1000;
			_delay_ms(100);
			while(1){
				hx=loadCellRead();
				uint16_t weightNow=hx*1000;
				weightNow+=feedingWeight;
				//LcdSetCursor(0,1,lcddata);
				//sprintf(lcddata,"%04u %04u",weightNow,weight);
				_delay_ms(100);
				if (weightNow<=weight)
				{break;
				}

			}
			LcdCommand(LCD_CLEARDISPLAY);
			
			_delay_ms(1000);
			for(uint8_t j=0;j<100;j++){
				PORTD|=(1<<7);
				for(uint8_t i=0;i<9;i++){
					_delay_us(100);
				}
				PORTD&=~(1<<7);
				
				
				for(uint8_t i=0;i<9;i++){
					_delay_us(100);
				}
			}
			}_delay_ms(500);
			
			PORTC&=~(1<<7);
			PORTD|=(1<<6);
		}
		
		_delay_ms(100);
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

void addContactNo(){
	
	LcdCommand(LCD_CLEARDISPLAY);
	LcdSetCursor(0,0,"Enter Contact No.");
	_delay_ms(500);
	uint8_t pos=0;
	uint8_t numbers[10]={0,0,0,0,0,0,0,0,0,0};
	uint8_t posData[10][2]={{0,9},{1,9},{2,9},{3,9},{4,9},{5,9},{6,9},{7,9},{8,9},{9,9}};


	sprintf(lcddata,"%u%u%u%u%u%u%u%u%u%u",numbers[0],numbers[1],numbers[2],numbers[3],numbers[4],numbers[5],numbers[6],numbers[7],numbers[8],numbers[9]);
	LcdSetCursor(0,1,lcddata);
	LcdSetCursor(pos,1,"");
	LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKON);

	while(1){
		uint8_t keyout=getkeyNum();
		
		if (keyout<=posData[pos][1])
		{
			numbers[pos]=keyout;
			sprintf(lcddata,"%u%u%u%u%u%u%u%u%u%u",numbers[0],numbers[1],numbers[2],numbers[3],numbers[4],numbers[5],numbers[6],numbers[7],numbers[8],numbers[9]);
			LcdSetCursor(0,1,lcddata);
			pos++;
			if (pos>10)
			{pos=0;
			}
			LcdSetCursor(posData[pos][0],1,"");
			_delay_ms(200);
		}
		
		else if (keyout==14)
		{
			LcdCommand(LCD_CLEARDISPLAY);
			LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKOFF);
			LcdSetCursor(0,0,"Number Added");
			eeprom_write_word((uint8_t*)phoneNoAddress[0],9);
			eeprom_write_word((uint8_t*)phoneNoAddress[1],4);
			for (int i=1; i<10;i++)
			{
				eeprom_write_word((uint8_t*)phoneNoAddress[i+1],numbers[i]);
				
			}
			
			
			_delay_ms(500);
			break;
		}
	}
	
}

void setOngoingTime(){
	LcdCommand(LCD_CLEARDISPLAY);
	LcdSetCursor(0,0,"Add current Time ");
	_delay_ms(500);
	
	uint8_t pos=0;
	uint8_t numbers[6]={0,0,0,0,0,0};
	uint8_t posData[6][2]={{0,2},{1,9},{3,5},{4,9},{6,5},{7,9}};
	
	
	sprintf(lcddata,"%u%u:%u%u:%u%u",numbers[0],numbers[1],numbers[2],numbers[3],numbers[4],numbers[5]);
	LcdSetCursor(0,1,lcddata);
	LcdSetCursor(pos,1,"");
	LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKON);
	
	while(1){
		uint8_t keyout=getkeyNum();
		
		if (keyout<=posData[pos][1])
		{
			numbers[pos]=keyout;
			sprintf(lcddata,"%u%u:%u%u:%u%u",numbers[0],numbers[1],numbers[2],numbers[3],numbers[4],numbers[5]);
			LcdSetCursor(0,1,lcddata);
			pos++;
			if (pos>5)
			{pos=0;
			}
			LcdSetCursor(posData[pos][0],1,"");
			_delay_ms(200);
		}
		
		else if(keyout==13){
			break;
		}
	
		
		else if (keyout==14)
		{
			LcdCommand(LCD_CLEARDISPLAY);
			LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKOFF);
			LcdSetCursor(0,0,"Time Added");
			setTime((numbers[0]*10)+numbers[1], (numbers[2]*10)+numbers[3], (numbers[4]*10)+numbers[5]);
			break;
		}
	}
}
void adddata(){
	LcdCommand(LCD_CLEARDISPLAY);
	
	setOngoingTime();
	//addContactNo();
	LcdSetCursor(0,0,"Select Schedule");
	uint8_t pos=0;
	char*text[4]={"Feeding time 1","Feeding time 2","Feeding time 3","Food weight    "};
	LcdSetCursor(0,1,text[pos]);
	while(1){
		uint8_t key=getkeyNum();//15
		if (key==15)
		{ 
			pos++;
			if (pos>3)
			{pos=0;
			}
			LcdSetCursor(0,1,text[pos]);
			
		}
		
		else if(key==13){
			break;
		}
		
		if (key==14)
		{
			LcdCommand(LCD_CLEARDISPLAY);
			LcdSetCursor(0,0,"Selected");
			_delay_ms(1000);
			break;
		}
		
	}
	
	if (pos<3)
	{
		Addtime(pos);
		
	} 
	else
	{AddWeight(pos);
	}
	
}

void Addtime(uint8_t val){
	LcdCommand(LCD_CLEARDISPLAY);
	LcdSetCursor(0,0,"Enter time");
	_delay_ms(500);
	uint8_t pos=0;
	uint8_t numbers[4]={0,0,0,0};
	uint8_t posData[4][2]={{0,2},{1,9},{3,5},{4,9}};
	
	
	sprintf(lcddata,"%u%u:%u%u",numbers[0],numbers[1],numbers[2],numbers[3]);
	LcdSetCursor(0,1,lcddata);
	LcdSetCursor(pos,1,"");
	LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKON);
	
	while(1){
		uint8_t keyout=getkeyNum();
		
		if (keyout<=posData[pos][1])
		{
			numbers[pos]=keyout;
			sprintf(lcddata,"%u%u:%u%u",numbers[0],numbers[1],numbers[2],numbers[3]);
			LcdSetCursor(0,1,lcddata);
			pos++;
			if (pos>3)
			{pos=0;
			}
			LcdSetCursor(posData[pos][0],1,"");
			_delay_ms(200);
		}
		
		else if (keyout==14)
		{
		LcdCommand(LCD_CLEARDISPLAY);
		LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKOFF);
		LcdSetCursor(0,0,"Time Added");
		feedingTimes[val][0]=numbers[0]*10+numbers[1];
		feedingTimes[val][1]=numbers[2]*10+numbers[3];
		eeprom_write_byte((uint8_t*)feedingdataAdress[val],feedingTimes[val][0]);
		eeprom_write_byte((uint8_t*)feedingdataAdress[val]+1,feedingTimes[val][1]);
		_delay_ms(500);
		break;
		}
	}
	
}
void AddWeight(){
LcdCommand(LCD_CLEARDISPLAY);
LcdSetCursor(0,0,"Enter Weight");
_delay_ms(500);
uint8_t pos=0;
uint8_t numbers[4]={0,0,0,0};
uint8_t posData[4][2]={{0,9},{1,9},{2,9},{3,9}};


sprintf(lcddata,"%u%u%u%ug",numbers[0],numbers[1],numbers[2],numbers[3]);
LcdSetCursor(0,1,lcddata);
LcdSetCursor(pos,1,"");
LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKON);

while(1){
	uint8_t keyout=getkeyNum();
	
	if (keyout<=posData[pos][1])
	{
		numbers[pos]=keyout;
		sprintf(lcddata,"%u%u%u%ug",numbers[0],numbers[1],numbers[2],numbers[3]);
		LcdSetCursor(0,1,lcddata);
		pos++;
		if (pos>3)
		{pos=0;
		}
		LcdSetCursor(posData[pos][0],1,"");
		_delay_ms(200);
	}
	
	else if (keyout==14)
	{
		LcdCommand(LCD_CLEARDISPLAY);
		LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKOFF);
		LcdSetCursor(0,0,"Weight Added");
		feedingWeight=(numbers[0]*1000)+(numbers[1]*100)+(numbers[2]*10)+numbers[3];
		eeprom_write_word((uint16_t*)WeightDataAdress,feedingWeight);
		_delay_ms(500);
		break;
	}
}	
}


void setTime(uint8_t hSet,uint8_t mSet,uint8_t sSet){
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
	bit[2]=TWIReadNACK();
	TWIStop();
	secC=(((bit[0]&0xF0)>>4)*10)+(bit[0]&0xF);
	minC=(((bit[1]&0xF0)>>4)*10)+(bit[1]&0xF);
	hourC=(((bit[2]&0b00110000)>>4)*10)+(bit[2]&0xF);
}

char *readPhoneNo(){
	char *contactNo;
	contactNo[0]="+";
	
	
	for (int i=0; i<11;i++)
	{
		contactNo[i+1] = eeprom_read_word((uint8_t*)phoneNoAddress[i]);
	}
	
	return "+";
	//feedingWeight=eeprom_read_word((uint8_t*)WeightDataAdress);
}

void sendSMS(char*sms){
	char str[50];
	//char *querry = "AT+CMGS=\"%s\"",str;
	//sprintf(str,"AT+CMGS=\"%s\"\r",readPhoneNo());
	
	char *contactNo;
	contactNo[0]="+";
	
	
/*	for (int i=0; i<11;i++)
	{
		contactNo[i+1] = eeprom_read_word((uint8_t*)phoneNoAddress[i]);
	}*/
	sprintf(str,"AT+CMGS=\"%s\"",contactNo);
	
	PORTA|=(1<<7);// buzzer
	_delay_ms(500);
	USART_TxStringln("AT");
	_delay_ms(500);
	USART_TxStringln("AT");
	_delay_ms(500);
	//USART_TxStringln(str);
	USART_TxStringln("AT+CMGS=\"+947672323154\"");
	_delay_ms(500);
	USART_TxStringln("AT+CMGF=1");
	_delay_ms(500);
	USART_TxStringln(sms);
	_delay_ms(500);
	USART_Transmit(26);
	PORTA&=~(1<<7);
}

ISR(TIMER0_COMP_vect){//ultrasonic
	TimerCal++;
	TCNT0=0;
	
}

float loadCellRead(){
	hx711H=0;hx711L=0;  //clear variables
	for(uint8_t i=0;i<8;i++){  // Load cell data high 8 bits
		PORTD|=(1<<Load_clk); //Clock pin high
		_delay_us(10);
		if ((PIND&(1<<Load_data))>>Load_data)  //read data pin
		{hx711H|=(1<<(7-i));//set hx 711 varible
		}
		else
		{hx711H&=~(1<<(7-i));
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
void servo(uint8_t val){
	
}