/*
Autor: Mohamad Sadeque
Driver i2c
 */ 

#include <xc.h>
typedef enum { escrita, leitura } mode_i2c;
typedef enum { false, true } bool;

void setupI2C();
void ERROR();
bool MT_transmit(uint8_t addr, uint8_t data);
bool MT_receive(uint8_t addr);
#define START 0x08
#define MT_SLA_ACK 0x18
#define MR_SLA_ACK 0x40
#define MT_DATA_ACK 0x28
#define MR_DATA_ACK 0x50
#define MR_DATA_NACK 0x58

int main(void)
{
	setupI2C();
	PORTB = 0x00;
	PORTC |= (1<<PORTC3);
	if(!MT_transmit(0x40,0xff)) PORTB = 0xFF;
	if(!MT_receive(0x41)) PORTB = 0xAF;



    while(1)
    {
        //TODO:: Please write your application code 
	if(!(PINC & (1<<PINC3))) MT_receive(0x41);
	}
}

void setupI2C(){
	DDRC = 0x30; 
	TWBR = (1<<TWBR1); // Configurando fator de divisão da taxa de bits para 2 
	TWSR |= (1<<TWPS1); // Configurando Prescaler da taxa  de bits para 16
}




bool MT_transmit(uint8_t addr, uint8_t data){
	TWCR = (1<< TWINT)|(1<<TWSTA)|(1<<TWEN); // start
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != START) return false; 
	TWDR  = addr;				// envia endereço no modo de escrita
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_SLA_ACK) return false; 
	TWDR  = data;				// envia dados
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8)!= MT_DATA_ACK) return false;
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); // stop
	return true;
}

bool MT_receive(uint8_t addr){
	TWCR = (1<< TWINT)|(1<<TWSTA)|(1<<TWEN); // start
	
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != START) return false;
	TWDR  = addr;				// envia endereço no modo de leitura
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MR_SLA_ACK) return false;	// confere ACK do escravo após endereçamento
	
	TWCR = (1<<TWINT) | (1<<TWEN);					// Recebe byte de dados
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MR_DATA_NACK) return false;
	
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); // stop
	PORTD = TWDR;
	return true;
}

