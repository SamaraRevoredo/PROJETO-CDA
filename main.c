/*
 * SDR.c
 *
 * Created: 26/03/2021 17:20:46
 * Author : kaike
 */ 

#include <avr/io.h>
#define F_CPU 16000000


int main(void)
{
	DDRB = (1<<DDRB); // DESTINADO AO DISPLAY LCD
	
	DDRD = (1<DDRD); // DESTINA A REDE R2R
	
	DDRC = DDRC | B000000011; // PC0 = MSG RECEBIDA, PC1 = LEITURA POT, PC2 = R, PC3 = M, PC4 = P
	PORTC = B00011100; // DEFINE OS BOTÕES PULLUP
	
	
    /* Replace with your application code */
    while (1) 
    {
    }
}

