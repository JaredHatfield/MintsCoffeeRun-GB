# MintsCoffeeRun-GB

Purpose: a small Game Boy project built with GBDK-2020 for `Mint's Coffee Run`.

Key files:
- `src/main.c` boots the ROM, handles screen flow, and routes into level gameplay.
- `src/levels/` contains per-level gameplay code, with each implemented level defined in its own source file.
- `src/sprites/mint.piskel` is the source of truth for Mint's sprite art.
- `src/sprites/goal.piskel` is the source of truth for the goal sprite art.
- `src/sprites/mint_sprite.c` and `src/sprites/mint_sprite.h` are the generated base-size sprite assets.
- `src/sprites/mint_title_sprite.c` and `src/sprites/mint_title_sprite.h` are the generated enlarged title-screen sprite assets.
- `src/sprites/goal_sprite.c` and `src/sprites/goal_sprite.h` are the generated goal sprite assets.
- `scripts/piskel_to_gbdk_sprite.py` converts `.piskel` art into GBDK-ready C code.
- `justfile` contains the main developer commands.

Workflow:
- Edit sprite art in `.piskel` files, not in generated `.c` or `.h` files.
- Run `just sprites` to regenerate all sprite code from the `.piskel` sources.
- `just build` regenerates sprites and then builds the ROM.
- When gameplay flow, controls, progression, or screen behavior changes, update `GAMEPLAY.md` in the same change.

Constraints:
- Sprite dimensions should be multiples of 8 pixels.
- The current converter targets Game Boy OBJ sprites, so art must use at most 3 opaque colors plus transparency.
