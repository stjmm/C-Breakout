#include <glad/glad.h>
#include "sprite_renderer.h"
#include "shader.h"
#include "texture.h"

static void init_draw_data(SpriteRenderer *renderer) {
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,  0.0f, 0.0f, 

        0.0f, 1.0f,  0.0f, 1.0f, 
        1.0f, 1.0f,  1.0f, 1.0f, 
        1.0f, 0.0f,  1.0f, 0.0f
    };

    unsigned int vbo;
    glGenVertexArrays(1, &renderer->quad_vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(renderer->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void sprite_renderer_init(SpriteRenderer *renderer, Shader *shader) {
    renderer->shader = shader;
    init_draw_data(renderer);
}

void sprite_renderer_draw(SpriteRenderer *renderer, Texture *texture,
                          vec2 position, vec2 size, float rotate, vec3 color) {
    shader_use(renderer->shader);

    mat4x4 model;
    mat4x4_identity(model);

    mat4x4_translate(model, position[0], position[1], 0.0f);

    if (rotate != 0.0f) {
        mat4x4 center;
        mat4x4_identity(center);
        mat4x4_translate(center, 0.5f * size[0], 0.5f * size[1], 0.0f);

        mat4x4 rotate_z;
        mat4x4_rotate_Z(rotate_z, model, rotate);

        mat4x4_translate(model, -0.5f * size[0], -0.5f * size[1], 0.0f);
        mat4x4_mul(model, rotate_z, model);
    }

    mat4x4 scale;
    mat4x4_identity(scale);
    mat4x4_scale_aniso(scale, scale, size[0], size[1], 1.0f);
    mat4x4_mul(model, model, scale);

    shader_use(renderer->shader);
    shader_set_mat4(renderer->shader, "model", model);
    shader_set_vec3(renderer->shader, "sprite_color", color);

    glActiveTexture(GL_TEXTURE0);
    texture_bind(texture);

    glBindVertexArray(renderer->quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void sprite_renderer_destroy(SpriteRenderer *renderer) {
    glDeleteVertexArrays(1, &renderer->quad_vao);
}
