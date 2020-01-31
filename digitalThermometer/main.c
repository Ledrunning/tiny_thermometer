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

uint8_t c=0,lowByteRh, highByteRh, lowByteTemp, highByteTemp, checkSum;
uint16_t temperatureResult, humidityResult;
const int negativePointShift = -10;

void print_negative_temperature(char* tBuffer, int negativeTemp);
void print_humidity(char* buffer);
void print_temperature(char* buffer, uint16_t temp_after_point);
void request();                /* Microcontroller send start pulse/request */
void response();				/* receive response from DHT11 */
uint8_t receive_data();			/* receive data */
int get_checksum();

int main(void) {
	
	char tBuffer[STRING_SIZE], hBuffer[STRING_SIZE];
	uint16_t temp_buffer;
	uint16_t temp_buffer_after_point;
	int negativeTemp;

	lcdInit();
	lcdClear();
	lcdSetDisplay(LCD_DISPLAY_ON);
	lcdSetCursor(LCD_CURSOR_OFF);
	
	while (1) {

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
		if ((get_checksum() & 255) != checkSum)
		{
			lcdGotoXY(0,0);
			lcdPuts("Sens    ");
			lcdGotoXY(1,0);
			lcdPuts("Err     ");
		}
		else
		{
			humidityResult = (lowByteRh * 256 + highByteRh ) / DOZEN;
			print_humidity(hBuffer);
			
			temperatureResult = (lowByteTemp * 256 + highByteTemp );
			
			if(temperatureResult > TEMP_MASK)
			{
				negativeTemp = -(TEMP_MASK & temperatureResult);
				print_negative_temperature(tBuffer, negativeTemp);
			}
			else{
				temp_buffer = temperatureResult;
				temp_buffer_after_point = temp_buffer % DOZEN;
				print_temperature(tBuffer, temp_buffer_after_point);
			}
			
		}
		_delay_ms(1000);
	}
}

void print_humidity(char* buffer)
{
	lcdGotoXY(0,0);
	lcdPuts("H=");
	lcdGotoXY(0,2);
	itoa(humidityResult, buffer, DOZEN);
	lcdPuts(buffer);
	lcdGotoXY(0, 5);
	lcdPuts("%");
}

void print_negative_temperature(char* buffer, int negativeTemp){
	
	if(negativeTemp < negativePointShift) {
		lcdGotoXY(1,0);
		lcdPuts("T=");
		lcdGotoXY(1,2);
		itoa(negativeTemp/DEC, buffer, DEC);
		lcdPuts(buffer);
		lcdGotoXY(1,4);
		lcdPuts("C");
	}
	else{
		lcdGotoXY(1,0);
		lcdPuts("T=");
		lcdGotoXY(1,2);
		itoa(negativeTemp/DEC, buffer, DEC);
		lcdPuts(buffer);
		lcdGotoXY(1,5);
		lcdPuts("C");
	}
	
}

void print_temperature(char* buffer, uint16_t temp_after_point)
{
	lcdGotoXY(1,0);
	lcdPuts("T=");
	lcdGotoXY(1,2);
	itoa(temperatureResult, buffer, DEC);
	lcdPuts(buffer);
	lcdGotoXY(1,4);
	lcdPuts(".");
	lcdGotoXY(1,5);
	itoa(temp_after_point, buffer, DEC);
	lcdPuts(buffer);
	lcdGotoXY(1,6);
	lcdPuts("C");
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
