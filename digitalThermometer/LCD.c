// LCD lib teken from https://micro-pi.ru/%d1%81%d1%85%d0%b5%d0%bc%d0%b0-%d0%bf%d0%be%d0%b4%d0%ba%d0%bb%d1%8e%d1%87%d0%b5%d0%bd%d0%b8%d1%8f-hd44780-%d0%ba-atmega16/
//#define F_CPU 11059200UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "LCD.h"

/*
  Отправляет младшую половину байта byte в LCD. Если state == 0,
  то передаётся как команда, если нет, то как данные.
 */
void lcdSendNibble(char byte, char state) {
  // Пины управления - на выход
  LCDCONTROLDDR |= 1 << LCD_RS | 1 << LCD_RW | 1 << LCD_E;
  // Пины данных - на выход
  LCDDATADDR |= 1 << LCD_D4 | 1 << LCD_D5 | 1 << LCD_D6 | 1 << LCD_D7;
  // Режим записи, RW = 0
  LCDCONTROLPORT &= ~(1 << LCD_RW);
  // Устанавливаем 1 в RS
  if (state) {
    // если отдаём данные
    LCDCONTROLPORT |= 1 << LCD_RS;
  } else {
    LCDCONTROLPORT &= ~(1 << LCD_RS);
  }
  // Взводим строб
  LCDCONTROLPORT |= 1 << LCD_E;
  // Обнуляем пины данных
  LCDDATAPORT &= ~(1 << LCD_D4 | 1 << LCD_D5 | 1 << LCD_D6 | 1 << LCD_D7);
  // Записываем младшую
  if (byte & (1 << 3)) {
    // половину байта
    LCDDATAPORT |= 1 << LCD_D7;
  }
  // byte в порт вывода данных
  if (byte & (1 << 2)) {
    LCDDATAPORT |= 1 << LCD_D6;
  }

  if (byte & (1 << 1)) {
    LCDDATAPORT |= 1 << LCD_D5;
  }

  if (byte & (1 << 0)) {
    LCDDATAPORT |= 1 << LCD_D4;
  }
  // Пауза
  _delay_us(LCD_STROBEDELAY_US);
  // Опускаем строб. Полубайт ушёл
  LCDCONTROLPORT &= ~(1 << LCD_E);
}

/*
  Читает половину байта из LCD. Если state == 0, то читается команда,
  если нет, то данные.
 */
char lcdGetNibble(char state) {
  char temp = 0;
  // Пины управления - на выход
  LCDCONTROLDDR |= 1 << LCD_RS | 1 << LCD_RW | 1 << LCD_E;
  // Режим чтения
  LCDCONTROLPORT |= 1 << LCD_RW;
  // Устанавливаем 1 в RS
  if (state) {
    // если получаем данные
    LCDCONTROLPORT |= (1 << LCD_RS);
  } else {
    LCDCONTROLPORT &= ~(1 << LCD_RS);
  }
  // Взводим строб
  LCDCONTROLPORT |= 1 << LCD_E;
  // Пины данных - на вход
  LCDDATADDR &= ~(1 << LCD_D4 | 1 << LCD_D5 | 1 << LCD_D6 | 1 << LCD_D7);
  // с подтяжкой
  LCDDATAPORT |= 1 << LCD_D4 | 1 << LCD_D5 | 1 << LCD_D6 | 1 << LCD_D7;
  // Пауза
  _delay_us(LCD_STROBEDELAY_US);
  // Опускаем строб
  LCDCONTROLPORT &= ~(1 << LCD_E);
  // Читаем пины
  if (LCDDATAPIN & (1 << LCD_D7)) {
    // во временную переменную 
    temp |= 1 << 3;
  }

  if (LCDDATAPIN & (1 << LCD_D6)) {
    temp |= 1 << 2;
  }

  if (LCDDATAPIN & (1 << LCD_D5)) {
    temp |= 1 << 1;
  }

  if (LCDDATAPIN & (1 << LCD_D4)) {
    temp |= 1 << 0;
  }
  // возвращаем прочитанное
  return temp;
}

/*
  Читает байт из LCD. Если state == 0, то читается команда,
  если нет, то данные.
 */
char lcdRawGetByte(char state) {
  char temp = 0;

  temp |= lcdGetNibble(state);
  temp = temp << 4;
  temp |= lcdGetNibble(state);

  return temp;
}

/*
  Отравляет байт в LCD. Если state == 0, то передаётся как команда,
  если нет, то как данные.
 */
void lcdRawSendByte(char byte, char state) {
  lcdSendNibble((byte >> 4), state);
  lcdSendNibble(byte, state);
}

/*
  Читает состояние LCD, возвращает 0xff, если флаг занятости установлен,
  и 0x00, если нет.
 */
