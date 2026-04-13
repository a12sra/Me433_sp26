#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// mcp I2C address 
#define MCP_ADDR    0x20

// mcp registers
#define IODIR       0x00   // 1=input, 0=output
#define GPIO_REG    0x09   // read pin states
#define OLAT        0x0A   // write pin states

// pico I2C pins
#define SDA_PIN     4      // GP4 pin 6
#define SCL_PIN     5      // GP5 pin 7

#define HEARTBEAT_PIN 15   

void mcp_write(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(i2c_default, MCP_ADDR, buf, 2, false);
}

uint8_t mcp_read(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(i2c_default, MCP_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, MCP_ADDR, &value, 1, false);
    return value;
}

// init mcp
void mcp_init() {
    // GP0=input(1), GP7=output(0), rest=input(1)
    // bit 7=GP7, bit 0=GP0
    // 0b01111111 = 0x7F  ;GP7 output, all others input
    mcp_write(IODIR, 0x7F);

    // start with GP7 LED off
    mcp_write(OLAT, 0x00);
}

int main() {
    stdio_init_all();

    // init I2C at 100kHz
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // heartbeat LED on Pico
    gpio_init(HEARTBEAT_PIN);
    gpio_set_dir(HEARTBEAT_PIN, GPIO_OUT);

    mcp_init();

    printf("MCP23008 init done\n");

// //b;ink led
//     while (true) {
//         // heartbeat
//         gpio_put(HEARTBEAT_PIN, 1);
//         sleep_ms(500);
//         gpio_put(HEARTBEAT_PIN, 0);
//         sleep_ms(500);

//         // blink MCP GP7 LED
//         mcp_write(OLAT, 0x80);   // GP7 high = LED on  (bit 7 set)
//         sleep_ms(500);
//         mcp_write(OLAT, 0x00);   // GP7 low  = LED off
//         sleep_ms(500);

//         printf("blinking...\n");
//     }
// }

//pt2
while (true) {
        gpio_put(HEARTBEAT_PIN, 1);
        sleep_ms(50);
        gpio_put(HEARTBEAT_PIN, 0);
        sleep_ms(50);

        // read GPIO 
        uint8_t pins = mcp_read(GPIO_REG);

        // GP0 is bit 0 button pressed = 0 pulled low
      
        uint8_t button = pins & 0x01;

        if (button == 0) {
            // button pressed turn LED on set bit 7 of OLAT
            mcp_write(OLAT, 0x80);
            printf("button pressed - LED on\n");
        } else {
            // button not pressed  turn LED off
            mcp_write(OLAT, 0x00);
            printf("button released - LED off\n");
        }

        sleep_ms(20);   // small debounce delay
    }
}