//lab3
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#ifndef _BV
#define _BV(bit)				(1<<(bit))
#endif

#ifndef inb
#define	inb(addr)			(addr)
#endif

#ifndef outb
#define	outb(addr, data)	addr = (data)
#endif

#ifndef sbi
#define sbi(reg,bit)		reg |= (_BV(bit))
#endif

#ifndef cbi
#define cbi(reg,bit)		reg &= ~(_BV(bit))
#endif

#ifndef tbi
#define tbi(reg,bit)		reg ^= (_BV(bit))
#endif

#define 	bit_is_set(sfr, bit)   (_SFR_BYTE(sfr) & _BV(bit))
#define 	bit_is_clear(sfr, bit)   (!(_SFR_BYTE(sfr) & _BV(bit)))
#define 	loop_until_bit_is_set(sfr, bit)   do { } while (bit_is_clear(sfr, bit))
#define 	loop_until_bit_is_clear(sfr, bit)   do { } while (bit_is_set(sfr, bit))

#define MIN(a,b)			((a<b)?(a):(b))
#define MAX(a,b)			((a>b)?(a):(b))
#define ABS(x)				((x>0)?(x):(-x))

volatile uint8_t i=0, reset=1, stop=0 ;
volatile uint16_t liczba7Seg;

volatile char znaki[4];

char cyfra[10] = { 0b1111011, 0b1111111, 0b1110000, 0b1011111, 0b1011011,
		0b0110011, 0b1111001, 0b1101101, 0b0110000, 0b1111110};

void TimerInit() {  //Inicjalizacja Timer1 do wywolywania przerwania z czêstotliwoœci¹ 2Hz
sbi(TCCR1B, WGM12);	//Wybranie trybu pracy CTC z TOP OCR1A
sbi(TCCR1B, CS12);	//Wybranie dzielnika czestotliwosci
OCR1A=15625;	    //Zapisanie do OCR1A wartosci odpowiadajacej 0,5s
cbi(TIMSK, OCIE1A);	//Uruchomienie przerwania OCIE1A
}


void seg7Init() { //Inicjalizacja portow do obs³ugi wyswietlacza 7 segmentowego
DDRC = 0xFF;	  //Inicjalizacja kolumn
PORTC = 0x7B;	  //Inicjalizacja segmentu
}

void seg7ShowCyfra(uint8_t cyfraDoWyswietlenia) {	//Wyswietla na wyswietlaczu 7 segmentowym cyfre z argumentu
	PORTC = cyfra[cyfraDoWyswietlenia];				//co to robi - wytlumaczyc prowadzacemu
}

void alarm (){
	i=0;                //zerujemy licznik
	cbi(TIMSK, OCIE1A); //wy³¹czamy przerwanie
	sbi(PORTB, PB0);    //w³¹czamy diode
	_delay_ms(5000);    //czekamy 5 s
	cbi(PORTB, PB0);    //wy³¹czamy diode
	stop = 1;           //ustawiamy flage stop na 1
}

int main() {
	TimerInit();    //inicjujemy przerwanie
	seg7Init();     //inicjujemy wyœwietlacz
	sei();          //funkcja uruchamia globalne przerwania

	sbi(DDRB, PB0); //ustawiamy pin B0 jako wyjœcie (dioda)

	sbi(MCUCR, ISC01); //ustawiamy przerwanie na zbocze malej¹ce
	sbi(GICR,  INT0);  //w³¹czamy przerwanie z zewnatrz
	sbi(PORTD, PD2);   //ustawiamy pin D2 jako wejœcie pull-up

	while (1) {            //g³ówna pêtla
		if(i > 9) alarm(); //w³¹cz alarm po odliczeniu 9 liczb
	}
	return 0;
}

ISR(TIMER1_COMPA_vect) {//Funkcja uruchamiana z przerwaniem po przepelnieniu licznika w timer1
	seg7ShowCyfra(i);   //wyswietla na segmencie liczbe za pomoca tablicy
	i++;                //zwieksza licznik o jeden
}

ISR(INT0_vect){                  //funkcja uruchamiana z przerwaniem po pojawieniu sie stanu niskiego na pinie INT0/PD2
	if(reset == 0 && stop == 0){ //pierwsze klikniecie stopuje warunkiem jest nie wykonane wczesniej stopowanie ani reset
		cbi(TIMSK, OCIE1A);      //zatrzymuje zegar
		stop = 1;                //po wykonanym stopie ustawiamy flage stop na 1
	}else if(reset == 0 && stop == 1){//resetujemy warunkiem jest wykonany stop i nie wykonany reset
		i=0;              //zerujemy licznik liczb
		stop=0;           //zerujemy flage stopu
		reset=1;          //ustawiamy flage reset na 1
		seg7ShowCyfra(0); //wysietlamy poczatkowa liczbe
	}else{            //startujemy
		reset=0;      //zerujemy flage reset
		sbi(TIMSK, OCIE1A); //w³¹czamy zegar
	}
}
