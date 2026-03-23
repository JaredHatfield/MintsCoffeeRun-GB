#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>

#include "../sprites/goal_sprite.h"
#include "../sprites/mint_sprite.h"
#include "platformer_level.h"

#define LEVEL_BG_TILE_BASE 160u
#define LEVEL_BG_TILE_COUNT 2u
#define LEVEL_BLANK_TILE 160u
#define LEVEL_GROUND_TILE 161u
#define LEVEL_GROUND_ROW 16u
#define PLAYER_MIN_X 0
#define PLAYER_MAX_X 144
#define BLOCK_SPRITE_TILE (MINT_SPRITE_TILE_COUNT + GOAL_SPRITE_TILE_COUNT)
#define GOAL_SPRITE_INDEX 4u
#define BLOCK_SPRITE_FIRST_INDEX 5u
#define MAX_BLOCK_SPRITES 8u
#define SUBPIXELS_PER_PIXEL 16
#define MOVE_ACCEL_GROUND 6
#define MOVE_ACCEL_AIR 3
#define MOVE_FRICTION_GROUND 5
#define MOVE_FRICTION_AIR 1
#define MAX_RUN_SPEED 20
#define JUMP_VELOCITY -40
#define DOUBLE_JUMP_VELOCITY -40
#define GRAVITY_ASCEND 3
#define GRAVITY_DESCEND 5
#define MAX_FALL_SPEED 56
#define COYOTE_FRAMES 4u
#define JUMP_BUFFER_FRAMES 4u

static INT16 player_x;
static INT16 player_y;
static INT16 velocity_x;
static INT16 velocity_y;
static UINT8 jump_count;
static UINT8 coyote_timer;
static UINT8 jump_buffer_timer;

static const unsigned char level_bg_tiles[LEVEL_BG_TILE_COUNT * 16u] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0xFF, 0xFF, 0xFF, 0xFF, 0xDB, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xDB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const unsigned char block_sprite_tile[16] = {
    0xFF, 0xFF, 0x81, 0xFF, 0xBD, 0xFF, 0xA5, 0xFF,
    0xA5, 0xFF, 0xBD, 0xFF, 0x81, 0xFF, 0xFF, 0xFF
};

static const unsigned char ground_row_tiles[20] = {
    LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE,
    LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE,
    LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE,
    LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE,
    LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE, LEVEL_GROUND_TILE
};

static INT16 level_player_left(void) {
    return player_x / SUBPIXELS_PER_PIXEL;
}

static INT16 level_player_top(void) {
    return player_y / SUBPIXELS_PER_PIXEL;
}

static UINT8 ranges_overlap(INT16 a_start, UINT8 a_size, INT16 b_start, UINT8 b_size) {
    return (a_start < (INT16)(b_start + b_size)) && ((INT16)(a_start + a_size) > b_start);
}

static UINT8 rects_overlap(INT16 ax, INT16 ay, UINT8 aw, UINT8 ah, INT16 bx, INT16 by, UINT8 bw, UINT8 bh) {
    return ranges_overlap(ax, aw, bx, bw) && ranges_overlap(ay, ah, by, bh);
}

static UINT8 level_is_grounded(const platformer_level_t *level) {
    UINT8 block_index;
    INT16 player_left = level_player_left();
    INT16 player_bottom = level_player_top() + MINT_SPRITE_HEIGHT;

    if (player_bottom == level->ground_y) {
        return 1u;
    }

    for (block_index = 0u; block_index != level->block_count; ++block_index) {
        const level_rect_t *block = &level->blocks[block_index];

        if (player_bottom == block->y &&
            ranges_overlap(player_left, MINT_SPRITE_WIDTH, block->x, block->width)) {
            return 1u;
        }
    }

    return 0u;
}

static void draw_level_background(void) {
    set_bkg_data(LEVEL_BG_TILE_BASE, LEVEL_BG_TILE_COUNT, level_bg_tiles);
    set_bkg_tiles(0u, LEVEL_GROUND_ROW, 20u, 1u, ground_row_tiles);
    set_bkg_tiles(0u, LEVEL_GROUND_ROW + 1u, 20u, 1u, ground_row_tiles);
}

