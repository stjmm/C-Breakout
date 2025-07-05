#pragma once

#include "platform.h"

typedef enum {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
    GAME_CLOSE
} GameState;

typedef struct {
    GameState state;

    Platform platform;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_render(Game *game);
void game_destroy(Game *game);
