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
#include <stdbool.h>

#define DHT_PORT        PORTD
#define DHT_DDR         DDRD
#define DHT_PIN         PIND
#define DHT22_PIN 6
#define STRING_SIZE 3

uint8_t c=0,lowByteRh, highByteRh, lowByteTemp, highByteTemp, checkSum;
uint16_t temperatureResult, humidityResult;

void request();                /* Microcontroller send start pulse/request */
void response();				/* receive response from DHT11 */
uint8_t receive_data();			/* receive data */
int get_checksum();

// if (highByteTemp >> 7 & 1) { temperatureResult *= -1; }
// Маски достаточно, но нужна маска & 0x7F чтобы знаковый бит обнулить
int main(void) {
	
	char buffer[STRING_SIZE];
	int temp_buffer;
    uint16_t temp_buffer_after_point;

	lcdInit();
	lcdClear();
	lcdSetDisplay(LCD_DISPLAY_ON);
	lcdSetCursor(LCD_CURSOR_OFF);
	
	while (1) {

		request();		/* send start pulse */
		response();		/* receive response */
		lowByteRh=receive_data();	/* store first eight bit in I_RH */
		highByteRh=receive_data();	/* store next eight bit in D_RH */
		lowByteTemp=receive_data();	/* store next eight bit in I_Temp */
		highByteTemp=receive_data();	/* store next eight bit in D_Temp */
		checkSum=receive_data();/* store next eight bit in CheckSum */
		
		if (get_checksum() != checkSum)
		{
			lcdGotoXY(0,1);
			lcdPuts("Error");
		}
		else
		{
			lcdClear();
			lcdGotoXY(0,0);
			lcdPuts("H=");
			lcdGotoXY(0,3);
			humidityResult = (lowByteRh * 256 + highByteRh ) / 10;
			itoa(humidityResult, buffer, 10);
			lcdPuts(buffer);
			
			lcdGotoXY(1,0);
			lcdPuts("T=");
			lcdGotoXY(1,3);
			
			temperatureResult = (lowByteTemp * 256 + highByteTemp );
			temp_buffer = temperatureResult;
						
			itoa(temperatureResult, buffer, 10);
			lcdPuts(buffer);
			lcdGotoXY(1,5);
			lcdPuts(".");
			lcdGotoXY(1,6);
			temp_buffer_after_point = temp_buffer % 10;
			itoa(temp_buffer_after_point, buffer, 10);
			lcdPuts(buffer);
			lcdGotoXY(1,8);
			lcdPuts("C");
			
		}
		_delay_ms(1000);
	}
}

void request()                /* Microcontroller send start pulse/request */
{
	DDRD |= (1<<DHT22_PIN);
	PORTD &= ~(1<<DHT22_PIN);    /* set to low pin */
	_delay_ms(20);            /* wait for 20ms */
	PORTD |= (1<<DHT22_PIN);	/* set to high pin */
}

void response()				/* receive response from DHT11 */
{
	DDRD &= ~(1<<DHT22_PIN);
	while(PIND & (1<<DHT22_PIN));
	while((PIND & (1<<DHT22_PIN))==0);
	while(PIND & (1<<DHT22_PIN));
}

uint8_t receive_data()			/* receive data */
{
	int q;
	for (q=0; q<8; q++)
	{
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

int get_checksum(){
	
	return lowByteRh + highByteRh + lowByteTemp + highByteTemp;
}
