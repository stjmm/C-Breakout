#pragma once

#include "shader.h"
#include "texture.h"

void resource_load_shader(const char *name, const char *vs_path, const char *fs_path);
void resource_use_shader(const char *name);
Shader *resource_get_shader(const char *name);

void resource_load_texture(const char *name, const char *path, int alpha);
void resource_bind_texture(const char *name);
Texture *resource_get_texture(const char *name);

void resource_destroy(void);
