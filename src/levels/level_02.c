#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>

#include "../sprites/goal_sprite.h"
#include "../sprites/mint_sprite.h"
#include "level_02.h"

#define LEVEL_2_TILE_BASE 160u
#define LEVEL_2_TILE_COUNT 2u
#define LEVEL_2_GROUND_TILE 161u
#define LEVEL_2_GROUND_ROW 16u
#define LEVEL_2_PLAYER_START_X 16
#define LEVEL_2_PLAYER_START_Y 112
#define LEVEL_2_PLAYER_MIN_X 0
#define LEVEL_2_PLAYER_MAX_X 144
#define LEVEL_2_SURFACE_Y 128
#define LEVEL_2_GOAL_X 144
#define LEVEL_2_GOAL_Y 120
#define LEVEL_2_MOVE_SPEED 1
#define LEVEL_2_JUMP_SPEED -10
#define LEVEL_2_MAX_FALL_SPEED 6

static INT16 mint_x;
static INT16 mint_y;
static INT8 mint_velocity_y;
static UINT8 jumps_used;

static const unsigned char level_2_bg_tiles[LEVEL_2_TILE_COUNT * 16u] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0xFF, 0xFF, 0xFF, 0xFF, 0xDB, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xDB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const unsigned char level_2_ground_row[20] = {
    LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE,
    LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE,
    LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE,
    LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE,
    LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE, LEVEL_2_GROUND_TILE
};

static UINT8 rects_overlap(INT16 ax, INT16 ay, UINT8 aw, UINT8 ah, INT16 bx, INT16 by, UINT8 bw, UINT8 bh) {
    return (ax < (INT16)(bx + bw)) && ((INT16)(ax + aw) > bx) &&
           (ay < (INT16)(by + bh)) && ((INT16)(ay + ah) > by);
}

static void draw_level_2_entities(void) {
    mint_sprite_show(0u, 0u, 0u, (UINT8)(mint_x + 8), (UINT8)(mint_y + 16));
    goal_sprite_show(MINT_SPRITE_TILE_COUNT, MINT_SPRITE_TILES_PER_FRAME, 0u,
                     (UINT8)(LEVEL_2_GOAL_X + 8), (UINT8)(LEVEL_2_GOAL_Y + 16));
}

static void level_2_enter(void) {
    cls();
    set_sprite_data(0u, MINT_SPRITE_TILE_COUNT, mint_sprite_tiles);
    set_sprite_data(MINT_SPRITE_TILE_COUNT, GOAL_SPRITE_TILE_COUNT, goal_sprite_tiles);
    set_bkg_data(LEVEL_2_TILE_BASE, LEVEL_2_TILE_COUNT, level_2_bg_tiles);
    set_bkg_tiles(0u, LEVEL_2_GROUND_ROW, 20u, 1u, level_2_ground_row);
    set_bkg_tiles(0u, LEVEL_2_GROUND_ROW + 1u, 20u, 1u, level_2_ground_row);

    gotoxy(6, 1);
    printf("Level 02");

    mint_x = LEVEL_2_PLAYER_START_X;
    mint_y = LEVEL_2_PLAYER_START_Y;
    mint_velocity_y = 0;
    jumps_used = 0u;

    draw_level_2_entities();
}

static level_event_t level_2_update(UINT8 input, UINT8 pressed) {
    if ((pressed & J_A) != 0u && jumps_used < 2u) {
        mint_velocity_y = LEVEL_2_JUMP_SPEED;
        ++jumps_used;
    }

    if ((input & J_LEFT) != 0u && mint_x > LEVEL_2_PLAYER_MIN_X) {
        --mint_x;
    }
    if ((input & J_RIGHT) != 0u && mint_x < LEVEL_2_PLAYER_MAX_X) {
        ++mint_x;
    }

    if (mint_velocity_y < LEVEL_2_MAX_FALL_SPEED) {
        ++mint_velocity_y;
    }

    mint_y += mint_velocity_y;

    if ((mint_y + MINT_SPRITE_HEIGHT) >= LEVEL_2_SURFACE_Y) {
        mint_y = LEVEL_2_SURFACE_Y - MINT_SPRITE_HEIGHT;
        mint_velocity_y = 0;
        jumps_used = 0u;
    }

    draw_level_2_entities();

    if (rects_overlap(mint_x, mint_y, MINT_SPRITE_WIDTH, MINT_SPRITE_HEIGHT,
                      LEVEL_2_GOAL_X, LEVEL_2_GOAL_Y, GOAL_SPRITE_WIDTH, GOAL_SPRITE_HEIGHT)) {
        return LEVEL_EVENT_COMPLETE;
    }

    return LEVEL_EVENT_NONE;
}

const level_definition_t level_02_definition = {
    level_2_enter,
    level_2_update
};