static void draw_level_entities(const platformer_level_t *level) {
    UINT8 block_index;

    mint_sprite_show(0u, 0u, 0u,
                     (UINT8)(level_player_left() + 8),
                     (UINT8)(level_player_top() + 16));
    goal_sprite_show(MINT_SPRITE_TILE_COUNT, GOAL_SPRITE_INDEX, 0u,
                     (UINT8)(level->goal.x + 8),
                     (UINT8)(level->goal.y + 16));

    for (block_index = 0u; block_index != level->block_count && block_index < MAX_BLOCK_SPRITES; ++block_index) {
        set_sprite_tile((UINT8)(BLOCK_SPRITE_FIRST_INDEX + block_index), BLOCK_SPRITE_TILE);
        move_sprite((UINT8)(BLOCK_SPRITE_FIRST_INDEX + block_index),
                    (UINT8)(level->blocks[block_index].x + 8),
                    (UINT8)(level->blocks[block_index].y + 16));
    }

    while (block_index < MAX_BLOCK_SPRITES) {
        move_sprite((UINT8)(BLOCK_SPRITE_FIRST_INDEX + block_index), 0u, 0u);
        ++block_index;
    }
}

static void apply_horizontal_input(UINT8 input, UINT8 grounded) {
    INT8 direction = 0;
    INT16 acceleration = grounded != 0u ? MOVE_ACCEL_GROUND : MOVE_ACCEL_AIR;
    INT16 friction = grounded != 0u ? MOVE_FRICTION_GROUND : MOVE_FRICTION_AIR;

    if ((input & J_LEFT) != 0u) {
        direction = -1;
    }
    if ((input & J_RIGHT) != 0u) {
        direction = 1;
    }

    if (direction < 0) {
        velocity_x -= acceleration;
        if (velocity_x < -MAX_RUN_SPEED) {
            velocity_x = -MAX_RUN_SPEED;
        }
    } else if (direction > 0) {
        velocity_x += acceleration;
        if (velocity_x > MAX_RUN_SPEED) {
            velocity_x = MAX_RUN_SPEED;
        }
    } else if (velocity_x > 0) {
        velocity_x -= friction;
        if (velocity_x < 0) {
            velocity_x = 0;
        }
    } else if (velocity_x < 0) {
        velocity_x += friction;
        if (velocity_x > 0) {
            velocity_x = 0;
        }
    }
}

static void resolve_horizontal_movement(const platformer_level_t *level) {
    UINT8 block_index;
    INT16 next_x = player_x + velocity_x;
    INT16 next_left = next_x / SUBPIXELS_PER_PIXEL;
    INT16 player_top = level_player_top();

    if (next_left < PLAYER_MIN_X) {
        next_left = PLAYER_MIN_X;
        next_x = next_left * SUBPIXELS_PER_PIXEL;
        velocity_x = 0;
    } else if (next_left > PLAYER_MAX_X) {
        next_left = PLAYER_MAX_X;
        next_x = next_left * SUBPIXELS_PER_PIXEL;
        velocity_x = 0;
    }

    for (block_index = 0u; block_index != level->block_count; ++block_index) {
        const level_rect_t *block = &level->blocks[block_index];

        if (!rects_overlap(next_left, player_top, MINT_SPRITE_WIDTH, MINT_SPRITE_HEIGHT,
                           block->x, block->y, block->width, block->height)) {
            continue;
        }

        if (velocity_x > 0) {
            next_left = block->x - MINT_SPRITE_WIDTH;
        } else if (velocity_x < 0) {
            next_left = block->x + block->width;
        }

        next_x = next_left * SUBPIXELS_PER_PIXEL;
        velocity_x = 0;
    }

    player_x = next_x;
}

