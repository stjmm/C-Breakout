#include "game_level.h"
#include "resource_manager.h"
#include "game_object.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void game_level_init(GameLevel *level) {
    level->brick_count = 0;
}

void game_level_draw(GameLevel *level, SpriteRenderer *renderer) {
    for (int i = 0; i < level->brick_count; i++) {
        if(!level->bricks[i].is_destroyed)
            game_object_draw(&level->bricks[i], renderer);
    }
}

int game_level_is_completed(GameLevel *level) {
    for (int i = 0; i < level->brick_count; ++i) {
        if (!level->bricks[i].is_solid && !level->bricks[i].is_destroyed)
            return 0;
    }
    return 1;
}

static void init_level(GameLevel *level, int tile_data[MAX_LEVEL_ROWS][MAX_LEVEL_COLS], int rows, int cols, int level_width, int level_height) {
    level->brick_count = 0;

    float unit_width = level_width / (float)cols;
    float unit_height = level_height / (float)rows;

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int tile = tile_data[y][x];
            if (tile == 0) continue;

            vec2 pos = {unit_width * x, unit_height * y};
            vec2 size = {unit_width, unit_height};
            vec3 color = {1.0f, 1.0f, 1.0f};
            Texture *tex = NULL;
            int is_solid = 0;

            if (tile == 1) {
                tex = resource_get_texture("block_solid");
                vec3_dup(color, (vec3){0.8f, 0.8f, 0.7f});
                is_solid = 1;
            } else {
                tex = resource_get_texture("block");
                if (tile == 2) vec3_dup(color, (vec3){0.2f, 0.6f, 1.0f});
                else if (tile == 3) vec3_dup(color, (vec3){0.0f, 0.7f, 0.0f});
                else if (tile == 4) vec3_dup(color, (vec3){0.8f, 0.8f, 0.4f});
                else if (tile == 5) vec3_dup(color, (vec3){1.0f, 0.5f, 0.0f});
            }

            GameObject *brick = &level->bricks[level->brick_count++];
            game_object_init(brick, pos, size, tex, color, (vec2){0.0f, 0.0f});
            brick->is_solid = is_solid;
        }
    }
}

void game_level_load(GameLevel *level, const char *path, int level_width, int level_height) {
    FILE *file = fopen(path, "rb");
    if (!file)
        ERROR_EXIT("Failed to open level file: %s", path);

    int tile_data[MAX_LEVEL_ROWS][MAX_LEVEL_COLS] = {0};
    int rows = 0, cols = 0;

    char line[128];
    while (fgets(line, sizeof(line), file) && rows < MAX_LEVEL_ROWS) {
        int col = 0;
        char *token = strtok(line, " \t\r\n");
        while (token && col < MAX_LEVEL_COLS) {
            tile_data[rows][col++] = atoi(token);
            token = strtok(NULL, " \t\r\n");
        }

        if (col > cols) cols = col;
        rows++;
    }

    fclose(file);
    init_level(level, tile_data, rows, cols, level_width, level_height);
}
