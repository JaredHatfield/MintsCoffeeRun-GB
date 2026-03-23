#ifndef MINT_SPRITE_H
#define MINT_SPRITE_H

#include <gb/gb.h>

#define MINT_SPRITE_WIDTH 16u
#define MINT_SPRITE_HEIGHT 16u
#define MINT_SPRITE_WIDTH_TILES 2u
#define MINT_SPRITE_HEIGHT_TILES 2u
#define MINT_SPRITE_FRAME_COUNT 1u
#define MINT_SPRITE_TILES_PER_FRAME (MINT_SPRITE_WIDTH_TILES * MINT_SPRITE_HEIGHT_TILES)
#define MINT_SPRITE_TILE_COUNT (MINT_SPRITE_FRAME_COUNT * MINT_SPRITE_TILES_PER_FRAME)

extern const unsigned char mint_sprite_tiles[];

void mint_sprite_show(UINT8 first_tile, UINT8 first_sprite, UINT8 frame, UINT8 x, UINT8 y);

#endif
