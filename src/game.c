#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <linmath.h>

#include "game.h"
#include "game_object.h"
#include "platform.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_level.h"
#include "shader.h"
#include "particle.h"

#include <math.h>
#include <string.h>

SpriteRenderer renderer;
GameLevel level;
GameObject player;
Ball ball;
ParticleGenerator pg;

static void do_collisions(Game *game);

void game_init(Game *game) {
    memset(game, 0, sizeof(Game));

    platform_init(&game->platform, 800, 600, "Breakout");
    game->state = GAME_ACTIVE;

    game_level_init(&level);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    resource_load_shader("sprite", "./res/shader/sprite.vert", "./res/shader/sprite.frag");
    resource_load_shader("particle", "./res/shader/particle.vert", "./res/shader/particle.frag");
    resource_load_texture("background", "./res/image/background.jpg", 0);
    resource_load_texture("block", "./res/image/block.png", 0);
    resource_load_texture("block_solid", "./res/image/block_solid.png", 0);
    resource_load_texture("player", "./res/image/paddle.png", 1);
    resource_load_texture("ball", "./res/image/awesomeface.png", 1);
    resource_load_texture("particle", "./res/image/particle.png", 1);

    game_level_load(&level, "./res/level/standard.txt", game->platform.window.width, game->platform.window.height / 2);

    Shader *sprite_shader = resource_get_shader("sprite");
    Shader *particle_shader = resource_get_shader("particle");
    mat4x4 projection;
    mat4x4_ortho(projection, 0.0f, game->platform.window.width, game->platform.window.height, 0.0f, -1.0f, 1.0f);
    shader_use(sprite_shader);
    shader_set_mat4(sprite_shader, "projection", projection);
    shader_set_int(sprite_shader, "image", 0);

    shader_use(particle_shader);
    shader_set_mat4(particle_shader, "projection", projection);
    shader_set_int(particle_shader, "image", 0);

    sprite_renderer_init(&renderer, sprite_shader);
    pg_init(&pg, resource_get_shader("particle"), resource_get_texture("particle"), 500);
    
    // Init player paddle
    vec2 player_size = {100.0f, 20.0f};
    vec2 player_pos = {game->platform.window.width / 2.0f - player_size[0] / 2.0f, game->platform.window.height - player_size[1]};
    game_object_init(&player, player_pos, player_size, resource_get_texture("player"), (vec3){1.0f, 1.0f, 1.0f}, (vec2){500.0f, 500.0f});

    float ball_radius = 12.5f;
    vec2 ball_velocity = {100.0f, -350.0f};
    vec2 ball_pos = {player_pos[0] + player_size[0] / 2.0f - ball_radius, player_pos[1] - ball_radius * 2.0f};
    ball_init(&ball, ball_pos, ball_radius, ball_velocity, resource_get_texture("ball"));
}

void game_update(Game *game) {
    platform_poll_events(&game->platform);

    ball_move(&ball, game->platform.time.delta, game->platform.window.width, game->platform.window.height);

    do_collisions(game);

    pg_update(&pg, &ball.object, game->platform.time.delta, 300.0f, (vec2){ball.radius, ball.radius});

    float player_velocity = player.velocity[0] * game->platform.time.delta;

    if(game->platform.input.keys[GLFW_KEY_ESCAPE]) {
        game->state = GAME_CLOSE;
    }
    if(game->platform.input.keys[GLFW_KEY_A]) {
        if (player.position[0] >= 0.0f) {
            player.position[0] -= player_velocity;
            if(ball.stuck)
                ball.object.position[0] -= player_velocity;
        }
    }
    if(game->platform.input.keys[GLFW_KEY_D]) {
        if (player.position[0] <= game->platform.window.width - player.size[0]) {
            player.position[0] += player_velocity;
            if(ball.stuck)
                ball.object.position[0] += player_velocity;
        }
    }
    if(game->platform.input.keys[GLFW_KEY_SPACE]) {
        ball.stuck = 0;
    }
}

void game_render(Game *game) {
    glClearColor(1.0f, 0.5f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shader *sprite_shader = resource_get_shader("sprite");
    shader_use(sprite_shader);

    // Background
    sprite_renderer_draw(&renderer, resource_get_texture("background"), (vec2){0.0f, 0.0f}, (vec2){game->platform.window.width, game->platform.window.height}, 0.0f, (vec3){1.0f, 1.0f, 1.0f});

    // Bricks
    game_level_draw(&level, &renderer);

    // Player
    game_object_draw(&player, &renderer);

    // Particles
    pg_draw(&pg);

    // Ball
    game_object_draw(&ball.object, &renderer);

    platform_swap_buffers(&game->platform);
}

void game_destroy(Game *game) {
    platform_destroy(&game->platform);
    pg_free(&pg);
}

static void do_collisions(Game *game) {
    for (int i = 0; i < level.brick_count; i++) {
        GameObject *brick = &level.bricks[i];
        if (!brick->is_destroyed) {
            Collision collision = aabb_circle(&ball, brick);
            if(collision.is_collision) {
                if(!brick->is_solid)
                    brick->is_destroyed = 1;

                Direction dir = collision.direction;
                vec2 diff;
                vec2_dup(diff, collision.difference);

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
    }

    // Floor collision
    if (ball.object.position[1] >= game->platform.window.height) {
        ball_reset(&ball);
        player.position[0] = (float)(game->platform.window.width / 2.0f - player.size[0] / 2.0f);
        game_level_load(&level, "./res/level/standard.txt", game->platform.window.width, game->platform.window.height / 2);
    }
}
