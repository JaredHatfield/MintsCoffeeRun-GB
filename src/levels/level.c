#include "level.h"

#include "level_01.h"
#include "level_02.h"

static const level_definition_t *const level_definitions[] = {
    &level_01_definition,
    &level_02_definition
};

const level_definition_t *level_get(UINT8 level_index) {
    if (level_index >= (sizeof(level_definitions) / sizeof(level_definitions[0]))) {
        return 0;
    }

    return level_definitions[level_index];
}

UINT8 level_exists(UINT8 level_index) {
    return level_get(level_index) != 0;
}
