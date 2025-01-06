/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"
#include "ChrFont0.h"

void show_ball_and_racket(int x, int y);
void play();
void start();
void result();
void show_start();
void show_result();
int btn_check_0();
int btn_check_1();
int btn_check_2();
int btn_check_3();
void led_set(int data);
void led_blink();
void lcd_init();
void lcd_putc(int y, int x, int c);
void lcd_sync_vbuf();
void lcd_clear_vbuf();

#define INIT 0
#define OPENING 1
#define MENU 2
#define PLAY 3
#define RESULT 4
#define ENDING 5

#define VS_MODE 0
#define VS_CPU 1

int state = INIT, x_pos = 0, y_pos = 3;
int mode = VS_MODE;
int point1 = 0, point2 = 0;
static unsigned int seed = 1;
int x_dir = 1, y_dir = 0;		   // 初期方向
int max_delay = 2, min_delay = 50; // 最大・最小の遅延値
int delay = 20;					   // 現在の遅延値
int toggle_speed = 0;			   // 遅延切り替えフラグ
int racket1 = 3;
int racket2 = 3;

int kypd_scan()
{
	volatile int *ioa_ptr = (int *)0xff14;
	*ioa_ptr = 0x07; /* 0111 */
	for (int i = 0; i < 1; i++)
		; /* Wait */
	if ((*ioa_ptr & 0x80) == 0)
		return 0x1;
	if ((*ioa_ptr & 0x40) == 0)
		return 0x4;
	if ((*ioa_ptr & 0x20) == 0)
		return 0x7;
	if ((*ioa_ptr & 0x10) == 0)
		return 0x0;
	*ioa_ptr = 0x0b; /* 1011 */
	for (int i = 0; i < 1; i++)
		; /* Wait */
	if ((*ioa_ptr & 0x80) == 0)
		return 0x2;
	if ((*ioa_ptr & 0x40) == 0)
		return 0x5;
	if ((*ioa_ptr & 0x20) == 0)
		return 0x8;
	if ((*ioa_ptr & 0x10) == 0)
		return 0xf;
	*ioa_ptr = 0x0d; /* 1101 */
	for (int i = 0; i < 1; i++)
		; /* Wait */
	if ((*ioa_ptr & 0x80) == 0)
		return 0x3;
	if ((*ioa_ptr & 0x40) == 0)
		return 0x6;
	if ((*ioa_ptr & 0x20) == 0)
		return 0x9;
	if ((*ioa_ptr & 0x10) == 0)
		return 0xe;
	*ioa_ptr = 0x0e; /* 1110 */
	for (int i = 0; i < 1; i++)
		; /* Wait */
	if ((*ioa_ptr & 0x80) == 0)
		return 0xa;
	if ((*ioa_ptr & 0x40) == 0)
		return 0xb;
	if ((*ioa_ptr & 0x20) == 0)
		return 0xc;
	if ((*ioa_ptr & 0x10) == 0)
		return 0xd;
	return -1;
}

int rand()
{
	seed = seed * 1103515245 + 12345;
	return (seed / 65536) % 32768;
}

