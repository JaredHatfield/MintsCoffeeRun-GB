#ifndef GOAL_SPRITE_H
#define GOAL_SPRITE_H

#include <gb/gb.h>

#define GOAL_SPRITE_WIDTH 8u
#define GOAL_SPRITE_HEIGHT 8u
#define GOAL_SPRITE_WIDTH_TILES 1u
#define GOAL_SPRITE_HEIGHT_TILES 1u
#define GOAL_SPRITE_FRAME_COUNT 1u
#define GOAL_SPRITE_TILES_PER_FRAME (GOAL_SPRITE_WIDTH_TILES * GOAL_SPRITE_HEIGHT_TILES)
#define GOAL_SPRITE_TILE_COUNT (GOAL_SPRITE_FRAME_COUNT * GOAL_SPRITE_TILES_PER_FRAME)

extern const unsigned char goal_sprite_tiles[];

void goal_sprite_show(UINT8 first_tile, UINT8 first_sprite, UINT8 frame, UINT8 x, UINT8 y);

#endif
