//laby2
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "HD44780.h"

#ifndef _BV
#define _BV(bit)				(1<<(bit))
#endif
#ifndef sbi
#define sbi(reg,bit)		reg |= (_BV(bit))
#endif

#ifndef cbi
#define cbi(reg,bit)		reg &= ~(_BV(bit))
#endif

int main() {

	LCD_Initalize();
	LCD_Home();

	char klawiatura[17]="-987+654C321=#0*"; //cyferki z klawiatury
	char text[10]; //bufor do szczytywania liczby
	uint8_t ptr=0, znak=0, ile_cyfr=0, limit=4;
	uint16_t liczba=0;

	DDRD=0xF0;   //ustawiamy rejestr kierunku na wyjœcie dla nó¿ek 1-4 oraz wyjœcie 5-8
	PORTD=0xFF;  //ustawiamy wyprowadzenie nó¿ek 1-4 na pull-up oraz 5-8 na stan wysoki
	LCD_Clear(); //czyœcimy ekran

	while (1) {
		for(uint8_t i=0; i<4; i++) //zmienna i do operowania wierszami
				for(uint8_t j=0; j<4; j++){	//zmienna j do operowania kolumnami
					cbi(PORTD, 4+j); //ustawiamy nó¿kê 4+j (5-8) na stan niski
						if (bit_is_clear(PIND, i)){ //sprawdzamy czy na i-tej (1-4) nó¿ce pojawi³ siê stan niski

							if (znak == 0 && klawiatura[i*4+j] == '='){
							    memset(text,' ',9);
								ptr=0;
							}

							if ( '0' <= klawiatura[i*4+j] && klawiatura[i*4+j] <= '9' ){
								if( ptr == 0 )memset(text,' ',9);
								if( ptr < limit ){
									text[ptr] = klawiatura[i*4+j];
									ptr++;
								}
							}

							if ( (klawiatura[i*4+j] == '-' || klawiatura[i*4+j] == '+') && znak == 0){
								text[ptr]=klawiatura[i*4+j];
								ptr++;
								ile_cyfr = ptr;
								limit = 4 + ptr;
								liczba = atoi(text);
								if ( klawiatura[i*4+j] == '-' ) {znak = 1;}else znak = 2;
							}

							if ( klawiatura[i*4+j] == '=' && znak != 0 ){
								if (ile_cyfr == 1) {memset(text,' ',9);}else memset(text,' ',ile_cyfr);
							    if(znak == 1) {liczba = liczba - atoi(text);}else liczba = liczba + atoi(text);
							    memset(text,' ',9);
								ptr=0;
								znak=0;
								sprintf(text, "%d", liczba); //do buffora text ³adujemy komunikat o wciœniêtym przycisku
								LCD_Clear(); //czyœcimy ekran
								LCD_GoTo(1, 0); //ustawiamy miejsce do wyœwietlania na ekranie (kolumna, wiersz)
								LCD_WriteText(text); //wyœwietlamy komunikat na ekran
							}
							LCD_Clear(); //czyœcimy ekran
							LCD_GoTo(0, 0);
							LCD_WriteText(text); //wyœwietlamy komunikat na ekran

							while(bit_is_clear(PIND, i));
						}
					sbi(PORTD, 4+j); //ustawiamy ponowanie nó¿kê 4+j na stan wysoki
				}
	}
}
///************************************************************************************************************************************************/
//#include <avr/io.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
//#include <util/delay.h>
//#include <avr/sfr_defs.h>
//#include <math.h>
//#include <avr/interrupt.h>
//#include <avr/pgmspace.h>
//
//#include "HD44780.h"
//
//#ifndef _BV
//#define _BV(bit)				(1<<(bit))
//#endif
//#ifndef sbi
//#define sbi(reg,bit)		reg |= (_BV(bit))
//#endif
//
//#ifndef cbi
//#define cbi(reg,bit)		reg &= ~(_BV(bit))
//#endif
//
//int main() {
//
//	LCD_Initalize();
//	LCD_Home();
//
//	char klawiatura[17]="-987+654C321=#0*"; //cyferki z klawiatury
//	char text[17]; //bufor do wyœwietlania na ekranie
//
//	DDRD=0xF0;   //ustawiamy rejestr kierunku na wyjœcie dla nó¿ek 1-4 oraz wyjœcie 5-8
//	PORTD=0xFF;  //ustawiamy wyprowadzenie nó¿ek 1-4 na pull-up oraz 5-8 na stan wysoki
//	LCD_Clear(); //czyœcimy ekran
//
//	while (1) {
//		for(uint8_t i=0; i<4; i++){ //zmienna i do operowania wierszami
//			if(bit_is_set(PIND, i))
//				for(uint8_t j=0; j<4; j++){	//zmienna j do operowania kolumnami
//					cbi(PORTD, 4+j); //ustawiamy nó¿kê 4+j (5-8) na stan niski
//						if (bit_is_clear(PIND, i)){ //sprawdzamy czy na i-tej (1-4) nó¿ce pojawi³ siê stan niski
//							LCD_GoTo(0, 1); //ustawiamy miejsce do wyœwietlania na ekranie
//							sprintf(text, "Wcisnieto: %c", klawiatura[i*4+j]); //do buffora text ³adujemy komunikat o wciœniêtym przycisku
//							LCD_WriteText(text); //wyœwietlamy komunikat na ekran
//						}
//					sbi(PORTD, 4+j); //ustawiamy ponowanie nó¿kê 4+j na stan wysoki
//				}
//			else i--;
//		}
//	}
//}

/****************************************************************************************************************************************************/

//#include <avr/io.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
//#include <util/delay.h>
//#include <avr/sfr_defs.h>
//#include <math.h>
//#include <avr/interrupt.h>
//#include <avr/pgmspace.h>
//
//#include "HD44780.h"
//
//#ifndef _BV
//#define _BV(bit)				(1<<(bit))
//#endif
//#ifndef sbi
//#define sbi(reg,bit)		reg |= (_BV(bit))
//#endif
//
//#ifndef cbi
//#define cbi(reg,bit)		reg &= ~(_BV(bit))
//#endif
//
//int main() {
//	LCD_Initalize();
//	LCD_Home();
//	char text[17];
//	uint8_t g=0, m=0, s=0;
//
//	while (1) {
//
//		if (s>=60) {m++; s=0;} //je¿eli licznik sekund przekroczy 60 to zwiêkszamy minuty o 1
//		if (m>=60) {g++; m=0;} //je¿eli licznik minut przekroczy 60 to zwiêkszamy godziny o 1
//		if (g>=24) {g=0;} //je¿eli licznik godzin przekroczy 24 to resetujemy zegar do 0
//
//		LCD_Clear(); //czyœcimy ekran
//		LCD_GoTo(0, 0); //ustawiamy kursor na ekranie
//		sprintf(text, "%d:%d:%d", g,m,s++); //³adujemy do buffora text komunikat o godzinie w formacie GG:MM:SS
//		LCD_WriteText(text); //wyœwietlamy komunikat na ekranie
//
//		LCD_GoTo(0, 1); //ustawiamy kursor na ekranie
//		LCD_WriteText("254009"); //wypisujemy indeks
//
//		_delay_ms(100); //opóŸnienie
//	}
//}
