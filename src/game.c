#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <linmath.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "game.h"
#include "game_object.h"
#include "platform.h"
#include "powerup.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_level.h"
#include "shader.h"
#include "particle.h"
#include "post_processor.h"
#include "text_renderer.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

SpriteRenderer renderer;
GameLevel level;
GameObject player;
Ball ball;
ParticleGenerator pg;
PostProcessor effects;
Powerups ps;
TextRenderer tr;

ma_result result;
ma_engine engine;

static void do_collisions(Game *game);

static float shake_time = 0.0f;
static int lives = 3;
static const char *level_files[] = {
    "./res/level/bounce.txt",
    "./res/level/gaps.txt",
    "./res/level/standard.txt",
    "./res/level/invader.txt"
};
static int level_count = 4;
static int selected_level = 2;

void game_init(Game *game) {
    memset(game, 0, sizeof(Game));
    platform_init(&game->platform, 800, 600, "Breakout");
    game->state = GAME_MENU;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    resource_load_shader("sprite", "./res/shader/sprite.vert", "./res/shader/sprite.frag");
    resource_load_shader("particle", "./res/shader/particle.vert", "./res/shader/particle.frag");
    resource_load_shader("framebuffer", "./res/shader/fb.vert", "./res/shader/fb.frag");
    resource_load_shader("text", "./res/shader/text.vert", "./res/shader/text.frag");

    resource_load_texture("background", "./res/image/background.jpg", 0);
    resource_load_texture("block", "./res/image/block.png", 0);
    resource_load_texture("block_solid", "./res/image/block_solid.png", 0);
    resource_load_texture("player", "./res/image/paddle.png", 1);
    resource_load_texture("ball", "./res/image/awesomeface.png", 1);
    resource_load_texture("particle", "./res/image/particle.png", 1);
    resource_load_texture("powerup_speed", "./res/image/powerup_speed.png", 1);
    resource_load_texture("powerup_sticky", "./res/image/powerup_sticky.png", 1);
    resource_load_texture("powerup_increase", "./res/image/powerup_increase.png", 1);
    resource_load_texture("powerup_confuse", "./res/image/powerup_confuse.png", 1);
    resource_load_texture("powerup_chaos", "./res/image/powerup_chaos.png", 1);
    resource_load_texture("powerup_passthrough", "./res/image/powerup_passthrough.png", 1);

    game_level_init(&level);
    game_level_load(&level, "./res/level/standard.txt", game->platform.window.width, game->platform.window.height / 2);

    Shader *sprite_shader = resource_get_shader("sprite");
    Shader *particle_shader = resource_get_shader("particle");
    Shader *text_shader = resource_get_shader("text");
    mat4x4 projection;
    mat4x4_ortho(projection, 0.0f, game->platform.window.width, game->platform.window.height, 0.0f, -1.0f, 1.0f);
    shader_use(sprite_shader);
    shader_set_mat4(sprite_shader, "projection", projection);
    shader_set_int(sprite_shader, "image", 0);
    shader_use(particle_shader);
    shader_set_mat4(particle_shader, "projection", projection);
    shader_set_int(particle_shader, "image", 0);
    shader_use(text_shader);
    shader_set_mat4(text_shader, "projection", projection);

    sprite_renderer_init(&renderer, sprite_shader);
    pg_init(&pg, resource_get_shader("particle"), resource_get_texture("particle"), 500);
    post_processor_init(&effects, resource_get_shader("framebuffer"), game->platform.window.fb_width, game->platform.window.fb_height);
    powerups_init(&ps);
    text_renderer_init(&tr, resource_get_shader("text"), game->platform.window.width, game->platform.window.height);

    text_renderer_load(&tr, "./res/font/OCRAEXT.ttf", 24);
    
    // Init player paddle
    vec2 player_size = {100.0f, 20.0f};
    vec2 player_pos = {game->platform.window.width / 2.0f - player_size[0] / 2.0f, game->platform.window.height - player_size[1]};
    game_object_init(&player, player_pos, player_size, resource_get_texture("player"), (vec3){1.0f, 1.0f, 1.0f}, (vec2){500.0f, 500.0f});

    // Ball
    float ball_radius = 12.5f;
    vec2 ball_velocity = {100.0f, -350.0f};
    vec2 ball_pos = {player_pos[0] + player_size[0] / 2.0f - ball_radius, player_pos[1] - ball_radius * 2.0f};
    ball_init(&ball, ball_pos, ball_radius, ball_velocity, resource_get_texture("ball"));

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
    }

    ma_engine_play_sound(&engine, "./res/audio/breakout.mp3", NULL);
}

