//--------------------------------------------------//
//CÓDIGO PARA LER E ESCREVER NO RTC
//--------------------------------------------------//
// UFRN - SISTEMAS DIGITAIS
// AUTOR: GRUPO 4
// REFERENCIAS:
// Lima, Charles Borges; Villaça, Marco V. M. AVR e Arduino Técnicas de Projeto. © 2012. 2 edição.
// Datasheet DS1307

//BIBLIOTECAS
#include <avr/io.h>

//Definições da comunicação TWI
#define TW_START         0x08
#define TW_MT_SLA_ACK    0x18
#define TW_MT_DATA_ACK   0x28
#define TW_REP_START     0x10
#define TW_MR_SLA_ACK    0x40
#define TW_MR_SLA_NACK   0x48
#define TW_MR_DATA_NACK  0x58


//Definiçoes para realizar a comunicação I2C
#define inicia_bit() TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)
#define para_bit() TWCR = (1<<TWINT)|(1<<TWEN)| (1<<TWSTO)
#define espera_envio() while (!(TWCR & (1<<TWINT)))
#define envia_byte() TWCR = (1<<TWINT) | (1<<TWEN)
#define recebe_byte() TWCR = (1<<TWINT) | (1<<TWEN)
#define espera_recebimento() while (!(TWCR & (1<<TWINT)))
#define recebe_byte_ret_nack() TWCR = (1<<TWINT) | (1<<TWEN)

//testes para envio de dados
#define teste_envio_start() if((TWSR & 0xF8) != TW_START)
#define teste_envio_end_escrita() if((TWSR & 0xF8) != TW_MT_SLA_ACK)
#define teste_envio_dado() if((TWSR & 0xF8) != TW_MT_DATA_ACK)
#define teste_envio_restart() if((TWSR & 0xF8) != TW_REP_START)
#define teste_envio_end_leitura() if((TWSR & 0xF8) != TW_MR_SLA_ACK)
#define teste_recebe_byte_ret_nack() if((TWSR & 0xF8) != TW_MR_DATA_NACK) 

//Define os endereços de acordo com o datasheet DS1307
#define sec_adress    0x00
#define min_adress    0x01
#define hrs_adress    0x02
#define dia_adress    0x03
#define data_adress   0x04
#define mes_adress    0x05
#define ano_adress    0x06
#define reg_controle  0x07

typedef enum
{
	DOMINGO = 1,
	SEGUNDA,
	TERCA,
	QUARTA,
	QUINTA,
	SEXTA,
	SABADO
} RTC_Dias;

typedef enum
{
	JANEIRO = 1,
	FEVEREIRO,
	MARCO,
	ABRIL,
	MAIO,
	JUNHO,
	JULHO,
	AGOSTO,
	SETEMBRO,
	OUTUBRO,
	NOVEMBRO,
	DEZEMBRO
} RTC_Meses;

typedef struct rtc_tm
{
	uint8_t segundos;
	uint8_t minutos;
	uint8_t horas;
	RTC_Dias dias_s;
	uint8_t dia;
	RTC_Meses mes;
	uint8_t ano;
} RTC_Time;

RTC_Time c_time; //Variável global do tempo

//FUNÇÕES
uint8_t bcd_to_decimal(uint8_t number);
uint8_t decimal_to_bcd(uint8_t number);
unsigned char le_RTC(unsigned char endereco);
void escreve_RTC(unsigned char dado_i2c, unsigned char endereco);
void RTC_Set(RTC_Time *time_t);
RTC_Time *RTC_Get();

int main(void){
	//TESTE PARA LEITURA
		//RTC_Get();
		//RTC_Set(&c_time);

	//TESTE PARA ESCRITA
	//Teste para escrever no RTC uma data 10/08/98 às 14:25:33 segunda-feira
		/*
		RTC_Time T;
		T.ano = 98;
		T.dia = 10;
		T.dias_s = 2;
		T.mes = 8;
		T.horas = 14;
		T.minutos = 25;
		T.segundos = 33;
		RTC_Set(&T);
		*/
	
	//PARA REALIZAR OPERAÇÕES COM O TEMPO ATUAL BASTA:
	//CHAMAR A FUNÇÃO RTC.Get() para ler o relógio
	//Chamar a variavel global ctime e colocar "." para acessar os valores internos
	//EXEMPLO:
	/*
		int eh_meu_aniversario = 0;
		RTC_Get();
		if(c_time.dia == 10 && c_time.mes == 8 && c_time.ano == 98){
			eh_meu_aniversario = 1;
		}
	*/
	while(1){

	}
}


