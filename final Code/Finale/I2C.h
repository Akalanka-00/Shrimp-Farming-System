
//I2C Header


void TWIInit();
void TWIStart(void);
void TWIStop(void);
uint8_t TWIWriteAddrs(uint8_t u8data,uint8_t u9data);
uint8_t TWIWriteData(uint8_t u8data);
uint8_t TWIReadACK(void);
uint8_t TWIReadNACK(void);


void TWIInit(){
	//set SCL to 100kHz
	TWSR = 0x00; //prescale =1
	TWBR = 0x48;  // 0x48=72
	//enable TWI
	TWCR = (1<<TWEN);
}

void TWIStart(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
}
//send stop signal
void TWIStop(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
	while(TWCR&(1<<TWSTO));	/* Wait until stop condition execution */
}

uint8_t TWIWriteAddrs(uint8_t u8data,uint8_t u9data/*=0*/)
{	
	TWDR = (u8data<<1)|u9data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
	return TWSR &(0xF8);
}
uint8_t TWIWriteData(uint8_t u8data)
{
	TWDR = u8data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
	return TWSR &(0xF8);
}

uint8_t TWIReadACK(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}
//read byte with NACK
uint8_t TWIReadNACK(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}