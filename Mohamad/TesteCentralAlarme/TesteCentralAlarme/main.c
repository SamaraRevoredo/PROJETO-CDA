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
//MDE
bool flag_timeout = false;
uint8_t ultima_tecla;
void rotina_geral_dos_botoes();
uint8_t modo;
void rotina_programacao();
void rotina_desativado();
void rotina_ativado();
enum modos{
	desativado,
	ativado,
	programacao
	};
enum teclas{
	nenhuma_tecla,
	t_1,
	t_2,
	t_3,
	t_p,
	t_4,
	t_5,
	t_6,
	t_a,
	t_7,
	t_8,
	t_9,
	t_d,
	t_r,
	t_0,
	t_s,
	t_e
};
enum permissao{
	acesso_negado,
	ativacao,
	mestre
	};
	
bool leitura_senha(bool modo_leitura);
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
		rotina_geral_dos_botoes();
		switch (modo)
		{
			case desativado:
				rotina_desativado();
			break;
			case ativado:
				rotina_ativado();
			break;
			case programacao:
				rotina_programacao();
			break;			
		}
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
	return true;
}


void send_instruction_lcd_i2c(uint8_t data){
	uint8_t aux_inst = 0x00;
	
	aux_inst &= ~(1<<RS);  // RS = 0 => intruções
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
	aux_inst &= ~(1<<RS);  // RS = 0 => intruções
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
	aux_inst &= ~(1<<RS);  // RS = 0 => intruções
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


// MDE
 
void rotina_desativado(){
	if(ultima_tecla == t_p){
		// display: "digite senha mestre"

	}
	else if(ultima_tecla == t_a){
		
	}
}

bool leitura_senha(bool modo_leitura){
	rotina_geral_dos_botoes();
	if(flag_timeout || ultima_tecla == t_r || ultima_tecla == t_s){
		flag_timeout = false;
		return false;
	}
	return true;
}


bool algarismo(uint8_t tecla){
	if( tecla != t_p && tecla != t_a && tecla != t_d 
		&& tecla != t_e && tecla != t_s && tecla != t_r && tecla != nenhuma_tecla  )return true;
	return false;
			

}

void rotina_ativado(){
}
void rotina_programacao(){
	
}
void rotina_geral_dos_botoes(){
}