/*
 * CDA.c
 *
 * Created: 14/04/2021 14:27:59
 * Author : kaike
 */ 

#include <avr/io.h>


#define F_CPU 8000000UL
#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
unsigned char dataLog[] = "usuario, dia_semana, hora, modo \r";

int i = 0;

static inline void print();

int main(void)
{
	
	UBRR0H = (uint8_t)(UBRR_VALUE>>8);

	UBRR0L = (uint8_t)UBRR_VALUE;
	
    UCSR0C = 0x06;
	
	UCSR0B = (1<<TXEN0); /* Enable  transmitter  */ 
	
    /* Replace with your application code */
    while (1) 
    {
		print();
    }
}

static inline void print(){
	
	i = 0;
	while(dataLog[i] != 0) /*mostra a msg no terminal uart */
	{
		while (!( UCSR0A & (1<<UDRE0))); /* Espera esvaziar o buffer*/
		UDR0 = dataLog[i];               /* Coloca a mensagem em um buffer, depois envia a mensagem */
		i++;                             /* incrementa o contador           */
	}
}
	
	
	

