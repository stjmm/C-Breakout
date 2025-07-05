#pragma once

#include "game_object.h"
#include "sprite_renderer.h"

#define MAX_LEVEL_ROWS 64
#define MAX_LEVEL_COLS 64
#define MAX_LEVEL_BRICKS (MAX_LEVEL_ROWS * MAX_LEVEL_COLS)

typedef struct {
    GameObject bricks[MAX_LEVEL_BRICKS];
    int brick_count;
} GameLevel;

void game_level_init(GameLevel *level);
void game_level_load(GameLevel *level, const char *path, int level_width, int level_height);
void game_level_draw(GameLevel *level, SpriteRenderer *renderer);
int  game_level_is_completed(GameLevel *level);
