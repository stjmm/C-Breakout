#pragma once

#include "shader.h"

typedef struct {
    unsigned int msfbo, fbo, rbo;
    unsigned int vao;
    Shader *shader;
    unsigned int texture;
    int width, height;
    int confuse, chaos, shake;
} PostProcessor;

void post_processor_init(PostProcessor *pp, Shader *shader, int width, int height);
void post_processor_begin_render(PostProcessor *pp);
void post_processor_end_render(PostProcessor *pp);
void post_processor_render(PostProcessor *pp, float time);
