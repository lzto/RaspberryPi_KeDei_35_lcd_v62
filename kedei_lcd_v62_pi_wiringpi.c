 // Original code - KeDei V5.0 code - Conjur
 //  https://www.raspberrypi.org/forums/viewtopic.php?p=1019562
 //  Mon Aug 22, 2016 2:12 am - Final post on the KeDei v5.0 code.
 // References code - Uladzimir Harabtsou l0nley
 //  https://github.com/l0nley/kedei35
 //
 // KeDei 3.5inch LCD V5.0 module for Raspberry Pi
 // Modified by FREE WING, Y.Sakamoto
 // http://www.neko.ne.jp/~freewing/
 //
 // WiringPi library version
 // gcc -o kedei_lcd_v50_pi_wiringpi kedei_lcd_v50_pi_wiringpi.c -lwiringPi
 // sudo ./kedei_lcd_v50_pi_wiringpi
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <wiringPiSPI.h>
#include <wiringPi.h>

#define LCD_CS 1
#define TOUCH_CS 0

#define LCD_WIDTH  480
#define LCD_HEIGHT 320

#define uint8_t unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long

uint8_t lcd_rotations[4] = {
	0b11101010,	//   0
	0b01001010,	//  90
	0b00101010,	// 180
	0b10001010	// 270
};

volatile uint8_t color;
volatile uint8_t lcd_rotation;
volatile uint16_t lcd_h;
volatile uint16_t lcd_w;

uint16_t colors[16] = {
	0b0000000000000000,				/* BLACK	000000 */
	0b0000000000010000,				/* NAVY		000080 */
	0b0000000000011111,				/* BLUE		0000ff */
	0b0000010011000000,				/* GREEN	009900 */
	0b0000010011010011,				/* TEAL		009999 */
	0b0000011111100000,				/* LIME		00ff00 */
	0b0000011111111111,				/* AQUA		00ffff */
	0b1000000000000000,				/* MAROON	800000 */
	0b1000000000010000,				/* PURPLE	800080 */
	0b1001110011000000,				/* OLIVE	999900 */
	0b1000010000010000,				/* GRAY		808080 */
	0b1100111001111001,				/* SILVER	cccccc */
	0b1111100000000000,				/* RED		ff0000 */
	0b1111100000011111,				/* FUCHSIA	ff00ff */
	0b1111111111100000,				/* YELLOW	ffff00 */
	0b1111111111111111				/* WHITE	ffffff */
};


void delayms(int ms) {
	delay(ms);
}


int lcd_open(void) {
	int r;
    r = wiringPiSetup();
    if (r<0)
        return -1;

	r = wiringPiSPISetup(LCD_CS, 32000000);
	if(r<0)
        return -1;
#if 0
	r = wiringPiSPISetup(TOUCH_CS, 1000000);
	if(r<0)
        return -1;
#endif
    wiringPiSetup();
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
	return 0;
}

int lcd_close(void) {

	return 0;
}

int spi_transmit(int devsel, uint8_t *data, int len)
{
    int ret = 0;
    digitalWrite(10, HIGH);
	ret = wiringPiSPIDataRW(devsel, (unsigned char*)data, len);
    digitalWrite(10, LOW);
    return ret;
}

void lcd_rst(void)
{
	uint8_t buff[4];
	buff[0] = 0x00;
	buff[1] = 0x01;
	buff[2] = 0x00;
    buff[3] = 0x00;
	spi_transmit(LCD_CS, &buff[0], sizeof(buff));
	delayms(50);

	buff[0] = 0x00;
	buff[1] = 0x00;
	buff[2] = 0x00;
	buff[3] = 0x00;
	spi_transmit(LCD_CS, &buff[0], sizeof(buff));
	delayms(100);
    
	buff[0] = 0x00;
	buff[1] = 0x01;
	buff[2] = 0x00;
	buff[3] = 0x00;
	spi_transmit(LCD_CS, &buff[0], sizeof(buff));
	delayms(50);
}


void lcd_cmd(uint8_t cmd)
{
    uint8_t b1[3];
    b1[0] = 0x11;
	b1[1] = 0x00;
	b1[2] = cmd;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));
}


void lcd_data(uint8_t dat)
{
    uint8_t b1[3];
    b1[0] = 0x15;
    b1[1] = 0x00;
    b1[2] = dat;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));
}

void lcd_color(uint16_t col)
{
    uint8_t b1[3];
    b1[0] = 0x15;
    b1[1] = col>>8;
    b1[2] = col&0xFF;
    spi_transmit(LCD_CS, &b1[0], sizeof(b1));
}

