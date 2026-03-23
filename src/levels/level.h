#ifndef LEVEL_H
#define LEVEL_H

#include <gb/gb.h>

typedef enum level_event_t {
    LEVEL_EVENT_NONE = 0,
    LEVEL_EVENT_COMPLETE
} level_event_t;

typedef struct level_definition_t {
    void (*enter)(void);
    level_event_t (*update)(UINT8 input, UINT8 pressed);
} level_definition_t;

const level_definition_t *level_get(UINT8 level_index);
UINT8 level_exists(UINT8 level_index);

#endif
