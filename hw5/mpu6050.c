#include "mpu6050.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <stdint.h>
#include <stddef.h>

// registers
#define MPU_ADDR        0x68
#define PWR_MGMT_1      0x6B
#define ACCEL_CONFIG    0x1C
#define GYRO_CONFIG     0x1B
#define ACCEL_XOUT_H    0x3B
#define WHO_AM_I        0x75

#define ACCEL_SCALE     0.000061f
#define GYRO_SCALE      0.007630f

static void write_reg(i2c_inst_t *i2c, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(i2c, MPU_ADDR, buf, 2, false);
}

static void read_regs(i2c_inst_t *i2c, uint8_t reg, uint8_t *buf, size_t len) {
    i2c_write_blocking(i2c, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c, MPU_ADDR, buf, len, false);
}

static int16_t combine(uint8_t hi, uint8_t lo) {
    return (int16_t)((hi << 8) | lo);
}

// public functions

void mpu6050_check_whoami(i2c_inst_t *i2c) {
    uint8_t val;
    read_regs(i2c, WHO_AM_I, &val, 1);
    printf("WHO_AM_I = 0x%02X\n", val);

    if (val != 0x68 && val != 0x98) {
        printf("MPU6050 not found!\n");
        while (1); // hang
    }
}

void mpu6050_init(i2c_inst_t *i2c) {
    write_reg(i2c, PWR_MGMT_1, 0x00);
    sleep_ms(10);
    write_reg(i2c, ACCEL_CONFIG, 0x00);
    write_reg(i2c, GYRO_CONFIG, 0x18);
}

MPUData mpu6050_read(i2c_inst_t *i2c) {
    uint8_t raw[14];
    read_regs(i2c, ACCEL_XOUT_H, raw, 14);

    MPUData d;

    d.ax = combine(raw[0], raw[1]) * ACCEL_SCALE;
    d.ay = combine(raw[2], raw[3]) * ACCEL_SCALE;
    d.az = combine(raw[4], raw[5]) * ACCEL_SCALE;

    d.temp_c = combine(raw[6], raw[7]) / 340.0f + 36.53f;

    d.gx = combine(raw[8], raw[9]) * GYRO_SCALE;
    d.gy = combine(raw[10], raw[11]) * GYRO_SCALE;
    d.gz = combine(raw[12], raw[13]) * GYRO_SCALE;

    return d;
}