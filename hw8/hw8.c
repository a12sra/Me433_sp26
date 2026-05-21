#include <stdio.h>
#include "pico/stdlib.h"
#include <math.h>
#include "hardware/spi.h"


//void upate dac (uint8 chnnal, float voltage)
//void update dac from ran (int)
//void spiraminit()
//void spi_ram_write(uint16, uint8*, int)
//void spi_ram_read(uint16, uint8*, int)
//void ram_wrie_sine()

#define SPI_PORT    spi0
#define PIN_SCK     18   
#define PIN_MOSI    19   
#define PIN_MISO    16   
#define PIN_CS_DAC  17   
#define PIN_CS_RAM  20   

#define RAM_CMD_WRITE   0x02   
#define RAM_CMD_READ    0x03   
#define RAM_CMD_WRSR    0x01   
#define RAM_MODE_SEQ    0x40   

#define VREF            3.3f
#define DAC_MAX         1023        
#define DAC_CH_A        0
#define NUM_SAMPLES     1000        

union FloatInt {
    float    f;
    uint32_t i;
};

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

void writeDAC_raw(uint16_t cmd) {
    uint8_t data[2];
    data[0] = (cmd >> 8) & 0xFF;
    data[1] =  cmd       & 0xFF;
    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_DAC);
}

void writeDAC(int channel, float voltage) {
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > VREF) voltage = VREF;
    uint16_t code = (uint16_t)((voltage / VREF) * DAC_MAX);
    if (code > DAC_MAX) code = DAC_MAX;
    uint16_t cmd = 0;
    cmd |= (channel & 0x1) << 15; 
    cmd |= (0 << 14);            
    cmd |= (1 << 13);             
    cmd |= (1 << 12);              
    cmd |= (code & 0x3FF) << 2;    
    writeDAC_raw(cmd);
}

uint16_t voltage_to_dac_cmd(int channel, float voltage) {
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > VREF) voltage = VREF;
    uint16_t code = (uint16_t)((voltage / VREF) * DAC_MAX);
    if (code > DAC_MAX) code = DAC_MAX;
    uint16_t cmd = 0;
    cmd |= (channel & 0x1) << 15;
    cmd |= (1 << 13);  
    cmd |= (1 << 12);   
    cmd |= (code & 0x3FF) << 2;
    return cmd;
}

void spi_ram_init(void) {
    uint8_t buf[2];
    buf[0] = RAM_CMD_WRSR;   
    buf[1] = RAM_MODE_SEQ;  
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, buf, 2);
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_write_buf(uint16_t addr, const uint8_t *data, uint16_t len) {
    uint8_t header[3];
    header[0] = RAM_CMD_WRITE;
    header[1] = (addr >> 8) & 0xFF;  
    header[2] =  addr       & 0xFF;  
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, header, 3);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_read_buf(uint16_t addr, uint8_t *data, uint16_t len) {
    uint8_t header[3];
    header[0] = RAM_CMD_READ;
    header[1] = (addr >> 8) & 0xFF;
    header[2] =  addr       & 0xFF;
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, header, 3);
    spi_read_blocking(SPI_PORT, 0x00, data, len);
    cs_deselect(PIN_CS_RAM);
}

int main(void) {
    stdio_init_all();

    spi_init(SPI_PORT, 20 * 1000 * 1000);   // 20 MHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_DAC);
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);   // deassert

    gpio_init(PIN_CS_RAM);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);   // deassert

    spi_ram_init();

    uint8_t sine_bytes[NUM_SAMPLES * 2];

    for (int i = 0; i < NUM_SAMPLES; i++) {
        float voltage = ((sinf(2.0f * (float)M_PI * i / NUM_SAMPLES) + 1.0f) / 2.0f) * VREF;

        uint16_t cmd = voltage_to_dac_cmd(DAC_CH_A, voltage);

        sine_bytes[2 * i]     = (cmd >> 8) & 0xFF;  // MSB
        sine_bytes[2 * i + 1] =  cmd       & 0xFF;  // LSB
    }

    spi_ram_write_buf(0x0000, sine_bytes, NUM_SAMPLES * 2);

uint8_t header[3];
header[0] = RAM_CMD_READ;
header[1] = 0x00;
header[2] = 0x00;

while (true) {
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, header, 3);
    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint8_t raw[2];
        spi_read_blocking(SPI_PORT, 0x00, raw, 2);
        uint16_t cmd = ((uint16_t)raw[0] << 8) | raw[1];
        writeDAC_raw(cmd);
        sleep_ms(1);
    }
    cs_deselect(PIN_CS_RAM);
}
    return 0;
}