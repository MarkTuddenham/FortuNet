/*  Author: Steve Gunn
 * Licence: This work is licensed under the Creative Commons Attribution License.
 *           View this license at http://creativecommons.org/about/licenses/
 *
 *
 *  - Jan 2015  Modified for LaFortuna (Rev A, black edition) [KPZ]
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdarg.h>
#include "font.h"
#include "ili934x.h"
#include "lcd.h"

lcd display;

void init_lcd() {
    /* Enable extended memory interface with 10 bit addressing */
    XMCRB = _BV(XMM2) | _BV(XMM1);
    XMCRA = _BV(SRE);
    DDRC |= _BV(RESET);
    DDRB |= _BV(BLC);
    _delay_ms(1);
    PORTC &= ~_BV(RESET);
    _delay_ms(20);
    PORTC |= _BV(RESET);
    _delay_ms(120);
    write_cmd(DISPLAY_OFF);
    write_cmd(SLEEP_OUT);
    _delay_ms(60);
    write_cmd_data(INTERNAL_IC_SETTING,          0x01);
    write_cmd(POWER_CONTROL_1);
        write_data16(0x2608);
    write_cmd_data(POWER_CONTROL_2,              0x10);
    write_cmd(VCOM_CONTROL_1);
        write_data16(0x353E);
    write_cmd_data(VCOM_CONTROL_2, 0xB5);
    write_cmd_data(INTERFACE_CONTROL, 0x01);
        write_data16(0x0000);
    write_cmd_data(PIXEL_FORMAT_SET, 0x55);     /* 16bit/pixel */
    set_orientation(West);
    clear_screen();
    display.x = 0;
    display.y = 0;
    display.background = BLACK;
    display.foreground = WHITE;
    write_cmd(DISPLAY_ON);
    _delay_ms(50);
    write_cmd_data(TEARING_EFFECT_LINE_ON, 0x00);
    EICRB |= _BV(ISC61);
    PORTB |= _BV(BLC);
}

void lcd_brightness(uint8_t i) {
    /* Configure Timer 2 Fast PWM Mode 3 */
    TCCR2A = _BV(COM2A1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS20);
    OCR2A = i;
}

void set_orientation(orientation o) {
    display.orient = o;
    write_cmd(MEMORY_ACCESS_CONTROL);
    if (o==North) {
        display.width = LCDWIDTH;
        display.height = LCDHEIGHT;
        write_data(0x48);
    }
    else if (o==West) {
        display.width = LCDHEIGHT;
        display.height = LCDWIDTH;
        write_data(0xE8);
    }
    else if (o==South) {
        display.width = LCDWIDTH;
        display.height = LCDHEIGHT;
        write_data(0x88);
    }
    else if (o==East) {
        display.width = LCDHEIGHT;
        display.height = LCDWIDTH;
        write_data(0x28);
    }
    write_cmd(COLUMN_ADDRESS_SET);
    write_data16(0);
    write_data16(display.width-1);
    write_cmd(PAGE_ADDRESS_SET);
    write_data16(0);
    write_data16(display.height-1);
}



void set_frame_rate_hz(uint8_t f) {
    uint8_t diva, rtna, period;
    if (f>118)
        f = 118;
    if (f<8)
        f = 8;
    if (f>60) {
        diva = 0x00;
    } else if (f>30) {
        diva = 0x01;
    } else if (f>15) {
        diva = 0x02;
    } else {
        diva = 0x03;
    }
    /*   !!! FIXME !!!  [KPZ-30.01.2015] */
    /*   Check whether this works for diva > 0  */
    /*   See ILI9341 datasheet, page 155  */
    period = 1920.0/f;
    rtna = period >> diva;
    write_cmd(FRAME_CONTROL_IN_NORMAL_MODE);
    write_data(diva);
    write_data(rtna);
}

void fill_rectangle(rectangle r, uint16_t col) {
    write_cmd(COLUMN_ADDRESS_SET);
    write_data16(r.left);
    write_data16(r.right);
    write_cmd(PAGE_ADDRESS_SET);
    write_data16(r.top);
    write_data16(r.bottom);
    write_cmd(MEMORY_WRITE);
/*  uint16_t x, y;
    for(x=r.left; x<=r.right; x++)
        for(y=r.top; y<=r.bottom; y++)
            write_data16(col);
*/
    uint16_t wpixels = r.right - r.left + 1;
    uint16_t hpixels = r.bottom - r.top + 1;
    uint8_t mod8, div8;
    uint16_t odm8, odd8;
    if (hpixels > wpixels) {
        mod8 = hpixels & 0x07;
        div8 = hpixels >> 3;
        odm8 = wpixels*mod8;
        odd8 = wpixels*div8;
    } else {
        mod8 = wpixels & 0x07;
        div8 = wpixels >> 3;
        odm8 = hpixels*mod8;
        odd8 = hpixels*div8;
    }
    uint8_t pix1 = odm8 & 0x07;
    while(pix1--)
        write_data16(col);

    uint16_t pix8 = odd8 + (odm8 >> 3);
    while(pix8--) {
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
    }
}

