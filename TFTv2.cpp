#include <TFTv2.h>
#include <SPI.h>
#include <init_cmd.h>

#define RED_8BPP     0xE0
#define GREEN_8BPP   0x1C
#define BLUE_8BPP    0x03

uint16_t rgb888_to_565(uint32_t v888)
{
    return ((v888 & 0xf80000ul) >> 8) | ((v888 & 0x00fc00ul) >> 5) | ((v888 & 0x0000f8ul) >> 3);
}

uint16_t rgb332_to_565(uint8_t v332)
{
    uint16_t t332 = (uint32_t)v332;
    return ((t332 & RED_8BPP) << 8) | ((t332 & RED_8BPP & (RED_8BPP<<1)) << 5) | 
        ((t332 & GREEN_8BPP) << 6) | ((t332 & GREEN_8BPP) << 3) | 
        ((t332 & BLUE_8BPP) << 3) | ((t332 & BLUE_8BPP) << 1) | ((t332 & BLUE_8BPP)>>1);
}

void TFT::setOrientation(uint8_t orientation)
{
    if(orientation!=m_orientation && (orientation==PORTRAIT || orientation==LANDSCAPE))
    {
        swap(uint16_t,m_maxX,m_maxY)
        m_orientation=orientation;
    }
}

void TFT::sendCMD(uint8_t index)
{
    TFT_DC_LOW;
    SPI.transfer(index);
}

void TFT::sendData(uint8_t data)
{
    TFT_DC_HIGH;
    SPI.transfer(data);
}

uint8_t TFT::readData()
{
    TFT_DC_HIGH;
    return SPI.transfer(0);
}

void TFT::sendLongData(uint16_t data)
{
    TFT_DC_HIGH;
    SPI.transfer((uint8_t)(data>>8));
    SPI.transfer((uint8_t)(data&0xff));
}

uint8_t TFT::Read_Register(uint8_t Addr, uint8_t xParameter)
{
    sendCMD(0xd9);                 /* ext command                  */
    sendData(0x10+xParameter);     /* 0x11 is the first Parameter  */
    sendCMD(Addr);
    return readData();
}

void TFT::TFTinit(uint8_t orientation)
{
    setOrientation(orientation);
    TFTinit();
}

void TFT::TFTinit(void)
{
    SPI.begin();
    TFT_PORT_DIR;
    TFT_DC_HIGH;
    TFT_CS_HIGH;
    uint8_t TFTDriver=0;

    TFT_RST_ON;
    delay(10);
    TFT_RST_OFF;

    TFT_CS_LOW;

    for(uint8_t i=0;i<3;i++) TFTDriver = readID();

    for(uint16_t i=0;;)
    {
        uint8_t type = pgm_read_byte(&init_ili9341[i++]);
        if (type==END) { break;}
        else {
            uint8_t value = pgm_read_byte(&init_ili9341[i++]);
            if(type==CMD) { sendCMD(value); }
            else if(type==DATA) {
                for(uint8_t j=0;j<value;j++)
                {
                    uint8_t data = pgm_read_byte(&init_ili9341[i++]);
                    sendData(data);
                }
            }
            else if(type==DELAY) { delay(value); }
        }
    }

    TFT_CS_HIGH;

    fillScreen();
}

uint8_t TFT::readID(void)
{
    uint8_t i=0;
    uint8_t data[3] ;
    uint8_t ID[3] = {0x00, 0x93, 0x41};
    uint8_t ToF=1;
    for(i=0;i<3;i++)
    {
        data[i]=Read_Register(0xd3,i+1);
        if(data[i] != ID[i]) ToF=0;
    }
    return ToF;
}

void TFT::setScreen(uint16_t startX, uint16_t endX, uint16_t startY, uint16_t endY)
{
    if(m_orientation==LANDSCAPE) {
        endX = m_maxX-endX;
        startX = m_maxX-startX;
        swap(uint16_t,startX,endX);
    }

    sendCMD(m_orientation==PORTRAIT?0x2A:0x2B);
    sendLongData(startX);
    sendLongData(endX);
    sendCMD(m_orientation==PORTRAIT?0x2B:0x2A);
    sendLongData(startY);
    sendLongData(endY);
}

