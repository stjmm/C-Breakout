#pragma once

#include <linmath.h>

#include "shader.h"

#define MAX_CHARACTERS 128

typedef struct {
    unsigned int texture_id;
    vec2 size;
    vec2 bearing;
    int advance;
    
} Character;

typedef struct {
    Character characters[MAX_CHARACTERS];
    Shader *text_shader;
    unsigned int vao, vbo;
} TextRenderer;

void text_renderer_init(TextRenderer *tr, Shader *shader, int width, int height);
void text_renderer_load(TextRenderer *tr, const char *font_path, int font_size);
void text_renderer_render(TextRenderer *tr, const char *text, float x, float y, float scale, vec3 color);
