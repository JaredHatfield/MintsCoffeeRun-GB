#include "level_02.h"
#include "platformer_level.h"

static const platformer_level_t level_02 = {
    "Level 02",
    16,
    112,
    128,
    {144, 120, 8u, 8u},
    0,
    0u
};

static void level_02_enter(void) {
    platformer_enter(&level_02);
}

static level_event_t level_02_update(UINT8 input, UINT8 pressed) {
    return platformer_update(&level_02, input, pressed);
}

const level_definition_t level_02_definition = {
    level_02_enter,
    level_02_update
};