void TFT::fillScreen(uint16_t XL, uint16_t XR, uint16_t YU, uint16_t YD, uint16_t color)
{
    uint32_t XY=0, i=0;

    if(XL > XR) swap(uint16_t,XL,XR);
    if(YU > YD) swap(uint16_t,YU,YD);

    XL = constrain(XL, 0, m_maxX);
    XR = constrain(XR, 0, m_maxX);
    YU = constrain(YU, 0, m_maxY);
    YD = constrain(YD, 0, m_maxY);

    XY = (uint32_t)(XR-XL+1)*(uint32_t)(YD-YU+1);

    TFT_CS_LOW;
    setScreen(XL, XR, YU, YD);
    sendCMD(0x2c);

    TFT_DC_HIGH;

    for(i=0;i < XY;i++)
    {
        SPI.transfer(color>>8);
        SPI.transfer(color&0xff);
    }

    TFT_CS_HIGH;
}

void TFT::fillScreen(uint16_t color)
{
    fillScreen(0, m_maxX, 0, m_maxY, color);
}

void TFT::setPixel(uint16_t poX, uint16_t poY, uint16_t color)
{
    TFT_CS_LOW;
    setScreen(poX, poX, poY, poY);
    sendCMD(0x2c);
    sendLongData(color);
    TFT_CS_HIGH;
}

void TFT::fillRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint16_t color)
{
    fillScreen(poX, poX + length - 1, poY, poY + width - 1, color);
}

void TFT::drawHLine(uint16_t poX, uint16_t poY, uint16_t length, uint16_t color)
{
    TFT_CS_LOW;
    setScreen(poX, poX + length - 1, poY, poY);
    sendCMD(0x2c);
    for(uint16_t i=0;i<length;i++) sendLongData(color);
    TFT_CS_HIGH;
}

void TFT::drawVLine(uint16_t poX, uint16_t poY, uint16_t length, uint16_t color)
{
    TFT_CS_LOW;
    setScreen(poX, poX, poY, poY + length - 1);
    sendCMD(0x2c);
    for(uint16_t i=0;i<length;i++) sendLongData(color);
    TFT_CS_HIGH;
}

void TFT::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int16_t x = x1-x0, y = y1-y0, dy = -abs(y), dx = abs(x), err = dx+dy, e2;
    int8_t sx = (x0<x1?1:-1), sy = (y0<y1?1:-1);
    for (;;){                                                           /* loop                         */
        setPixel(x0,y0,color);
        e2 = 2*err;
        if (e2 >= dy) {                                                 /* e_xy+e_x > 0                 */
            if (x0 == x1) break;
            err += dy; x0 += sx;
        }
        if (e2 <= dx) {                                                 /* e_xy+e_y < 0                 */
            if (y0 == y1) break;
            err += dx; y0 += sy;
        }
    }
}

void TFT::drawRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint16_t color)
{
    drawHLine(poX, poY, length, color);
    drawHLine(poX, poY + width -1, length, color);
    drawVLine(poX, poY, width, color);
    drawVLine(poX + length - 1, poY, width, color);
}