void game_update(Game *game) {
    platform_poll_events(&game->platform);

    if (game->state == GAME_ACTIVE) {
        ball_move(&ball, game->platform.time.delta, game->platform.window.width, game->platform.window.height);

        do_collisions(game);

        pg_update(&pg, &ball.object, game->platform.time.delta, 300.0f, (vec2){ball.radius, ball.radius});

        powerups_update(&ps, game->platform.time.delta, game->platform.window.height);

        float player_velocity = player.velocity[0] * game->platform.time.delta;

        if (game->platform.input.keys[GLFW_KEY_ESCAPE]) {
            game->state = GAME_CLOSE;
        }
        if (game->platform.input.keys[GLFW_KEY_A]) {
            if (player.position[0] >= 0.0f) {
                player.position[0] -= player_velocity;
                if(ball.stuck)
                    ball.object.position[0] -= player_velocity;
            }
        }
        if (game->platform.input.keys[GLFW_KEY_D]) {
            if (player.position[0] <= game->platform.window.width - player.size[0]) {
                player.position[0] += player_velocity;
                if(ball.stuck)
                    ball.object.position[0] += player_velocity;
            }
        }
        if (game->platform.input.keys[GLFW_KEY_SPACE]) {
            ball.stuck = 0;
        }

        if (effects.shake) {
            shake_time -= game->platform.time.delta;
            if (shake_time <= 0.0f)
                effects.shake = 0;
        }
    } else if (game->state == GAME_MENU) {
        if (game->platform.input.keys[GLFW_KEY_ESCAPE]) {
            game->state = GAME_CLOSE;
        }
        if (game->platform.input.keys[GLFW_KEY_W]) {
            selected_level = (selected_level - 1 + level_count) % level_count;
            game_level_load(&level, level_files[selected_level], game->platform.window.width, game->platform.window.height / 2);
        }

        if (game->platform.input.keys[GLFW_KEY_S]) {
            selected_level = (selected_level + 1) % level_count;
            game_level_load(&level, level_files[selected_level], game->platform.window.width, game->platform.window.height / 2);
        }
        if (game->platform.input.keys[GLFW_KEY_ENTER]) {
            game->state = GAME_ACTIVE;
            game_level_load(&level, level_files[selected_level], game->platform.window.width, game->platform.window.height / 2);
        }
    }
}

void game_render(Game *game) {
    post_processor_begin_render(&effects);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sprite_renderer_draw(&renderer, resource_get_texture("background"), (vec2){0.0f, 0.0f}, (vec2){game->platform.window.width, game->platform.window.height}, 0.0f, (vec3){1.0f, 1.0f, 1.0f});
    if (game->state == GAME_ACTIVE || game->state  == GAME_MENU) {
        game_level_draw(&level, &renderer);
        game_object_draw(&player, &renderer);
        pg_draw(&pg);
        game_object_draw(&ball.object, &renderer);
        powerups_draw(&ps, &renderer);
    }

    post_processor_end_render(&effects);
    post_processor_render(&effects, game->platform.time.delta);

    if (game->state == GAME_ACTIVE) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Lives: %d", lives);
        text_renderer_render(&tr, buffer, 5.0f, 5.0f, 1.0f, (vec3){1.0f, 1.0f, 1.0f});
    }

    if (game->state == GAME_MENU) {
        text_renderer_render(&tr, "Press ENTER to start", 250.0f, game->platform.window.height / 2.0f, 1.0f, (vec3){1.0f, 1.0f, 1.0f});
        text_renderer_render(&tr, "Press W or S to select level", 245.0f, game->platform.window.height / 2.0f + 20.0f, 0.75f, (vec3){1.0f, 1.0f, 1.0f});
    }

    platform_swap_buffers(&game->platform);
}

void game_destroy(Game *game) {
    ma_engine_uninit(&engine);
    sprite_renderer_destroy(&renderer);
    pg_free(&pg);
    platform_destroy(&game->platform);
    powerups_destroy(&ps);
}

static void do_collisions(Game *game) {
    for (int i = 0; i < level.brick_count; i++) {
        GameObject *brick = &level.bricks[i];
        if (!brick->is_destroyed) {
            Collision collision = aabb_circle(&ball, brick);
            if(collision.is_collision) {
                if(!brick->is_solid) {
                    brick->is_destroyed = 1;
                    powerups_spawn(&ps, brick);
                    ma_engine_play_sound(&engine, "./res/audio/bleep.mp3", NULL);
                } else {
                    shake_time = 0.05f;
                    effects.shake = 1;
                    ma_engine_play_sound(&engine, "./res/audio/solid.wav", NULL);
                }

                Direction dir = collision.direction;
                vec2 diff;
                vec2_dup(diff, collision.difference);

                if (ball.pass_through && !brick->is_solid) continue;
                if (dir == DIRECTION_LEFT || dir == DIRECTION_RIGHT) {
                    ball.object.velocity[0] = -ball.object.velocity[0];
                    float penetration = ball.radius - fabs(diff[0]);
                    if (dir == DIRECTION_LEFT)
                        ball.object.position[0] += penetration;
                    else
                        ball.object.position[0] -= penetration;
                } else {
                    ball.object.velocity[1] = -ball.object.velocity[1];
                    float penetration = ball.radius - fabs(diff[1]);
                    if (dir == DIRECTION_DOWN)
                        ball.object.position[1] += penetration;
                    else
                        ball.object.position[1] -= penetration;
                }
            }
        }
    }

    // Paddle collision
    Collision paddle_collision = aabb_circle(&ball, &player);
    if (!ball.stuck && paddle_collision.is_collision) {
        float center_board = player.position[0] + player.size[0] / 2.0f;
        float distance = (ball.object.position[0] + ball.radius) - center_board;
        float percentage = distance / (player.size[0] / 2.0f);
        float strength = 2.0f;

        vec2 old_velocity;
        vec2_dup(old_velocity, ball.object.velocity);

        ball.object.velocity[0] = 100.0f * percentage * strength;
        ball.object.velocity[1] = -fabsf(old_velocity[1]);

        vec2 norm;
        vec2_dup(norm, ball.object.velocity);
        vec2_norm(norm, norm);
        vec2_scale(ball.object.velocity, norm, vec2_len(old_velocity));

        ball.stuck = ball.sticky;
        ma_engine_play_sound(&engine, "./res/audio/bleep.wav", NULL);
    }

    // Floor collision
    if (ball.object.position[1] >= game->platform.window.height) {
        lives--;
        if (lives == 0) {
            game_level_load(&level, level_files[selected_level], game->platform.window.width, game->platform.window.height / 2);
            lives = 3;
            game->state = GAME_MENU;
        }
        ball_reset(&ball);
        player.position[0] = (float)(game->platform.window.width / 2.0f - player.size[0] / 2.0f);
    }

    powerups_collision(&ps, &player);
}
