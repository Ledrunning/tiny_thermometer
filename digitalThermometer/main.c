/*
* digitalThermometer.c
*
* Created: 03.01.2020 17:55:27
* Author : Osman Mazinov
*/
#ifndef  F_CPU
#define	F_CPU 8000000U
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "LCD.h"
#include <string.h>
#include <math.h>

#define DHT_PORT        PORTD
#define DHT_DDR         DDRD
#define DHT_PIN         PIND
#define DHT22_PIN 6
#define STRING_SIZE 3
#define DEC 10
#define DOZEN 10
#define TEMP_MASK 0x7FFF
#define MINIMUM_TEMP -40
#define MAXIMUM_TEMP 80
#define ZERO_POINT 0
#define START_DELAY 3000

uint8_t c=0, lowByteRh, highByteRh, lowByteTemp, highByteTemp, checkSum;
uint16_t temperatureResult, humidityResult;
const int NEGATIVE_POINT = -10;

void init();
void print_negative_temperature(char* tBuffer, int negativeTemp); /* print temperature lower than 0 C */
void print_humidity(char* buffer);								  /* print humidity */
void print_temperature(char* buffer, uint16_t temp_after_point);  /* print temperature above than 0 C */
void request();													  /* Microcontroller send start pulse/request */
void response();												  /* receive response from DHT11 */
uint8_t receive_data();											  /* receive data */
int get_checksum();
void print_error();

void USART_Init( unsigned int baudrate );
void send_Uart(unsigned char uart_data);
void send_Uart_str(unsigned char *s);
void send_int_Uart(unsigned int c);

unsigned char USART_Receive( void );
void USART_Transmit( unsigned char data );

int main(void) {
	
	char t_buffer[STRING_SIZE], h_buffer[STRING_SIZE];
	uint16_t temp_buffer;
	uint16_t temp_buffer_after_point;
	int negative_temp;
	
	lcdInit();
	lcdGotoXY(0,0);
	lcdPuts("Tiny    ");
	lcdGotoXY(1,0);
	lcdPuts("Term    ");
	_delay_ms(START_DELAY);
	lcdClear();
	lcdSetDisplay(LCD_DISPLAY_ON);
	lcdSetCursor(LCD_CURSOR_OFF);
	USART_Init(47); // 19200
	
	while (1) {

      send_int_Uart(21);

		request();		/* send start pulse */
		response();		/* receive response */
		/*
		5 bytes of DHT data are as follows:
		[0] RH integral
		[1] RH decimal
		[2] Temp integral
		[3] Temp decimal
		[4] checksum is the sum of all four bytes AND 255
		*/
		lowByteRh=receive_data();	/* store first eight bit in I_RH */
		highByteRh=receive_data();	/* store next eight bit in D_RH */
		lowByteTemp=receive_data();	/* store next eight bit in I_Temp */
		highByteTemp=receive_data();	/* store next eight bit in D_Temp */
		checkSum=receive_data();/* store next eight bit in CheckSum */
		
		/* (DHTdata[0] + DHTdata[1] + DHTdata[2] + DHTdata[3]) & 255) will = DHTdata[4] IF the checksum is good. */
		if ((get_checksum() & 255) != checkSum) {
			print_error();
		}
		else {
			humidityResult = (lowByteRh * 256 + highByteRh ) / DOZEN;
			print_humidity(h_buffer);
			temperatureResult = (lowByteTemp * 256 + highByteTemp );
			
			if(temperatureResult > TEMP_MASK) {
				negative_temp = -(TEMP_MASK & temperatureResult); /* shoud be devide by DOZEN */
				print_negative_temperature(t_buffer, negative_temp);
			}
			else {
				temp_buffer = temperatureResult;
				temp_buffer_after_point = temp_buffer % DOZEN;
				print_temperature(t_buffer, temp_buffer_after_point);
			}
			
		}
		_delay_ms(1000);
	}
}

void print_humidity(char* buffer) {
	if(humidityResult < 100) {
		lcdGotoXY(0,0);
		lcdPuts("H= ");
		lcdGotoXY(0,3);
		itoa(humidityResult, buffer, DOZEN);
		lcdPuts(buffer);
		lcdGotoXY(0, 5);
		lcdPuts("%");
	}
	else {
		lcdGotoXY(0,0);
		lcdPuts("H=");
		lcdGotoXY(0,2);
		itoa(humidityResult, buffer, DOZEN);
		lcdPuts(buffer);
		lcdGotoXY(0, 5);
		lcdPuts("%");
	}
}

