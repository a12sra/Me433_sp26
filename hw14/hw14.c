#include <stdio.h>
#include "pico/stdlib.h"

#define DT_PIN  14
#define SCK_PIN 15

void hx711_init() {
    gpio_init(SCK_PIN);
    gpio_init(DT_PIN);
    gpio_set_dir(SCK_PIN, GPIO_OUT);
    gpio_set_dir(DT_PIN,  GPIO_IN);
    gpio_put(SCK_PIN, 0);
}

int32_t hx711_read() {
    while (gpio_get(DT_PIN) == 1) {
        tight_loop_contents();
    }

    uint32_t raw = 0;

    for (int i = 0; i < 24; i++) {
        gpio_put(SCK_PIN, 1);
        sleep_us(1);
        raw = (raw << 1) | gpio_get(DT_PIN);
        gpio_put(SCK_PIN, 0);
        sleep_us(1);
    }

    gpio_put(SCK_PIN, 1);
    sleep_us(1);
    gpio_put(SCK_PIN, 0);
    sleep_us(1);

    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }

    return (int32_t)raw;
}

int main() {
    stdio_init_all();
    hx711_init();

    float A = 0.9f;
    float B = 1.0f - A;

    while (1) {
        int n = 0;
        scanf("%d", &n);

        float iir = 0.0f;
        int first = 1;

        for (int i = 0; i < n; i++) {
            uint32_t t_ms = to_ms_since_boot(get_absolute_time());
            int32_t  raw  = hx711_read();

            if (first) {
                iir   = (float)raw;
                first = 0;
            } else {
                iir = A * iir + B * (float)raw;
            }

            printf("%lu,%d,%.1f\n", t_ms, raw, iir);
        }
    }

    return 0;
}