#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stdint.h>
#include "pico/stdlib.h"


typedef enum {
    BUTTON,
    JOYSTICK
} Event_Type;

// pinout/id enum for button based inputs
typedef enum {
    BUTTON_A = 27,
    BUTTON_B = 32
} Button_ID;

typedef enum {
    RELEASE = 0x0,
    PRESS = 0xFF
} Button_Action;

typedef struct {
    Button_ID id;
    uint8_t debounce;
    Button_Action poll_type;
} Button;

typedef struct {
    Button_ID id;
    Button_Action action;
} Button_Event;


typedef enum {
    JOYSTICK_A
} Joystick_ID;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CENTER
} Joystick_Action;

// normalized joystick x and y pos between -1 and 1
typedef struct {
    float x;
    float y;
} Joystick_Pos;

// polling struct
typedef struct {
    Joystick_Pos pos;

    // pin ids
    int x_id;
    int y_id;

    // polling threshold either deadzone or -deadzone
    Joystick_Pos prev_pos;
} Joystick;

typedef struct {
    Joystick_ID id;
    Joystick_Action action;
} Joystick_Event;

typedef struct {
    Event_Type type;
    union {
        Joystick_Event joystick;
        Button_Event button;
    };
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
void enable_joystick_events(bool en, float normalized_deadzone);

#endif
