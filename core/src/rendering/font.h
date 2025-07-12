#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

// https://learnopengl.com/In-Practice/Text-Rendering

struct Character {
    unsigned int ID;  // ID handle of the glyph texture
    glm::ivec2   size;       // Size of glyph
    glm::ivec2   bearing;    // Offset from baseline to left/top of glyph
    unsigned int advance;    // Offset to advance to next glyph
};

struct Font {
    unsigned int height;
    Character characters[128];
};

void fontInit();
void fontFinish();
std::shared_ptr<Font> loadFont(std::string fontName);
void drawText(std::shared_ptr<Font> font, std::string text, float x, float y, float scale=1.f, glm::vec3 color = glm::vec3(1,1,1));
float calcTextWidth(std::shared_ptr<Font> font, std::string text, float scale = 1.f);