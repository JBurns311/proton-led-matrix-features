#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "event_queue.h"

int main () {
    stdio_init_all();
    sleep_ms(5000); // give time for the serial monitor to init

    printf("Initializing IO\n");
    init_io();

    printf("Initiatizing Event Handling Timer\n");
    struct repeating_timer event_handling_timer;
    init_event_handling(&event_handling_timer);

    while (true) {
        while (events_queued()) {
            Event e = poll_event();

            if (e.action == PRESS) {
                switch (e.id) {
                    case BUTTON_A: printf("Button A Pressed\n"); break;
                }
            } else if (e.action == RELEASE) {
                switch (e.id) {
                    case BUTTON_A: printf("Button A Released\n"); break;
                }
            }
        }
    }

    return 0;
}
