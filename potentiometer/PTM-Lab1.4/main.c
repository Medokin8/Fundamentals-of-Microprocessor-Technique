#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "HD44780.h"

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

 // Gotowe zaimplementowane:
#define bit_is_set(sfr, bit)   (_SFR_BYTE(sfr) & _BV(bit))
#define bit_is_clear(sfr, bit)   (!(_SFR_BYTE(sfr) & _BV(bit)))
#define loop_until_bit_is_set(sfr, bit)   do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit)   do { } while (bit_is_set(sfr, bit))
#define MIN(a,b)			((a<b)?(a):(b))
#define MAX(a,b)			((a>b)?(a):(b))
#define ABS(x)				((x>0)?(x):(-x))

void ADC_init(){ //funkcja konfigurujaca przetwornik A/D
	//AVCC - REFS1 = 0, REFS0 = 1
	sbi(ADMUX,REFS0); //REFS1 = 0, ustawiamy REFS0 = 1

	//konfiguracja Rejestru ADCSRA - podzielnika czestotliwosci
	sbi(ADCSRA,ADPS0); //dobrieramy prescaler 8MHz:128=62500<100kHz
	sbi(ADCSRA,ADPS1); //128 odpowiada ADPS2=ADPS1=ADPS0=1,
	sbi(ADCSRA,ADPS2); //czestotlwiosc przetwornika to 62500HZ

	//ADCSRA - uruchomienie ukladu
	sbi(ADCSRA,ADEN); //ADEN 1=on, 0=off
}

uint16_t ADC_measure(uint32_t napiecie){ //zmienia otrzymana wartosc 16bitowa 0-1024
	return (napiecie*500)/1022;					 //zwracajac wartosc z zakresu 0 - 500
}

uint16_t ADC_10bit(uint8_t PIN){ //funkcja majaca numer pinu z ktorego dokonujemy pomiar
								 //zwraca wartosc 16bitowa z zakresu 0-1024
	switch(PIN){ //ADMUX - konfiguracja kanalu pomiarowego miedzy pinami PA0-PA7
	case 0:	//pomiar z PIN0
		cbi(ADMUX,MUX0);
		cbi(ADMUX,MUX1);
		cbi(ADMUX,MUX2);
		break;
	case 1: //pomiar z PIN1
		sbi(ADMUX,MUX0);
		cbi(ADMUX,MUX1);
		cbi(ADMUX,MUX2);
		break;
	case 2: //pomiar z PIN2
		cbi(ADMUX,MUX0);
		sbi(ADMUX,MUX1);
		cbi(ADMUX,MUX2);
		break;
	case 3: //pomiar z PIN3
		sbi(ADMUX,MUX0);
		sbi(ADMUX,MUX1);
		cbi(ADMUX,MUX2);
		break;
	case 4: //pomiar z PIN4
		cbi(ADMUX,MUX0);
		cbi(ADMUX,MUX1);
		sbi(ADMUX,MUX2);
		break;
	case 5: //pomiar z PIN5
		sbi(ADMUX,MUX0);
		cbi(ADMUX,MUX1);
		sbi(ADMUX,MUX2);
		break;
	case 6: //pomiar z PIN6
		cbi(ADMUX,MUX0);
		sbi(ADMUX,MUX1);
		sbi(ADMUX,MUX2);
		break;
	case 7: //pomiar z PIN7
		sbi(ADMUX,MUX0);
		sbi(ADMUX,MUX1);
		sbi(ADMUX,MUX2);
		break;
	}

	sbi(ADCSRA,ADSC); //rozpoczecie pomiaru

	loop_until_bit_is_clear(ADCSRA,ADSC); //oczekiwanie na zakonczenie pomiaru,
										  //warunkiem czysty bit ADSC z rejestru ADCSRA
	return ADC; //zwracamy bit ADC zawierajacy wartosc 16bitowa z zakresu 0-1024
}


void LCD_10bit(uint16_t napiecie){ //funkcja majaca wartosc 0-500 wypisuje napis X,XX[V]

	char text[8]="0,00[V]"; //inicjalizujemy tablice znakow napisem 0,00[V]

	//(LICZBA/X)%10+48 zwraca nam cyfrê setek/dziesiatek/jednosci (X=100/10/1) liczby LICZBA, aby moc zapisac jako znak dodajemy 48 (ASCII)
	text[0]=(napiecie/100)%10+48; //otrzymujemy liczbe jednosci
	text[2]=(napiecie/10)%10+48;  //otrzymujemy liczbe dziesietnych
	text[3]=(napiecie/1)%10+48;   //otrzymujemy liczbe setnych

	LCD_WriteText(text); //wypisujemy z formatu XXX liczbe w formacie X,XX[V]
}

void komparator (uint16_t napiecie, uint16_t histereza, uint16_t odniesienie){ //funkcja porownuje otrzymane wartosci

	sbi(DDRC,PC3);  //ustawiamy port DDRC nozke PC3 jako wyjscie
	LCD_GoTo(8, 1); //ustawiamy miejsce w ktorym bedzie pisany komunikat

	if (napiecie < (odniesienie - histereza/2) || napiecie > (odniesienie + histereza/2)){ //warunkiem jest napiecie ktore jest po za zakresem odniesienia+-histereza/2
		sbi(PORTC,PC3); 	  //ustawiamy nozke PC3 na stan wysoki (DIODA SWIECI)
		LCD_WriteText("ON "); //wysietlamy komunikat o spelnionym warunku
	}else {
		cbi(PORTC,PC3); 	  //ustawiamy nozke PC3 na stan niski (DIODA NIE SWIECI)
		LCD_WriteText("OFF"); //wysietlamy komunikat o niespelnionym warunku
	}
}

int main() {

	uint16_t napiecie=0, histereza=0, odniesienie=0; //inicjalizujemy zmienne pomocnicze

	LCD_Initalize(); //inicjalizujemy wyswietlacz LCD
	LCD_Home();		 //ustawiamy poczatkowe wspolrzedne
	LCD_Clear();	 //czyœcimy ekran

	ADC_init(); //inicjalizujemy przetwornik A/D

	while(1) {
		napiecie=ADC_measure(ADC_10bit(0)); //zapisujemy pomiar napiecia z nozki PA0 do zmiennej napiecie w formacie 0-500
		LCD_GoTo(0, 0);						//ustawiamy miejsce do wypisania pomiaru
		LCD_10bit(napiecie);				//wysietlamy ten pomiar w formacie X,XX[V]

		odniesienie=ADC_measure(ADC_10bit(1));  //zapisujemy pomiar napiecia z nozki PA1 do zmiennej odniesienie w formacie 0-500
		LCD_GoTo(8, 0);							//ustawiamy miejsce do wypisania pomiaru
		LCD_10bit(odniesienie);					//wysietlamy ten pomiar w formacie X,XX[V]

		histereza=ADC_measure(ADC_10bit(2)); //zapisujemy pomiar napiecia z nozki PA2 do zmiennej histereza w formacie 0-500
		LCD_GoTo(0, 1);						 //ustawiamy miejsce do wypisania pomiaru
		LCD_10bit(histereza);				 //wysietlamy ten pomiar w formacie X,XX[V]

		komparator(napiecie, histereza, odniesienie); //porownujemy otrzymane wyniki
	}

return 0;
}
