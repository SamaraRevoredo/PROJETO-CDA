/*
Autor: Mohamad Sadeque
Driver i2c
 */ 

#include <xc.h>
typedef enum { escrita, leitura } mode_i2c;
typedef enum { false, true } bool;
	
#define addr_Sensores_escrita 0x40
#define addr_Sensores_leitura 0x41

#define addr_LCD_escrita 0x42
#define addr_LCD_leitura 0x43

void setupI2C();
bool MT_transmit(uint8_t addr, uint8_t data);
bool MT_receive(uint8_t addr);
#define START 0x08
#define MT_SLA_ACK 0x18
#define MR_SLA_ACK 0x40
#define MT_DATA_ACK 0x28
#define MR_DATA_ACK 0x50
#define MR_DATA_NACK 0x58

// LCD
void send_instruction_lcd(uint8_t data);
void send_instruction_lcd_nibble(uint8_t data);
void inicializa_display();
void send_word_lcd(uint8_t data);
void set_DDRAM_ADDR(uint8_t pos);
void envia_frase(char *frase,uint8_t pos);


void inicializa_display_i2c();
void send_instruction_lcd_i2c(uint8_t data);
void send_instruction_lcd_nibble_i2c(uint8_t data);
void send_word_lcd_i2c(uint8_t data);
void set_DDRAM_ADDR_i2c(uint8_t pos);
void envia_frase_i2c(char *frase,uint8_t pos);
void passarValor(const char *data);
char aux_string[15];
enum display_pinos{
	DB4,
	DB5,
	DB6,
	DB7,
	EN = 5,
	RW,
	RS
};
enum posicao_lcd{
	l1C1,
	l1C2,
	l1C3,
	l1C4,
	l1C5,
	l1C6,
	l1C7,
	l1C8,
	l1C9,
	l1C10,
	l1C11,
	l1C12,
	l1C13,
	l1C14,
	l1C15,
	l1C16,
	l2C1 = 0x40,
	l2C2,
	l2C3,
	l2C4,
	l2C5,
	l2C6,
	l2C7,
	l2C8,
	l2C9,
	l2C10,
	l2C11,
	l2C12,
	l2C13,
	l2C14,
	l2C15,
	l2C16
};
long int x = 0;
void delay_ms(long int n){
	
	
	x = 0;
	
	long int m = 1000 * n;
	
	while(x <= m){
		x++;
	}
	
	
}

void delay_ns(long int n){
	
	
	x = 0;
	
	long int m = 1 * n;
	
	while(x <= m){
		x++;
	}
	
	
}
int main(void)
{
	setupI2C();
	DDRB = 0x0F;
	PORTB = 0xF0;
	PORTC |= (1<<PORTC3);
	if(!MT_transmit(0x40,0xff)) PORTB = 0xFF;
	if(!MT_receive(0x41)) PORTB = 0xAF;
	inicializa_display_i2c();
	passarValor("DIGITE A SENHA");
	envia_frase_i2c(aux_string,l1C1);
	set_DDRAM_ADDR_i2c(l2C7);
	
	uint8_t testeChar = '*';
	for(int i = 0; i < 4; i++) send_word_lcd_i2c(testeChar);
	DDRD = 0xFE;
    while(1)
    {
        //TODO:: Please write your application code 
	if(!(PINC & (1<<PINC3))) MT_receive(0x41);

	}
}

void setupI2C(){
	DDRC = 0x30; 
	TWBR = (1<<TWBR1); // Configurando fator de divis�o da taxa de bits para 2 
	TWSR |= (1<<TWPS1); // Configurando Prescaler da taxa  de bits para 16
}




bool MT_transmit(uint8_t addr, uint8_t data){
	TWCR = (1<< TWINT)|(1<<TWSTA)|(1<<TWEN); // start
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != START) return false; 
	TWDR  = addr;				// envia endere�o no modo de escrita
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
	TWDR  = addr;				// envia endere�o no modo de leitura
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MR_SLA_ACK) return false;	// confere ACK do escravo ap�s endere�amento
	
	TWCR = (1<<TWINT) | (1<<TWEN);					// Recebe byte de dados
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MR_DATA_NACK) return false;
	
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); // stop
	return true;
}


