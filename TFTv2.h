/*
 2012 Copyright (c) Seeed Technology Inc.

 Authors: Albert.Miao & Loovee, 
 Visweswara R (with initializtion code from TFT vendor)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
#ifndef TFTv2_h
#define TFTv2_h

#if defined(ARDUINO) && ARDUINO >= 100
#define SEEEDUINO
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <avr/pgmspace.h>

#include <SPI.h>

//Basic Colors
#define RED         0xf800
#define GREEN       0x07e0
#define BLUE        0x001f
#define BLACK       0x0000
#define YELLOW      0xffe0
#define WHITE       0xffff

//Other Colors
#define CYAN        0x07ff
#define BRIGHT_RED  0xf810
#define GRAY1       0x8410
#define GRAY2       0x4208

//TFT resolution 240*320
#define X_SIZE      240
#define Y_SIZE      320

#define PORTRAIT    0
#define LANDSCAPE   1

#define LEFT 0
#define RIGHT 9999
#define CENTER 9998

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

#define TFT_PORT_DIR {DDRD |= 0x10;DDRE |= 0x08;DDRH |= 0x18;}
#define TFT_CS_LOW  {/*DDRE |= 0x08;*/PORTE &=~ 0x08;}
#define TFT_CS_HIGH {/*DDRE |= 0x08;*/PORTE |=  0x08;}
#define TFT_DC_LOW  {/*DDRH |= 0x08;*/PORTH &=~ 0x08;}
#define TFT_DC_HIGH {/*DDRH |= 0x08;*/PORTH |=  0x08;}
#define TFT_BL_OFF  {/*DDRH |= 0x10;*/PORTH &=~ 0x10;}
#define TFT_BL_ON   {/*DDRH |= 0x10;*/PORTH |=  0x10;}
#define TFT_RST_OFF {/*DDRD |= 0x10;*/PORTD |=  0x10;}
#define TFT_RST_ON  {/*DDRD |= 0x10;*/PORTD &=~ 0x10;}

#elif defined(__AVR_ATmega32U4__)

#define TFT_PORT_DIR {DDRD |= 0x90;DDRC |= 0x40;DDRE |= 0x40;}
#define TFT_CS_LOW  {/*DDRC |= 0x40;*/PORTC &=~ 0x40;}
#define TFT_CS_HIGH {/*DDRC |= 0x40;*/PORTC |=  0x40;}
#define TFT_DC_LOW  {/*DDRD |= 0x80;*/PORTD &=~ 0x80;}
#define TFT_DC_HIGH {/*DDRD |= 0x80;*/PORTD |=  0x80;}
#define TFT_BL_OFF  {/*DDRE |= 0x40;*/PORTE &=~ 0x40;}
#define TFT_BL_ON   {/*DDRE |= 0x40;*/PORTE |=  0x40;}
#define TFT_RST_OFF {/*DDRD |= 0x10;*/PORTD |=  0x10;}
#define TFT_RST_ON  {/*DDRD |= 0x10;*/PORTD &=~ 0x10;}

#else

#define TFT_PORT_DIR {DDRD |= 0xF0;}
#define TFT_CS_LOW  {/*DDRD |= 0x20;*/PORTD &=~ 0x20;}
#define TFT_CS_HIGH {/*DDRD |= 0x20;*/PORTD |=  0x20;}
#define TFT_DC_LOW  {/*DDRD |= 0x40;*/PORTD &=~ 0x40;}
#define TFT_DC_HIGH {/*DDRD |= 0x40;*/PORTD |=  0x40;}
#define TFT_BL_OFF  {/*DDRD |= 0x80;*/PORTD &=~ 0x80;}
#define TFT_BL_ON   {/*DDRD |= 0x80;*/PORTD |=  0x80;}
#define TFT_RST_OFF {/*DDRD |= 0x10;*/PORTD |=  0x10;}
#define TFT_RST_ON  {/*DDRD |= 0x10;*/PORTD &=~ 0x10;}

