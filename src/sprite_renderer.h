#pragma once

#include <linmath.h>

#include "shader.h"
#include "texture.h"

typedef struct {
    Shader *shader;
    unsigned int quad_vao;
} SpriteRenderer;

void sprite_renderer_init(SpriteRenderer *renderer, Shader *shader);
void sprite_renderer_draw(SpriteRenderer *renderer, Texture *texture,
                          vec2 position, vec2 size, float rotate, vec3 color);
void sprite_renderer_destroy(SpriteRenderer *renderer);
