/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"
#include "ChrFont0.h"

void show_ball(int pos);
void play();
void start();
void result();
void show_start();
void show_result();
int  btn_check_0();
int  btn_check_1();
int  btn_check_2();
int  btn_check_3();
void led_set(int data);
void led_blink();
void lcd_init();
void lcd_putc(int y, int x, int c);
void lcd_sync_vbuf();
void lcd_clear_vbuf();

#define INIT	0
#define OPENING	1
#define MENU	2
#define PLAY	3
#define RESULT	4
#define ENDING	5

#define VS_MODE	0
#define VS_CPU	1

int state = INIT, pos = 0;
int mode = VS_MODE;
int point1 = 0, point2 = 0;

/* interrupt_handler() is called every 100msec */
void interrupt_handler() {
	static int cnt;
	if (state == INIT) {
	} else if (state == OPENING) {
		cnt = 0;
	} else if (state == MENU) {
		show_start();
	} else if (state == PLAY) {
		/* Display a ball */
		pos = (cnt < 12) ? cnt : 23 - cnt;
		show_ball(pos);
		if (++cnt >= 24) {
			cnt = 0;
		}
	} else if (state == RESULT) {
		show_result();
	} else if (state == ENDING) {
	}
	lcd_sync_vbuf();
}

void show_start() {
	lcd_clear_vbuf();
	lcd_puts(1, 0, "==========");
	lcd_puts(2, 0, "MatsuTeNI");
	lcd_puts(3, 0, "==========");
	if (mode == VS_MODE) {
		lcd_puts(4, 0, "2vsmode  <<");
		lcd_puts(5, 0, "cpumode");
	} else if (mode == VS_CPU) {
		lcd_puts(4, 0, "2vsmode");
		lcd_puts(5, 0, "cpumode  <<");
	}
	lcd_puts(6, 0, "Go:Push1");
	lcd_sync_vbuf();
}

void show_result() {
	lcd_clear_vbuf();
	lcd_puts(1, 0, "==========");
	lcd_puts(2, 0, "Finished!");
	lcd_puts(3, 0, "==========");
	lcd_puts(4, 0, "1P: " + ('0' + point1));
	lcd_puts(5, 0, "2P: " + ('0' + point2));
	if (point1 == point2)
		lcd_puts(6, 0, "Draw");
	else if (point1 > point2)
		lcd_puts(6, 0, "1P Wins!");
	else
		lcd_puts(6, 0, "2P Wins!");
	lcd_puts(7, 1, "Re:  Push1");
	lcd_sync_vbuf();
}

void main() {
	while (1) {
		if (state == INIT) {
			lcd_init();
			state = OPENING; 
		} else if (state == OPENING) {
			state = MENU;
		} else if (state == MENU) {
			start();
			state = PLAY;
		} else if (state == PLAY) {
			play();
			state = RESULT;
		} else if (state == RESULT) {
			result();
			state = ENDING;
		} else if (state == ENDING) {
			state = OPENING;
		}
	}
}
void start() {
	while (1) {
		if (btn_check_0()) {
			mode = VS_MODE;
		} else if (btn_check_1()) {
			mode = VS_CPU;
		} else if (btn_check_2()) {
			state = PLAY;
			break;
		}
	}
}

void result() {
	while (1) {
		if (btn_check_2()) {
			point1 = 0;
			point2 = 0;
			state = PLAY;
			break;
		}
	}
}

void play() {
	point1 = 0;
	point2 = 0;		
	while (1) {
		/* Button0 is pushed when the ball is in the left edge */
		if (pos == 0 && btn_check_0() ) {
			led_blink();	/* Blink LEDs when hit */
			point1++;
		/* Button3 is pushed when the ball is in the right edge */
		} else if (pos == 11 && btn_check_3()) {
			led_blink();	/* Blink LEDs when hit */
			point2++;
		} else if (btn_check_1()) {
			state = RESULT;
			break;		/* Stop the game */
		}

	}
}
void show_ball(int pos) {
	lcd_clear_vbuf();
	lcd_putc(3, pos, '*');
	lcd_putc(7, 0, '0' + point1);
	lcd_putc(7, 11, '0' + point2);                                                     
}

