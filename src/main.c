#include <gb/gb.h>
#include <gbdk/console.h>
#include <gbdk/font.h>
#include <stdio.h>

#include "sprites/mint_title_sprite.h"

void main(void) {
    font_t title_font;

    DISPLAY_OFF;

    font_init();
    title_font = font_load(font_ibm);
    font_set(title_font);

    gotoxy(1, 4);
    printf("Mint's Coffee Run");

    SPRITES_8x8;
    set_sprite_data(0, MINT_TITLE_SPRITE_TILE_COUNT, mint_title_sprite_tiles);
    mint_title_sprite_show(0, 0, 0, 64, 80);

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    while (1) {
        wait_vbl_done();
    }
}
