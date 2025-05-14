#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "event_queue.h"

static Button BUTTONS[BUTTON_CT] = {
    (Button){ .id = BUTTON_A, .debounce = 0x00, .poll_type = PRESS }
};

volatile static Event_Queue queue = (Event_Queue){ .start_idx = 0, .len = 0 };

static void enqueue_event(Event e) {
    assert(queue.len <= EVENT_QUEUE_SIZE);
    int32_t insert_idx = (queue.start_idx + queue.len) % EVENT_QUEUE_SIZE;
    queue.len++;
    queue.array[insert_idx] = e;
}

Event poll_event() {
    assert(queue.len > 0);
    int32_t pop_idx = queue.start_idx;
    queue.start_idx = (queue.start_idx + 1) % EVENT_QUEUE_SIZE;
    queue.len--;
    return queue.array[pop_idx];
}

bool events_queued() {
    return queue.len > 0;
}

void init_io() {
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_function(BUTTON_A, GPIO_FUNC_SIO);
}

void init_event_handling(struct repeating_timer* t) {
    add_repeating_timer_ms(1, event_handling_callback, NULL, t);
}

bool event_handling_callback(__unused struct repeating_timer *t) {
    // printf("BUTTON_A = %d\n", gpio_get(BUTTON_A));
    for (int i = 0; i < BUTTON_CT; i++) {
        Button* b = &BUTTONS[i];
        b->debounce <<= 1;
        b->debounce |= (uint)gpio_get(b->id);


        // printf("debounce = %d, poll type = %d\n", b->debounce, b->poll_type);
        if (b->debounce == (uint8_t)b->poll_type) {
            enqueue_event((Event){ .id = b->id, .action = b->poll_type});
            b->poll_type = b->poll_type == PRESS ? RELEASE : PRESS;

            /*if (b->poll_type == PRESS) {*/
            /*    b->poll_type = RELEASE;*/
            /*    printf("Button Pressed\n");*/
            /*} else if (b->poll_type == RELEASE) {*/
            /*    b->poll_type = PRESS;*/
            /*    printf("Button Released\n");*/
            /*}*/
        }
    }

    return true;
}