void TFT::drawCircle(uint16_t poX, uint16_t poY, uint16_t r, uint16_t color)
{
    int32_t x = -r, err = 2-2*r, e2;
    uint16_t y = 0;
    do {
        setPixel(poX-x, poY+y, color);
        setPixel(poX-x, poY-y, color);
        if(x<0) { 
            setPixel(poX+x, poY+y, color);
            setPixel(poX+x, poY-y, color);
        }
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}

void TFT::fillCircle(uint16_t poX, uint16_t poY, uint16_t r, uint16_t color)
{
    int x = -r, err = 2-2*r, e2, y = 0;
    do {
        drawVLine(poX-x, poY-y, 2*y+1, color);
        if(x<0) drawVLine(poX+x, poY-y, 2*y+1, color);

        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);

}

void TFT::drawTriangle(uint16_t poX1, uint16_t poY1, uint16_t poX2, uint16_t poY2, uint16_t poX3, uint16_t poY3, uint16_t color)
{
    drawLine(poX1, poY1, poX2, poY2, color);
    drawLine(poX1, poY1, poX3, poY3, color);
    drawLine(poX2, poY2, poX3, poY3, color);
}

void TFT::drawBitmap(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint8_t* bits)
{
    uint32_t XY=0;

    XY = (uint32_t)(length)*(uint32_t)(width);

    if (poX+length-1>m_maxX || poY+width-1>m_maxY) return;

    TFT_CS_LOW;
    setScreen(poX, poX+length-1, poY, poY+width-1);
    sendCMD(0x2c);

    TFT_DC_HIGH;

    for(uint32_t i=0;i<XY;i++)
    {
        uint16_t pix = rgb332_to_565(bits[i]);
        SPI.transfer(pix>>8);
        SPI.transfer(pix&0xff);
    }

    TFT_CS_HIGH;
}

/* Writing functionalities */
void TFT::drawChar(uint8_t ascii, uint16_t poX, uint16_t poY, uint8_t size, uint16_t fgcolor, Font* font)
{
    if(font==NULL) font=&m_defaultFont;
    if((ascii<font->offset)||(ascii>(font->offset+font->numchars))) ascii = '?'-32;

    uint8_t* cfont = font->cfont;
    for (uint8_t i=0;i<(font->x_size);i++) {
        uint8_t temp = pgm_read_byte(&cfont[(ascii-font->offset)*font->x_size+i]);
        for(uint8_t f=0;f<8;f++)
        {
            if((temp>>f)&0x01) fillRectangle(poX+i*size, poY+f*size, size, size, fgcolor);
        }
    }
}

void TFT::drawString(char const *string, uint16_t poX, uint16_t poY, uint8_t size, uint16_t fgcolor, Font* font)
{
    if(font==NULL) font=&m_defaultFont;
    while(*string)
    {
        drawChar(*string, poX, poY, size, fgcolor, font);
        *string++;

        if(poX < m_maxX) poX += (font->x_size+font->space)*size; /* Move cursor right */
    }
}

void TFT::print(char const *st, uint16_t x, uint16_t y, uint8_t size, uint16_t fgColor, Font* font)
{
    if(font==NULL) font=&m_defaultFont;
    int stl, i;

    stl = strlen(st);

    if (m_orientation==PORTRAIT)
    {
    if (x==RIGHT)
        x=(m_maxX+1)-(stl*(font->x_size+font->space));
    if (x==CENTER)
        x=((m_maxX+1)-(stl*(font->x_size+font->space)))/2;
    }
    else
    {
    if (x==RIGHT)
        x=(m_maxY+1)-(stl*(font->x_size+font->space));
    if (x==CENTER)
        x=((m_maxY+1)-(stl*(font->x_size+font->space)))/2;
    }

    for (i=0; i<stl; i++)
        drawChar(*st++, x + (i*(font->x_size+font->space)), y, size, fgColor, font);
}

uint8_t TFT::drawNumber(long long_num, uint16_t poX, uint16_t poY, uint8_t size, uint16_t fgcolor, Font* font)
{
    uint8_t char_buffer[10] = "";
    uint8_t i = 0;
    uint8_t f = 0;

    if (long_num < 0)
    {
        f=1;
        drawChar('-',poX, poY, size, fgcolor, font);
        long_num = -long_num;
        if(poX < m_maxX)
        {
            poX += (font->x_size+font->space)*size;                                    /* Move cursor right            */
        }
    }
    else if (long_num == 0)
    {
        f=1;
        drawChar('0',poX, poY, size, fgcolor, font);
        return f;
        if(poX < m_maxX)
        {
            poX += (font->x_size+font->space)*size;                                    /* Move cursor right            */
        }
    }


    while (long_num > 0)
    {
        char_buffer[i++] = long_num % 10;
        long_num /= 10;
    }

    f = f+i;
    for(;i > 0;i--)
    {
        drawChar('0'+ char_buffer[i - 1], poX, poY, size, fgcolor, font);
        if(poX < m_maxX)
        {
            poX+=(font->x_size+font->space)*size;                                      /* Move cursor right            */
        }
    }
    return f;
}

uint8_t TFT::drawFloat(float floatNumber, uint8_t decimal, uint16_t poX, uint16_t poY, uint8_t size, uint16_t fgcolor, Font* font)
{
    uint16_t temp=0;
    float decy=0.0;
    float rounding = 0.5;
    uint8_t f=0;
    if(floatNumber<0.0)
    {
        drawChar('-',poX, poY, size, fgcolor, font);
        floatNumber = -floatNumber;
        if(poX < m_maxX)
        {
            poX+=(font->x_size+font->space)*size;                                      /* Move cursor right            */
        }
        f =1;
    }
    for (uint8_t i=0;i<decimal;++i)
    {
        rounding /= 10.0;
    }
    floatNumber += rounding;

    temp = (uint16_t)floatNumber;
    uint8_t howlong=drawNumber(temp,poX, poY, size, fgcolor, font);
    f += howlong;
    if((poX+8*size*howlong) < m_maxX)
    {
        poX+=(font->x_size+font->space)*size*howlong;                                  /* Move cursor right            */
    }

    if(decimal>0)
    {
        drawChar('.',poX, poY, size, fgcolor, font);
        if(poX < m_maxX)
        {
            poX+=(font->x_size+font->space)*size;                                      /* Move cursor right            */
        }
        f +=1;
    }
    decy = floatNumber-temp;                                           /* decimal part,  4             */
    for(uint8_t i=0;i<decimal;i++)                                      
    {
        decy *=10;                                                     /* for the next decimal         */
        temp = decy;                                                   /* get the decimal              */
        drawNumber(temp,poX, poY, size, fgcolor, font);
        floatNumber = -floatNumber;
        if(poX < m_maxX)
        {
            poX+=(font->x_size+font->space)*size;                                      /* Move cursor right            */
        }
        decy -= temp;
    }
    f +=decimal;
    return f;
}

uint8_t TFT::drawFloat(float floatNumber, uint16_t poX, uint16_t poY, uint8_t size, uint16_t fgcolor, Font* font)
{
    uint8_t decimal=2;
    uint16_t temp=0;
    float decy=0.0;
    float rounding = 0.5;
    uint8_t f=0;
    if(floatNumber<0.0)                                                 /* floatNumber < 0              */
    {
        drawChar('-',poX, poY, size, fgcolor, font);                         /* add a '-'                    */
        floatNumber = -floatNumber;
        if(poX < m_maxX)
        {
            poX+=(font->x_size+font->space)*size;                                      /* Move cursor right            */
        }
        f =1;
    }
    for (uint8_t i=0;i<decimal;++i)
    {
        rounding /= 10.0;
    }
    floatNumber += rounding;

    temp = (uint16_t)floatNumber;
    uint8_t howlong=drawNumber(temp, poX, poY, size, fgcolor, font);
    f += howlong;
    if((poX+8*size*howlong) < m_maxX)
    {
        poX+=(font->x_size+font->space)*size*howlong;                                  /* Move cursor right            */
    }


    if(decimal>0)
    {
        drawChar('.',poX, poY, size, fgcolor, font);
        if(poX < m_maxX)
        {
            poX += (font->x_size+font->space)*size;                                    /* Move cursor right            */
        }
        f +=1;
    }
    decy = floatNumber-temp;                                           /* decimal part,                */
    for(uint8_t i=0;i<decimal;i++)
    {
        decy *=10;                                                     /* for the next decimal         */
        temp = decy;                                                   /* get the decimal              */
        drawNumber(temp, poX, poY, size, fgcolor, font);
        floatNumber = -floatNumber;
        if(poX < m_maxX)
        {
            poX += (font->x_size+font->space)*size;                                    /* Move cursor right            */
        }
        decy -= temp;
    }
    f += decimal;
    return f;
}

void TFT::loadFont(uint8_t* font, Font* sfont)
{
    sfont->cfont=(font+FONT_HEADER_SIZE);
    sfont->x_size=pgm_read_byte(&font[0]);
    sfont->y_size=pgm_read_byte(&font[1]);
    sfont->offset=pgm_read_byte(&font[2]);
    sfont->numchars=pgm_read_byte(&font[3]);
    sfont->space=pgm_read_byte(&font[4]);
}