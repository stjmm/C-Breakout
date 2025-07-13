#include <glad/glad.h>

#include "post_processor.h"
#include "texture.h"
#include "util.h"

static void init_render_data(PostProcessor *pp) {
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,

        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 1.0f, 1.0f
    };

    unsigned int vbo;
    glGenVertexArrays(1, &pp->vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(pp->vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void post_processor_init(PostProcessor *pp, Shader *shader, int width, int height) {
    pp->shader = shader;
    pp->width = width;
    pp->height = height;

    pp->confuse = pp->shake = pp->chaos = 0;

    glGenFramebuffers(1, &pp->msfbo);
    glGenFramebuffers(1, &pp->fbo);
    glGenRenderbuffers(1, &pp->rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, pp->msfbo);
    glBindRenderbuffer(GL_RENDERBUFFER, pp->rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, pp->rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
       ERROR_EXIT("Failed to initialize MSFBO.\n");

    glBindFramebuffer(GL_FRAMEBUFFER, pp->fbo);
    glGenTextures(1, &pp->texture);
    glBindTexture(GL_TEXTURE_2D, pp->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pp->texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        ERROR_EXIT("Failed to initialize MSFBO.\n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    init_render_data(pp);

    shader_use(pp->shader);
    shader_set_int(pp->shader, "scene", 0);
    float offset = 1.0f / 300.0f;
    float offsets[9][2] = {
        { -offset,  offset  },  // top-left
        {  0.0f,    offset  },  // top-center
        {  offset,  offset  },  // top-right
        { -offset,  0.0f    },  // center-left
        {  0.0f,    0.0f    },  // center-center
        {  offset,  0.0f    },  // center - right
        { -offset, -offset  },  // bottom-left
        {  0.0f,   -offset  },  // bottom-center
        {  offset, -offset  }   // bottom-right
    };
    glUniform2fv(glGetUniformLocation(pp->shader->id, "offsets"), 9, (float *)offsets);
    int edge_kernel[9] = {
        -1, -1, -1,
        -1,  8, -1,
        -1, -1, -1
    };
    glUniform1iv(glGetUniformLocation(pp->shader->id, "edge_kernel"), 9, edge_kernel);
    float blur_kernel[9] = {
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
        2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
    };
    glUniform1fv(glGetUniformLocation(pp->shader->id, "blur_kernel"), 9, blur_kernel);
}

void post_processor_begin_render(PostProcessor *pp) {
    glBindFramebuffer(GL_FRAMEBUFFER, pp->msfbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void post_processor_render(PostProcessor *pp, float time) {
    shader_use(pp->shader);
    shader_set_float(pp->shader, "time", time);
    shader_set_int(pp->shader, "confuse", pp->confuse);
    shader_set_int(pp->shader, "chaos", pp->chaos);
    shader_set_int(pp->shader, "shake", pp->shake);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pp->texture);
    glBindVertexArray(pp->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void post_processor_end_render(PostProcessor *pp) {
    glViewport(0, 0, pp->width, pp->height);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, pp->msfbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pp->fbo);
    glBlitFramebuffer(0, 0, pp->width, pp->height, 0, 0, pp->width, pp->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
