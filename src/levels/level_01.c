#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>

#include "../sprites/goal_sprite.h"
#include "../sprites/mint_sprite.h"
#include "level_01.h"

#define LEVEL_1_TILE_BASE 160u
#define LEVEL_1_TILE_COUNT 2u
#define LEVEL_1_BLANK_TILE 160u
#define LEVEL_1_GROUND_TILE 161u
#define LEVEL_1_GROUND_ROW 16u
#define LEVEL_1_PLAYER_START_X 16
#define LEVEL_1_PLAYER_START_Y 112
#define LEVEL_1_PLAYER_MIN_X 0
#define LEVEL_1_PLAYER_MAX_X 144
#define LEVEL_1_SURFACE_Y 128
#define LEVEL_1_BLOCK_X 84
#define LEVEL_1_BLOCK_Y 108
#define LEVEL_1_BLOCK_SIZE 8
#define LEVEL_1_GOAL_X 144
#define LEVEL_1_GOAL_Y 120
#define LEVEL_1_BLOCK_SPRITE_TILE (MINT_SPRITE_TILE_COUNT + GOAL_SPRITE_TILE_COUNT)
#define LEVEL_1_BLOCK_SPRITE_INDEX 5u
#define LEVEL_1_MOVE_SPEED 1
#define LEVEL_1_JUMP_SPEED -10
#define LEVEL_1_MAX_FALL_SPEED 6

static INT16 mint_x;
static INT16 mint_y;
static INT8 mint_velocity_y;
static UINT8 jumps_used;

static const unsigned char level_1_bg_tiles[LEVEL_1_TILE_COUNT * 16u] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0xFF, 0xFF, 0xFF, 0xFF, 0xDB, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xDB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const unsigned char level_1_block_sprite_tile[16] = {
    0xFF, 0xFF, 0x81, 0xFF, 0xBD, 0xFF, 0xA5, 0xFF,
    0xA5, 0xFF, 0xBD, 0xFF, 0x81, 0xFF, 0xFF, 0xFF
};

static const unsigned char level_1_ground_row[20] = {
    LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE,
    LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE,
    LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE,
    LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE,
    LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE, LEVEL_1_GROUND_TILE
};

static UINT8 ranges_overlap(INT16 a_start, UINT8 a_size, INT16 b_start, UINT8 b_size) {
    return (a_start < (INT16)(b_start + b_size)) && ((INT16)(a_start + a_size) > b_start);
}

static UINT8 rects_overlap(INT16 ax, INT16 ay, UINT8 aw, UINT8 ah, INT16 bx, INT16 by, UINT8 bw, UINT8 bh) {
    return ranges_overlap(ax, aw, bx, bw) && ranges_overlap(ay, ah, by, bh);
}

static UINT8 player_is_on_block(void) {
    return (mint_y + MINT_SPRITE_HEIGHT == LEVEL_1_BLOCK_Y) &&
           ranges_overlap(mint_x, MINT_SPRITE_WIDTH, LEVEL_1_BLOCK_X, LEVEL_1_BLOCK_SIZE);
}

static UINT8 player_is_supported(void) {
    return (mint_y + MINT_SPRITE_HEIGHT == LEVEL_1_SURFACE_Y) || player_is_on_block();
}

static void draw_level_1_background(void) {
    set_bkg_data(LEVEL_1_TILE_BASE, LEVEL_1_TILE_COUNT, level_1_bg_tiles);
    set_bkg_tiles(0u, LEVEL_1_GROUND_ROW, 20u, 1u, level_1_ground_row);
    set_bkg_tiles(0u, LEVEL_1_GROUND_ROW + 1u, 20u, 1u, level_1_ground_row);
}

static void draw_level_1_entities(void) {
    mint_sprite_show(0u, 0u, 0u, (UINT8)(mint_x + 8), (UINT8)(mint_y + 16));
    set_sprite_tile(LEVEL_1_BLOCK_SPRITE_INDEX, LEVEL_1_BLOCK_SPRITE_TILE);
    move_sprite(LEVEL_1_BLOCK_SPRITE_INDEX, (UINT8)(LEVEL_1_BLOCK_X + 8), (UINT8)(LEVEL_1_BLOCK_Y + 16));
    goal_sprite_show(MINT_SPRITE_TILE_COUNT, MINT_SPRITE_TILES_PER_FRAME, 0u,
                     (UINT8)(LEVEL_1_GOAL_X + 8), (UINT8)(LEVEL_1_GOAL_Y + 16));
}