void fill_rectangle_indexed(rectangle r, uint16_t *col) {
    uint16_t x, y;
    write_cmd(COLUMN_ADDRESS_SET);
    write_data16(r.left);
    write_data16(r.right);
    write_cmd(PAGE_ADDRESS_SET);
    write_data16(r.top);
    write_data16(r.bottom);
    write_cmd(MEMORY_WRITE);
    for(x=r.left; x<=r.right; x++)
        for(y=r.top; y<=r.bottom; y++)
            write_data16(*col++);
}

void clear_screen() {
    display.x = 0;
    display.y = 0;
    rectangle r = {0, display.width-1, 0, display.height-1};
    fill_rectangle(r, display.background);
}

void display_char(char c) {
    uint16_t x, y;
    PGM_P fdata;
    uint8_t bits, mask;
    uint16_t sc=display.x, ec=display.x + 4, sp=display.y, ep=display.y + 7;

    /*   New line starts a new line, or if the end of the
         display has been reached, clears the display.
    */
    if (c == '\n') {
        display.x=0; display.y+=8;
        if (display.y >= display.height) { clear_screen(); }
        return;
    }

    if (c < 32 || c > 126) return;
    fdata = (c - ' ')*5 + font5x7;
    write_cmd(PAGE_ADDRESS_SET);
    write_data16(sp);
    write_data16(ep);
    for(x=sc; x<=ec; x++) {
        write_cmd(COLUMN_ADDRESS_SET);
        write_data16(x);
        write_data16(x);
        write_cmd(MEMORY_WRITE);
        bits = pgm_read_byte(fdata++);
        for(y=sp, mask=0x01; y<=ep; y++, mask<<=1)
            write_data16((bits & mask) ? display.foreground : display.background);
    }
    write_cmd(COLUMN_ADDRESS_SET);
    write_data16(x);
    write_data16(x);
    write_cmd(MEMORY_WRITE);
    for(y=sp; y<=ep; y++)
        write_data16(display.background);

    display.x += 6;
    if (display.x >= display.width) { display.x=0; display.y+=8; }
}

void display_string(char *str) {
    uint8_t i;
    for(i=0; str[i]; i++)
        display_char(str[i]);
}

void display_move(uint16_t x, uint16_t y) {
    display.x = x;
    display.y = y;
}

void display_color(uint16_t fg, uint16_t bg) {
	display.foreground = fg;
	display.background = bg;

}

void display_string_xy(char *str, uint16_t x, uint16_t y) {
    uint8_t i;
    display.x = x;
    display.y = y;
    for(i=0; str[i]; i++)
        display_char(str[i]);
}

// void putchar(char c);
static char* bf;
static char buf[12];
static unsigned int num;
static char uc;
static char zs;

static void out(char c) {
    *bf++ = c;
    }

static void outDgt(char dgt) {
	out(dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10));
	zs=1;
    }

static void divOut(unsigned int div) {
    unsigned char dgt=0;
	num &= 0xffff; // just for testing the code  with 32 bit ints
	while (num>=div) {
		num -= div;
		dgt++;
		}
	if (zs || dgt>0)
		outDgt(dgt);
    }

void tfp_printf(char *fmt, ...){
	va_list va;
	char ch;
	char* p;

	va_start(va,fmt);

	while ((ch=*(fmt++))) {
		if (ch!='%') {
			display_char(ch);
			}
		else {
			char lz=0;
			char w=0;
			ch=*(fmt++);
			if (ch=='0') {
				ch=*(fmt++);
				lz=1;
				}
			if (ch>='0' && ch<='9') {
				w=0;
				while (ch>='0' && ch<='9') {
					w=(((w<<2)+w)<<1)+ch-'0';
					ch=*fmt++;
					}
				}
			bf=buf;
			p=bf;
			zs=0;
			switch (ch) {
				case 0:
					goto abort;
				case 'u':
				case 'd' :
					num=va_arg(va, unsigned int);
					if (ch=='d' && (int)num<0) {
						num = -(int)num;
						out('-');
						}
					divOut(10000);
					divOut(1000);
					divOut(100);
					divOut(10);
					outDgt(num);
					break;
				case 'x':
				case 'X' :
				    uc= ch=='X';
					num=va_arg(va, unsigned int);
					divOut(0x1000);
					divOut(0x100);
					divOut(0x10);
					outDgt(num);
					break;
				case 'c' :
					out((char)(va_arg(va, int)));
					break;
				case 's' :
					p=va_arg(va, char*);
					break;
				case '%' :
					out('%');
				default:
					break;
				}
			*bf=0;
			bf=p;
			while (*bf++ && w > 0)
				w--;
			while (w-- > 0)
				display_char(lz ? '0' : ' ');
			while ((ch= *p++))
				display_char(ch);
			}
		}
	abort:;
	va_end(va);
	}

  // void putchar(char c){
  //   display_char(c);
  // }




// }
