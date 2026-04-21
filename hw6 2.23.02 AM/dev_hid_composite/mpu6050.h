
#ifndef MPU6050_H
#define MPU6050_H
#include <stdint.h>
#include <stddef.h>
#include "hardware/i2c.h"

// struct for sensor data
typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float temp_c;
} MPUData;

// public functions
void mpu6050_init(i2c_inst_t *i2c);
void mpu6050_check_whoami(i2c_inst_t *i2c);
MPUData mpu6050_read(i2c_inst_t *i2c);

#endif