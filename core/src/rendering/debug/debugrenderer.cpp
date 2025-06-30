#include "rendering/shader.h"
#include "rendering/texture3d.h"
#include <GL/glew.h>
#include <GL/gl.h>

extern int viewportWidth, viewportHeight;
extern Texture3D* fontTexture;
extern Shader* fontShader;

void renderChar(char c, int x, int y, float scale=1.f) {
    fontShader->use();
    fontTexture->activate(1);
    fontShader->set("fontTex", 1);

    fontShader->set("charIndex", (int)c);

    // https://stackoverflow.com/a/10631263
    int tex = fontShader->getAttribute("aTexCoord");

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

void renderString(std::string str, int x, int y, float scale=1.f) {
    for (int i = 0; i < (int)str.length(); i++) {
        char c = str[i];
        renderChar(c, x+i*8*scale, y, scale);
    }
}

// Draws debug info window
// Including info about framerates, etc.
void renderDebugInfo() {
    // renderString("Hello, test!", 50, 50, 2);
    renderString("Hello, test!", 0, 0, 1);
}