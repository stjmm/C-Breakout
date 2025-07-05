#include <glad/glad.h>
#include "shader.h"

#include <string.h>

#include "util.h"

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file)
        ERROR_EXIT("Failed to open file: %s\n", path);

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *data = malloc(size + 1);

    fread(data, 1, size, file);
    data[size] = '\0';
    fclose(file);

    return data;
}

static void check_compile_errors(unsigned int object, const char *type) {
    int success;
    char log[1024];
    if (strcmp(type, "PROGRAM") == 0) {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, NULL, log);
            ERROR_EXIT("SHADER ERROR. LINKING ERROR. TYPE: %s\n%s", type, log);
        }
    } else {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, log);
            ERROR_EXIT("SHADER ERROR. COMPILE ERROR. TYPE: %s\n%s", type, log);
        }
    }
}

void shader_compile(Shader *shader, const char *vs_path, const char *fs_path) {
    const char *vs_source = read_file(vs_path);
    const char *fs_source = read_file(fs_path);

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vs_source, NULL);
    glCompileShader(vertex);
    check_compile_errors(vertex, "VERTEX");

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fs_source, NULL);
    glCompileShader(fragment);
    check_compile_errors(fragment, "FRAGMENT");

    shader->id = glCreateProgram();
    glAttachShader(shader->id, vertex);
    glAttachShader(shader->id, fragment);
    glLinkProgram(shader->id);
    check_compile_errors(shader->id, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    free((char *)vs_source);
    free((char *)fs_source);
}

void shader_use(Shader *shader) {
    glUseProgram(shader->id);
}

void shader_destroy(Shader *shader) {
    glDeleteShader(shader->id);
}

void shader_set_int(Shader *shader, const char *name, int value) {
    glUniform1i(glGetUniformLocation(shader->id, name), value);
}

void shader_set_float(Shader *shader, const char *name, float value) {
    glUniform1f(glGetUniformLocation(shader->id, name), value);
}

void shader_set_vec2(Shader *shader, const char *name, vec2 value) {
    glUniform2fv(glGetUniformLocation(shader->id, name), 1, value);
}

void shader_set_vec3(Shader *shader, const char *name, vec3 value) {
    glUniform3fv(glGetUniformLocation(shader->id, name), 1, value);
}

void shader_set_vec4(Shader *shader, const char *name, vec4 value) {
    glUniform4fv(glGetUniformLocation(shader->id, name), 1, value);
}

void shader_set_mat4(Shader *shader, const char *name, mat4x4 value) {
    glUniformMatrix4fv(glGetUniformLocation(shader->id, name), 1, GL_FALSE, &value[0][0]);
}
