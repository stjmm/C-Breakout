#include <glad/glad.h>
#include <freetype/freetype.h>
#include <stdio.h>
#include FT_FREETYPE_H

#include "text_renderer.h"
#include "util.h"

void text_renderer_init(TextRenderer *tr, Shader *shader, int width, int height) {
    tr->text_shader = shader;
    shader_use(tr->text_shader);
    shader_set_int(tr->text_shader, "text", 0);

    glGenVertexArrays(1, &tr->vao);
    glGenBuffers(1, &tr->vbo);
    glBindVertexArray(tr->vao);
    glBindBuffer(GL_ARRAY_BUFFER, tr->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void text_renderer_load(TextRenderer *tr, const char *font_path, int font_size) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        ERROR_EXIT("Failed to load FreeType library.");
    }

    FT_Face face;
    if (FT_New_Face(ft, font_path, 0, &face)) {
        ERROR_EXIT("Failed to load font: %s", font_path);
        return;
    }

    
    FT_Set_Pixel_Sizes(face, 0, font_size);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < MAX_CHARACTERS; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            ERROR_EXIT("Failed to load Glyph %c\n", c);
        }

        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character ch = {
            tex,
            { face->glyph->bitmap.width, face->glyph->bitmap.rows },
            { face->glyph->bitmap_left, face->glyph->bitmap_top },
            face->glyph->advance.x
        };

        tr->characters[c] = ch;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void text_renderer_render(TextRenderer* tr, const char* text, float x, float y, float scale, float color[3]) {
    shader_use(tr->text_shader);
    shader_set_vec3(tr->text_shader, "text_color", color);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(tr->vao);

    Character H = tr->characters['H'];

    for (size_t i = 0; i < strlen(text); ++i) {
        Character ch = tr->characters[(unsigned char)text[i]];

        float xpos = x + ch.bearing[0] * scale;
        float ypos = y + (H.bearing[1] - ch.bearing[1]) * scale;

        float w = ch.size[0] * scale;
        float h = ch.size[1] * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos,     ypos,       0.0f, 0.0f },

            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, tr->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
