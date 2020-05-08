#include "HUD.h"

#include <stdlib.h>
#include <iostream>
#include "Application.h"

using namespace std;
using namespace glm;

void HUD::init() {
    // int pid = app.shaderManager.getPid("freetype");
    // int texloc = glGetUniformLocation(pid, "text");
    // app.shaderManager.bind("freetype");
    // glUniform1i(texloc, 6);
    // app.shaderManager.unbind();

    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }
    if (FT_New_Face(ft, "../resources/fonts/zorque.ttf", 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (GLubyte c = 0; c < 128; c++) {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        CHECKED_GL_CALL(glGenTextures(1, &texture));
        CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, texture));
        CHECKED_GL_CALL(glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        ));
        // Set texture options
        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        // Now store character for later use
        Character character = {
            texture, 
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    CHECKED_GL_CALL(glGenVertexArrays(1, &TextVAO));
    CHECKED_GL_CALL(glGenBuffers(1, &TextVBO));
    CHECKED_GL_CALL(glBindVertexArray(TextVAO));
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, TextVBO));
    CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW));
    CHECKED_GL_CALL(glEnableVertexAttribArray(0));
    CHECKED_GL_CALL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0));
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECKED_GL_CALL(glBindVertexArray(0)); 

    color = vec3(1,1,1);
    x = app.width / 2;
    y = app.height / 2;
    scale = 1.0f;
}

void HUD::draw() {
    app.shaderManager.bind("freetype");
    CHECKED_GL_CALL(glEnable(GL_BLEND));
    CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    CHECKED_GL_CALL(glUniform3f(app.shaderManager.getUniform("textColor"), color.x, color.y, color.z));
    CHECKED_GL_CALL(glUniformMatrix4fv(app.shaderManager.getUniform("projection"), 1, GL_FALSE, value_ptr(app.orthoProjection))); 
    CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));
    CHECKED_GL_CALL(glBindVertexArray(TextVAO));

    cout << tooltip << endl;

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = tooltip.begin(); c != tooltip.end(); c++) {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update TextVBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };
        // Render glyph texture over quad
        CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, ch.TextureID));
        // Update content of TextVBO memory
        CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, TextVBO));
        CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices)); 
        CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        // Render quad
        CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    CHECKED_GL_CALL(glBindVertexArray(0));
    CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    CHECKED_GL_CALL(glDisable(GL_BLEND));
    app.shaderManager.unbind();
}

