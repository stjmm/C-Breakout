#pragma once

#include "game_object.h"
#include "sprite_renderer.h"

typedef struct {
    Powerup *list;
    int count;
    int capacity;
} Powerups;

void powerups_init(Powerups *ps);
void powerups_spawn(Powerups *ps, GameObject *brick);
void powerups_update(Powerups *ps, float dt, float window_h);
void powerups_draw(Powerups *ps, SpriteRenderer *renderer);
void powerups_collision(Powerups *ps, GameObject *paddle);
void powerups_destroy(Powerups *ps);

int powerups_is_other_active(Powerups *ps, const char *type);
void powerup_activate(Powerup *p);