// 18bit color mode
void lcd_colorRGB(uint8_t r, uint8_t g, uint8_t b) {
	uint8_t b1[3];

	uint16_t col = ((r<<8) & 0xF800) | ((g<<3) & 0x07E0) | ((b>>3) & 0x001F);

	// 18bit color mode ???
	// 0xF800 R(R5-R1, DB17-DB13)
	// 0x07E0 G(G5-G0, DB11- DB6)
	// 0x001F B(B5-B1, DB5 - DB1)
	// 0x40 = R(R0, DB12), 0x20 = B(B0, DB0)
    b1[0] = 0x15;
	b1[1] = col>>8;
	b1[2] = col&0x00FF;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));
}


void lcd_setrotation(uint8_t m)
{
	lcd_cmd(0x36);
    lcd_data(lcd_rotations[m]);
	if (m&1) {
		lcd_h = LCD_WIDTH;
		lcd_w = LCD_HEIGHT;
	} else {
		lcd_h = LCD_HEIGHT;
		lcd_w = LCD_WIDTH;
	}
}

void lcd_init(void)
{
//reset display
	lcd_rst();
//kedei 6.2
    lcd_cmd(0x00);
    delayms(10);
    lcd_cmd(0xFF); lcd_cmd(0xFF);
    delayms(10);
    lcd_cmd(0xFF); lcd_cmd(0xFF); lcd_cmd(0xFF); lcd_cmd(0xFF);
    delayms(15);
    lcd_cmd(0x11);
    delayms(150);

    lcd_cmd(0xB0); lcd_data(0x00);
    lcd_cmd(0xB3); lcd_data(0x02); lcd_data(0x00); lcd_data(0x00); lcd_data(0x00);
    lcd_cmd(0xB9); lcd_data(0x01); lcd_data(0x00); lcd_data(0x0F); lcd_data(0x0F);
    lcd_cmd(0xC0); lcd_data(0x13); lcd_data(0x3B); lcd_data(0x00); lcd_data(0x02);
                   lcd_data(0x00); lcd_data(0x01); lcd_data(0x00); lcd_data(0x43);
    lcd_cmd(0xC1); lcd_data(0x08); lcd_data(0x0F); lcd_data(0x08); lcd_data(0x08);
    lcd_cmd(0xC4); lcd_data(0x11); lcd_data(0x07); lcd_data(0x03); lcd_data(0x04);
    lcd_cmd(0xC6); lcd_data(0x00);
    lcd_cmd(0xC8); lcd_data(0x03); lcd_data(0x03); lcd_data(0x13); lcd_data(0x5C);
                   lcd_data(0x03); lcd_data(0x07); lcd_data(0x14); lcd_data(0x08);
                   lcd_data(0x00); lcd_data(0x21); lcd_data(0x08); lcd_data(0x14);
                   lcd_data(0x07); lcd_data(0x53); lcd_data(0x0C); lcd_data(0x13);
                   lcd_data(0x03); lcd_data(0x03); lcd_data(0x21); lcd_data(0x00);
    lcd_cmd(0x35); lcd_data(0x00);
    lcd_cmd(0x36); lcd_data(0x60);
    lcd_cmd(0x3A); lcd_data(0x55);
    lcd_cmd(0x44); lcd_data(0x00); lcd_data(0x01);
    lcd_cmd(0xD0); lcd_data(0x07); lcd_data(0x07); lcd_data(0x1D); lcd_data(0x03);
    lcd_cmd(0xD1); lcd_data(0x03); lcd_data(0x30); lcd_data(0x10);
    lcd_cmd(0xD2); lcd_data(0x03); lcd_data(0x14); lcd_data(0x04);
    lcd_cmd(0x29);

    delayms(30);

    lcd_cmd(0x2A); lcd_data(0x00); lcd_data(0x00); lcd_data(0x01); lcd_data(0x3F);
    lcd_cmd(0x2B); lcd_data(0x00); lcd_data(0x00); lcd_data(0x01); lcd_data(0xE0);
    lcd_cmd(0xB4); lcd_data(0x00);

    lcd_cmd(0x2C);

    delayms(10);
    lcd_setrotation(0);
}

void lcd_setframe(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	lcd_cmd(0x2A);
	    lcd_data(x>>8);
        lcd_data(x&0xFF);
	    lcd_data(((w+x)-1)>>8);
        lcd_data(((w+x)-1)&0xFF);
	lcd_cmd(0x2B);
	    lcd_data(y>>8);
        lcd_data(y&0xFF);
	    lcd_data(((h+y)-1)>>8);
        lcd_data(((h+y)-1)&0xFF);
	lcd_cmd(0x2C);
}

