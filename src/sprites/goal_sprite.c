#include "goal_sprite.h"

/* Generated from src/sprites/goal.piskel by scripts/piskel_to_gbdk_sprite.py. */
const unsigned char goal_sprite_tiles[] = {
    /* tile 0 */
    0x00, 0x00, 0x18, 0x66, 0x18, 0x66, 0x66, 0x18,
    0x66, 0x18, 0x18, 0x66, 0x18, 0x66, 0x00, 0x00
};

void goal_sprite_show(UINT8 first_tile, UINT8 first_sprite, UINT8 frame, UINT8 x, UINT8 y) {
    UINT8 sprite_index = first_sprite;
    UINT8 tile_index = first_tile + (frame * GOAL_SPRITE_TILES_PER_FRAME);
    UINT8 row;
    UINT8 col;

    for (row = 0; row != GOAL_SPRITE_HEIGHT_TILES; ++row) {
        for (col = 0; col != GOAL_SPRITE_WIDTH_TILES; ++col) {
            set_sprite_tile(sprite_index, tile_index);
            move_sprite(sprite_index, x + (col << 3), y + (row << 3));
            ++sprite_index;
            ++tile_index;
        }
    }
}
