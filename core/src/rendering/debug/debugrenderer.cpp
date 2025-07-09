#include "rendering/shader.h"
#include "rendering/texture.h"
#include "timeutil.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/ext/vector_float4.hpp>
#include <string>

extern int viewportWidth, viewportHeight;
extern Texture* debugFontTexture;
extern Shader* debugFontShader;
extern Shader* identityShader;

void drawChar(char c, int x, int y, float scale=1.f) {
    debugFontShader->use();
    debugFontTexture->activate(1);
    debugFontShader->set("fontTex", 1);

    debugFontShader->set("charIndex", (int)c);

    // https://stackoverflow.com/a/10631263
    int tex = debugFontShader->getAttribute("aTexCoord");

    y = viewportHeight - y - 16*scale;
    float x0 = float(x)/viewportWidth, y0 = float(y)/viewportHeight, x1 = ((float)x + 8*scale)/viewportWidth, y1 = ((float)y + 16*scale)/viewportHeight;
    x0 *= 2, y0 *= 2, x1 *= 2, y1 *= 2;
    x0 -= 1, y0 -= 1, x1 -= 1, y1 -= 1;

    glBegin(GL_QUADS);
        glVertex3f(x0, y0, 0);      glVertexAttrib2f(tex, 1, 1); 
        glVertex3f(x1, y0, 0);      glVertexAttrib2f(tex, 1, 0); 
        glVertex3f(x1, y1, 0);      glVertexAttrib2f(tex, 0, 0); 
        glVertex3f(x0, y1, 0);      glVertexAttrib2f(tex, 0, 1); 
    glEnd();
}

void drawString(std::string str, int x, int y, float scale=1.f) {
    for (int i = 0; i < (int)str.length(); i++) {
        char c = str[i];
        drawChar(c, x+i*8*scale, y, scale);
    }
}

void drawRect(int x, int y, int w, int h, glm::vec4 color) {
    identityShader->use();
    identityShader->set("aColor", color);

    float x0 = 2*float(x)/viewportWidth-1, y0 = 2*float(y)/viewportHeight-1, x1 = 2*float(x + w)/viewportWidth-1, y1 = 2*float(y + h)/viewportHeight-1;
    float tmp;
    tmp = -y0, y0 = -y1, y1 = tmp;

    glBegin(GL_QUADS);
        glVertex3f(x0, y0, 0);
        glVertex3f(x1, y0, 0);
        glVertex3f(x1, y1, 0);
        glVertex3f(x0, y1, 0);
    glEnd();
}

static tu_time_t lastTime;
extern tu_time_t renderTime;
extern tu_time_t physTime;
extern tu_time_t schedTime;

// Draws debug info window
// Including info about framerates, etc.
void renderDebugInfo() {
    tu_time_t timePassed = tu_clock_micros() - lastTime;
    float frames = 1/(((float)timePassed)/1'000'000);

    glDisable(GL_DEPTH_TEST);

    drawRect(0, 0, 200, 16*8, glm::vec4(0.2f,0.2f,0.2f,0.8f));
    drawString("FPS: " + std::to_string((int)frames), 0, 16*0);
    drawString(" 1/: " + std::to_string((float)timePassed/1'000'000), 0, 16*1);

    frames = 1/(((float)renderTime)/1'000'000);
    drawString("RPS: " + std::to_string((int)frames), 0, 16*2);
    drawString(" 1/: " + std::to_string((float)renderTime/1'000'000), 0, 16*3);

    frames = 1/(((float)physTime)/1'000'000);
    drawString("PPS: " + std::to_string((int)frames), 0, 16*4);
    drawString(" 1/: " + std::to_string((float)physTime/1'000'000), 0, 16*5);

    frames = 1/(((float)schedTime)/1'000'000);
    drawString("SPS: " + std::to_string((int)frames), 0, 16*6);
    drawString(" 1/: " + std::to_string((float)schedTime/1'000'000), 0, 16*7);

    lastTime = tu_clock_micros();
}