//lcd_fillframe
//fills an area of the screen with a single color.
void lcd_fillframe(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t col)
{
	int span=h*w;
	int q;
	lcd_setframe(x,y,w,h);
	for(q=0;q<span;q++)
    {
        lcd_color(col);
    }
}

void lcd_fill(uint16_t col)
{
    lcd_fillframe(0, 0, lcd_w, lcd_h, col);
}

void lcd_fillframeRGB(uint16_t x, uint16_t y,
        uint16_t w, uint16_t h,
        uint8_t r, uint8_t g, uint8_t b)
{
	int span=h*w;
	lcd_setframe(x,y,w,h);
	int q;
	for(q=0;q<span;q++)
    {
        lcd_colorRGB(r, g, b);
    }
}

void lcd_fillRGB(uint8_t r, uint8_t g, uint8_t b)
{
	lcd_fillframeRGB(0, 0, lcd_w, lcd_h, r, g, b);
}

void lcd_img(char *fname, uint16_t x, uint16_t y) {

	uint8_t buf[54];
	uint16_t p, c;
	uint32_t isize, ioffset, iwidth, iheight, ibpp, fpos, rowbytes;

	FILE *f = fopen(fname, "rb");
	if (f != NULL) {
		fseek(f, 0L, SEEK_SET);
		fread(buf, 30, 1, f);

		isize =	 buf[2] + (buf[3]<<8) + (buf[4]<<16) + (buf[5]<<24);
		ioffset = buf[10] + (buf[11]<<8) + (buf[12]<<16) + (buf[13]<<24);
		iwidth =	buf[18] + (buf[19]<<8) + (buf[20]<<16) + (buf[21]<<24);
		iheight = buf[22] + (buf[23]<<8) + (buf[24]<<16) + (buf[25]<<24);
		ibpp =		buf[28] + (buf[29]<<8);

		printf("\n\n");
		printf("File Size: %u\nOffset: %u\nWidth: %u\nHeight: %u\nBPP: %u\n\n",isize,ioffset,iwidth,iheight,ibpp);

		lcd_setframe(x,y,iwidth,iheight); //set the active frame...
		rowbytes=(iwidth*3) + 4-((iwidth*3)%4);
		for (p=iheight-1;p>0;p--) {
			// p = relative page address (y)
			fpos = ioffset+(p*rowbytes);
			fseek(f, fpos, SEEK_SET);
			for (c=0;c<iwidth;c++) {
				// c = relative column address (x)
				fread(buf, 3, 1, f);

				// B buf[0]
				// G buf[1]
				// R buf[2]
				// 18bit color mode
				lcd_colorRGB(buf[2], buf[1], buf[0]);
			}
		}

		fclose(f);
	}
}


void loop() {

	//Update rotation
	lcd_setrotation(lcd_rotation);

	//Fill entire screen with new color
	lcd_fillframe(0,0,lcd_w,lcd_h,colors[color]);

	//Make a color+1 box, 5 pixels from the top-left corner, 20 pixels high, 95 (100-5) pixels from right border.
	lcd_fillframe(5,5,lcd_w-100,20,colors[(color+1) & 0xF]);

	//increment color
	color++;
	//if color is overflowed, reset to 0
	if (color==16) {color=0;}

	//increment rotation
	lcd_rotation++;

	//if rotation is overflowed, reset to 0
	if (lcd_rotation==4) lcd_rotation=0;

	delayms(500);
}

int main(int argc,char *argv[])
{
	if(lcd_open())
    {
        printf("error!\n");
        exit(-1);
    }

    lcd_init();

    lcd_fill(0x0); //black out the screen.
	// 24bit Bitmap only
	lcd_img("kedei_lcd_v50_pi.bmp", 50, 5);
	delayms(500);
	lcd_fillRGB(0xFF, 0x00, 0x00);
	lcd_fillRGB(0x00, 0xFF, 0x00);
	lcd_fillRGB(0xFF, 0xFF, 0x00);
	lcd_fillRGB(0x00, 0x00, 0xFF);
	lcd_fillRGB(0xFF, 0x00, 0xFF);
	lcd_fillRGB(0x00, 0xFF, 0xFF);
	lcd_fillRGB(0xFF, 0xFF, 0xFF);
	lcd_fillRGB(0x00, 0x00, 0x00);
	// 24bit Bitmap only
	lcd_img("kedei_lcd_v50_pi.bmp", 50, 5);
	delayms(500);
	// Demo
	color=0;
	lcd_rotation=0;
	loop();	loop();	loop();	loop();
	loop();	loop();	loop();	loop();
	loop();	loop();	loop();	loop();
	loop();	loop();	loop();	loop();
	loop();

	// 24bit Bitmap only
	lcd_img("kedei_lcd_v50_pi.bmp", 50, 5);
	lcd_close();
}

