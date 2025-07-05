#pragma once

#include <linmath.h>

#include "sprite_renderer.h"
#include "texture.h"

typedef enum {
    DIRECTION_UP,
    DIRECTION_RIGHT,
    DIRECTION_DOWN,
    DIRECTION_LEFT
} Direction;

typedef struct {
    int is_collision;
    Direction direction;
    vec2 difference;
} Collision;

typedef struct {
    vec2 position;
    vec2 size;
    vec2 velocity;
    vec3 color;
    float rotation;
    Texture *sprite;

    int is_solid;
    int is_destroyed;
} GameObject;

typedef struct {
    GameObject object;
    float radius;
    int stuck;
    vec2 initial_position, initial_velocity;
} Ball;

void game_object_init(GameObject *obj, vec2 pos, vec2 size, Texture *sprite, vec3 color, vec2 velocity);
void game_object_draw(GameObject *obj, SpriteRenderer *renderer);

void ball_init(Ball *ball, vec2 pos, float radius, vec2 velocity, Texture *sprite);
void ball_move(Ball *ball, float dt, int window_width, int window_height);
void ball_reset(Ball *ball);

int aabb(Ball *ball, GameObject *object);
Collision aabb_circle(Ball *ball, GameObject *object);

Direction vector_direction(vec2 target);