char lcdIsBusy(void) {
  /*
    if (lcdRawGetByte(LCD_COMMAND) & (1<<7))
      return 0xff;
    else
      return 0x00;
   */
  _delay_ms(DELAY);
  return 0x00;
}

/*
  Выполняет начальную инициализацию дисплея. Четырёхбитный режим. 
 */
void lcdInit(void) {
  while (lcdIsBusy());
  lcdSendNibble(0b0010, LCD_COMMAND);
  while (lcdIsBusy());
  lcdRawSendByte(0b00101000, LCD_COMMAND);
  while (lcdIsBusy());
  lcdRawSendByte(0b00000001, LCD_COMMAND);
  while (lcdIsBusy());
  lcdRawSendByte(0b00000110, LCD_COMMAND);
  while (lcdIsBusy());
  lcdRawSendByte(0b00001100, LCD_COMMAND);
}

/*
  Устанавливает режим курсора: 0 - выключен, 2 - включен, 3 - моргает.
  Если на момент запуска LCD был выключен (lcdSetDisplay), то он будет включен.
 */
void lcdSetCursor(char cursor) {
  while (lcdIsBusy());

  lcdRawSendByte((0b00001100 | cursor), LCD_COMMAND);
}

/*
  Включает или выключает отображение символов LCD.
  При каждом вызове выключает курсор.
 */
void lcdSetDisplay(char state) {
  while (lcdIsBusy());

  lcdRawSendByte((0b00001000 | state), LCD_COMMAND);
}

/*
  Очищает LCD.
 */
void lcdClear(void) {
  while (lcdIsBusy());

  lcdRawSendByte(0b00000001, LCD_COMMAND);
}

/*
  Устанавливает курсор в заданную позицию.
 */
void lcdGotoXY(char str, char col) {
  while (lcdIsBusy());

  lcdRawSendByte((0b10000000 | ((0x40 * str) + col)), LCD_COMMAND);
}

/*
  Сдвигает область отображения на указанное количество символов
  вправо или влево.
 */
void lcdDisplayScroll(char pos, char dir) {
  while (pos) {
    while (lcdIsBusy());

    lcdRawSendByte((0b00011000 | dir), LCD_COMMAND);
    pos--;
  }
}

/*
  Выводит строку из RAM в позицию курсора.
 */
void lcdPuts(char *str) {
  while (*str) {
    while (lcdIsBusy());

    lcdRawSendByte(*str++, LCD_DATA);
  }
}

void printChar(uint8_t data) {
	while (lcdIsBusy());
	lcdRawSendByte(data, LCD_DATA);
}

/*
  Выводит строку из flash в позицию курсора.
 */
void lcdPutsf(char *str) {
  while (pgm_read_byte(str)) {
    while (lcdIsBusy());

    lcdRawSendByte(pgm_read_byte(str++), LCD_DATA);
  }
}

/*
  Выводит строку из eeprom в позицию курсора.
 */
void lcdPutse(uint8_t *str) {
  while (eeprom_read_byte(str)) {
    while (lcdIsBusy());

    lcdRawSendByte((char) (eeprom_read_byte(str++)), LCD_DATA);
  }
}

/*
  Загружает символ в знакогенератор.
 */
void lcdLoadCharacter(char code, char *pattern) {
  while (lcdIsBusy());

  lcdRawSendByte((code << 3) | 0b01000000, LCD_COMMAND);

  for (char i = 0; i <= 7; i++) {
    while (lcdIsBusy());

    lcdRawSendByte(*pattern++, LCD_DATA);
  }
  while (lcdIsBusy());
  lcdRawSendByte(0b10000000, LCD_COMMAND);
}

/*
  Загружает символ из flash в знакогенератор.
 */
void lcdLoadCharacterf(char code, char *pattern) {
  while (lcdIsBusy());

  lcdRawSendByte((code << 3) | 0b01000000, LCD_COMMAND);

  for (char i = 0; i <= 7; i++) {
    while (lcdIsBusy());

    lcdRawSendByte(pgm_read_byte(pattern++), LCD_DATA);
  }
  while (lcdIsBusy());
  lcdRawSendByte(0b10000000, LCD_COMMAND);
}

/*
  Загружает символ из eeprom в знакогенератор.
 */
void lcdLoadCharactere(char code, char *pattern) {
  while (lcdIsBusy());

  lcdRawSendByte((code << 3) | 0b01000000, LCD_COMMAND);

  for (char i = 0; i <= 7; i++) {
    while (lcdIsBusy());

    lcdRawSendByte(eeprom_read_byte(pattern++), LCD_DATA);
  }
  while (lcdIsBusy());
  lcdRawSendByte(0b10000000, LCD_COMMAND);
}

void lcdPutData(uint8_t data) {
	
	while (lcdIsBusy());
	lcdRawSendByte(data, LCD_DATA);

}