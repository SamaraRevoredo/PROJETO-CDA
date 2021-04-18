/*
Autor: Mohamad Sadeque
Driver i2c
 */ 

#include <xc.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Definição UART
#define F_CPU 8000000UL
#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
uint8_t i = 0;

//Definição TIMER1
const uint16_t T1_init = 0;
//T1_comp = tempo * freq / 256
const uint16_t T1_comp = 15625;


typedef enum { escrita, leitura } mode_i2c;
typedef enum { false, true } bool;
	
#define addr_Sensores_escrita 0x40
#define addr_Sensores_leitura 0x41
#define addr_RTC_escrita 0xD0
#define addr_RTC_leitura 0xD1
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
uint16_t limite_timeout = 99;
uint16_t limite_sirene = 0;
uint16_t limite_ativacao = 0;
bool altera_senha();
bool flag_alarme = false;
bool flag_ativacao = false;
volatile uint16_t contador = 0, contador_R = 0;
void limpa_tela();
uint8_t ultima_tecla;
void rotina_geral_dos_botoes();
uint8_t modo = 0;
uint8_t contador_senha = 0;
void rotina_programacao();
void rotina_desativado();
void rotina_ativado();
void print_tecla(uint8_t tecla );
uint8_t leitura_teclado();
void rotina_recuperacao();
const uint8_t senha_fabrica[4][4] = { {1,2,3,4},{4,3,2,1},{0,0,0,0},{3,5,9,6}};
uint8_t senhas[4][4] = { {1,2,3,4},{4,3,2,1},{0,0,0,0},{3,5,9,6}};
uint8_t sensores_habilitados;
uint8_t zona_habilita;
uint8_t sensores_zona[3];
void leitura_sensores();
bool leitura_de_parametro();
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
uint8_t validade_senha = acesso_negado;	
bool leitura_senha(bool modo_leitura);
uint8_t get_numero(uint8_t tecla);
uint8_t algarismo(uint8_t tecla);

 //--- Rotina de Interrupção Timer1 1Hz---
 ISR(TIMER1_COMPA_vect)
 {
	 
	 TCNT1 = T1_init;      //reinicializa TIMER1
	 contador++;
	 contador_R++;
	 
	 if (modo == programacao && contador > limite_timeout)
	 {
		 flag_timeout = true;
		 modo = desativado;
		 PORTD &= ~(1<<PD3); // DESLIGA LED PROGRAMACAO
		 contador = 0;
		 //Desabilita Interrupção do Timer1
		 TIMSK1 &= ~(1 << OCIE1A);
	 }
	 else if(flag_alarme && contador > limite_sirene){
		 flag_alarme = false;
		 PORTD &= ~(1<<PD2); // DESLIGA SIRENE
		 PORTD &= ~(1<<PD7) | ~(1<<PD6) | ~(1<<PD5); // DESLIGA AS ZONAS
		 contador = 0;
		 //Desabilita Interrupção do Timer1
		 TIMSK1 &= ~(1 << OCIE1A);
	 }
	 
	 else if(modo == ativado && flag_ativacao == false && contador > limite_ativacao){
		 flag_ativacao = true;
		 contador = 0;
		 //Desabilita Interrupção do Timer1
		 TIMSK1 &= ~(1 << OCIE1A);
	 }
		 
	 


	 
 } //end ISR

// RTC
void print_RTC();
void ler_RTC();
uint8_t dia, dez_hora,hora, dez_minuto, minuto;
// LCD
void inicializa_display_i2c();
void send_instruction_lcd_i2c(uint8_t data);
void send_instruction_lcd_nibble_i2c(uint8_t data);
void send_word_lcd_i2c(uint8_t data);
void set_DDRAM_ADDR_i2c(uint8_t pos);
void envia_frase_i2c(char *frase,uint8_t pos);
void passarValor(const char *data);
char aux_string[16];
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
	//if(!MT_receive(0x41)) PORTB = 0xAF;
	inicializa_display_i2c();
	//passarValor("DIGITE A SENHA");
	//envia_frase_i2c(aux_string,l1C1);
	//set_DDRAM_ADDR_i2c(l2C7);
	
	//uint8_t testeChar = '*';
	//for(int i = 0; i < 4; i++) send_word_lcd_i2c(testeChar);
	DDRD = 0xFE;
	
	//	Modo de Comparação
	TCCR1A = 0;
	//	Prescaler 1:256
	TCCR1B |=  (1 << CS12);
	TCCR1B &= ~(1 << CS11);
	TCCR1B &= ~(1 << CS10);
	//	Inicializa Registradores
	TCNT1 = T1_init;
	OCR1A = T1_comp;

	
	
	sei();
	
	UBRR0H = (uint8_t)(UBRR_VALUE>>8);

	UBRR0L = (uint8_t)UBRR_VALUE;
	
	UCSR0C = 0x06;
	
	UCSR0B = (1<<TXEN0); /* Habilita o transmissor  */
	
	
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

