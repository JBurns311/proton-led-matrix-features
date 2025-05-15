#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/adc.h"
#include "event_queue.h"

int main () {
    stdio_init_all();
    sleep_ms(3000); // give time for the serial monitor to init

    printf("Initializing IO\n");
    init_io();

    printf("Initializing ADC\n");
    adc_init();
    adc_gpio_init(40);
    adc_set_round_robin(0b11);

    printf("Initiatizing Event Handling Timer\n");
    struct repeating_timer event_handling_timer;
    init_event_handling(&event_handling_timer);

    while (true) {
        while (events_queued()) {
            Event e = poll_event();

            if (e.action == PRESS) {
                switch (e.id) {
                    case BUTTON_A: printf("Button A Pressed\n"); break;
                    case BUTTON_B: printf("Button B Pressed\n"); break;
                }
            } else if (e.action == RELEASE) {
                switch (e.id) {
                    case BUTTON_A: printf("Button A Released\n"); break;
                    case BUTTON_B: printf("Button B Released\n"); break;
                }
            }
        }

        printf("ADC = %d\n", adc_read());
        printf("ADC = %d\n", adc_read());
        sleep_ms(500);
    }

    return 0;
}
