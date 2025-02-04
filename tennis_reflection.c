#include "crt0.c"
#include "ChrFont0.h"

void show_ball(int x, int y);
void play();
int btn_check_0();
int btn_check_1();
int btn_check_3();
void led_set(int data);
void led_blink();
void lcd_init();
void lcd_putc(int y, int x, int c);
void lcd_puts(int y, int x, char *str);
void lcd_sync_vbuf();
void lcd_clear_vbuf();
void delay_ms(int ms);

// 独自のrandとsrand関数を定義
static unsigned int seed = 1;

int rand() {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

void srand(unsigned int s) {
    seed = s;
}

#define INIT    0
#define OPENING 1
#define PLAY    2
#define ENDING  3

int state = INIT, x_pos = 0, y_pos = 3;
int x_dir = 1, y_dir = 0; // 初期方向
int max_delay = 2, min_delay = 50; // 最大・最小の遅延値
int delay = 20; // 現在の遅延値
int toggle_speed = 0; // 遅延切り替えフラグ

/* interrupt_handler() is called every 100msec */
void interrupt_handler() {
    static int cnt;
    static int delay_cnt = 0;

    if (state == INIT) {
    } else if (state == OPENING) {
        cnt = 0;
        delay = min_delay;
    } else if (state == PLAY) {
        if (++delay_cnt >= delay) {
            delay_cnt = 0;

            // x_posとy_posを更新
            x_pos += x_dir;
            y_pos += y_dir;

            // 左端の場合
            if (x_pos <= 0) {
                if (!btn_check_0()) { // ボタンが押されなかった場合
                    lcd_clear_vbuf();
                    lcd_puts(2, 1, "P2's point");
                    lcd_sync_vbuf();
                    delay_ms(3000); // 3秒間表示
                    x_pos = 0;
                    y_pos = 3;
                } else {
                    led_blink();
                }
                x_dir = 1; // 右方向に移動
                y_dir = (rand() % 3) - 1; // -1, 0, 1のランダム値
                toggle_speed = !toggle_speed;
                delay = toggle_speed ? max_delay : min_delay;
            }
            // 右端の場合
            else if (x_pos >= 11) {
                if (!btn_check_3()) { // ボタンが押されなかった場合
                    lcd_clear_vbuf();
                    lcd_puts(2, 1, "P1's point");
                    lcd_sync_vbuf();
                    delay_ms(3000); // 3秒間表示
                    x_pos = 11;
                    y_pos = 3;
                } else {
                    led_blink();
                }
                x_dir = -1; // 左方向に移動
                y_dir = (rand() % 3) - 1; // -1, 0, 1のランダム値
                toggle_speed = !toggle_speed;
                delay = toggle_speed ? max_delay : min_delay;
            }
            // 上端の場合
            else if (y_pos <= 0) {
                y_pos = 0; // 上端で固定
                y_dir = 1; // 下方向に移動
            }
            // 下端の場合
            else if (y_pos >= 6) {
                y_pos = 6; // 下端で固定
                y_dir = -1; // 上方向に移動
            }

            // ボールの表示
            show_ball(x_pos, y_pos);
        }
    } else if (state == ENDING) {
    }
    lcd_sync_vbuf();
}

void main() {
    srand(42); // 初期乱数シードを設定
    while (1) {
        if (state == INIT) {
            lcd_init();
            state = OPENING;
        } else if (state == OPENING) {
            state = PLAY;
        } else if (state == PLAY) {
            play();
            state = ENDING;
        } else if (state == ENDING) {
            state = OPENING;
        }
    }
}

void play() {
    while (1) {
        if (btn_check_1()) { // ボタン1が押されたらゲームを終了
            break;
        }
    }
}

void show_ball(int x, int y) {
    lcd_clear_vbuf();
    lcd_putc(y, x, '*');
}

/* Switch functions */
int btn_check_0() {
    volatile int *sw_ptr = (int *)0xff04;;
    return (*sw_ptr & 0x10) ? 1 : 0;
}

int btn_check_1() {
    volatile int *sw_ptr = (int *)0xff04;;
    return (*sw_ptr & 0x20) ? 1 : 0;
}

int btn_check_3() {
    volatile int *sw_ptr = (int *)0xff04;;
    return (*sw_ptr & 0x80) ? 1 : 0;
}

/* LED functions */
void led_set(int data) {
    volatile int *led_ptr = (int *)0xff08;
    *led_ptr = data;
}

void led_blink() {
    led_set(0xf);
    for (int i = 0; i < 300000; i++);
    led_set(0x0);
    for (int i = 0; i < 300000; i++);
    led_set(0xf);
    for (int i = 0; i < 300000; i++);
    led_set(0x0);
}

/* Delay function */
void delay_ms(int ms) {
    for (int i = 0; i < ms * 3000; i++);
}

/* LCD functions */
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
    lcd_pwr_on();
    lcd_cmd(0xa0);
    lcd_cmd(0x20);
    lcd_cmd(0x15);
    lcd_cmd(0);
    lcd_cmd(95);
    lcd_cmd(0x75);
    lcd_cmd(0);
    lcd_cmd(63);
    lcd_cmd(0xaf);
}

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

void lcd_puts(int y, int x, char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        lcd_putc(y, x + i, str[i]);
    }
}