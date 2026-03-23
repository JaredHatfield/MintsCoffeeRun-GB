#include <gb/gb.h>
#include <gbdk/console.h>
#include <gbdk/font.h>
#include <stdio.h>

#include "sprites/mint_sprite.h"
#include "sprites/mint_title_sprite.h"

#define TOTAL_SPRITES 40u
#define LEVEL_COUNT 25u
#define LEVELS_PER_ROW 5u
#define LEVEL_ROWS 5u
#define LEVEL_SELECT_TOP_Y 4u
#define TITLE_DURATION_FRAMES 120u
#define LEVEL_1_MIN_X 16u
#define LEVEL_1_MAX_X 152u
#define LEVEL_1_Y 120u
#define LEVEL_TILE_BASE 128u
#define LEVEL_TILE_COUNT 22u
#define LEVEL_TILE_BLANK 128u
#define LEVEL_TILE_CURSOR 129u
#define LEVEL_TILE_DARK_DIGIT_BASE 130u
#define LEVEL_TILE_LIGHT_DIGIT_BASE 140u

typedef enum screen_state_t {
    SCREEN_TITLE = 0,
    SCREEN_LEVEL_SELECT,
    SCREEN_LEVEL_1
} screen_state_t;

static font_t game_font;
static screen_state_t current_screen;
static UINT8 previous_input;
static UINT8 title_timer;
static UINT8 selected_level;
static UINT8 unlocked_level_count;
static UINT8 mint_x;
static unsigned char level_select_tiles[LEVEL_TILE_COUNT * 16u];

static const unsigned char digit_masks[10][8] = {
    {0x3Cu, 0x66u, 0x6Eu, 0x76u, 0x66u, 0x66u, 0x3Cu, 0x00u},
    {0x18u, 0x38u, 0x18u, 0x18u, 0x18u, 0x18u, 0x3Cu, 0x00u},
    {0x3Cu, 0x66u, 0x06u, 0x0Cu, 0x30u, 0x60u, 0x7Eu, 0x00u},
    {0x3Cu, 0x66u, 0x06u, 0x1Cu, 0x06u, 0x66u, 0x3Cu, 0x00u},
    {0x0Cu, 0x1Cu, 0x2Cu, 0x4Cu, 0x7Eu, 0x0Cu, 0x1Eu, 0x00u},
    {0x7Eu, 0x60u, 0x7Cu, 0x06u, 0x06u, 0x66u, 0x3Cu, 0x00u},
    {0x3Cu, 0x66u, 0x60u, 0x7Cu, 0x66u, 0x66u, 0x3Cu, 0x00u},
    {0x7Eu, 0x66u, 0x06u, 0x0Cu, 0x18u, 0x18u, 0x18u, 0x00u},
    {0x3Cu, 0x66u, 0x66u, 0x3Cu, 0x66u, 0x66u, 0x3Cu, 0x00u},
    {0x3Cu, 0x66u, 0x66u, 0x3Eu, 0x06u, 0x66u, 0x3Cu, 0x00u}
};

static const unsigned char cursor_mask[8] = {
    0x00u, 0x04u, 0x0Cu, 0x1Cu, 0x0Cu, 0x04u, 0x00u, 0x00u
};

static void hide_all_sprites(void) {
    UINT8 sprite_index;

    for (sprite_index = 0; sprite_index != TOTAL_SPRITES; ++sprite_index) {
        move_sprite(sprite_index, 0, 0);
    }
}

static UINT8 is_level_unlocked(UINT8 level_index) {
    return level_index < unlocked_level_count;
}

static void build_level_select_tile(UINT8 tile_offset, const unsigned char *mask_rows, UINT8 color) {
    UINT8 row;
    unsigned char *tile_data = &level_select_tiles[tile_offset * 16u];

    for (row = 0; row != 8u; ++row) {
        unsigned char row_mask = mask_rows[row];

        tile_data[row * 2u] = (color & 0x01u) != 0u ? row_mask : 0u;
        tile_data[(row * 2u) + 1u] = (color & 0x02u) != 0u ? row_mask : 0u;
    }
}

static void load_level_select_tiles(void) {
    UINT8 digit;

    build_level_select_tile(1u, cursor_mask, 3u);

    for (digit = 0u; digit != 10u; ++digit) {
        build_level_select_tile(2u + digit, digit_masks[digit], 3u);
        build_level_select_tile(12u + digit, digit_masks[digit], 1u);
    }

    set_bkg_data(LEVEL_TILE_BASE, LEVEL_TILE_COUNT, level_select_tiles);
}

static UINT8 level_tile_for_digit(UINT8 digit, UINT8 unlocked) {
    if (unlocked != 0u) {
        return (UINT8)(LEVEL_TILE_DARK_DIGIT_BASE + digit);
    }

    return (UINT8)(LEVEL_TILE_LIGHT_DIGIT_BASE + digit);
}