static void resolve_vertical_movement(const platformer_level_t *level) {
    UINT8 block_index;
    INT16 current_top = level_player_top();
    INT16 current_bottom = current_top + MINT_SPRITE_HEIGHT;
    INT16 next_y = player_y + velocity_y;
    INT16 next_top = next_y / SUBPIXELS_PER_PIXEL;
    INT16 next_bottom = next_top + MINT_SPRITE_HEIGHT;

    if (velocity_y > 0) {
        for (block_index = 0u; block_index != level->block_count; ++block_index) {
            const level_rect_t *block = &level->blocks[block_index];

            if (current_bottom <= block->y &&
                next_bottom >= block->y &&
                ranges_overlap(level_player_left(), MINT_SPRITE_WIDTH, block->x, block->width)) {
                next_top = block->y - MINT_SPRITE_HEIGHT;
                next_y = next_top * SUBPIXELS_PER_PIXEL;
                velocity_y = 0;
                next_bottom = block->y;
                break;
            }
        }

        if (next_bottom >= level->ground_y) {
            next_top = level->ground_y - MINT_SPRITE_HEIGHT;
            next_y = next_top * SUBPIXELS_PER_PIXEL;
            velocity_y = 0;
        }
    } else if (velocity_y < 0) {
        for (block_index = 0u; block_index != level->block_count; ++block_index) {
            const level_rect_t *block = &level->blocks[block_index];
            INT16 block_bottom = block->y + block->height;

            if (current_top >= block_bottom &&
                next_top <= block_bottom &&
                ranges_overlap(level_player_left(), MINT_SPRITE_WIDTH, block->x, block->width)) {
                next_top = block_bottom;
                next_y = next_top * SUBPIXELS_PER_PIXEL;
                velocity_y = 0;
                break;
            }
        }
    }

    player_y = next_y;
}

void platformer_enter(const platformer_level_t *level) {
    cls();
    set_sprite_data(0u, MINT_SPRITE_TILE_COUNT, mint_sprite_tiles);
    set_sprite_data(MINT_SPRITE_TILE_COUNT, GOAL_SPRITE_TILE_COUNT, goal_sprite_tiles);
    set_sprite_data(BLOCK_SPRITE_TILE, 1u, block_sprite_tile);
    draw_level_background();

    gotoxy(6, 1);
    printf("%s", level->title);

    player_x = level->start_x * SUBPIXELS_PER_PIXEL;
    player_y = level->start_y * SUBPIXELS_PER_PIXEL;
    velocity_x = 0;
    velocity_y = 0;
    jump_count = 0u;
    coyote_timer = COYOTE_FRAMES;
    jump_buffer_timer = 0u;

    draw_level_entities(level);
}

level_event_t platformer_update(const platformer_level_t *level, UINT8 input, UINT8 pressed) {
    UINT8 grounded = level_is_grounded(level);

    if (grounded != 0u) {
        coyote_timer = COYOTE_FRAMES;
        jump_count = 0u;
    } else if (coyote_timer != 0u) {
        --coyote_timer;
    }

    if ((pressed & J_A) != 0u) {
        jump_buffer_timer = JUMP_BUFFER_FRAMES;
    } else if (jump_buffer_timer != 0u) {
        --jump_buffer_timer;
    }

    if (jump_buffer_timer != 0u) {
        if ((grounded != 0u || coyote_timer != 0u) && jump_count == 0u) {
            velocity_y = JUMP_VELOCITY;
            jump_count = 1u;
            coyote_timer = 0u;
            jump_buffer_timer = 0u;
            grounded = 0u;
        } else if (grounded == 0u && jump_count == 1u) {
            velocity_y = DOUBLE_JUMP_VELOCITY;
            jump_count = 2u;
            jump_buffer_timer = 0u;
        }
    }

    apply_horizontal_input(input, grounded);
    resolve_horizontal_movement(level);

    if (velocity_y < 0) {
        velocity_y += GRAVITY_ASCEND;
    } else {
        velocity_y += GRAVITY_DESCEND;
    }

    if (velocity_y > MAX_FALL_SPEED) {
        velocity_y = MAX_FALL_SPEED;
    }

    resolve_vertical_movement(level);
    draw_level_entities(level);

    if (rects_overlap(level_player_left(), level_player_top(), MINT_SPRITE_WIDTH, MINT_SPRITE_HEIGHT,
                      level->goal.x, level->goal.y, level->goal.width, level->goal.height)) {
        return LEVEL_EVENT_COMPLETE;
    }

    return LEVEL_EVENT_NONE;
}
