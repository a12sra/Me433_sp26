#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"

#define SDA_PIN      4
#define SCL_PIN      5
#define HEARTBEAT   15   

// 1 char at pos xy
void drawChar(int x, int y, char c) {
    if (c < 0x20 || c > 0x7F) c = 0x20;  
    int index = c - 0x20;

    for (int col = 0; col < 5; col++) {
        unsigned char column_bits = ASCII[index][col];
        for (int row = 0; row < 8; row++) {
            if (column_bits & (1 << row)) {
                ssd1306_drawPixel(x + col, y + row, 1);
            } else {
                ssd1306_drawPixel(x + col, y + row, 0);
            }
        }
    }
}

// null-terminated string at xy
// 6px per character
void drawMessage(int x, int y, char *msg) {
    int cursor = x;
    while (*msg != '\0') {
        drawChar(cursor, y, *msg);
        cursor += 6;   
        msg++;
    }
}

int main() {
    stdio_init_all();

    // i2c init
    i2c_init(i2c_default, 400 * 1000);  
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // heartbeat LED 
    gpio_init(HEARTBEAT);
    gpio_set_dir(HEARTBEAT, GPIO_OUT);

    // ADC init GP26 = ADC0
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    // OLED init 
    ssd1306_setup();

    // // PART 1 
    // bool pixel_on = false;
    // while (true) {
    //     gpio_put(HEARTBEAT, pixel_on);
    //     ssd1306_clear();
    //     ssd1306_drawPixel(64, 16, pixel_on ? 1 : 0);
    //     ssd1306_update();
    //     pixel_on = !pixel_on;
    //     sleep_ms(500);
    // }

    // PART 2 & 3

    // ssd1306_clear();
    // // fill the screen: 25 chars across 4 rows dow;n each row 8px
    // char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXY";
    // drawMessage(0,  0, alphabet);
    // drawMessage(0,  8, "abcdefghijklmnopqrstuvwxy");
    // drawMessage(0, 16, "0123456789 !@#$%^&*()-+=");
    // drawMessage(0, 24, "Hello World from Pico 2! ");
    // ssd1306_update();
    // while(true) { tight_loop_contents(); }

    // PART 4 — ADC voltage + FPS on display

    unsigned int frame_time_us;
    unsigned int fps;
    char msg[30];

    while (true) {
        // heartbeat
        gpio_put(HEARTBEAT, 1);
        sleep_ms(10);
        gpio_put(HEARTBEAT, 0);

        // time the frame
        unsigned int t_start = to_us_since_boot(get_absolute_time());

        // read ADC
        uint16_t raw = adc_read();
        float voltage = (float)raw / 4095.0f * 3.3f;

        // build display
        ssd1306_clear();

        // row 0: title
        drawMessage(0, 0, "ADC Voltage:");

        // row 1: voltage value
        sprintf(msg, "%.3f V", voltage);
        drawMessage(0, 8, msg);

        // row 2: raw ADC value
        sprintf(msg, "Raw: %d", raw);
        drawMessage(0, 16, msg);

        // push to display
        ssd1306_update();

        // calculate FPS
        unsigned int t_end = to_us_since_boot(get_absolute_time());
        frame_time_us = t_end - t_start;
        fps = 1000000 / frame_time_us;

        // row 3: FPS (bottom row)
        ssd1306_clear();

        drawMessage(0, 0,  "ADC Voltage:");
        sprintf(msg, "%.3f V", voltage);
        drawMessage(0, 8,  msg);
        sprintf(msg, "Raw: %d", raw);
        drawMessage(0, 16, msg);
        sprintf(msg, "FPS: %d", fps);
        drawMessage(0, 24, msg);   // bottom row y=24

        ssd1306_update();

        printf("Voltage: %.3f V | FPS: %d\n", voltage, fps);
    }
}