static void draw_level_cell(UINT8 level_index) {
    UINT8 row = level_index / LEVELS_PER_ROW;
    UINT8 col = level_index % LEVELS_PER_ROW;
    UINT8 level_number = level_index + 1u;
    UINT8 unlocked = is_level_unlocked(level_index);
    UINT8 tens = level_number / 10u;
    UINT8 ones = level_number % 10u;
    UINT8 tile_x = 1u + (col * 4u);
    UINT8 tile_y = LEVEL_SELECT_TOP_Y + (row * 2u);
    unsigned char tiles[3];

    tiles[0] = level_index == selected_level ? LEVEL_TILE_CURSOR : LEVEL_TILE_BLANK;
    tiles[1] = tens != 0u ? level_tile_for_digit(tens, unlocked) : LEVEL_TILE_BLANK;
    tiles[2] = level_tile_for_digit(ones, unlocked);
    set_bkg_tiles(tile_x, tile_y, 3u, 1u, tiles);
}

static void draw_level_select_grid(void) {
    UINT8 level_index;

    for (level_index = 0; level_index != LEVEL_COUNT; ++level_index) {
        draw_level_cell(level_index);
    }
}

static void draw_title_screen(void) {
    cls();
    hide_all_sprites();
    title_timer = 0u;

    gotoxy(1, 4);
    printf("Mint's Coffee Run");

    set_sprite_data(0, MINT_TITLE_SPRITE_TILE_COUNT, mint_title_sprite_tiles);
    mint_title_sprite_show(0, 0, 0, 64u, 88u);

    current_screen = SCREEN_TITLE;
}

static void draw_level_select_screen(void) {
    cls();
    hide_all_sprites();
    load_level_select_tiles();

    gotoxy(7, 1);
    printf("Levels");
    draw_level_select_grid();
    gotoxy(1, 16);
    printf("Mint's Coffee Run");

    current_screen = SCREEN_LEVEL_SELECT;
}

static void draw_level_1_screen(void) {
    cls();
    hide_all_sprites();

    mint_x = 80u;

    gotoxy(6, 2);
    printf("Level 01");
    gotoxy(1, 5);
    printf("LEFT/RIGHT move");
    gotoxy(4, 16);
    printf("B: Levels");

    set_sprite_data(0, MINT_SPRITE_TILE_COUNT, mint_sprite_tiles);
    mint_sprite_show(0, 0, 0, mint_x, LEVEL_1_Y);

    current_screen = SCREEN_LEVEL_1;
}

static void open_selected_level(void) {
    if (selected_level == 0u) {
        draw_level_1_screen();
        return;
    }

    draw_level_select_screen();
}

static void move_level_selection(INT8 dx, INT8 dy) {
    INT8 row = (INT8)(selected_level / LEVELS_PER_ROW);
    INT8 col = (INT8)(selected_level % LEVELS_PER_ROW);
    INT8 next_row = (INT8)(row + dy);
    INT8 next_col = (INT8)(col + dx);

    if (next_row >= 0 && next_row < LEVEL_ROWS && next_col >= 0 && next_col < LEVELS_PER_ROW) {
        selected_level = (UINT8)((next_row * LEVELS_PER_ROW) + next_col);
        draw_level_select_grid();
    }
}

static void update_title_screen(UINT8 pressed) {
    (void)pressed;

    if (title_timer < TITLE_DURATION_FRAMES) {
        ++title_timer;
    }

    if (title_timer >= TITLE_DURATION_FRAMES) {
        draw_level_select_screen();
    }
}

static void update_level_select_screen(UINT8 pressed) {
    if ((pressed & J_LEFT) != 0u) {
        move_level_selection(-1, 0);
    } else if ((pressed & J_RIGHT) != 0u) {
        move_level_selection(1, 0);
    } else if ((pressed & J_UP) != 0u) {
        move_level_selection(0, -1);
    } else if ((pressed & J_DOWN) != 0u) {
        move_level_selection(0, 1);
    }

    if ((pressed & J_A) != 0u && is_level_unlocked(selected_level)) {
        open_selected_level();
    }
}

static void update_level_1_screen(UINT8 input, UINT8 pressed) {
    if ((input & J_LEFT) != 0u && mint_x > LEVEL_1_MIN_X) {
        --mint_x;
    }

    if ((input & J_RIGHT) != 0u && mint_x < LEVEL_1_MAX_X) {
        ++mint_x;
    }

    mint_sprite_show(0, 0, 0, mint_x, LEVEL_1_Y);

    if ((pressed & J_B) != 0u) {
        draw_level_select_screen();
    }
}

void main(void) {
    UINT8 input;
    UINT8 pressed;

    DISPLAY_OFF;

    font_init();
    game_font = font_load(font_ibm);
    font_set(game_font);

    SPRITES_8x8;
    unlocked_level_count = 1u;
    selected_level = 0u;
    previous_input = 0u;

    draw_title_screen();

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    while (1) {
        input = joypad();
        pressed = input & (UINT8)~previous_input;
        previous_input = input;

        switch (current_screen) {
            case SCREEN_TITLE:
                update_title_screen(pressed);
                break;
            case SCREEN_LEVEL_SELECT:
                update_level_select_screen(pressed);
                break;
            case SCREEN_LEVEL_1:
                update_level_1_screen(input, pressed);
                break;
            default:
                draw_title_screen();
                break;
        }

        wait_vbl_done();
    }
}