//Converte BCD para decimal
uint8_t bcd_to_decimal(uint8_t number)
{
	return ( (number >> 4) * 10 + (number & 0x0F) );
}

//Converte decimal para BCD
uint8_t decimal_to_bcd(uint8_t number)
{
	return ( ((number / 10) << 4) + (number % 10) );
}

unsigned char le_RTC(unsigned char endereco)
{
	inicia_bit();
	espera_envio();
	teste_envio_start();

	TWDR = 0xD0; //carrega o endereço para acesso do DS1307 (bit0 = 0, escrita)
	//para outro CI basta trocar este endereço
	envia_byte();
	espera_envio();
	teste_envio_end_escrita();

	TWDR = endereco; //ajuste do ponteiro de endereço para a leitura do DS1307
	envia_byte();
	espera_envio();
	teste_envio_dado();
	inicia_bit(); //reinício
	espera_envio();
	teste_envio_restart();
	TWDR = 0xD1; //carrega o endereço para acesso do DS1307 (bit0 = 1 é leitura)
	//automaticamente o ATmega chaveia para o estado de recepção
	envia_byte();
	espera_envio();
	teste_envio_end_leitura();
	recebe_byte_ret_nack(); //só lê um byte, por isso retorna um NACK
	espera_recebimento();
	teste_recebe_byte_ret_nack();
	para_bit();
	return TWDR; //retorna byte recebido
} //Fonte: Lima(2012)

void escreve_RTC(unsigned char dado_i2c, unsigned char endereco)
{
	inicia_bit();
	espera_envio();
	teste_envio_start();

	TWDR = 0xD0; //carrega o endereço para acesso do DS1307 (bit0 = 0 é escrita)
	//para outro CI basta trocar este endereço
	envia_byte();
	espera_envio();
	teste_envio_end_escrita();

	TWDR = endereco; //carrega o endereço para escrita do dado no DS1307
	envia_byte();
	espera_envio();
	teste_envio_dado();
	TWDR = dado_i2c; //carrega o dado para escrita no endereço especificado
	envia_byte();
	espera_envio();
	teste_envio_dado();
	para_bit();
} //Fonte: Lima(2012)

RTC_Time *RTC_Get()
{
	//le_dados
	c_time.segundos = le_RTC(sec_adress);
	c_time.minutos  = le_RTC(min_adress);
	c_time.horas    = le_RTC(hrs_adress);
	c_time.dias_s   = le_RTC(dia_adress);
	c_time.dia      = le_RTC(data_adress);
	c_time.mes      = le_RTC(mes_adress);
	c_time.ano      = le_RTC(ano_adress);

	// converte BCD para decimal
	c_time.segundos = bcd_to_decimal(c_time.segundos);
	c_time.minutos  = bcd_to_decimal(c_time.minutos);
	c_time.horas    = bcd_to_decimal(c_time.horas);
	c_time.dia      = bcd_to_decimal(c_time.dia);
	c_time.mes      = bcd_to_decimal(c_time.mes);
	c_time.ano      = bcd_to_decimal(c_time.ano);
	
	return &c_time;
}

void RTC_Set(RTC_Time *time_t)
{
	// converte decimal para BCD
	time_t->dia      = decimal_to_bcd(time_t->dia);
	time_t->mes      = decimal_to_bcd(time_t->mes);
	time_t->ano      = decimal_to_bcd(time_t->ano);
	time_t->horas    = decimal_to_bcd(time_t->horas);
	time_t->minutos  = decimal_to_bcd(time_t->minutos);
	time_t->segundos = decimal_to_bcd(time_t->segundos);
	
	//Escreve dados
	escreve_RTC(time_t->segundos, sec_adress);
	escreve_RTC(time_t->minutos, min_adress);
	escreve_RTC(time_t->horas, hrs_adress);
	escreve_RTC(time_t->dias_s, dia_adress);
	escreve_RTC(time_t->dia, data_adress);
	escreve_RTC(time_t->mes, mes_adress);
	escreve_RTC(time_t->ano, ano_adress);
}
