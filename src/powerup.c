#include "powerup.h"

#include "post_processor.h"
#include "resource_manager.h"
#include "util.h"

#include <stdlib.h>
#include <linmath.h>
#include <miniaudio.h>

extern Ball ball;
extern GameObject player;
extern PostProcessor effects;
extern ma_result result;
extern ma_engine engine;

static int should_spawn(int chance) { return (rand() % chance ) == 0; }
static struct { const char *type; vec3 color; float dur; int chance; const char *tex; } tbl[] = {
    {"speed",            {0.5f,0.5f,1.0f},  0.0f, 75, "powerup_speed"},
    {"sticky",           {1.0f,0.5f,1.0f}, 10.0f, 75, "powerup_sticky"},
    {"pass-through",     {0.5f,1.0f,0.5f}, 10.0f, 75, "powerup_passthrough"},
    {"pad-size-increase",{1.0f,0.6f,0.4f},  0.0f, 75, "powerup_increase"},
    {"confuse",          {1.0f,0.3f,0.3f}, 15.0f, 15, "powerup_confuse"},
    {"chaos",            {0.9f,0.25f,0.25f},15.0f, 15, "powerup_chaos"}
};
static void powerups_ensure_capacity(Powerups *ps) {
    if (ps->count >= ps->capacity) {
        ps->capacity += 100;
        ps->list = realloc(ps->list, ps->capacity * sizeof(Powerup));
        if (!ps->list) {
            ERROR_EXIT("Failed to realloc powerups list.");
        }
    }
}

void powerups_init(Powerups *ps) {
    ps->capacity = 100;
    ps->count = 0;
    ps->list = calloc(ps->capacity, sizeof(Powerup));
}

void powerups_spawn(Powerups *ps, GameObject *brick) {
    for (int i = 0; i < sizeof tbl / sizeof *tbl; i++) {
        if (should_spawn(tbl[i].chance)) {
            powerups_ensure_capacity(ps);
            Powerup *p = &ps->list[ps->count++];
            memset(p, 0, sizeof *p);

            p->type = tbl[i].type;
            p->duration = tbl[i].dur;
            p->activated = 0;

            // Brick falling
            vec2_dup(p->object.position, brick->position);
            p->object.size[0] = 40.0f; p->object.size[1] = 25.0f;
            p->object.velocity[0] = 0.0f; p->object.velocity[1] = 150.0f;
            vec3_dup(p->object.color, tbl[i].color);
            p->object.sprite = resource_get_texture(tbl[i].tex);
            p->object.is_destroyed = 0;
        }
    }
}

void powerups_update(Powerups *ps, float dt, float window_h) {
    for (int i = 0; i < ps->count;) {
        Powerup *p = &ps->list[i];

        vec2 delta; 
        vec2_scale(delta, p->object.velocity, dt);
        vec2_add(p->object.position, p->object.position, delta);

        // Duration handling
        if (p->activated && p->duration > 0.f) {
            p->duration -= dt;
            if (p->duration <= 0.f) {
                p->activated = 0;

                if (!strcmp(p->type, "sticky") && !powerups_is_other_active(ps, "sticky")) {
                    ball.sticky = 0;
                    player.color[0] = player.color[1] = player.color[2] = 1.0f;
                } else if (!strcmp(p->type, "pass-through") && !powerups_is_other_active(ps, "pass-through")) {
                    ball.pass_through = 0;
                    ball.object.color[0] = ball.object.color[1] = ball.object.color[2] = 1.0f;
                } else if (!strcmp(p->type, "confuse") && !powerups_is_other_active(ps, "confuse")) {
                    effects.confuse = 0;
                } else if (!strcmp(p->type, "chaos") && !powerups_is_other_active(ps, "chaos")) {
                    effects.chaos = 0;
                } else if (!strcmp(p->type, "pad-size-increase") && !powerups_is_other_active(ps, "pad-size-increase")) {
                    player.size[0] -= 50.0f;
                } else if (!strcmp(p->type, "speed") && !powerups_is_other_active(ps, "speed")) {
                    vec2_dup(ball.object.velocity, ball.initial_velocity);
                }
            }

            if (p->object.position[1] >= window_h)
                p->object.is_destroyed = 1;
        }

        int kill =  p->object.is_destroyed ||
                    p->object.position[1] >= window_h ||
                   (!p->activated && p->duration <= 0.0f);

        if (kill) {
            ps->list[i] = ps->list[--ps->count];
            continue;
        }
        ++i;
    }
}


void powerups_draw(Powerups *ps, SpriteRenderer *renderer) {
    for (int i = 0; i < ps->count; ++i)
        if (!ps->list[i].activated && !ps->list[i].object.is_destroyed)
            game_object_draw(&ps->list[i].object, renderer);
}

void powerups_collision(Powerups *ps, GameObject *paddle) {
    for (int i = 0; i < ps->count; ++i) {
        Powerup *p = &ps->list[i];
        if (!p->activated && !p->object.is_destroyed && aabb(paddle, &p->object)) {
            powerup_activate(p);
            p->activated = 1;
            p->object.is_destroyed = 1;
            ma_engine_play_sound(&engine, "./res/audio/powerup.wav", NULL);
        }
    }
}

int powerups_is_other_active(Powerups *ps, const char *type) {
    for (int i = 0; i < ps->count; ++i)
        if (ps->list[i].activated && !strcmp(ps->list[i].type,type))
            return 1;
    return 0;
}

void powerup_activate(Powerup *p) {
    if (!strcmp(p->type, "speed")) {
        vec2_scale(ball.object.velocity, ball.object.velocity, 1.2f);
    } else if (!strcmp(p->type, "sticky")) {
        ball.sticky = 1;
        player.color[0] = 1.0f; player.color[1] = 0.5f; player.color[2] = 1.0f;
    } else if (!strcmp(p->type, "pass-through")) {
        ball.pass_through = 1;
        ball.object.color[0] = 1.0f; ball.object.color[1] = ball.object.color[2] = 0.5f;
    } else if (!strcmp(p->type, "pad-size-increase")) {
        player.size[0] += 50.f;
    } else if (!strcmp(p->type, "confuse") && !effects.chaos) {
        effects.confuse = 1;
    } else if (!strcmp(p->type, "chaos") && !effects.confuse) {
        effects.chaos = 1;
    }
}

void powerups_destroy(Powerups *ps) {
    free(ps->list);
    ps->list = NULL;
    ps->count = 0;
    ps->capacity = 0;
}
