#include "font.h"
#include "logger.h"
#include "panic.h"
#include "rendering/assets.h"
#include "rendering/shader.h"

#include <glad/gl.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

// https://learnopengl.com/In-Practice/Text-Rendering

FT_Library freetype;
Shader* fontShader;

extern int viewportWidth, viewportHeight;

unsigned int textVAO, textVBO;

void fontInit() {
    if (FT_Error err = FT_Init_FreeType(&freetype)) {
        Logger::fatalErrorf("Failed to initialize Freetype: [%d]", err);
        panic();
    }

    fontShader = new Shader("assets/shaders/font.vs", "assets/shaders/font.fs");

    // Set up buffer
    glGenVertexArrays(1, &textVAO);
    glBindVertexArray(textVAO);

    glGenBuffers(1, &textVBO);    
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    // Dynamic, because we update the vertices often           V~~~~~~~~~~~~~~
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);      
}

void fontFinish() {
    if (FT_Error err = FT_Done_FreeType(freetype)) {
        Logger::fatalErrorf("Failed to free Freetype: [%d]", err);
        panic();
    }
}

static void loadCharTexture(FT_Face& face, FT_BitmapGlyph& glyph_bitmap, Character& character) {
    // Generate texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        glyph_bitmap->bitmap.width,
        glyph_bitmap->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        glyph_bitmap->bitmap.buffer
    );
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // now store character for later use
    character.ID = texture;
    character.size = glm::ivec2(glyph_bitmap->bitmap.width, glyph_bitmap->bitmap.rows);
    character.bearing = glm::ivec2(glyph_bitmap->left, glyph_bitmap->top);
    character.advance = (unsigned int)face->glyph->advance.x;
}

std::shared_ptr<Font> loadFont(std::string fontName) {
    std::string fontPath = resolveAssetPath("assets/font/" + fontName);

    FT_Face face;
    if (FT_Error err = FT_New_Face(freetype, fontPath.c_str(), 0, &face)) {
        Logger::fatalErrorf("Failed to create font '%s': [%d]", fontName.c_str(), err);
        panic();
    }

    std::shared_ptr<Font> font = std::make_shared<Font>();

    FT_Set_Pixel_Sizes(face, 0, 16);
    font->height = face->size->metrics.y_ppem;

    FT_Stroker stroker;
    FT_Stroker_New(freetype, &stroker);
    FT_Stroker_Set(stroker, 2 * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

    // Load each glyph
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph 
        if (FT_Error err = FT_Load_Char(face, c, FT_LOAD_DEFAULT)) {
            Logger::errorf("Failed to load glyph %d in font '%s': [%d]", c, fontName.c_str(), err);
            continue;
        }

        FT_Glyph glyph;
        FT_BitmapGlyph glyph_bitmap;
    
        Character character;

        // Render base
        FT_Get_Glyph(face->glyph, &glyph);
        FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
        glyph_bitmap = (FT_BitmapGlyph)glyph;

        loadCharTexture(face, glyph_bitmap, character);
        font->characters[c] = character;
        FT_Done_Glyph(glyph);
        // TODO: Find out how to clear FT_BitmapGlyph... I cant import FT_Bitmap_Done for some reason

        // Render stroked
        // https://stackoverflow.com/a/28078293/16255372
        FT_Get_Glyph(face->glyph, &glyph);
        FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
        FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
        glyph_bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);

        loadCharTexture(face, glyph_bitmap, character);
        font->strokeCharacters[c] = character;
        FT_Done_Glyph(glyph);
    }
    
    FT_Stroker_Done(stroker);
    FT_Done_Face(face);

    return font;
}

void drawText(std::shared_ptr<Font> font, std::string text, float x, float y, float scale, glm::vec3 color, bool drawStroke) {
    // activate corresponding render state
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // TODO: Figure out why when changed to GL_ONE this causes graphical errors

    fontShader->use();
    fontShader->set("textColor", color);
    fontShader->set("text", 0);
    glActiveTexture(GL_TEXTURE0);

    glm::mat4 projection = glm::ortho(0.0f, (float)viewportWidth, 0.0f, (float)viewportHeight);
    fontShader->set("projection", projection);

    // This is not in the learnopengl guide but it is VERY important
    // I'm surprised I missed it but honestly... not so much. I'm an idiot
    glBindVertexArray(textVAO);

    // iterate through all characters
    for (size_t i = 0; i < text.size(); i++) {
        unsigned char c = text[i];
        Character ch = drawStroke ? font->strokeCharacters[c] : font->characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = viewportHeight - y - font->height - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.ID);

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.ID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

float calcTextWidth(std::shared_ptr<Font> font, std::string text, float scale, bool stroke) {
    float x = 0;
    // iterate through all characters
    for (size_t i = 0; i < text.size(); i++) {
        unsigned char c = text[i];
        Character ch = stroke ? font->strokeCharacters[c] : font->characters[c];

        x += (ch.advance >> 6) * scale;
    }

    return x;
}