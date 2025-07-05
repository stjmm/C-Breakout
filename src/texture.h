#pragma once

typedef struct {
    unsigned int id;
    int width, height;
} Texture;

void texture_load(Texture *texture, const char *path, int alpha);
void texture_bind(Texture *texture);
void texture_destroy(Texture *texture);
