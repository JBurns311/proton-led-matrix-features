#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stdint.h>
#include "pico/stdlib.h"

#define BUTTON_CT 1
#define EVENT_QUEUE_SIZE BUTTON_CT * 4

// pinout enum for button based inputs
typedef enum {
    BUTTON_A = 27
} Input;

typedef enum {
    RELEASE = 0x0,
    PRESS = 0xFF
} Event_Action;

typedef struct {
    Input id;
    uint8_t debounce;
    Event_Action poll_type;
} Button;

typedef struct {
    Input id;
    Event_Action action;
} Event;

typedef struct {
    Event array[EVENT_QUEUE_SIZE];
    int32_t start_idx;
    uint32_t len;
} Event_Queue;

void init_io();
void init_event_handling(struct repeating_timer* t);
bool event_handling_callback(__unused struct repeating_timer *t);
Event poll_event();
bool events_queued();

#endif
