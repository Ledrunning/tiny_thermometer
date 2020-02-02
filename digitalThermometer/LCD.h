// LCD lib teken from https://micro-pi.ru/%d1%81%d1%85%d0%b5%d0%bc%d0%b0-%d0%bf%d0%be%d0%b4%d0%ba%d0%bb%d1%8e%d1%87%d0%b5%d0%bd%d0%b8%d1%8f-hd44780-%d0%ba-atmega16/
#ifndef LCD_H_
#define LCD_H_

#define LCDDATAPORT        PORTB  // Порт и пины,
#define LCDDATADDR         DDRB   // к которым подключены
#define LCDDATAPIN         PINB   // сигналы D4-D7.
#define LCD_D4             3
#define LCD_D5             4
#define LCD_D6             5
#define LCD_D7             6

#define LCDCONTROLPORT     PORTB  // Порт и пины,
#define LCDCONTROLDDR      DDRB   // к которым подключены
#define LCD_RS             0      // сигналы RS, RW и E.
#define LCD_RW             1
#define LCD_E              2

#define LCD_STROBEDELAY_US 5      // Задержка строба

#define LCD_COMMAND        0
#define LCD_DATA           1

#define LCD_CURSOR_OFF     0
#define LCD_CURSOR_ON      2
#define LCD_CURSOR_BLINK   3

#define LCD_DISPLAY_OFF    0
#define LCD_DISPLAY_ON     4

#define LCD_SCROLL_LEFT    0
#define LCD_SCROLL_RIGHT   4

#define LCD_STROBDOWN      0
#define LCD_STROBUP        1

#define DELAY              1

void lcdSendNibble(char byte, char state);
char lcdGetNibble(char state);
char lcdRawGetByte(char state);
void lcdRawSendByte(char byte, char state);
char lcdIsBusy(void);
void lcdInit(void);
void lcdSetCursor(char cursor);
void lcdSetDisplay(char state);
void lcdClear(void);
void lcdGotoXY(char str, char col);
void lcdDisplayScroll(char pos, char dir);
void lcdPuts(char *str);
void lcdPutsf(char *str);
void lcdPutse(uint8_t *str);
void lcdLoadCharacter(char code, char *pattern);
void lcdLoadCharacterf(char code, char *pattern);
void lcdLoadCharactere(char code, char *pattern);
void printChar(uint8_t data);
void lcdPutData(uint8_t data);
#endif /* LCD_H_ */