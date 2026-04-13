#include <stdio.h> // set pico_enable_stdio_usb to 1 in CMakeLists.txt 
#include "pico/stdlib.h" // CMakeLists.txt must have pico_stdlib in target_link_libraries
#include "hardware/pwm.h" // CMakeLists.txt must have hardware_pwm in target_link_libraries
#include "hardware/adc.h" // CMakeLists.txt must have hardware_adc in target_link_libraries

//#define PWMPIN 16

#define SERVO_PIN    16
#define CLK_DIV      150.0f
#define WRAP         20000       // 20ms period at 1MHz tick
#define SERVO_MIN    1000        // 1ms  = 0°
#define SERVO_MAX    2000        // 2ms  = 180°
#define STEP_DELAY_MS 15

void servo_set_angle(uint gpio, float degrees) {
    if (degrees < 0)   degrees = 0;
    if (degrees > 180) degrees = 180;
    uint16_t pulse = (uint16_t)(SERVO_MIN + (degrees / 180.0f) * (SERVO_MAX - SERVO_MIN));
    pwm_set_gpio_level(gpio, pulse);
}
int main() {
    stdio_init_all();

    // Configure GP16 as PWM
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);

    pwm_set_clkdiv(slice, CLK_DIV);   // 150MHz / 150 = 1MHz
    pwm_set_wrap(slice, WRAP);         // 1MHz / 20000 = 50Hz
    pwm_set_enabled(slice, true);

    printf("Servo sweep starting...\n");

    while (true) {
        // Sweep 0°- 180°
        for (float angle = 0; angle <= 180; angle += 1.0f) {
            servo_set_angle(SERVO_PIN, angle);
            sleep_ms(STEP_DELAY_MS);
        }
        // Sweep 180°-0°
        for (float angle = 180; angle >= 0; angle -= 1.0f) {
            servo_set_angle(SERVO_PIN, angle);
            sleep_ms(STEP_DELAY_MS);
        }
    }
}