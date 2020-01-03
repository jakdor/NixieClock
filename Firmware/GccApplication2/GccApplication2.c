/*
 * Nixie Clock
 *
 * Created: 2016-06-30
 * Author: Jakub Dorda jak_dor
 * MCU: Atmega8A
 */ 

#define F_CPU 8000000UL

//#include <stddef.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
//#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "i2c_master.h"

//global variables
volatile uint8_t stage = 1;

volatile uint8_t s = 0;
volatile uint8_t m = 0;
volatile uint8_t h = 0;

volatile uint8_t R_s = 0;
volatile uint8_t R_m = 0;
volatile uint8_t R_h = 0;

volatile uint16_t inc = 0;
//volatile uint8_t check = 0;

/*
//--------------------------------------------------------------

//zmienne odbioru danych
volatile unsigned char odb_x;			//odebrana liczba X
volatile unsigned char odb_flaga = 0;	//flaga informuj¹ca odebraniu liczby
volatile unsigned int usart_bufor_ind;	//indeks bufora nadawania
char usart_bufor[30] = "Hello world!";	//bufor nadawania

void usart_inicjuj(void)
{
	#define BAUD 9600        //prêdkoœæ transmisji
	#include <util/setbaud.h> 
	
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	#if USE_2X
	UCSRA |=  (1<<U2X);
	#else
	UCSRA &= ~(1<<U2X);
	#endif
	
	UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);  
	//bitów danych: 8
	//bity stopu:  1
	//parzystoœæ:  brak
	//w³¹cz nadajnik i odbiornik oraz ich przerwania
	//przerwania nadajnika w funkcji wyslij_wynik()
	UCSRB = (1<<TXEN) | (1<<RXEN) | (1<<RXCIE);
}

ISR(USART_RXC_vect)
{
	//odebranie bajtu
	odb_x = UDR;   
	odb_flaga = 1; //flaga odbioru
}

ISR(USART_UDRE_vect){
	
	//sprawdŸ, czy bajt do wys³ania jest znakiem koñca tekstu, czyli zerem
	if(usart_bufor[usart_bufor_ind]!= 0)
	{
		UDR = usart_bufor[usart_bufor_ind++];
	}
	else
	{
		//koniec napisu
		UCSRB &= ~(1<<UDRIE); 
	}
}

void wyslij_wynik(void){
	
	//(CR+LF) - nowa linijka
	unsigned char z;
	for(z=0; z<30; z++){
		if(usart_bufor[z]==0)
		{  
			usart_bufor[z]   = 13; 
			usart_bufor[z+1]  = 10; 
			usart_bufor[z+2]  = 0;  
			break;
		}
	}

	//Zaczekaj, a¿ bufor nadawania bêdzie pusty
	while (!(UCSRA & (1<<UDRE)));
	
	usart_bufor_ind = 0;
	
	UCSRB |= (1<<UDRIE);
}

//--------------------------------------------------------------
*/

void rulette(uint16_t rulette_timer, uint8_t Hformat);
void nixie(uint8_t h, uint8_t m, uint8_t s);
void nixie_settings(uint16_t flash_timer, uint8_t set, uint8_t flag, uint8_t h, uint8_t m, uint8_t s);
void display(uint8_t num);
void anode(uint8_t num);
uint8_t blackout(uint8_t start, uint8_t end);
uint8_t input_cooldown(uint16_t ms);
inline void cooldown_reset(void);
inline uint8_t conv(uint8_t num);
inline uint8_t conv_write(uint8_t num);