/*
 * Switch functions
 */
int btn_check_0() {
	volatile int *sw_ptr = (int *)0xff04;;
	return (*sw_ptr & 0x10) ? 1 : 0;
}
int btn_check_1() {
	volatile int *sw_ptr = (int *)0xff04;;
	return (*sw_ptr & 0x20) ? 1 : 0;
}

int btn_check_2() {
	volatile int *sw_ptr = (int *)0xff04;;
	return (*sw_ptr & 0x40) ? 1 : 0;
}

int btn_check_3() {
	volatile int *sw_ptr = (int *)0xff04;;
	return (*sw_ptr & 0x80) ? 1 : 0;
}
/*
 * LED functions
 */
void led_set(int data) {
	volatile int *led_ptr = (int *)0xff08;
	*led_ptr = data;
}
void led_blink() {
	led_set(0xf);				/* Turn on */
	for (int i = 0; i < 300000; i++);	/* Wait */
	led_set(0x0);				/* Turn off */
	for (int i = 0; i < 300000; i++);	/* Wait */
	led_set(0xf);				/* Turn on */
	for (int i = 0; i < 300000; i++);	/* Wait */
	led_set(0x0);				/* Turn off */
}
/*
 * LCD functions
 */
unsigned char lcd_vbuf[64][96];
void lcd_wait(int n) {
        for (int i = 0; i < n; i++);
}

void lcd_cmd(unsigned char cmd) {
        volatile int *lcd_ptr = (int *)0xff0c;
        *lcd_ptr = cmd;
        lcd_wait(1000);
}
void lcd_data(unsigned char data) {
        volatile int *lcd_ptr = (int *)0xff0c;
        *lcd_ptr = 0x100 | data;
        lcd_wait(200);
}
void lcd_pwr_on() {
        volatile int *lcd_ptr = (int *)0xff0c;
        *lcd_ptr = 0x200;
        lcd_wait(700000);
}
void lcd_init() {
        lcd_pwr_on();   /* Display power ON */
        lcd_cmd(0xa0);  /* Remap & color depth */
        lcd_cmd(0x20);
        lcd_cmd(0x15);  /* Set column address */
        lcd_cmd(0);
        lcd_cmd(95);
        lcd_cmd(0x75);  /* Set row address */
        lcd_cmd(0);
        lcd_cmd(63);
        lcd_cmd(0xaf);  /* Display ON */
}

unsigned char lcd_vbuf[64][96];
void lcd_set_vbuf_pixel(int row, int col, int r, int g, int b) {
        r >>= 5; g >>= 5; b >>= 6;
        lcd_vbuf[row][col] = ((r << 5) | (g << 2) | (b << 0)) & 0xff;
}
void lcd_clear_vbuf() {
        for (int row = 0; row < 64; row++)
                for (int col = 0; col < 96; col++)
                        lcd_vbuf[row][col] = 0;
}

void lcd_sync_vbuf() {
        for (int row = 0; row < 64; row++)
                for (int col = 0; col < 96; col++)
                        lcd_data(lcd_vbuf[row][col]);
}

void lcd_putc(int y, int x, int c) {
        for (int v = 0; v < 8; v++)
                for (int h = 0; h < 8; h++)
                        if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
                                lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 0, 255, 0);
}

void lcd_putc_red(int y, int x, int c) {
        for (int v = 0; v < 8; v++)
                for (int h = 0; h < 8; h++)
                        if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
                                lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 255, 0, 0);
}

void lcd_putc_blue(int y, int x, int c) {
        for (int v = 0; v < 8; v++)
                for (int h = 0; h < 8; h++)
                        if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
                                lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 0, 0, 255);
}

void lcd_puts(int y, int x, char *str) {
        for (int i = x; i < 12; i++)
                if (str[i] < 0x20 || str[i] > 0x7f)
                        break;
                else
                        lcd_putc(y, i, str[i]);
}

