#include <glad/glad.h>
#include <stdlib.h>

#include "particle.h"
#include "linmath.h"
#include "shader.h"
#include "texture.h"

static void respawn(Particle *p, GameObject *o, vec2 offset) {
    float random = ((rand() % 100) - 50) / 10.0f;
    float shade = 0.5f + (rand() % 100) / 100.0f;
    
    vec2_add(p->position, o->position, offset);
    p->position[0] += random; p->position[1] += random;

    p->color[0] = p->color[1] = p->color[2] = shade;
    p->color[3] = 1.0f;
    p->life = 1.0f;

    vec2_scale(p->velocity, o->velocity, -0.1f);
}

void pg_init(ParticleGenerator *pg, Shader *shader, Texture *texture, int max_count) {
    memset(pg, 0, sizeof *pg);
    pg->capacity = max_count;
    pg->particles = calloc(max_count, sizeof(Particle));
    pg->shader = shader;
    pg->texture = texture;

    float quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    unsigned int vbo;
    glGenVertexArrays(1, &pg->vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(pg->vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void pg_update(ParticleGenerator *pg, GameObject *emitter, float dt, float rate, vec2 offset) {
    int new_particles = (int)(dt * rate);
    while (new_particles--) {
        respawn(&pg->particles[pg->first_dead], emitter, offset);
        pg->first_dead = (pg->first_dead + 1) % pg->capacity;
    }

    for (int i = 0; i < pg->capacity; i++) {
        Particle *p = &pg->particles[i];
        if (p->life <= 0.0f) continue;

        p->life -= dt;
        if (p->life <= 0.0f) continue;

        vec2 tmp;
        vec2_scale(tmp, p->velocity, dt);
        vec2_add(p->position, p->position, tmp);
        p->color[3] = p->life;
    }
}

void pg_draw(ParticleGenerator *pg) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    shader_use(pg->shader);

    glActiveTexture(GL_TEXTURE0);
    texture_bind(pg->texture);
    glBindVertexArray(pg->vao);

    for (int i = 0; i < pg->capacity; i++) {
        Particle *p = &pg->particles[i];
        if (p->life <= 0.0f) continue;

        shader_set_vec2(pg->shader, "offset", p->position);
        shader_set_vec4(pg->shader, "particle_color", p->color);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void pg_free(ParticleGenerator *pg) {
    free(pg->particles);
    pg->particles = NULL;
}