int main(void)
{
	/*
	usart_inicjuj(); //inicjuj modu³ USART
	sei();           //w³¹cz przerwania globalne

	wyslij_wynik();  //wyœlij z zapisanym w buforze: "hello world!"
	*/
	
	// I/0 config
	DDRD = (1<<0) | (1<<1) | (1<<5) | (1<<6) | (1<<7);
	DDRB = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5);
	DDRC = (1<<0) | (1<<1) | (1<<2) | (1<<3);
	
	//DDRD &= ~(1<<PD2) | ~(1<<PD3) | ~(1<<PD4);
	
	//PWM
	//RED -> PB3/OCR2 | GREEN -> PB2/OCR1B | BLUE -> PB1/OCR1A
	TCCR1A = (1<<COM1A1) | (1<<COM1B1) | (1<<WGM10);
	TCCR1B = (1<<CS12);
	TCCR2 = (1 << COM21) | (1 << WGM20) | (1<<CS22) | (1<<CS21);
	
	//OCR2 = 255;
	//OCR1B = 255;
	//OCR1A = 255;
	
	//RGB
	//RED - 270Ohm | GREEN/BLUE - 220Ohm
	
	//local variables
	uint8_t loop = 0;
		
	uint16_t rulette_timer = 0;
	uint8_t srand_flag = 1;
	
	uint16_t flash_timer = 0;
	uint8_t set = 1;
	uint8_t change_flag = 0;
	
	uint8_t h_alarm = 0;
	uint8_t m_alarm = 0;
	uint8_t alarm_flag = 1;
	uint8_t alarmOn = 0;
	uint16_t alarm_timer = 0;
	
	uint8_t h_nightStart = 0;
	uint8_t m_nightStart = 0;
	uint8_t h_nightEnd = 0;
	uint8_t m_nightEnd = 0;
	uint8_t night_flag = 0;
	uint8_t night_check = 1;
		
	uint8_t RGBred = 0;
	uint8_t RGBgreen = 0;
	uint8_t RGBblue = 0;
	uint8_t RGB_flag = 0;
	uint16_t RGB_timer = 30000;
	uint8_t RGBred_s = 0;
	uint8_t RGBgreen_s = 0;
	uint8_t RGBblue_s = 0;
	uint8_t RGB_hour = 0;
	uint8_t RGB_hour_stage = 1;
	
	uint8_t Separator_flag = 1;
	uint16_t Separator_timer = 0;
	
	uint8_t Hformat = 0;
	
	//eeprom access
	
	h_alarm = eeprom_read_byte((uint8_t *) 0);
	m_alarm = eeprom_read_byte((uint8_t *) 1);
	alarm_flag = eeprom_read_byte((uint8_t *) 2);
	
	h_nightStart = eeprom_read_byte((uint8_t *) 3);
	m_nightStart = eeprom_read_byte((uint8_t *) 4);
	h_nightEnd = eeprom_read_byte((uint8_t *) 5);
	m_nightEnd = eeprom_read_byte((uint8_t *) 6);
	
	RGBred = eeprom_read_byte((uint8_t *) 7);
	RGBgreen = eeprom_read_byte((uint8_t *) 8);
	RGBblue = eeprom_read_byte((uint8_t *) 9);
	RGB_flag = eeprom_read_byte((uint8_t *) 10);
	
	Separator_flag = eeprom_read_byte((uint8_t *) 11);
	
	Hformat = eeprom_read_byte((uint8_t *) 12);

	night_flag = eeprom_read_byte((uint8_t *) 13);

	//Watchdog
	WDTCR |= (1<<WDCE) | (1<<WDE);
	wdt_enable(WDTO_60MS);
	
	//RBG setting
	if(RGB_flag && !night_flag){
		OCR2 = RGBred;
		OCR1B = RGBgreen;
		OCR1A = RGBblue;
	}
	
	while(1) 
	{
		anode(99);
		display(99);
		
		while(loop==0) //main loop
		{
			wdt_reset();

			i2c_start(0xD0);
			i2c_write(0x00);
			
			i2c_start(0xD1);
			
			s = conv(i2c_read_ack());
			m = conv(i2c_read_ack());
			h = conv(i2c_read_ack());
			
			i2c_read_nack();
	
			i2c_stop();

			if(srand_flag == 1){ //rand seed
				srand(h+m+s);
				srand_flag = 0;
			}

			if(night_check){ //night mode check
				if((h * 100 + m) < (h_nightStart * 100 + m_nightStart)){
					night_flag = 0;
					eeprom_update_byte ((uint8_t *) 13, night_flag );
				}
				else if((h * 100 + m) > (h_nightEnd * 100 + m_nightEnd)){
					night_flag = 0;
					eeprom_update_byte ((uint8_t *) 13, night_flag );
				}

				if(night_flag == 0 && Separator_flag == 2){
					PORTD |= (1<<PD0);
				}

				night_check = 0;
			}
			
			if(h_nightStart != h_nightEnd || m_nightStart != m_nightEnd){ //night mode
				if(h == h_nightStart && m == m_nightStart && s == 0){ 
					night_flag = 1;
					eeprom_update_byte ((uint8_t *) 13, night_flag );
					anode(99);
					display(99);
					OCR2 = 0;
					OCR1B = 0;
					OCR1A = 0;
					PORTD &= ~(1<<PD0);
				}
				else if(h == h_nightEnd && m == m_nightEnd && s == 0){
					night_flag = 0;
					eeprom_update_byte ((uint8_t *) 13, night_flag );
					OCR2 = RGBred;
					OCR1B = RGBgreen;
					OCR1A = RGBblue;
					if(Separator_flag == 2){
						PORTD |= (1<<PD0);
					}
				}
			}
			
			if(night_flag == 0){
				if(m == 59 && s >= 50){ //rulette for cathode poisoning
					++rulette_timer;
					rulette(rulette_timer, Hformat);	
				}
				else{ //normal operating
					if(Hformat){ //12H display format
						if(h == 0){ nixie(h+12,m,s); }
						else if(h > 12){ nixie(h-12,m,s); }
						else{ nixie(h,m,s); }
					}
					else{
						nixie(h,m,s);
					}
				}
			}
			
			if(m == 0 && s == 0){
				rulette_timer = 0;
			}
			
			if(alarmOn == 0){
				if(!(PIND & (1<<PD3)) && input_cooldown(600)){ //enter settings loop
					loop = 1;
				}
				
				if(alarm_flag == 1){ //alarm off 
					if(!(PIND & (1<<PD2)) && input_cooldown(600)){
						alarmOn = 2;
					}
				}
				else{ //alarm on
					if(!(PIND & (1<<PD2)) && input_cooldown(600)){
						alarmOn = 3;
					}
				}
				
				if(Hformat == 1){ //12H format off
					if(!(PIND & (1<<PD4)) && input_cooldown(600)){
						Hformat = 0;
						eeprom_update_byte ((uint8_t *) 12, Hformat );
					}
				}
				else{ //12H format on
					if(!(PIND & (1<<PD4)) && input_cooldown(600)){
						Hformat = 1;
						eeprom_update_byte ((uint8_t *) 12, Hformat );
					}
				}
			}
						
			if(h == h_alarm && m == m_alarm && s == 0 && alarm_flag == 1){ alarmOn = 1; } //alarm
			if(alarmOn == 1){ //alarm ring
				++alarm_timer;
				if(alarm_timer == 600){ alarm_timer = 0; }
			
				if(alarm_timer == 0){
					PORTD |= (1<<PD1);
				}
				else if(alarm_timer < 300){ //buzzer fix
					anode(99);
					display(99);
					PORTD &= ~(1<<PD0);
				}
				else if(alarm_timer == 300){
					PORTD &= ~(1<<PD1);
				}
				
				if(!(PIND & (1<<PD2)) && input_cooldown(1000)){ 
					alarmOn = 0;
					alarm_timer = 0;
					PORTD &= ~(1<<PD1);

					if(Separator_flag > 0){
						PORTD |= (1<<PD0);
					}
				}
				else if(!(PIND & (1<<PD3)) && input_cooldown(1000)){
					alarmOn = 0;
					alarm_timer = 0;
					PORTD &= ~(1<<PD1);

					if(Separator_flag > 0){
						PORTD |= (1<<PD0);
					}
				}
				else if(!(PIND & (1<<PD4)) && input_cooldown(1000)){
					alarmOn = 0;
					alarm_timer = 0;
					PORTD &= ~(1<<PD1);
					
					if(Separator_flag > 0){
						PORTD |= (1<<PD0);
					}
				}
			}
			else if(alarmOn == 2){ //set off
				++alarm_timer;
				if(alarm_timer == 1){
					PORTD |= (1<<PD1);
				}
				else if(alarm_timer == 400){
					PORTD &= ~(1<<PD1);
					alarmOn = 0;
					alarm_timer = 0;
					alarm_flag = 0;
					eeprom_update_byte ((uint8_t *) 2, alarm_flag );
				}
			}
			else if(alarmOn == 3){ //set on
				++alarm_timer;
				if(alarm_timer == 1){
					PORTD |= (1<<PD1);
				}
				else if(alarm_timer == 200){
					PORTD &= ~(1<<PD1);
				}
				else if(alarm_timer == 400){
					PORTD |= (1<<PD1);
				}
				else if(alarm_timer == 600){
					PORTD &= ~(1<<PD1);
				}
				else if(alarm_timer == 800){
					PORTD |= (1<<PD1);
				}
				else if(alarm_timer == 1000){
					PORTD &= ~(1<<PD1);
					alarmOn = 0;
					alarm_timer = 0;
					alarm_flag = 1;
					eeprom_update_byte ((uint8_t *) 2, alarm_flag );
				}
			}
			
			if((RGB_flag == 1 || RGB_flag == 2) && !night_flag){ //RBG random color shift
				if(RGB_timer == 15000 && RGB_flag == 1){ //mode 1 only
					RGBred_s = rand() % 160;
					RGBgreen_s = rand() % 160;
					RGBblue_s = rand() % 160;
				}
				else if(RGB_timer == 30000){
					switch(rand() % 3 + 1){
						case 1:
							RGBred_s = 220;
							RGBgreen_s = 0;
							RGBblue_s = 0;
							break;
						case 2:
							RGBred_s = 0;
							RGBgreen_s = 220;
							RGBblue_s = 0;
							break;
						case 3:
							RGBred_s = 0;
							RGBgreen_s = 0;
							RGBblue_s = 220;
							break;
					}
					
					RGB_timer = 0;
				}
				
				if((RGBred_s != RGBred || RGBgreen_s != RGBgreen || RGBblue_s != RGBblue) && RGB_timer % 20 == 0){
					if(RGBred_s > RGBred){ ++RGBred; }
					else if(RGBred_s < RGBred){ --RGBred; }
					if(RGBgreen_s > RGBgreen){ ++RGBgreen; }
					else if(RGBgreen_s < RGBgreen){ --RGBgreen; }
					if(RGBblue_s > RGBblue){ ++RGBblue; }
					else if(RGBblue_s < RGBblue){ --RGBblue; }
					OCR2 = RGBred;
					OCR1B = RGBgreen;
					OCR1A = RGBblue;
				}
				
				++RGB_timer;
			}
			
			if((RGB_flag == 3 || RGB_flag == 4) && night_flag == 0){ //RGB change every hour
				if(h != RGB_hour){
					
					switch(RGB_hour_stage){
						case 1:
							if(RGB_flag != 3){++RGB_hour_stage;}
						case 2:
							RGBred = 230;
							RGBgreen = 0;
							RGBblue = 0;
							break;
						case 3:
							if(RGB_flag != 3){++RGB_hour_stage;}
						case 4:
							RGBred = 0;
							RGBgreen = 230;
							RGBblue = 0;
							break;
						case 5:
							if(RGB_flag != 3){++RGB_hour_stage;}
						case 6:
							RGBred = 0;
							RGBgreen = 0;
							RGBblue = 230;
							break;
					}
										
					if((RGB_hour_stage == 1 || RGB_hour_stage == 3 || RGB_hour_stage == 5) && RGB_flag == 3){ //mode 3 only
						RGBred = rand() % 160;
						RGBgreen = rand() % 160;
						RGBblue = rand() % 160;
					}
					
					++RGB_hour_stage;
					if(RGB_hour_stage == 7){ RGB_hour_stage = 1; }
						
					OCR2 = RGBred;
					OCR1B = RGBgreen;
					OCR1A = RGBblue;
					
					RGB_hour = h;
				}
			}
						
			if(Separator_flag==1 && night_flag == 0){ //Flash separators
				if(Separator_timer == 1){
					PORTD |= (1<<PD0);
				}
				else if(Separator_timer == 501){
					PORTD &= ~(1<<PD0);
				}
				else if(Separator_timer == 1001){
					Separator_timer = 0;
				}
				++Separator_timer;
			}
									
			cooldown_reset();
			
			_delay_ms(2);
						
			/*
			if(odb_flaga){ //usart debugging
				odb_flaga = 0;
			
				sprintf(usart_bufor, "%d:%d:%d", h,m,s);
				wyslij_wynik();  
			}
			*/
		
		}
		
		while(loop == 1) //settings loop - time
		{					
			wdt_reset();

			nixie_settings(flash_timer, set, 0, 10, h, m);
			
			++flash_timer;
			if(flash_timer == 700){ flash_timer = 0; }
			else if(flash_timer == 3000){ flash_timer = 0; }
			
			if(!(PIND & (1<<PD3)) && input_cooldown(500)){
				++set;
				flash_timer = 0;
			}
			
			if(set==1){
				if(!(PIND & (1<<PD4)) && input_cooldown(500)){ //next setting page
					++loop;
				}
				
			}
			else if(set==2){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++h;
					if(h==24){h = 0;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--h;
					if(h==255){h = 23;}
				}
			}
			else if(set==3){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++m;
					if(m==60){m = 0;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--m;
					if(m==255){m = 59;}
				}
			}
					
				
			cooldown_reset();
			
			_delay_ms(2);
			
			if(set == 4){ //exit
				if(change_flag == 1){
					
					s = 0;
							
					i2c_init();
							
					i2c_start(0xD0);
					i2c_write(0x00);
							
					i2c_write(conv_write(s));
					i2c_write(conv_write(m));
					i2c_write(conv_write(h));
							
					i2c_write(0x01);
					i2c_write(0x01);
					i2c_write(0x01);
					i2c_write(0x00);
							
					i2c_write(0x00);
							
					i2c_stop();
					
					change_flag = 0;
				}
				set = 1;
				loop = 0;
			}
			
			/*
			if(odb_flaga){ //usart debugging
				odb_flaga = 0;
				
				sprintf(usart_bufor, "1 :%d:%d", h,m);
				wyslij_wynik();
			}
			*/
		}
		
		while(loop == 2) //settings loop - alarm
		{
			wdt_reset();
			
			nixie_settings(flash_timer, set, 0, 20, h_alarm, m_alarm);
			
			++flash_timer;
			if(flash_timer == 700){ flash_timer = 0; }
			else if(flash_timer == 3000){ flash_timer = 0; }
			
			if(!(PIND & (1<<PD3)) && input_cooldown(500)){
				++set;
				flash_timer = 0;
			}
			
			if(set==1){
				if(!(PIND & (1<<PD4)) && input_cooldown(500)){ //next setting page
					++loop;
				}
				
			}
			else if(set==2){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++h_alarm;
					if(h_alarm==24){h_alarm = 0;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--h_alarm;
					if(h_alarm==255){h_alarm = 23;}
				}
			}
			else if(set==3){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++m_alarm;
					if(m_alarm==60){m_alarm = 0;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--m_alarm;
					if(m_alarm==255){m_alarm = 59;}
				}
			}
			
			
			cooldown_reset();
			
			_delay_ms(2);
			
			if(set == 4){ //exit
				if(change_flag == 1){
					alarm_flag = 1;
					eeprom_update_byte ((uint8_t *) 0, h_alarm );
					eeprom_update_byte ((uint8_t *) 1, m_alarm );
					eeprom_update_byte ((uint8_t *) 2, alarm_flag );
					change_flag = 0;
				}
				set = 1;
				loop = 0;
			}
			
			/*
			if(odb_flaga){ //usart debugging
				odb_flaga = 0;
				
				sprintf(usart_bufor, "2 :%d:%d", h_alarm,m_alarm);
				wyslij_wynik();
			}
			*/
		}
		
		while(loop == 3) //settings night mode start
		{
			wdt_reset();
			
			nixie_settings(flash_timer, set, 0, 30, h_nightStart, m_nightStart);
					
			++flash_timer;
			if(flash_timer == 700){ flash_timer = 0; }
			else if(flash_timer == 3000){ flash_timer = 0; }
					
			if(!(PIND & (1<<PD3)) && input_cooldown(500)){
				++set;
				flash_timer = 0;
			}
					
			if(set==1){
				if(!(PIND & (1<<PD4)) && input_cooldown(500)){ //next setting page
					++loop;
				}
						
			}
			else if(set==2){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++h_nightStart;
					if(h_nightStart==24){h_nightStart = 0;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--h_nightStart;
					if(h_nightStart==255){h_nightStart = 23;}
				}
			}
			else if(set==3){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++m_nightStart;
					if(m_nightStart==60){m_nightStart = 0;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--m_nightStart;
					if(m_nightStart==255){m_nightStart = 59;}
				}
			}
					
					
			cooldown_reset();
					
			_delay_ms(2);
					
			if(set == 4){ //exit
				if(change_flag == 1){
					eeprom_update_byte ((uint8_t *) 3, h_nightStart );
					eeprom_update_byte ((uint8_t *) 4, m_nightStart );
					change_flag = 0;
				}
				set = 1;
				loop = 0;
			}
					
			/*		
			if(odb_flaga){ //usart debugging
				odb_flaga = 0;
						
				sprintf(usart_bufor, "3 :%d:%d", h_nightStart,m_nightStart);
				wyslij_wynik();
			}
			*/
		}
				
		while(loop == 4) //settings night mode end
		{
			wdt_reset();
			
			nixie_settings(flash_timer, set, 0, 40, h_nightEnd, m_nightEnd);
			
			++flash_timer;
			if(flash_timer == 700){ flash_timer = 0; }
			else if(flash_timer == 3000){ flash_timer = 0; }
			
			if(!(PIND & (1<<PD3)) && input_cooldown(500)){
				++set;
				flash_timer = 0;
			}
			
			if(set==1){
				if(!(PIND & (1<<PD4)) && input_cooldown(500)){ //next setting page
					++loop;
					if(night_flag){ //no rgb/separator settings during the night mode
						set = 1;
						loop = 0;
					}
				}
				
			}
			else if(set==2){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++h_nightEnd;
					if(h_nightEnd==24){h_nightEnd = 0;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--h_nightEnd;
					if(h_nightEnd==255){h_nightEnd = 23;}
				}
			}
			else if(set==3){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++m_nightEnd;
					if(m_nightEnd==60){m_nightEnd = 0;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--m_nightEnd;
					if(m_nightEnd==255){m_nightEnd = 59;}
				}
			}
			
			
			cooldown_reset();
			
			_delay_ms(2);
			
			if(set == 4){ //exit
				if(change_flag == 1){
					eeprom_update_byte ((uint8_t *) 5, h_nightEnd );
					eeprom_update_byte ((uint8_t *) 6, m_nightEnd );
					change_flag = 0;
				}
				set = 1;
				loop = 0;
			}
			
			/*
			if(odb_flaga){ //usart debugging
				odb_flaga = 0;
				
				sprintf(usart_bufor, "4 :%d:%d", h_nightEnd,m_nightEnd);
				wyslij_wynik();
			}
			*/
		}
		
		while(loop == 5) //settings Separator & RGB 
		{
			wdt_reset();
			
			nixie_settings(flash_timer, set, 0, 50, Separator_flag, RGB_flag);
			
			++flash_timer;
			if(flash_timer == 700){ flash_timer = 0; }
			else if(flash_timer == 3000){ flash_timer = 0; }
			
			if(!(PIND & (1<<PD3)) && input_cooldown(500)){
				++set;
				flash_timer = 0;
			}
			
			if(set==1){
				if(!(PIND & (1<<PD4)) && input_cooldown(500)){ //next setting page
					++loop;
				}
				
			}
			else if(set==2){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++Separator_flag;
					if(Separator_flag==3){Separator_flag = 2;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--Separator_flag;
					if(Separator_flag==255){Separator_flag = 0;}
				}
			}
			else if(set==3){
				if(!(PIND & (1<<PD4)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					++RGB_flag;
					if(RGB_flag==6){RGB_flag = 5;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(200)){
					change_flag = 1;
					flash_timer = 1000;
					--RGB_flag;
					if(RGB_flag==255){RGB_flag = 0;}
				}
			}
			
			
			cooldown_reset();
			
			_delay_ms(2);
			
			if(set == 4){ //exit
				if(change_flag == 1){
					if(Separator_flag == 0){ PORTD &= ~(1<<PD0); }
					else if(Separator_flag == 2){ PORTD |= (1<<PD0);  }
					if(RGB_flag == 0){
						OCR2 = 0;
						OCR1B = 0;
						OCR1A = 0;
					}
					eeprom_update_byte ((uint8_t *) 10, RGB_flag );
					eeprom_update_byte ((uint8_t *) 11, Separator_flag );
					change_flag = 0;
				}
				set = 1;
				loop = 0;
			}
			
			/*
			if(odb_flaga){ //usart debugging
				odb_flaga = 0;
				
				sprintf(usart_bufor, "5 :%d:%d", Separator_flag,RGB_flag);
				wyslij_wynik();
			}
			*/
		}
		
		while(loop == 6) //settings RGB manual
		{
			wdt_reset();
			
			nixie_settings(flash_timer, set, 1, RGBred/5, RGBgreen/5, RGBblue/5);
			
			++flash_timer;
			if(flash_timer == 700){ flash_timer = 0; }
			else if(flash_timer == 3000){ flash_timer = 0; }
			
			if(!(PIND & (1<<PD3)) && input_cooldown(500)){
				++set;
				flash_timer = 0;
			}
			
			if(set==1){
				if(!(PIND & (1<<PD4)) && input_cooldown(40)){
					change_flag = 1;
					flash_timer = 1000;
					++RGBred;
					if(RGBred==0){RGBred = 250;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(40)){
					change_flag = 1;
					flash_timer = 1000;
					--RGBred;
					if(RGBred==255){RGBred = 0;}
				}
			}
			else if(set==2){
				if(!(PIND & (1<<PD4)) && input_cooldown(40)){
					change_flag = 1;
					flash_timer = 1000;
					++RGBgreen;
					if(RGBgreen==0){RGBgreen = 250;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(40)){
					change_flag = 1;
					flash_timer = 1000;
					--RGBgreen;
					if(RGBgreen==255){RGBgreen = 0;}
				}
			}
			else if(set==3){
				if(!(PIND & (1<<PD4)) && input_cooldown(40)){
					change_flag = 1;
					flash_timer = 1000;
					++RGBblue;
					if(RGBblue==0){RGBblue = 250;}
				}
				if(!(PIND & (1<<PD2)) && input_cooldown(40)){
					change_flag = 1;
					flash_timer = 1000;
					--RGBblue;
					if(RGBblue==255){RGBblue = 0;}
				}
			}
			
			
			OCR2 = RGBred;
			OCR1B = RGBgreen;
			OCR1A = RGBblue;
			
			cooldown_reset();
			
			_delay_ms(2);
			
			if(set == 4){ //exit
				if(change_flag == 1){
					RGB_flag = 5;
					eeprom_update_byte ((uint8_t *) 7, RGBred );
					eeprom_update_byte ((uint8_t *) 8, RGBgreen );
					eeprom_update_byte ((uint8_t *) 9, RGBblue );
					eeprom_update_byte ((uint8_t *) 10, RGB_flag );
					change_flag = 0;
				}
				set = 1;
				loop = 0;
			}
			
			/*
			if(odb_flaga){ //usart debugging
				odb_flaga = 0;
				
				sprintf(usart_bufor, "%d:%d:%d", RGBred/5,RGBgreen/5,RGBblue/5);
				wyslij_wynik();
			}
			*/
		}
		
	}
}

void rulette(uint16_t rulette_timer, uint8_t Hformat){

	if(Hformat){ //12H display format
		if(h == 0){ h += 12; }
		else if(h > 12){ h -= 12; }
	}
	
	if(rulette_timer % 45 == 0){
		R_h = rand() % 100;
		R_m = rand() % 100;
		R_s = rand() % 100;
	}
	
	if(s < 55){
		nixie(R_h,R_m,R_s);
	}
	else if(s == 55){
		R_h = (h/10)*10 + R_h%10;
		nixie(R_h,R_m,R_s);
	}
	else if(s == 56){
		nixie(h,R_m,R_s);
	}
	else if(s == 57){
		R_m = (m/10)*10 + R_m%10;
		nixie(h,R_m,R_s);
	}
	else if(s == 58){
		nixie(h,m,R_s);
	}
	else if(s == 59){
		R_s = (s/10)*10 + R_s%10;
		nixie(h,m,R_s);
	}
		
	return;
}

void nixie(uint8_t h, uint8_t m, uint8_t s){

	anode(99); //anti-ghosting
	display(99); 
	_delay_us(100);
	
	anode(stage);
		
	switch(stage){
		case 1:
			if(h<10){ display(0); }
			else{ display(h/10); }
			break;
		case 2:
			display(h%10);
			break;
		case 3:
			if(m<10){ display(0); }
			else{ display(m/10); }
			break;
		case 4:
			display(m%10);
			break;
		case 5:
			if(s<10){ display(0); }
			else{ display(s/10); }
			break;
		case 6:
			display(s%10);
			break;
	}
		
	++stage;
	if(stage==7){ stage = 1; }
		
	return;
}

void nixie_settings(uint16_t flash_timer, uint8_t set, uint8_t flag, uint8_t h, uint8_t m, uint8_t s){
	
	if(set == 1 && flag){ //flash chosen
		if(flash_timer<350){
			if(!blackout(1,2)){
				nixie(h,m,s);
			}
		}
		else{
			nixie(h,m,s);
		}
	}
	else if(set == 1){
		if(flash_timer<350){
			if(!blackout(1,2)){
				nixie(h,m,s);
			}
		}
		else{
			if(!blackout(2,2)){
				nixie(h,m,s);
			}
		}
	}
	else if(set == 2){
		if(flash_timer<350){
			if(!blackout(3,4)){
				nixie(h,m,s);
			}
		}
		else{
			nixie(h,m,s);
		}
	}
	else if(set == 3){
		if(flash_timer<350){
			if(!blackout(5,6)){
				nixie(h,m,s);
			}
		}
		else{
			nixie(h,m,s);
		}
	}
	
	return;
}

/*
	D -> PD7
	C -> PD5
	B -> PD6
	A -> PB0
*/

void display(uint8_t num){
	PORTD &= ~ (1<<PD5);
	PORTD &= ~ (1<<PD6);
	PORTD &= ~ (1<<PD7);
	PORTB &= ~ (1<<PB0);
	
	switch(num){
		case 0:
			break;
		case 1:
			PORTB |= (1<<PB0);
			break;
		case 2:
			PORTD |= (1<<PD6);
			break;
		case 3:
			PORTB |= (1<<PB0);
			PORTD |= (1<<PD6);
			break;
		case 4:
			PORTD |= (1<<PD5);
			break;
		case 5:
			PORTB |= (1<<PB0);
			PORTD |= (1<<PD5);
			break;
		case 6:
			PORTD |= (1<<PD6);
			PORTD |= (1<<PD5);
			break;
		case 7:
			PORTB |= (1<<PB0);
			PORTD |= (1<<PD6);
			PORTD |= (1<<PD5);
			break;
		case 8:
			PORTD |= (1<<PD7);
			break;
		case 9:
			PORTB |= (1<<PB0);
			PORTD |= (1<<PD7);
			break;
		case 99: //off
			PORTD |= (1<<PD6);
			PORTD |= (1<<PD7);
			break;
	}
	
	return;	
}

/*
	PC3 = 1
	...
	PB4 = 6
*/

void anode(uint8_t num){
	PORTC &= ~ (1<<PC3);
	PORTC &= ~ (1<<PC2);
	PORTC &= ~ (1<<PC1);
	PORTC &= ~ (1<<PC0);
	PORTB &= ~ (1<<PB5);
	PORTB &= ~ (1<<PB4);
	
	switch(num){
		case 1:
			PORTC |= (1<<PC3);
			break;
		case 2:
			PORTC |= (1<<PC2);
			break;
		case 3:
			PORTC |= (1<<PC1);
			break;
		case 4:
			PORTC |= (1<<PC0);
			break;
		case 5:
			PORTB |= (1<<PB5);
			break;
		case 6:
			PORTB |= (1<<PB4);
			break;
		case 99: //off
			break;
	}
	
	return;
}

uint8_t blackout(uint8_t start, uint8_t end){
	if(stage >= start && stage <= end){
		anode(99);
		display(99);
		_delay_us(100);
		++stage;
		if(stage == 7){stage = 1;}
		return 1;
	}
	
	return 0;
}

uint8_t input_cooldown(uint16_t ms){
	if(inc > 0){
		return 0;
	}
	else{
		inc = ms/2;
		return 1;
	}
}

inline void cooldown_reset(void){
	if(inc > 0){--inc;}
	return;
}

inline uint8_t conv(uint8_t num){
	return (num / 16)*10 + num % 16;
}

inline uint8_t conv_write(uint8_t num){
	return (num / 10)*16 + num % 10;
}

