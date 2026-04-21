/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "mpu6050.h"

// ───────── Pins ─────────
#define BUTTON_MODE_PIN 14
#define BUTTON_CLICK_PIN 16
#define LED_PIN 15

// ───────── State ─────────
static bool imu_mode = true;
static bool last_mode_btn = true;
static float t = 0.0f;

// ───────── Mouse report ─────────
static void send_hid_report(void)
{
    if (!tud_hid_ready()) return;

    int8_t dx = 0;
    int8_t dy = 0;
    uint8_t buttons = 0;

    // ───── Read buttons (active LOW) ─────
    bool mode_btn = gpio_get(BUTTON_MODE_PIN);
    bool click_btn = gpio_get(BUTTON_CLICK_PIN);

    // ───── Mode toggle with debounce ─────
    if (!mode_btn && last_mode_btn)
    {
        imu_mode = !imu_mode;
        sleep_ms(150); // simple debounce
    }
    last_mode_btn = mode_btn;

    gpio_put(LED_PIN, imu_mode);

    // ───── Click handling ─────
    if (!click_btn)
        buttons = MOUSE_BUTTON_LEFT;

    // ───── MODE 1: IMU control ─────
    if (imu_mode)
    {
        MPUData d = mpu6050_read(i2c0);

        // X axis → dx (discrete speed levels)
        if (d.ax > 0.5f) dx = 6;
        else if (d.ax > 0.2f) dx = 3;
        else if (d.ax < -0.5f) dx = -6;
        else if (d.ax < -0.2f) dx = -3;

        // Y axis → dy
        if (d.ay > 0.5f) dy = -6;
        else if (d.ay > 0.2f) dy = -3;
        else if (d.ay < -0.5f) dy = 6;
        else if (d.ay < -0.2f) dy = 3;
    }
    // ───── MODE 2: circular motion ─────
    else
    {
        t += 0.05f;
        dx = (int8_t)(3.0f * cosf(t));
        dy = (int8_t)(3.0f * sinf(t));
    }

    tud_hid_mouse_report(REPORT_ID_MOUSE, buttons, dx, dy, 0, 0);
}

// ───────── HID task ─────────
void hid_task(void)
{
    static uint32_t last = 0;

    if (board_millis() - last < 10) return;
    last += 10;

    send_hid_report();
}

// ───────── USB callbacks ─────────
void tud_mount_cb(void) {}
void tud_umount_cb(void) {}
void tud_suspend_cb(bool remote_wakeup_en) {}
void tud_resume_cb(void) {}

// ───────── MAIN ─────────
int main(void)
{
    board_init();
    stdio_init_all();

    tud_init(BOARD_TUD_RHPORT);

    // ───── GPIO setup ─────
    gpio_init(BUTTON_MODE_PIN);
    gpio_set_dir(BUTTON_MODE_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_MODE_PIN);

    gpio_init(BUTTON_CLICK_PIN);
    gpio_set_dir(BUTTON_CLICK_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_CLICK_PIN);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // ───── I2C setup ─────
    i2c_init(i2c0, 400 * 1000);

    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    sleep_ms(100);

    mpu6050_check_whoami(i2c0);
    mpu6050_init(i2c0);

    // ───── Main loop ─────
    while (1)
    {
        tud_task();
        hid_task();
    }
}