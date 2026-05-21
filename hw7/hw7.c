#include <stdio.h>
#include "pico/stdlib.h"
#include <math.h>
#include "hardware/spi.h"

/*
//pseudo code
//add the hardware spi in cmake list
//turen on spi pins:
spi_init(spi_default, 1000 * 1000*20); // the baud, or bits per second
gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
//switch to the pins using irl
//choose a chip select pin
cs_select(PIN_CS);
spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
cs_deselect(PIN_CS);
//precalculate or calculate onthe fly
float_v[100]
for i=0,i<1000, i++
v[i] = sine[i]
OR
while true:{
//call writedac
float t = 0
t=t+0.01
float voltage  = (sine(2*pi*f*t)+1)/2)*3*3 //make sin wave
writeDAC(channel,voltage)
sleep_ms(10)
}

void (writeDAC iint, channel, float v)

unit8_t data[2]
data[0] should look like like 0b01110000;
data[0]=data[0] | (chnnel&0b1)<<7 shift this way by 7) //put the channel bit in
uint16_t myV = v/3.3*10223  //0b11111111
data[0]=data[0] | myV (want first four bits so shift by 6 (myV>>6)&0b00001111)
data[1] = (myV<<2)&0xFF //0b11111100


data[1] = 0b11111100


cs_select(PIN_CS);
spi_write_blocking(SPI_PORT, data, 2); // where data is a uint8_t array with length len
cs_deselect(PIN_CS);
*/

#define SPI_PORT    spi0
#define PIN_SCK     18   
#define PIN_MOSI    19   
#define PIN_MISO    16   
#define PIN_CS      17   

#define SAMPLE_RATE_HZ   100          // dac update rate
#define SLEEP_US         (1000000 / SAMPLE_RATE_HZ)  // 10 000 ms

#define F_SINE           2.0f         
#define F_TRI            1.0f        
#define VREF             3.3f
#define DAC_MAX          1023        

#define DAC_CH_A         0            // channel A: bit15 = 0
#define DAC_CH_B         1            // channel B: bit15 = 1

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void writeDAC(int channel, float voltage) {
    if (voltage < 0.0f)    voltage = 0.0f;
    if (voltage > VREF)    voltage = VREF;

    uint16_t code = (uint16_t)((voltage / VREF) * DAC_MAX);
    if (code > DAC_MAX) code = DAC_MAX;

    uint16_t cmd = 0;
    cmd |= (channel & 0x1) << 15;   
    cmd |= (0 << 14);               
    cmd |= (1 << 13);               
    cmd |= (1 << 12);               
    cmd |= (code & 0x3FF) << 2;    

    uint8_t data[2];
    data[0] = (cmd >> 8) & 0xFF;
    data[1] = cmd & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

// main 
int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 20 * 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    float t = 0.0f;
    const float dt = 1.0f / SAMPLE_RATE_HZ;  

    while (true) {
        float v_sine = ((sinf(2.0f * M_PI * F_SINE * t) + 1.0f) / 2.0f) * VREF;

        float phase = fmodf(F_TRI * t, 1.0f);        
        float tri;
        if (phase < 0.5f)
            tri = phase * 2.0f;         
        else
            tri = (1.0f - phase) * 2.0f;
        float v_tri = tri * VREF;

        writeDAC(DAC_CH_A, v_sine);
        writeDAC(DAC_CH_B, v_tri);

        t += dt;
        if (t > 100.0f) t = 0.0f;

        sleep_us(SLEEP_US);
    }

    return 0;
}