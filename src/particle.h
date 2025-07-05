#pragma once

#include "game_object.h"
#include "shader.h"
#include "texture.h"

#include <linmath.h>

typedef struct {
    vec2 position;
    vec2 velocity;
    vec4 color;
    float life; // <=0 - dead
} Particle;

typedef struct {
    Particle *particles;
    int capacity;
    int first_dead;

    Shader *shader;
    Texture *texture;
    unsigned int vao;
} ParticleGenerator;

void pg_init(ParticleGenerator *pg, Shader *shader, Texture *texture, int max_count);
void pg_update(ParticleGenerator *pg, GameObject *emitter, float dt, float rate, vec2 offset);
void pg_draw(ParticleGenerator *pg);
void pg_free(ParticleGenerator *pg);
