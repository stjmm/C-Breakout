#pragma once

#include <linmath.h>

typedef struct {
    unsigned int id;
} Shader;

void shader_compile(Shader *shader, const char *vs_path, const char *fs_path);
void shader_use(Shader *shader);
void shader_destroy(Shader *shader);

void shader_set_int(Shader *shader, const char *name, int value);
void shader_set_float(Shader *shader, const char *name, float value);
void shader_set_vec2(Shader *shader, const char *name, vec2 value);
void shader_set_vec3(Shader *shader, const char *name, vec3 value);
void shader_set_vec4(Shader *shader, const char *name, vec4 value);
void shader_set_mat4(Shader *shader, const char *name, mat4x4 value);
