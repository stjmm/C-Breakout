#include "game_object.h"
#include "util.h"

#include <stdio.h>
#include <linmath.h>

void game_object_init(GameObject *obj, vec2 pos, vec2 size, Texture *sprite, vec3 color, vec2 velocity) {
    vec2_dup(obj->position, pos);
    vec2_dup(obj->size, size);
    vec2_dup(obj->velocity, velocity);
    vec3_dup(obj->color, color);

    obj->rotation = 0.0f;
    obj->sprite = sprite;
    obj->is_solid = 0;
    obj->is_destroyed = 0;
}

void game_object_draw(GameObject *obj, SpriteRenderer *renderer) {
    sprite_renderer_draw(renderer, obj->sprite, obj->position, obj->size, obj->rotation, obj->color);
}

void ball_init(Ball *ball, vec2 pos, float radius, vec2 velocity, Texture *sprite) {
    vec2 size = {radius * 2, radius * 2};
    vec3 color = {1.0f, 1.0f, 1.0f};

    game_object_init(&ball->object, pos, size, sprite, color, velocity);
    vec2_dup(ball->initial_position, pos);
    vec2_dup(ball->initial_velocity, velocity);
    ball->radius = radius;
    ball->stuck = 1;
}

void ball_move(Ball *ball, float dt, int window_width, int window_height) {
    if (!ball->stuck) {
        ball->object.position[0] += ball->object.velocity[0] * dt;
        ball->object.position[1] += ball->object.velocity[1] * dt;

        if (ball->object.position[0] <= 0.0f) {
            ball->object.velocity[0] = -ball->object.velocity[0];
            ball->object.position[0] = 0.0f;
        }
        if (ball->object.position[0] + ball->object.size[0] >= window_width) {
            ball->object.velocity[0] = -ball->object.velocity[0];
            ball->object.position[0] = window_width - ball->object.size[0];
        }

        if (ball->object.position[1] <= 0.0f) {
            ball->object.velocity[1] = -ball->object.velocity[1];
            ball->object.position[1] = 0.0f;
        }
    }
}

void ball_reset(Ball *ball) {
    vec2_dup(ball->object.position, ball->initial_position);
    vec2_dup(ball->object.velocity, ball->initial_velocity);
    ball->stuck = 1;
}

Direction vector_direction(vec2 target) {
    vec2 compass[] = {
        {0.0f, 1.0f},
        {1.0f, 0.0f},
        {0.0f, -1.0f},
        {-1.0f, 0.0f}
    };

    float max = 0.0f;
    int best_match = -1;

    vec2_norm(target, target);
    for(int i = 0; i < 4; i++) {
        float dot = vec2_dot(target, compass[i]);
        if (dot > max) {
            max = dot;
            best_match = i;
        }
    }

    return (Direction)best_match;
}

int aabb(Ball *ball, GameObject *object) {
    int collision_x = ball->object.position[0] + ball->object.size[0] >= object->position[0] &&
        object->position[0] + object->size[0] >= ball->object.position[0];

    int collision_y = ball->object.position[1] + ball->object.size[1] >= object->position[1] &&
        object->position[1] + object->size[1] >= ball->object.position[1];

    return collision_x && collision_y;
}

Collision aabb_circle(Ball *ball, GameObject *object) {
    vec2 center = {ball->object.position[0] + ball->radius, ball->object.position[1] + ball->radius};
    vec2 aabb_half_extends = {object->size[0] / 2.0f, object->size[1] / 2.0f};
    vec2 aabb_center = {object->position[0] + aabb_half_extends[0], object->position[1] + aabb_half_extends[1]};

    vec2 difference;
    vec2_sub(difference, center, aabb_center);
    
    vec2 clamped;
    for (int i = 0; i < 2; i++) {
        if (difference[i] < -aabb_half_extends[i])
            clamped[i] = -aabb_half_extends[i];
        else if (difference[i] > aabb_half_extends[i])
            clamped[i] = aabb_half_extends[i];
        else
            clamped[i] = difference[i];
    }

    vec2 closest;
    vec2_add(closest, aabb_center, clamped);
    
    vec2 diff;
    vec2_sub(diff, closest, center);

    Collision result;
    result.is_collision = vec2_len(diff) < ball->radius;
    vec2_dup(result.difference, diff);
    result.direction = vector_direction(diff);

    return result;
}
