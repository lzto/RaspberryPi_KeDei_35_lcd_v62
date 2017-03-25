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
 // Linux spidev version
 // gcc -o kedei_lcd_v50_pi_spidev kedei_lcd_v50_pi_spidev.c
 // sudo ./kedei_lcd_v50_pi_spidev
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define LCD_DEVICE "/dev/spidev0.1"
#define LCD_CS 1
#define TOUCH_CS 0
#define LCD_WIDTH  480
#define LCD_HEIGHT 320

#define uint8_t unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long

static const uint32_t mode = 0;
static const uint8_t bits = 8;
static const uint32_t speed = 32000000;

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
volatile int spih;

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

	struct timespec req;
	req.tv_sec  = 0;
	req.tv_nsec = ms * 1000000L;

	nanosleep(&req, NULL);
}


int lcd_open(void) {
	int ret;

	spih = open(LCD_DEVICE, O_WRONLY);
	if(spih<0) return -1;

	ret = ioctl(spih, SPI_IOC_WR_MODE32, &mode);
	if (ret<0) return -1;

	ret = ioctl(spih, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret<0) return -1;

	ret = ioctl(spih, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret<0) return -1;

	return 0;
}

int lcd_close(void) {
	close(spih);


	return 0;
}

int spi_transmit(int devsel, uint8_t *data, int len) {

	struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)data,
			.rx_buf = (unsigned long)NULL,
			.len = len,
			.delay_usecs = 0,
			.speed_hz = speed,
			.bits_per_word = bits
		};

	return ioctl(spih, SPI_IOC_MESSAGE(1), &tr);
}

void lcd_rst(void) {
	uint8_t buff[1];

	buff[0] = 0x00;
	spi_transmit(LCD_CS, &buff[0], sizeof(buff));
	delayms(150);

	buff[0] = 0x01;
	spi_transmit(LCD_CS, &buff[0], sizeof(buff));
	delayms(250);
}


void lcd_cmd(uint8_t cmd) {
	uint8_t b1[2];

	b1[0] = cmd>>1;
	b1[1] = ((cmd&1)<<5) | 0x11;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));

	b1[0] = cmd>>1;
	b1[1] = ((cmd&1)<<5) | 0x1B;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));
}


void lcd_data(uint8_t dat) {
	uint8_t b1[2];

	b1[0] = dat>>1;
	b1[1] = ((dat&1)<<5) | 0x15;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));

	b1[0] = dat>>1;
	b1[1] = ((dat&1)<<5) | 0x1F;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));
}


void lcd_color(uint16_t col) {
	uint8_t b1[3];

	// 18bit color mode ???
	// 0xF800 R(R5-R1, DB17-DB13)
	// 0x07E0 G(G5-G0, DB11- DB6)
	// 0x001F B(B5-B1, DB5 - DB1)
	// 0x40 = R(R0, DB12), 0x20 = B(B0, DB0)
	// copy Red/Blue color bit1 to bit0
	uint8_t pseud = ((col>>5) & 0x40) | ((col<<5) & 0x20);

	b1[0]= col>>8;
	b1[1]= col&0x00FF;
	b1[2]= pseud | 0x15;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));

	b1[0]= col>>8;
	b1[1]= col&0x00FF;
	b1[2]= pseud | 0x1F;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));
}


uint16_t colorRGB(uint8_t r, uint8_t g, uint8_t b) {

	uint16_t col = ((r<<8) & 0xF800) | ((g<<3) & 0x07E0) | ((b>>3) & 0x001F);
//	printf("%02x %02x %02x %04x\n", r, g, b, col);

	return col;
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
	uint8_t pseud = ((r<<6) & 0x40) | ((b<<5) & 0x20);

	b1[0]= col>>8;
	b1[1]= col&0x00FF;
	b1[2]= pseud | 0x15;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));

	b1[0]= col>>8;
	b1[1]= col&0x00FF;
	b1[2]= pseud | 0x1F;
	spi_transmit(LCD_CS, &b1[0], sizeof(b1));
}


void lcd_setrotation(uint8_t m) {
	lcd_cmd(0x36); lcd_data(lcd_rotations[m]);
	if (m&1) {
		lcd_h = LCD_WIDTH;
		lcd_w = LCD_HEIGHT;
	} else {
		lcd_h = LCD_HEIGHT;
		lcd_w = LCD_WIDTH;
	}
}