void srand(unsigned int s)
{
	seed = s;
}
/* interrupt_handler() is called every 100msec */
void interrupt_handler()
{
	static int cnt;
	static int delay_cnt = 0;

	if (state == INIT)
	{
	}
	else if (state == OPENING)
	{
		cnt = 0;
		delay = min_delay;
	}
	else if (state == MENU)
	{
		show_start();
	}
	else if (state == PLAY && mode == VS_MODE)
	{
			delay_cnt = 0;

			// x_posとy_posを更新
			x_pos += x_dir;
			y_pos += y_dir;

			// 左端の場合
			if (x_pos <= 0)
			{
				if ((kypd_scan() != 0x7 && kypd_scan() != 0x4) || point1 > y_pos || y_pos < point1 + 2)
				{ // ボタンが押されなかった場合
					lcd_clear_vbuf();
					lcd_puts(2, 1, "P2's point");
					point2++;
					lcd_sync_vbuf();
					delay_ms(3000); // 3秒間表示
					x_pos = 0;
					y_pos = 3;
				}
				else if (kypd_scan() == 0x7 && point1 <= y_pos <= point1 + 2)
				{
					led_blink();
				}
				else if (kypd_scan() == 0x4 && point1 <= y_pos <= point1 + 2)
				{
					led_blink();
				}
				else if (kypd_scan() == 0x4)
				{
					led_blink();
				}
				x_dir = 1;				  // 右方向に移動
				y_dir = (rand() % 3) - 1; // -1, 0, 1のランダム値
				toggle_speed = !toggle_speed;
				delay = toggle_speed ? max_delay : min_delay;
			}
			// 右端の場合
			else if (x_pos >= 11)
			{
				if ((kypd_scan() != 0xB && kypd_scan() != 0xC) || point2 > y_pos || y_pos < point2 + 2)
				{ // ボタンが押されなかった場合
					lcd_clear_vbuf();
					lcd_puts(2, 1, "P1's point");
					point1++;
					lcd_sync_vbuf();
					delay_ms(3000); // 3秒間表示
					x_pos = 11;
					y_pos = 3;
				}
				else if (kypd_scan() == 0xB && point2 <= y_pos && y_pos <= point2 + 2)
				{
					led_blink();
				}
				else if (kypd_scan() == 0xC && point2 <= y_pos && y_pos <= point2 + 2)
				{
					led_blink();
				}
				x_dir = -1;				  // 左方向に移動
				y_dir = (rand() % 3) - 1; // -1, 0, 1のランダム値
				toggle_speed = !toggle_speed;
				delay = toggle_speed ? max_delay : min_delay;
			}
			// 上端の場合
			else if (y_pos <= 0)
			{
				y_pos = 0; // 上端で固定
				y_dir = 1; // 下方向に移動
			}
			// 下端の場合
			else if (y_pos >= 6)
			{
				y_pos = 6;	// 下端で固定
				y_dir = -1; // 上方向に移動
			}

		if (kypd_scan() == 0x1)
		{
				racket1--;
				show_ball_and_racket(x_pos, y_pos);
		}
		else if (kypd_scan() == 0x0)
		{
				racket1++;
				show_ball_and_racket(x_pos, y_pos);
		}
		
		if (kypd_scan() == 0xa)
		{
				racket2--;
				show_ball_and_racket(x_pos, y_pos);
		}
		else if (kypd_scan() == 0xd)
		{
				racket2++;
				show_ball_and_racket(x_pos, y_pos);
		}

			// ボールの表示
			show_ball_and_racket(x_pos, y_pos);
	} else if (state == PLAY && mode == VS_CPU) {
		delay_cnt = 0;

		// x_posとy_posを更新
		x_pos += x_dir;
		y_pos += y_dir;

		// 左端の場合
		if (x_pos <= 0)
		{
			if ((kypd_scan() != 0x7 && kypd_scan() != 0x4) || point1 > y_pos || y_pos < point1 + 2)
			{ // ボタンが押されなかった場合
				lcd_clear_vbuf();
				lcd_puts(2, 1, "CPU's point");
				point2++;
				lcd_sync_vbuf();
				delay_ms(3000); // 3秒間表示
				x_pos = 0;
				y_pos = 3;
			}
			else if (kypd_scan() == 0x7 && point1 <= y_pos <= point1 + 2)
			{
				led_blink();
			}
			else if (kypd_scan() == 0x4 && point1 <= y_pos <= point1 + 2)
			{
				led_blink();
			}
			x_dir = 1;				  // 右方向に移動
			y_dir = (rand() % 3) - 1; // -1, 0, 1のランダム値
		}
		// 右端の場合
		else if (x_pos >= 11)
		{	
			if (point2 <= y_pos && y_pos <= point2 + 2)
			{
				led_blink();
			}			
			x_dir = -1;				  // 左方向に移動
			y_dir = (rand() % 3) - 1; // -1, 0, 1のランダム値
		}
		// 上端の場合
		else if (y_pos <= 0)
		{
			y_pos = 0; // 上端で固定
			y_dir = 1; // 下方向に移動
		}
		// 下端の場合
		else if (y_pos >= 6)
		{
			y_pos = 6;	// 下端で固定
			y_dir = -1; // 上方向に移動
		}

		if (kypd_scan() == 0x1)
		{
				racket1--;
				show_ball_and_racket(x_pos, y_pos);
		}
		else if (kypd_scan() == 0x0)
		{
				racket1++;
				show_ball_and_racket(x_pos, y_pos);
		}
		
		

			// ボールの表示
		show_ball_and_racket(x_pos, y_pos);
	}
	else if (state == RESULT)
	{
		show_result();
	}
	else if (state == ENDING)
	{
	}
	lcd_sync_vbuf();
}

