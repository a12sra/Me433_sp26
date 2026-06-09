gpio_init(10);
gpio_set_dir(10, GPIO_OUT);

gpio_init(11);
gpio_set_dir(11, GPIO_OUT);

while (1) {
    gpio_put(10, 1);
    gpio_put(11, 0);
    sleep_ms(2000);

    gpio_put(10, 0);
    gpio_put(11, 0);
    sleep_ms(1000);
}