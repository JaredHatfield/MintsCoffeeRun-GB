#include "level_01.h"
#include "platformer_level.h"

static const level_rect_t level_01_blocks[] = {
    {84, 108, 8u, 8u}
};

static const platformer_level_t level_01 = {
    "Level 01",
    16,
    112,
    128,
    {144, 120, 8u, 8u},
    level_01_blocks,
    1u
};

static void level_01_enter(void) {
    platformer_enter(&level_01);
}

static level_event_t level_01_update(UINT8 input, UINT8 pressed) {
    return platformer_update(&level_01, input, pressed);
}

const level_definition_t level_01_definition = {
    level_01_enter,
    level_01_update
};
