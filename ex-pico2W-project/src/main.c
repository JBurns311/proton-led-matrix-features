#include <hardware/gpio.h>
#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN 15

int main() {
    stdio_init_all();
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_function(LED_PIN, GPIO_FUNC_SIO);

    sleep_ms(5000);
    printf("Heyooo!!\n");

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(1000);
        gpio_put(LED_PIN, 0);
        sleep_ms(1000);
    }

    return 0;
}
