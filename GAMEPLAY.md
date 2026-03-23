# Mint's Coffee Run Gameplay

## Core flow

1. The ROM opens on the title screen for `Mint's Coffee Run`.
2. After the title card appears for a few seconds, the game transitions automatically to the level select screen.
3. Selecting a level starts gameplay for that level.
4. Completing a level should immediately advance to the next level instead of returning to level select.
5. The player should be able to return to the level select screen later and choose any level that has already been unlocked.
6. Walking Mint through the goal sprite is how a level is completed.

## Level select

- The level select screen shows `Levels` at the top.
- There are 25 total levels arranged in a 5 x 5 grid.
- All 25 level numbers are always visible in the grid.
- The cursor can move over every level in the grid, including locked ones.
- At the start of a new run, only level 1 is unlocked.
- Locked levels are shown with lighter text than unlocked levels.
- Pressing `A` starts the currently selected level only when that level is unlocked.
- The bottom of the level select screen shows the game name instead of instruction text.
- Unlocked levels remain selectable from the level select screen after they are earned.

## Progression

- Level unlocks are sequential.
- Beating level 1 unlocks level 2, beating level 2 unlocks level 3, and so on.
- After beating a level, gameplay should continue directly into the next level when one exists.
- The temporary `B` button return to level select has been removed.

## Current playable stub

- Gameplay code is split into per-level source files under `src/levels/`.
- The current implementation includes the title screen, the level select screen, level 1, and a temporary level 2 placeholder so level completion can advance forward.
- Level 1 is a simple test course with flat ground, one floating one-tile block, and an easy goal on the right side of the screen.
- Mint can move left and right and can double jump by pressing `A` twice before landing.
- Touching the goal completes the level and advances to the next available level.
