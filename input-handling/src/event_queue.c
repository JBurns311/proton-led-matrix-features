#include <assert.h>
#include <stdio.h>
#include "event_queue.h"
#include "hardware/gpio.h"

#define BUTTON_CT 2
#define JOYSTICK_CT 1
// queue buffer size much larger than distinct events to prevent buffer overflow in low update rate applications
#define EVENT_QUEUE_SIZE (BUTTON_CT * 16)

static Button BUTTONS[BUTTON_CT] = {
    (Button){ .id = BUTTON_A, .debounce = 0x00, .poll_type = PRESS },
    (Button){ .id = BUTTON_B, .debounce = 0x00, .poll_type = PRESS }
};

static bool joystick_events_en = false;
static float joystick_deadzone = 0.3;

static Joystick JOYSTICKS[JOYSTICK_CT] = {
    (Joystick){ .x_id = 40, .y_id = 41, .prev_pos.x = 0, .prev_pos.y = 0 }
};

// global input event queue
volatile static Event_Queue queue = (Event_Queue){ .start_idx = 0, .len = 0 };

static void enqueue_event(Event e) {
    assert(queue.len < EVENT_QUEUE_SIZE);
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

    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_set_function(BUTTON_B, GPIO_FUNC_SIO);
}

void init_event_handling(struct repeating_timer* t) {
    add_repeating_timer_ms(1, event_handling_callback, NULL, t);
}

bool event_handling_callback(__unused struct repeating_timer *t) {
    for (int i = 0; i < BUTTON_CT; i++) {
        Button* b = &BUTTONS[i];
        b->debounce <<= 1;
        b->debounce |= (uint)gpio_get(b->id);


        if (b->debounce == (uint8_t)b->poll_type) {
            enqueue_event((Event){ .type = BUTTON, .button.id = b->id, .button.action = b->poll_type});
            b->poll_type = b->poll_type == PRESS ? RELEASE : PRESS;
        }
    }

    if (joystick_events_en) {
        for (int i = 0; i < JOYSTICK_CT; i++) {
            Joystick* j = &JOYSTICKS[i];
            
        }
    }

    return true;
}