static void resolve_horizontal_movement(INT8 dx) {
    INT16 next_x = mint_x + dx;

    if (next_x < LEVEL_1_PLAYER_MIN_X) {
        next_x = LEVEL_1_PLAYER_MIN_X;
    } else if (next_x > LEVEL_1_PLAYER_MAX_X) {
        next_x = LEVEL_1_PLAYER_MAX_X;
    }

    if (dx > 0 &&
        rects_overlap(next_x, mint_y, MINT_SPRITE_WIDTH, MINT_SPRITE_HEIGHT,
                      LEVEL_1_BLOCK_X, LEVEL_1_BLOCK_Y, LEVEL_1_BLOCK_SIZE, LEVEL_1_BLOCK_SIZE)) {
        next_x = LEVEL_1_BLOCK_X - MINT_SPRITE_WIDTH;
    } else if (dx < 0 &&
               rects_overlap(next_x, mint_y, MINT_SPRITE_WIDTH, MINT_SPRITE_HEIGHT,
                             LEVEL_1_BLOCK_X, LEVEL_1_BLOCK_Y, LEVEL_1_BLOCK_SIZE, LEVEL_1_BLOCK_SIZE)) {
        next_x = LEVEL_1_BLOCK_X + LEVEL_1_BLOCK_SIZE;
    }

    mint_x = next_x;
}

static void resolve_vertical_movement(void) {
    INT16 next_y = mint_y + mint_velocity_y;

    if (mint_velocity_y > 0) {
        if ((mint_y + MINT_SPRITE_HEIGHT) <= LEVEL_1_BLOCK_Y &&
            (next_y + MINT_SPRITE_HEIGHT) >= LEVEL_1_BLOCK_Y &&
            ranges_overlap(mint_x, MINT_SPRITE_WIDTH, LEVEL_1_BLOCK_X, LEVEL_1_BLOCK_SIZE)) {
            next_y = LEVEL_1_BLOCK_Y - MINT_SPRITE_HEIGHT;
            mint_velocity_y = 0;
            jumps_used = 0u;
        } else if ((next_y + MINT_SPRITE_HEIGHT) >= LEVEL_1_SURFACE_Y) {
            next_y = LEVEL_1_SURFACE_Y - MINT_SPRITE_HEIGHT;
            mint_velocity_y = 0;
            jumps_used = 0u;
        }
    } else if (mint_velocity_y < 0) {
        if (mint_y >= (LEVEL_1_BLOCK_Y + LEVEL_1_BLOCK_SIZE) &&
            next_y < (LEVEL_1_BLOCK_Y + LEVEL_1_BLOCK_SIZE) &&
            ranges_overlap(mint_x, MINT_SPRITE_WIDTH, LEVEL_1_BLOCK_X, LEVEL_1_BLOCK_SIZE)) {
            next_y = LEVEL_1_BLOCK_Y + LEVEL_1_BLOCK_SIZE;
            mint_velocity_y = 0;
        }
    }

    mint_y = next_y;
}

static void level_1_enter(void) {
    cls();
    set_sprite_data(0u, MINT_SPRITE_TILE_COUNT, mint_sprite_tiles);
    set_sprite_data(MINT_SPRITE_TILE_COUNT, GOAL_SPRITE_TILE_COUNT, goal_sprite_tiles);
    set_sprite_data(LEVEL_1_BLOCK_SPRITE_TILE, 1u, level_1_block_sprite_tile);
    draw_level_1_background();

    gotoxy(6, 1);
    printf("Level 01");

    mint_x = LEVEL_1_PLAYER_START_X;
    mint_y = LEVEL_1_PLAYER_START_Y;
    mint_velocity_y = 0;
    jumps_used = 0u;

    draw_level_1_entities();
}

static level_event_t level_1_update(UINT8 input, UINT8 pressed) {
    INT8 dx = 0;

    if ((pressed & J_A) != 0u && jumps_used < 2u) {
        mint_velocity_y = LEVEL_1_JUMP_SPEED;
        ++jumps_used;
    }

    if ((input & J_LEFT) != 0u) {
        dx -= LEVEL_1_MOVE_SPEED;
    }
    if ((input & J_RIGHT) != 0u) {
        dx += LEVEL_1_MOVE_SPEED;
    }

    if (dx != 0) {
        resolve_horizontal_movement(dx);
    }

    if (!player_is_supported() || mint_velocity_y < 0) {
        if (mint_velocity_y < LEVEL_1_MAX_FALL_SPEED) {
            ++mint_velocity_y;
        }
        resolve_vertical_movement();
    } else {
        mint_velocity_y = 0;
    }

    draw_level_1_entities();

    if (rects_overlap(mint_x, mint_y, MINT_SPRITE_WIDTH, MINT_SPRITE_HEIGHT,
                      LEVEL_1_GOAL_X, LEVEL_1_GOAL_Y, GOAL_SPRITE_WIDTH, GOAL_SPRITE_HEIGHT)) {
        return LEVEL_EVENT_COMPLETE;
    }

    return LEVEL_EVENT_NONE;
}

const level_definition_t level_01_definition = {
    level_1_enter,
    level_1_update
};
