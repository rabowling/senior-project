#pragma once

#include <string>
#include <stdlib.h>
#include <map>
#include "GLSL.h"
#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    GLuint      TextureID;  // ID handle of the glyph texture
    glm::ivec2  Size;       // Size of glyph
    glm::vec2   Bearing;    // Offset from baseline to left/top of glyph
    GLuint      Advance;    // Offset to advance to next glyph
};

class HUD
{
    public:
        void draw();
        void init();
        void updateToolTip(std::string text) { tooltip = text; };

    private:
        std::string tooltip = "test";

        FT_Library ft;
	    FT_Face face;
	    std::map<GLchar, Character> Characters;
	    GLuint TextVAO, TextVBO;
        glm::vec3 color;
        GLfloat x;
        GLfloat y;
        GLfloat scale;

};