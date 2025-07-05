#include <glad/glad.h>
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "util.h"

void texture_load(Texture *texture, const char *path, int alpha) {
    int channels;

    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(path, &texture->width, &texture->height, &channels, 0);
    if (!data)
        ERROR_EXIT("Failed to load texture: %s", path);

    GLenum internal_format;
    GLenum image_format;
    if (alpha) {
        internal_format = GL_RGBA;
        image_format = GL_RGBA;
    } else {
        internal_format = GL_RGB;
        image_format = GL_RGB;
    }

    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture->width, texture->height, 0, image_format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

void texture_bind(Texture *texture) {
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void texture_destroy(Texture *texture) {
    glDeleteTextures(1, &texture->id);
    texture->id = 0;
}
