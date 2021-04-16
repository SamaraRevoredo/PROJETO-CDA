/*
 * CDA.c
 *
 * Created: 14/04/2021 14:27:59
 * Author : kaike
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 8000000UL
#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
uint8_t i = 0;

const uint16_t T1_init = 0;
// T1_comp = tempo * freq / 256
const uint16_t T1_comp = 15625;


static inline void print(char dataLog[]);

// --- Interrupção ---
ISR(TIMER1_COMPA_vect)
{
	
	TCNT1 = T1_init;      //reinicializa TIMER1
	PORTB ^= (1 << PB0);
	print("usuario, dia_semana, hora, modo \r");
	
} //end ISR
//
int main(void)
{
	DDRB |= (1 << PB0);
	 
	//Modo de Comparação
	TCCR1A = 0;
	 //Prescaler 1:256
	 TCCR1B |=  (1 << CS12);
	 TCCR1B &= ~(1 << CS11);
	 TCCR1B &= ~(1 << CS10);
	//Inicializa Registradores
	TCNT1 = T1_init;
	OCR1A = T1_comp;

	//Habilita Interrupção do Timer1
	TIMSK1 = (1 << OCIE1A);
	
	sei();
	
	UBRR0H = (uint8_t)(UBRR_VALUE>>8);

	UBRR0L = (uint8_t)UBRR_VALUE;
	
    UCSR0C = 0x06;
	
	UCSR0B = (1<<TXEN0); /* Habilita o transmissor  */ 
	
    /* Replace with your application code */
    while (1) 
    {
		
    }
}

static inline void print(char dataLog[]){
	
	i = 0;
	while(dataLog[i] != 0) /*mostra a msg no terminal uart */
	{
		while (!( UCSR0A & (1<<UDRE0))); /* Espera esvaziar o buffer*/
		UDR0 = dataLog[i];               /* Coloca a mensagem em um buffer, depois envia a mensagem */
		i++;                             /* incrementa o contador           */
	}
}
	

	