void send_instruction_lcd_i2c(uint8_t data){
	uint8_t aux_inst = 0x00;
	
	aux_inst &= ~(1<<RS);  // RS = 0 => intru��es
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (1 << EN);  // Seta o enable
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (data >> 4); // seta barramento de dados com nibble mais significativo
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst &= ~(1<<EN);  // EN = 0
	MT_transmit(addr_LCD_escrita,aux_inst);

	
	aux_inst &= 0x00;
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (1 << EN);  // Seta o enable
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= data & 0x0F; // seta barramento de dados com nibble mais significativo
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst &= ~(1<<EN);  // EN = 0
	MT_transmit(addr_LCD_escrita,aux_inst);

}

void send_instruction_lcd_nibble_i2c(uint8_t data){
	uint8_t aux_inst = 0x00;
	aux_inst &= ~(1<<RS);  // RS = 0 => intru��es
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (1 << EN);  // Seta o enable
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= data; // seta barramento de dados com nibble mais significativo
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst &= ~(1<<EN);  // EN = 0
	MT_transmit(addr_LCD_escrita,aux_inst);
}




void inicializa_display_i2c(){
	
	delay_ms(15);

	send_instruction_lcd_nibble_i2c(0x03);
	delay_ms(5);

	send_instruction_lcd_nibble_i2c(0x03);
	delay_ns(100);

	send_instruction_lcd_nibble_i2c(0x03);//
	delay_ns(40);
	
	send_instruction_lcd_nibble_i2c(0x02);//
	delay_ns(40);

	send_instruction_lcd_i2c(0x28); // function set
	delay_ns(40);

	send_instruction_lcd_i2c(0x01); // clear display
	delay_ms(2);
	send_instruction_lcd_i2c(0x06); //
	delay_ns(40);

	send_instruction_lcd_i2c(0x0C); // display
	delay_ns(40);
	
}


void send_word_lcd_i2c(uint8_t data){
	uint8_t aux_inst = 0x00;
	
	aux_inst |= (1<<RS);  // RS = 1 => dados
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (1 << EN);  // Seta o enable
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (data >> 4);// seta barramento de dados com nibble mais significativo
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst &= ~(1<<EN);  // EN = 0
	MT_transmit(addr_LCD_escrita,aux_inst);

	
	aux_inst &= 0xF0;
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (1 << EN);  // Seta o enable
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= data & 0x0F; // seta barramento de dados com nibble menos significativo
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst &= ~(1<<EN);  // EN = 0
	MT_transmit(addr_LCD_escrita,aux_inst);


	delay_ns(43);

}


void set_DDRAM_ADDR_i2c(uint8_t pos){
	uint8_t data = 0x80 | pos;
	uint8_t aux_inst = 0x00;
	aux_inst &= 0xC0;
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst &= ~(1<<RS);  // RS = 0 => intru��es
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (1 << EN);  // Seta o enable
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (data >> 4); // seta barramento de dados com nibble mais significativo
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst &= ~(1<<EN);  // EN = 0
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst &= 0xC0;
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= (1 << EN);  // Seta o enable
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst |= data & 0x0F; // seta barramento de dados com nibble menos significativo
	MT_transmit(addr_LCD_escrita,aux_inst);
	aux_inst &= ~(1<<EN);  // EN = 0
	MT_transmit(addr_LCD_escrita,aux_inst);

	delay_ns(43);
}

void envia_frase_i2c(char *frase,uint8_t pos){
	set_DDRAM_ADDR_i2c(pos);
	uint8_t cont = 0;
	while(frase[cont] != '\0'){
		send_word_lcd_i2c(frase[cont]);
		cont++;
	}
}

void passarValor(const char *data){
	uint8_t cont_valor = 0;
	while(data[cont_valor] != '\0'){
		aux_string[cont_valor] = data[cont_valor];
	cont_valor++;
	}
	aux_string[cont_valor] = '\0';
}