void print_negative_temperature(char* buffer, int negativeTemp) {

	int negativeData = negativeTemp/DEC;
	
	if((abs(negativeTemp) != abs(MINIMUM_TEMP))) {
		if(abs(negativeData) < abs(NEGATIVE_POINT)) {
			lcdGotoXY(1,0);
			lcdPuts("T= ");
			lcdGotoXY(1,3);
			itoa(negativeData, buffer, DEC);
			lcdPuts(buffer);
			lcdGotoXY(1,5);
			lcdPuts(" ");
			lcdGotoXY(1,6);
			lcdPuts("C ");
			
			if(negativeData == ZERO_POINT){
				
				lcdGotoXY(1,0);
				lcdPuts("T= ");
				lcdGotoXY(1,3);
				itoa(negativeData, buffer, DEC);
				lcdPuts(buffer);
				lcdGotoXY(1,4);
				lcdPuts(".");
				lcdGotoXY(1,5);
				lcdPuts("0");
				lcdGotoXY(1,6);
				lcdPuts("C ");
			}
		}
		else {
			lcdGotoXY(1,0);
			lcdPuts("T=");
			lcdGotoXY(1,2);
			itoa(negativeData, buffer, DEC);
			lcdPuts(buffer);
			lcdGotoXY(1,5);
			lcdPuts("C ");
		}
	}
	else {
		print_error();
	}
}

void print_temperature(char* buffer, uint16_t temp_after_point)
{
	uint16_t temperatureData = temperatureResult / DOZEN;
	
	if(temperatureData != MAXIMUM_TEMP) {
		
		if(temperatureData < DEC) {
			lcdGotoXY(1,0);
			lcdPuts("T= ");
			lcdGotoXY(1,3);
			itoa(temperatureData, buffer, DEC);
			lcdPuts(buffer);
			lcdGotoXY(1,4);
			lcdPuts(".");
			lcdGotoXY(1,5);
			itoa(temp_after_point, buffer, DEC);
			lcdPuts(buffer);
			lcdGotoXY(1,6);
			lcdPuts("C");
		}
		else {
			lcdGotoXY(1,0);
			lcdPuts("T=");
			lcdGotoXY(1,2);
			itoa(temperatureData, buffer, DEC);
			lcdPuts(buffer);
			lcdGotoXY(1,4);
			lcdPuts(".");
			lcdGotoXY(1,5);
			itoa(temp_after_point, buffer, DEC);
			lcdPuts(buffer);
			lcdGotoXY(1,6);
			lcdPuts("C");
		}
	}
	else {
		print_error();
	}
}

void request()  {
	
	DDRD |= (1<<DHT22_PIN);
	PORTD &= ~(1<<DHT22_PIN);    /* set to low pin */
	_delay_ms(20);            /* wait for 20ms */
	PORTD |= (1<<DHT22_PIN);	/* set to high pin */
}

void response()	{
	
	DDRD &= ~(1<<DHT22_PIN);
	while(PIND & (1<<DHT22_PIN));
	while((PIND & (1<<DHT22_PIN))==0);
	while(PIND & (1<<DHT22_PIN));
}

uint8_t receive_data() {
	
	int q;
	for (q=0; q<8; q++)	{
		while((PIND & (1<<DHT22_PIN)) == 0);  /* check received bit 0 or 1 */
		_delay_us(30);
		if(PIND & (1<<DHT22_PIN))/* if high pulse is greater than 30ms */
		c = (c<<1)|(0x01);	/* then its logic HIGH */
		else			/* otherwise its logic LOW */
		c = (c<<1);
		while(PIND & (1<<DHT22_PIN));
	}
	return c;
}

int get_checksum() {
	
	return lowByteRh + highByteRh + lowByteTemp + highByteTemp;
}

void print_error() {
	lcdGotoXY(0,0);
	lcdPuts("Sens    ");
	lcdGotoXY(1,0);
	lcdPuts("Err     ");
}

void USART_Init( unsigned int baudrate ) // USART Initialization
{
	UBRRH = (unsigned char) (baudrate>>8);
	UBRRL = (unsigned char) baudrate;
	UCSRA = (1<<U2X); //Удвоение скорости
	UCSRB = ( ( 1 << RXEN ) | ( 1 << TXEN ) ); // Confirm RX/TX USART
	UCSRC = (1<<USBS) | (3<<UCSZ0);
}

void send_Uart_str(unsigned char *s) //	Send string;
{
	while (*s != 0) USART_Transmit(*s++);
}

void send_int_Uart(unsigned int c)   //	Send number from 0000 to 9999;
{
	unsigned char temp;
	c=c%10000;
	temp=c/100;
	USART_Transmit(temp/10+'0');
	USART_Transmit(temp%10+'0');
	temp=c%100;
	USART_Transmit(temp/10+'0');
	USART_Transmit(temp%10+'0');
}

unsigned char USART_Receive( void ) //Функция приема данных
{
	while ( !(UCSRA & (1<<RXC)) ); 	//Ожидание приема символа
	return UDR; //Возврат символа
}

void USART_Transmit( unsigned char data ) //Функция отправки данных
{
	while ( !(UCSRA & (1<<UDRE)) ); //Ожидание опустошения буфера приема
	UDR = data; //Начало передачи данных
}