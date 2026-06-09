#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define SDA_PIN 8
#define SCL_PIN 9

#define AS5600_ADDR 0x36

#define HX711_DT 2
#define HX711_SCK 3

uint16_t readAngle() {

    uint8_t reg = 0x0C;
    uint8_t buf[2];

    i2c_write_blocking(I2C_PORT,
                       AS5600_ADDR,
                       &reg,
                       1,
                       true);

    i2c_read_blocking(I2C_PORT,
                      AS5600_ADDR,
                      buf,
                      2,
                      false);

    return ((buf[0] & 0x0F) << 8) | buf[1];
}

int32_t hx711_read() {

    while(gpio_get(HX711_DT));

    int32_t value = 0;

    for(int i=0;i<24;i++) {

        gpio_put(HX711_SCK,1);
        sleep_us(1);

        value <<= 1;
        value |= gpio_get(HX711_DT);

        gpio_put(HX711_SCK,0);
        sleep_us(1);
    }

    gpio_put(HX711_SCK,1);
    sleep_us(1);
    gpio_put(HX711_SCK,0);

    if(value & 0x800000)
        value |= 0xFF000000;

    return value;
}

int main() {

    stdio_init_all();

    i2c_init(I2C_PORT,400000);

    gpio_set_function(SDA_PIN,GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN,GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    gpio_init(HX711_DT);
    gpio_set_dir(HX711_DT,GPIO_IN);

    gpio_init(HX711_SCK);
    gpio_set_dir(HX711_SCK,GPIO_OUT);

    sleep_ms(2000);

    while(1){

        uint16_t rawAngle = readAngle();

        float angle =
            rawAngle * 360.0f / 4096.0f;

        int32_t force =
            hx711_read();

        printf("%.2f,%ld\n",
               angle,
               force);

        sleep_ms(20);
    }
}