// Funçao para imprimir datalog
static inline void print(char dataLog[]){
	
	i = 0;
	while(dataLog[i] != 0) /*mostra a msg no terminal uart */
	{
		while (!( UCSR0A & (1<<UDRE0))); /* Espera esvaziar o buffer*/
		UDR0 = dataLog[i];               /* Coloca a mensagem em um buffer, depois envia a mensagem */
		i++;                             /* incrementa o contador           */
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


void ler_RTC(){
	MT_transmit(addr_RTC_escrita,0x03);
	MT_receive(addr_RTC_leitura);
	dia = TWDR;
	
	MT_transmit(addr_RTC_escrita,0x02);
	MT_receive(addr_RTC_leitura);
	dez_hora = (TWDR >> 4);
	hora = TWDR & 0x0F;

	MT_transmit(addr_RTC_escrita,0x01);
	MT_receive(addr_RTC_leitura);
	dez_minuto = (TWDR >> 4);
	minuto = TWDR & 0x0F;
}

void print_RTC(){
	switch (dia){
		case 1:
		passarValor("DOM");
		break;
		case 2:
		passarValor("SEG");		
		break;
		case 3:
		passarValor("TER");		
		break;
		case 4:
		passarValor("QUA");		
		break;
		case 5:
		passarValor("QUI");		
		break;		
		case 6:
		passarValor("SEX");		
		break;
		case 7:
		passarValor("SAB");		
		break;								
	}
	envia_frase_i2c(aux_string,l1C1);
	aux_string[0] = dez_hora + 48;
	aux_string[1] = hora + 48;
	aux_string[2] = ':';
	aux_string[3] = dez_minuto + 48;
	aux_string[4] = minuto + 48;
	aux_string[5] = '\0';
	envia_frase_i2c(aux_string, l1C5);
	
		
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

void limpa_tela(){
	send_instruction_lcd_i2c(0x01); // clear display
	delay_ms(2);
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
		if(!leitura_senha(true)) return;
		if(validade_senha == mestre){
			// liga led prog
			PORTD |= (1 << PORTD3);
			modo = programacao;
			flag_timeout = false;
			contador = 0;
			TIMSK1 = (1 << OCIE1A);
			limpa_tela();
		}
	}
	else if(ultima_tecla == t_a){
		if(!leitura_senha(false)) return;
		if(validade_senha > acesso_negado){
			// liga led ativo
			PORTD |= (1 << PORTD4);
			modo = ativado;
			flag_ativacao = false;
			contador = 0;
			TIMSK1 = (1 << OCIE1A);
			limpa_tela();		
		}
		else{
			passarValor("ACESSO NEGADO   ");
			envia_frase_i2c(aux_string, l1C1);
			delay_ms(1000);
			limpa_tela();			
		}
}
}

bool leitura_senha(bool leitura_mestre){
	contador_senha = 0;
	uint8_t buffer_senha[4];
	if(leitura_mestre){
		passarValor("SENHA MESTRE:");
	}
	else{
		passarValor("SENHA ATIVACAO:");
	}
	envia_frase_i2c(aux_string,l1C1);
		set_DDRAM_ADDR_i2c(l2C6);
	while(1){
		
			rotina_geral_dos_botoes();
			if(flag_timeout || ultima_tecla == t_r || ultima_tecla == t_s){
				flag_timeout = false;
				passarValor("    ");
				envia_frase_i2c(aux_string,l2C6);
				return false;
			}
			else if(algarismo(ultima_tecla) && contador_senha < 4){
				buffer_senha[contador_senha] =  get_numero(ultima_tecla);
				send_word_lcd_i2c(0x2A);
				contador_senha++;
			}
			else if(leitura_mestre && contador_senha == 4){
				for(int i = 0; i < 4; i++){
					if (buffer_senha[i] != senhas[0][i])
					{
						validade_senha = acesso_negado;

						return true;
					}
					
				}
				validade_senha = mestre;
				return true;
			}
			else if(ultima_tecla == t_e && contador_senha == 4){
				passarValor("    ");
				envia_frase_i2c(aux_string,l2C6);
				for (int j = 0; j < 4; j++)
				{
					for(int i = 0; i < 4; i++){
						if (buffer_senha[i] != senhas[j][i])
						{

							validade_senha = acesso_negado;
							if(j == 3){
								passarValor("  ACESSO NEGADO ");
								envia_frase_i2c(aux_string, l1C1);
								delay_ms(500);								
								return true;
								 }
							break;
						}
						if(i == 3){
							validade_senha = ativacao;
							return true;
						}
						
					}
				}
			return true;
			
			}

	}

}


bool algarismo(uint8_t tecla){
	if( tecla != t_p && tecla != t_a && tecla != t_d 
		&& tecla != t_e && tecla != t_s && tecla != t_r && tecla != nenhuma_tecla  )return true;
	return false;
			}

void rotina_ativado(){
		
	if (flag_ativacao)
	{
		
		if (ultima_tecla == t_d)
		{
			leitura_senha(false);
			if ( validade_senha > acesso_negado){
				modo = desativado;
				PORTD &= ~(1<<PD4); // DESLIGA LED ATIVA
				limpa_tela();
		 }
			else{
					// display: "Informa acesso negado"
						passarValor("ACESSO NEGADO   ");
						envia_frase_i2c(aux_string, l1C1);
						delay_ms(1000);
						limpa_tela();					
				}				
		
		}
		else{
			leitura_sensores();
		}	
}
		}

void leitura_sensores(){
	
	if(!(PINC & (1<<PINC3))){
		
	 MT_receive(0x41); //
	 uint8_t sensores = TWDR;
	 sensores = sensores & sensores_habilitados;
	 if(!sensores){
			for(int i = 0; i < 3; i++){
				if(!(zona_habilita & (1 << i))) continue;
				if(sensores & sensores_zona[i]){
					PORTD |= (1<<PORTD2); // ATIVA SIRENE
					flag_alarme = true;
					// LIGA LED DA ZONA
					PORTD |= (1 << (i + 5));
					// REINICIA O TEMPORIZADOR
					contador = 0;
					//	Habilita Interrupção do Timer1
					TIMSK1 = (1 << OCIE1A);
				}
			}
	 }
	}	
}


void rotina_recuperacao(){
	passarValor("RECUPERACAO");
	envia_frase_i2c(aux_string,l1C1);
	for(int i =0; i< 4; i++){
		for(int j = 0; j<4;j++){
			senhas[i][j] = senha_fabrica[i][j];
		}
	}
	limite_ativacao = 0;
	limite_sirene = 0;
	limite_timeout = 99;
	delay_ms(100);
	limpa_tela();
	
}

void rotina_programacao(){
	// digite o primeiro parametro
	passarValor("1 PARAMETRO:");
	envia_frase_i2c(aux_string,l1C1);
	
	while(leitura_de_parametro()){
		if(ultima_tecla == t_a){
			// digite o segundo parametro
			passarValor("2 PARAMETRO:");
			envia_frase_i2c(aux_string,l1C1);
			set_DDRAM_ADDR_i2c(l2C1);
			send_word_lcd_i2c('A');
			while(leitura_de_parametro()){
				switch (ultima_tecla)
				{
					case t_2:
					send_word_lcd_i2c('2');						
					// Altera senha
					altera_senha();
					return;
					break;
					case t_3:
					send_word_lcd_i2c('3');						
					//habilita sensor
					return;
					break;
					case t_4:
					send_word_lcd_i2c('4');						
					//Associa sensor a zona
					return;
					break;
					case t_5:
					send_word_lcd_i2c('5');						
					//Habilita zona
					return;
					break;
					case t_6:
					send_word_lcd_i2c('6');						
					//Ajuste de temporizado de ativação
					return;
					break;
					case t_7:
					send_word_lcd_i2c('7');						
					//ajuste de temporizador de timeout
					return;
					break;		
					case t_8:
					send_word_lcd_i2c('8');						
					// ajuste de temporizado de permanência da sirene
					return;
					break;
					default:
					//Parametro incorreto
					limpa_linha(0);
					passarValor("INCORRETO");
					envia_frase_i2c(aux_string,l1C1);
					delay_ms(100);
					passarValor("2 PARAMETRO:");
					envia_frase_i2c(aux_string,l1C1);
					set_DDRAM_ADDR_i2c(l2C2);
					break;								
				}
			}
		}
		else if(ultima_tecla == t_d){
			
			// digite o segundo parametro
			passarValor("2 PARAMETRO:");
			envia_frase_i2c(aux_string,l1C1);
			set_DDRAM_ADDR_i2c(l2C1);
			send_word_lcd_i2c('D');			
			while(leitura_de_parametro()){
				switch (ultima_tecla)
				{
					case t_3:
					// Desabilita sensor
					send_word_lcd_i2c('3');	
					return;
					break;
					case t_4:
					send_word_lcd_i2c('4');	
					//Desassocia sensor a zona
					return;
					break;
					case t_5:
					send_word_lcd_i2c('5');						
					//Desabilita zona
					return;
					break;
					default:
					//Parametro incorreto
					break;
				}
			}				
		}
		else{
			// parametro incorreto
		}
	}
}

void limpa_linha(uint8_t linha){
	passarValor("                ");
	if(linha == 0){
		envia_frase_i2c(aux_string,l1C1);
	}
	else{
		envia_frase_i2c(aux_string,l2C1);		
	}
}
bool leitura_de_parametro(){
	do 
	{
	rotina_geral_dos_botoes();
	if(ultima_tecla == t_r || ultima_tecla == t_s || flag_timeout){
		modo = desativado;
		limpa_tela();
		PORTD = 0;
		return false;
	}	
	} while (ultima_tecla == nenhuma_tecla);
	return true;	
}
void rotina_geral_dos_botoes(){
		
		ultima_tecla = leitura_teclado();
		
		if (ultima_tecla == t_r)
		{
			contador_R = 0;
			//	Habilita Interrupção do Timer1
			TIMSK1 = (1 << OCIE1A);
			

			//	leitura do botao R
			while(leitura_teclado() != nenhuma_tecla){// espera o botão ser solto
					if (contador_R >= 10){	
					rotina_recuperacao();
					modo = desativado;
					break;	
				}
			}
			 
			contador_R = 0;

			
		}
		
		else if (ultima_tecla == t_s){
			
			while(leitura_teclado() != nenhuma_tecla); // espera o botão ser solto
			
			PORTD |= (1<<PD2); // LIGA A SIRENE
			
			while(leitura_teclado() != t_s); // espera o botão ser apertado
			while(leitura_teclado() != nenhuma_tecla); // espera o botão ser solto
			
			PORTD &= ~(1<<PD2); // DESLIGA SIRENE
			modo = desativado;
		}
		else{
			while(leitura_teclado() != nenhuma_tecla && ultima_tecla != nenhuma_tecla); // espera o botão ser solto
		}
		
}

uint8_t leitura_teclado(){
	uint8_t aux_ultima_tecla = nenhuma_tecla;
	for(int i = 0; i < 4; i++){
		PORTB = 0xF0 | ~(1 << i);
		for(int j = 4; j < 8; j++){
			if(!(PINB & (1<< j))){
				aux_ultima_tecla = 4*i + j -3;
				break;
			}
		}
		if(aux_ultima_tecla != nenhuma_tecla) return aux_ultima_tecla;
	}
	return aux_ultima_tecla;	
}

void print_tecla(uint8_t tecla ){
	switch (tecla)
	{
	case t_1 :
		passarValor("1");
		send_word_lcd_i2c("1");
		envia_frase_i2c(aux_string,l1C1);
		break;
	case t_2 :
		passarValor("2");
		envia_frase_i2c(aux_string,l1C1);
	break;
	case t_3 :
		passarValor("3");
		envia_frase_i2c(aux_string,l1C1);
	break;
	case t_p :
		passarValor("P");
		envia_frase_i2c(aux_string,l1C1);
	break;
	case t_4 :
	passarValor("4");
	envia_frase_i2c(aux_string,l1C1);
	break;
	case t_5 :
	passarValor("5");
	envia_frase_i2c(aux_string,l1C1);
	break;
	case t_6 :
	passarValor("6");
	envia_frase_i2c(aux_string,l1C1);
	break;
	case t_a :
	passarValor("A");
	envia_frase_i2c(aux_string,l1C1);
	break;	
	case t_7 :
	passarValor("7");
	envia_frase_i2c(aux_string,l1C1);
	break;
	case t_8 :
	passarValor("8");
	envia_frase_i2c(aux_string,l1C1);
	break;
	case t_9 :
	passarValor("9");
	envia_frase_i2c(aux_string,l1C1);
	break;
	case t_d :
	passarValor("D");
	envia_frase_i2c(aux_string,l1C1);
	break;		
	case t_r :
	passarValor("R");
	envia_frase_i2c(aux_string,l1C1);
	break;
	case t_0 :
	passarValor("0");
	envia_frase_i2c(aux_string,l1C1);
	break;
	case t_s :
	passarValor("S");
	envia_frase_i2c(aux_string,l1C1);
	break;
	case t_e :
	passarValor("E");
	envia_frase_i2c(aux_string,l1C1);
	break;			
	}
	
}


void get_tecla(uint8_t tecla){
		switch (tecla)
		{
			case t_1 :
			passarValor("1");
			break;
			case t_2 :
			passarValor("2");
			break;
			case t_3 :
			passarValor("3");
			break;
			case t_p :
			passarValor("P");
			break;
			case t_4 :
			passarValor("4");
			break;
			case t_5 :
			passarValor("5");
			break;
			case t_6 :
			passarValor("6");
			break;
			case t_a :
			passarValor("A");
			break;
			case t_7 :
			passarValor("7");
			break;
			case t_8 :
			passarValor("8");
			break;
			case t_9 :
			passarValor("9");
			break;
			case t_d :
			passarValor("D");
			break;
			case t_r :
			passarValor("R");
			break;
			case t_0 :
			passarValor("0");
			break;
			case t_s :
			passarValor("S");
			break;
			case t_e :
			passarValor("E");
			break;
		}
}

uint8_t get_numero(uint8_t tecla){
			switch (tecla)
			{
				case t_1 :
				return 0x01;
				break;
				case t_2 :
				return 0x02;
				break;
				case t_3 :
				return 0x03;
				break;
				case t_4 :
				return 0x04;
				break;
				case t_5 :
				return 0x05;
				break;
				case t_6 :
				return 0x06;
				break;
				case t_7 :
				return 0x07;
				break;
				case t_8 :
				return 0x08;
				break;
				case t_9 :
				return 0x09;
				break;
				default:
				return 0x00;
				break;
			}
}





bool altera_senha(){
	limpa_tela();
	passarValor("Escolha senha:");
	envia_frase_i2c(aux_string,l1C1);
	while(leitura_de_parametro()){

		if (algarismo(ultima_tecla) && get_numero(ultima_tecla) < 4)
		{
			uint8_t aux_modifica_senha =  get_numero(ultima_tecla);
			uint8_t aux_contador_senha = 0;
			limpa_tela();
			passarValor("digite senha:");
			envia_frase_i2c(aux_string,l1C1);
			set_DDRAM_ADDR_i2c(l2C6);
			uint8_t buffer_senha[4];
			while (leitura_de_parametro() && aux_contador_senha < 4)
			{
				if(algarismo(ultima_tecla) ){
					buffer_senha[aux_contador_senha] = get_numero(ultima_tecla);
					aux_contador_senha++;
					send_word_lcd_i2c('*');
				}
				
				if(aux_contador_senha == 4){
					while(leitura_de_parametro()){
						
						if(ultima_tecla == t_e){
							for(int i = 0 ; i < 4; i++) senhas[aux_modifica_senha][i] = buffer_senha[i];
							limpa_tela();
							return true;
						}
					}
					return false;
					
				}
			
			}
			return false;
		}
		else{
			//incorreto
		}
	}
}
