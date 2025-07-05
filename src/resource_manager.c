#include "resource_manager.h"

#include <string.h>

#include "util.h"
#include "shader.h"
#include "texture.h"

#define MAX_RESOURCES 64

static Shader shaders[MAX_RESOURCES];
static const char *shader_names[MAX_RESOURCES];
static int shader_count = 0;

static Texture textures[MAX_RESOURCES];
static const char *texture_names[MAX_RESOURCES];
static int texture_count = 0;

void resource_load_shader(const char *name, const char *vs_path, const char *fs_path) {
    if (shader_count >= MAX_RESOURCES) {
        ERROR_EXIT("ResourceManager: Exceeded maximum number of shaders (%d)\n", MAX_RESOURCES);
    }

    shader_compile(&shaders[shader_count], vs_path, fs_path);
    shader_names[shader_count] = strdup(name);
    shader_count++;
}

void resource_use_shader(const char *name) {
    for (int i = 0; i < shader_count; ++i) {
        if (strcmp(shader_names[i], name) == 0) {
            shader_use(&shaders[i]);
            return;
        }
    }
    ERROR_EXIT("ResourceManager: Shader '%s' not found\n", name);
}

Shader *resource_get_shader(const char *name) {
    for (int i = 0; i < shader_count; ++i) {
        if (strcmp(shader_names[i], name) == 0) {
            return &shaders[i];
        }
    }
    ERROR_EXIT("ResourceManager: Shader '%s' not found\n", name);
}

void resource_load_texture(const char *name, const char *path, int alpha) {
    if (texture_count >= MAX_RESOURCES) {
        ERROR_EXIT("ResourceManager: Exceeded maximum number of textures (%d)\n", MAX_RESOURCES);
    }

    texture_load(&textures[texture_count], path, alpha);
    texture_names[texture_count] = strdup(name);
    texture_count++;
}

void resource_bind_texture(const char *name) {
    for (int i = 0; i < texture_count; ++i) {
        if (strcmp(texture_names[i], name) == 0) {
            texture_bind(&textures[i]);
            return;
        }
    }
    ERROR_EXIT("ResourceManager: Texture '%s' not found\n", name);
}

Texture *resource_get_texture(const char *name) {
    for (int i = 0; i < texture_count; ++i) {
        if (strcmp(texture_names[i], name) == 0) {
            return &textures[i];
        }
    }
    ERROR_EXIT("ResourceManager: Shader '%s' not found\n", name);
}
