#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "mpu6050.h"

// oled
#define OLED_W      128
#define OLED_H      32
#define CX          64      
#define CY          16      
#define LINE_SCALE  24      


// line draw
static void draw_line(int x0, int y0, int x1, int y1) {
    int dx =  abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (1) {
        ssd1306_drawPixel(x0, y0, 1);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// tilt
static void draw_tilt(float ax, float ay) {
    ssd1306_clear();

    ssd1306_drawPixel(CX,     CY,     1);
    ssd1306_drawPixel(CX + 1, CY,     1);
    ssd1306_drawPixel(CX - 1, CY,     1);
    ssd1306_drawPixel(CX,     CY + 1, 1);
    ssd1306_drawPixel(CX,     CY - 1, 1);

    // endpoint scaled and clamped
    int ex = CX + (int)(ax * LINE_SCALE);
    int ey = CY + (int)(ay * LINE_SCALE);
    if (ex < 1)        ex = 1;
    if (ex > OLED_W-2) ex = OLED_W - 2;
    if (ey < 1)        ey = 1;
    if (ey > OLED_H-2) ey = OLED_H - 2;

    // line from centre to endpoint
    draw_line(CX, CY, ex, ey);

    // dot at tip
    ssd1306_drawPixel(ex,     ey,     1);
    ssd1306_drawPixel(ex + 1, ey,     1);
    ssd1306_drawPixel(ex - 1, ey,     1);
    ssd1306_drawPixel(ex,     ey + 1, 1);
    ssd1306_drawPixel(ex,     ey - 1, 1);

    ssd1306_update();
}

// i2c sncaer
static void i2c_scan(void) {
    printf("Scanning I2C0...\n");
    for (int addr = 0; addr < 128; addr++) {
        uint8_t buf;
        int ret = i2c_read_blocking(i2c0, addr, &buf, 1, false);
        if (ret >= 0) {
            printf("  Found device at 0x%02X\n", addr);
        }
    }
    printf("Scan done.\n");
}

// main
int main(void) {
    stdio_init_all();
    sleep_ms(2000);  

    // both share these gp4 gp5 
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    sleep_ms(100);  



    ssd1306_setup();

    mpu6050_check_whoami(i2c0);

    mpu6050_init(i2c0);


    printf("Starting loop...\n");

    const uint32_t PERIOD_US = 10000;  // 10ms = 100Hz

    while (true) {
        absolute_time_t t_start = get_absolute_time();

        MPUData d = mpu6050_read(i2c0);

        draw_tilt(d.ax, d.ay);

        int32_t remaining = (int32_t)PERIOD_US -
                            (int32_t)absolute_time_diff_us(t_start,
                                                           get_absolute_time());
        if (remaining > 0) sleep_us(remaining);
    }
}