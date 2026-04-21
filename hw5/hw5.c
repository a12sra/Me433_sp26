#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// mpu6050
#define MPU_ADDR        0x68
#define GYRO_CONFIG     0x1B
#define ACCEL_CONFIG    0x1C
#define PWR_MGMT_1      0x6B
#define ACCEL_XOUT_H    0x3B
#define WHO_AM_I        0x75

#define ACCEL_SCALE     0.000061f
#define GYRO_SCALE      0.007630f

// oled
#define OLED_W      128
#define OLED_H      32
#define CX          64      
#define CY          16      
#define LINE_SCALE  24      

// i2c
static void mpu_write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(i2c0, MPU_ADDR, buf, 2, false);
}

static void mpu_read_regs(uint8_t reg, uint8_t *buf, size_t len) {
    i2c_write_blocking(i2c0, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking (i2c0, MPU_ADDR, buf, len, false);
}

static int16_t combine(uint8_t hi, uint8_t lo) {
    return (int16_t)((uint16_t)hi << 8 | lo);
}

// who am i check
static void mpu_check_whoami(void) {
    uint8_t val;
    mpu_read_regs(WHO_AM_I, &val, 1);
    printf("WHO_AM_I = 0x%02X\n", val);
    if (val != 0x68 && val != 0x98) {
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        printf("MPU6050 not found! Check wiring.\n");
        while (true) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1); sleep_ms(200);
            gpio_put(PICO_DEFAULT_LED_PIN, 0); sleep_ms(200);
        }
    }
    printf("MPU6050 found!\n");
}

// mpu6050 init 
static void mpu_init(void) {
    mpu_write_reg(PWR_MGMT_1,   0x00);  
    sleep_ms(10);
    mpu_write_reg(ACCEL_CONFIG, 0x00);  
    mpu_write_reg(GYRO_CONFIG,  0x18);  
}

// sensor
typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float temp_c;
} MPUData;

//read
static MPUData mpu_read(void) {
    uint8_t raw[14];
    mpu_read_regs(ACCEL_XOUT_H, raw, 14);
    MPUData d;
    d.ax     = combine(raw[0],  raw[1])  * ACCEL_SCALE;
    d.ay     = combine(raw[2],  raw[3])  * ACCEL_SCALE;
    d.az     = combine(raw[4],  raw[5])  * ACCEL_SCALE;
    d.temp_c = combine(raw[6],  raw[7])  / 340.0f + 36.53f;
    d.gx     = combine(raw[8],  raw[9])  * GYRO_SCALE;
    d.gy     = combine(raw[10], raw[11]) * GYRO_SCALE;
    d.gz     = combine(raw[12], raw[13]) * GYRO_SCALE;
    return d;
}

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

    i2c_scan();

    ssd1306_setup();

    mpu_check_whoami();

    mpu_init();

    printf("Starting loop...\n");

    const uint32_t PERIOD_US = 10000;  // 10ms = 100Hz

    while (true) {
        absolute_time_t t_start = get_absolute_time();

        MPUData d = mpu_read();

        draw_tilt(d.ax, d.ay);

        int32_t remaining = (int32_t)PERIOD_US -
                            (int32_t)absolute_time_diff_us(t_start,
                                                           get_absolute_time());
        if (remaining > 0) sleep_us(remaining);
    }
}