void show_start()
{
	lcd_clear_vbuf();
	lcd_puts(1, 0, "==========");
	lcd_puts(2, 0, "MatsuTeNI");
	lcd_puts(3, 0, "==========");
	if (mode == VS_MODE)
	{
		lcd_puts(4, 0, "2vsmode  <<");
		lcd_puts(5, 0, "cpumode");
	}
	else if (mode == VS_CPU)
	{
		lcd_puts(4, 0, "2vsmode");
		lcd_puts(5, 0, "cpumode  <<");
	}
	lcd_puts(6, 0, "Go:Push2");
	lcd_sync_vbuf();
}

void show_result()
{
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

void main()
{
	srand(42);
	state = INIT;
	mode = VS_MODE;
	x_pos = 0;
	y_pos = 3;
	point1 = 0;
	point2 = 0;
	seed = 1;
	x_dir = 1;
	y_dir = 0;
	max_delay = 2;
	min_delay = 50;
	delay = 20;
	toggle_speed = 0;
	racket1 = 3;
	racket2 = 3;

	while (1)
	{
		if (state == INIT)
		{
			lcd_init();
			state = OPENING;
		}
		else if (state == OPENING)
		{
			state = MENU;
		}
		else if (state == MENU)
		{
			start();
			state = PLAY;
		}
		else if (state == PLAY)
		{
			play();
			state = RESULT;
		}
		else if (state == RESULT)
		{
			result();
			state = ENDING;
		}
		else if (state == ENDING)
		{
			state = OPENING;
		}
	}
}
void start()
{
	while (1)
	{
		if (btn_check_0())
		{
			mode = VS_MODE;
		}
		else if (btn_check_1())
		{
			mode = VS_CPU;
		}
		else if (btn_check_2())
		{
			state = PLAY;
			break;
		}
	}
}

void result()
{
	while (1)
	{
		if (btn_check_2())
		{
			point1 = 0;
			point2 = 0;
			state = PLAY;
			break;
		}
	}
}

void play()
{
	point1 = 0;
	point2 = 0;
	volatile int *seg7_ptr = (int *)0xff10;

	while (1)
	{
		*seg7_ptr = kypd_scan();
		// /* Button0 is pushed when the ball is in the left edge */
		if (point1 == 5 || point2 == 5)
		{
			state = RESULT;
			break; /* Stop the game */
		}
		if (x_pos == 0 && kypd_scan() == 0x7)
		{
			led_blink(); /* Blink LEDs when hit */
			/* Button3 is pushed when the ball is in the right edge */
		}
		else if (x_pos == 0 && kypd_scan() == 0x4)
		{
			led_blink(); /* Blink LEDs when hit */
		}
		else if (x_pos == 11 && kypd_scan() == 0xB)
		{
			led_blink(); /* Blink LEDs when hit */
		}
		else if (x_pos == 11 && kypd_scan() == 0xC)
		{
			led_blink(); /* Blink LEDs when hit */
		}
		else if (btn_check_1())
		{
			state = RESULT;
			break; /* Stop the game */
		}
	}
}
void show_ball_and_racket(int x, int y)
{
	lcd_clear_vbuf();
	lcd_putc(y, x, '*');
	lcd_putc(7, 0, '0' + point1);
	lcd_putc(7, 11, '0' + point2);
	lcd_putc(racket1, 0, 'O');
	lcd_putc(racket1 + 1, 0, '|');
	lcd_putc(racket1 + 2, 0, '|');
	if (mode == VS_MODE) {
		lcd_putc(racket2, 11, 'O');
		lcd_putc(racket2 + 1, 11, '|');
		lcd_putc(racket2 + 2, 11, '|');
	}
}

/*
 * Switch functions
 */
int btn_check_0()
{
	volatile int *sw_ptr = (int *)0xff04;
	;
	return (*sw_ptr & 0x10) ? 1 : 0;
}
int btn_check_1()
{
	volatile int *sw_ptr = (int *)0xff04;
	;
	return (*sw_ptr & 0x20) ? 1 : 0;
}

int btn_check_2()
{
	volatile int *sw_ptr = (int *)0xff04;
	;
	return (*sw_ptr & 0x40) ? 1 : 0;
}

int btn_check_3()
{
	volatile int *sw_ptr = (int *)0xff04;
	;
	return (*sw_ptr & 0x80) ? 1 : 0;
}
/*
 * LED functions
 */
void led_set(int data)
{
	volatile int *led_ptr = (int *)0xff08;
	*led_ptr = data;
}

void delay_ms(int ms)
{
	for (int i = 0; i < ms * 3000; i++)
		;
}

void led_blink()
{
	led_set(0xf); /* Turn on */
	for (int i = 0; i < 300000; i++)
		;		  /* Wait */
	led_set(0x0); /* Turn off */
	for (int i = 0; i < 300000; i++)
		;		  /* Wait */
	led_set(0xf); /* Turn on */
	for (int i = 0; i < 300000; i++)
		;		  /* Wait */
	led_set(0x0); /* Turn off */
}
/*
 * LCD functions
 */
unsigned char lcd_vbuf[64][96];
void lcd_wait(int n)
{
	for (int i = 0; i < n; i++)
		;
}

void lcd_cmd(unsigned char cmd)
{
	volatile int *lcd_ptr = (int *)0xff0c;
	*lcd_ptr = cmd;
	lcd_wait(1000);
}
void lcd_data(unsigned char data)
{
	volatile int *lcd_ptr = (int *)0xff0c;
	*lcd_ptr = 0x100 | data;
	lcd_wait(200);
}
void lcd_pwr_on()
{
	volatile int *lcd_ptr = (int *)0xff0c;
	*lcd_ptr = 0x200;
	lcd_wait(700000);
}
void lcd_init()
{
	lcd_pwr_on();  /* Display power ON */
	lcd_cmd(0xa0); /* Remap & color depth */
	lcd_cmd(0x20);
	lcd_cmd(0x15); /* Set column address */
	lcd_cmd(0);
	lcd_cmd(95);
	lcd_cmd(0x75); /* Set row address */
	lcd_cmd(0);
	lcd_cmd(63);
	lcd_cmd(0xaf); /* Display ON */
}

unsigned char lcd_vbuf[64][96];
void lcd_set_vbuf_pixel(int row, int col, int r, int g, int b)
{
	r >>= 5;
	g >>= 5;
	b >>= 6;
	lcd_vbuf[row][col] = ((r << 5) | (g << 2) | (b << 0)) & 0xff;
}
void lcd_clear_vbuf()
{
	for (int row = 0; row < 64; row++)
		for (int col = 0; col < 96; col++)
			lcd_vbuf[row][col] = 0;
}

void lcd_sync_vbuf()
{
	for (int row = 0; row < 64; row++)
		for (int col = 0; col < 96; col++)
			lcd_data(lcd_vbuf[row][col]);
}

void lcd_putc(int y, int x, int c)
{
	for (int v = 0; v < 8; v++)
		for (int h = 0; h < 8; h++)
			if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
				lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 0, 255, 0);
}

void lcd_putc_red(int y, int x, int c)
{
	for (int v = 0; v < 8; v++)
		for (int h = 0; h < 8; h++)
			if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
				lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 255, 0, 0);
}

void lcd_putc_blue(int y, int x, int c)
{
	for (int v = 0; v < 8; v++)
		for (int h = 0; h < 8; h++)
			if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
				lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 0, 0, 255);
}

void lcd_puts(int y, int x, char *str)
{
	for (int i = x; i < 12; i++)
		if (str[i] < 0x20 || str[i] > 0x7f)
			break;
		else
			lcd_putc(y, i, str[i]);
}
