#ifndef PLATFORMER_LEVEL_H
#define PLATFORMER_LEVEL_H

#include <gb/gb.h>

#include "level.h"

typedef struct level_rect_t {
    INT16 x;
    INT16 y;
    UINT8 width;
    UINT8 height;
} level_rect_t;

typedef struct platformer_level_t {
    const char *title;
    INT16 start_x;
    INT16 start_y;
    INT16 ground_y;
    level_rect_t goal;
    const level_rect_t *blocks;
    UINT8 block_count;
} platformer_level_t;

void platformer_enter(const platformer_level_t *level);
level_event_t platformer_update(const platformer_level_t *level, UINT8 input, UINT8 pressed);

#endif