#endif

/*#define cbi(reg, bitmask) *reg &= ~bitmask
#define sbi(reg, bitmask) *reg |= bitmask

#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);*/

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define FONT_HEADER_SIZE 5

#define END    0x00
#define CMD    0x01
#define DATA   0x02
#define DELAY  0x03

typedef struct
{
    uint8_t* cfont;
    uint8_t x_size;
    uint8_t y_size;
    uint8_t offset;
    uint8_t numchars;
    uint8_t space;
} Font;

extern uint8_t simpleFont[];
extern uint8_t init_ili9341[];

class TFT
{
public:
    TFT() { loadFont(simpleFont,&m_defaultFont); m_maxY = Y_SIZE-1; m_maxX = X_SIZE-1; m_orientation = PORTRAIT; };
    void TFTinit();
    void TFTinit(uint8_t orientation);
    void loadFont(uint8_t* font, Font* cfont);
    void backLightOn() {TFT_BL_ON};
    void backLightOff() {TFT_BL_OFF};
    void fillScreen(uint16_t XL, uint16_t XR, uint16_t YU, uint16_t YD, uint16_t color=BLACK);
    void fillScreen(uint16_t color=BLACK);
    void setPixel(uint16_t poX, uint16_t poY,uint16_t color=WHITE);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color=WHITE);
    void drawVLine(uint16_t poX, uint16_t poY, uint16_t length, uint16_t color=WHITE);
    void drawHLine(uint16_t poX, uint16_t poY, uint16_t length, uint16_t color=WHITE);
    void drawRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint16_t color=WHITE);
    void fillRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint16_t color=WHITE);
    void drawCircle(uint16_t poX, uint16_t poY, uint16_t r, uint16_t color=WHITE);
    void fillCircle(uint16_t poX, uint16_t poY, uint16_t r, uint16_t color=WHITE);
    void drawTriangle(uint16_t poX1, uint16_t poY1, uint16_t poX2, uint16_t poY2, uint16_t poX3, uint16_t poY3, uint16_t color=WHITE);
    void drawBitmap(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint8_t* bits);

    /* Writing functionalities */
    void drawChar(uint8_t ascii,uint16_t poX, uint16_t poY, uint8_t size=1, uint16_t fgcolor=WHITE, Font* font=NULL);
    void drawString(char const *string,uint16_t poX, uint16_t poY, uint8_t size=1, uint16_t fgcolor=WHITE, Font* font=NULL);
    void print(char const *st, uint16_t x, uint16_t y, uint8_t size=1, uint16_t fgcolor=WHITE, Font* font=NULL);
    uint8_t drawNumber(long long_num, uint16_t poX, uint16_t poY, uint8_t size=1, uint16_t fgcolor=WHITE, Font* font=NULL);
    uint8_t drawFloat(float floatNumber, uint8_t decimal, uint16_t poX, uint16_t poY, uint8_t size=1, uint16_t fgcolor=WHITE, Font* font=NULL);
    uint8_t drawFloat(float floatNumber, uint16_t poX, uint16_t poY, uint8_t size=1, uint16_t fgcolor=WHITE, Font* font=NULL);

    void setOrientation(uint8_t orientation);
    uint16_t getDisplayXSize() { return m_maxX+1; };
    uint16_t getDisplayYSize() { return m_maxY+1; };
Font m_defaultFont;
private:
    uint8_t m_orientation;
    uint16_t m_maxX;
    uint16_t m_maxY;
    

    void setScreen(uint16_t StartCol, uint16_t EndCol, uint16_t StartPage, uint16_t EndPage);
    void sendCMD(uint8_t index);
    void sendData(uint8_t data);
    uint8_t readData();
    void sendLongData(uint16_t data);
    uint8_t Read_Register(uint8_t Addr, uint8_t xParameter);
    uint8_t readID();
};

extern TFT Tft;

#endif