void lcd_init(void) {
	//reset display
	lcd_rst();

	/*
	KeDei 3.5 inch 480x320 TFT lcd from ali
	https://www.raspberrypi.org/forums/viewtopic.php?t=124961&start=162
	by Conjur > Thu Aug 04, 2016 11:57 am
	Initialize Parameter
	<Pulse reset>
	00
	11
	EE 02 01 02 01
	ED 00 00 9A 9A 9B 9B 00 00 00 00 AE AE 01 A2 00
	B4 00
	C0 10 3B 00 02 11
	C1 10
	C8 00 46 12 20 0C 00 56 12 67 02 00 0C
	D0 44 42 06
	D1 43 16
	D2 04 22
	D3 04 12
	D4 07 12
	E9 00
	C5 08
	36 2A
	3A 66
	2A 00 00 01 3F
	2B 00 00 01 E0
	35 00
	29
	00
	11
	EE 02 01 02 01
	ED 00 00 9A 9A 9B 9B 00 00 00 00 AE AF 01 A2 01 BF 2A
	*/

	lcd_cmd(0x00);
	lcd_cmd(0x11);delayms(200); //Sleep Out

	lcd_cmd(0xEE); lcd_data(0x02); lcd_data(0x01); lcd_data(0x02); lcd_data(0x01);
	lcd_cmd(0xED); lcd_data(0x00); lcd_data(0x00); lcd_data(0x9A); lcd_data(0x9A); lcd_data(0x9B); lcd_data(0x9B); lcd_data(0x00); lcd_data(0x00); lcd_data(0x00); lcd_data(0x00); lcd_data(0xAE); lcd_data(0xAE); lcd_data(0x01); lcd_data(0xA2); lcd_data(0x00);
	lcd_cmd(0xB4); lcd_data(0x00);

	// LCD_WIDTH
	lcd_cmd(0xC0); lcd_data(0x10); lcd_data(0x3B); lcd_data(0x00); lcd_data(0x02); lcd_data(0x11);
	lcd_cmd(0xC1); lcd_data(0x10);
	lcd_cmd(0xC8); lcd_data(0x00); lcd_data(0x46); lcd_data(0x12); lcd_data(0x20); lcd_data(0x0C); lcd_data(0x00); lcd_data(0x56); lcd_data(0x12); lcd_data(0x67); lcd_data(0x02); lcd_data(0x00); lcd_data(0x0C);

	lcd_cmd(0xD0); lcd_data(0x44); lcd_data(0x42); lcd_data(0x06);
	lcd_cmd(0xD1); lcd_data(0x43); lcd_data(0x16);
	lcd_cmd(0xD2); lcd_data(0x04); lcd_data(0x22);
	lcd_cmd(0xD3); lcd_data(0x04); lcd_data(0x12);
	lcd_cmd(0xD4); lcd_data(0x07); lcd_data(0x12);

	lcd_cmd(0xE9); lcd_data(0x00);
	lcd_cmd(0xC5); lcd_data(0x08);

	// 36 2A
	lcd_setrotation(0);
	lcd_cmd(0x3A); lcd_data(0x66);	// RGB666 18bit color
	//	2A 00 00 01 3F
	//	2B 00 00 01 E0
	lcd_cmd(0x35); lcd_data(0x00);

	lcd_cmd(0x29);delayms(200); // Display On
	lcd_cmd(0x00);	// NOP
	lcd_cmd(0x11);delayms(200); // Sleep Out

	//
	lcd_cmd(0xEE); lcd_data(0x02); lcd_data(0x01); lcd_data(0x02); lcd_data(0x01);
	lcd_cmd(0xED); lcd_data(0x00); lcd_data(0x00); lcd_data(0x9A); lcd_data(0x9A); lcd_data(0x9B); lcd_data(0x9B); lcd_data(0x00); lcd_data(0x00); lcd_data(0x00); lcd_data(0x00); lcd_data(0xAE); lcd_data(0xAF); lcd_data(0x01); lcd_data(0xA2); lcd_data(0x01); lcd_data(0xBF); lcd_data(0x2A);
}

void lcd_setframe(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	lcd_cmd(0x2A);
	lcd_data(x>>8); lcd_data(x&0xFF);
	lcd_data(((w+x)-1)>>8); lcd_data(((w+x)-1)&0xFF);
	lcd_cmd(0x2B);
	lcd_data(y>>8); lcd_data(y&0xFF);
	lcd_data(((h+y)-1)>>8); lcd_data(((h+y)-1)&0xFF);
	lcd_cmd(0x2C);
}

//lcd_fillframe
//fills an area of the screen with a single color.
void lcd_fillframe(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t col) {
	int span=h*w;
	lcd_setframe(x,y,w,h);
	int q;
	for(q=0;q<span;q++) { lcd_color(col); }
}

void lcd_fill(uint16_t col) {
	lcd_fillframe(0, 0, lcd_w, lcd_h, col);
}


void lcd_fillframeRGB(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b) {
	int span=h*w;
	lcd_setframe(x,y,w,h);
	int q;
	for(q=0;q<span;q++) { lcd_colorRGB(r, g, b); }
}

void lcd_fillRGB(uint8_t r, uint8_t g, uint8_t b) {
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


int main(int argc,char *argv[]) {

	lcd_open();

	lcd_init();

	lcd_fill(0); //black out the screen.

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

