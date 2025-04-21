#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
};

class TextRenderer {
public:
    TextRenderer(unsigned int width, unsigned int height);
    ~TextRenderer();
    
    bool Load(std::string font, unsigned int fontSize);
    
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f));
    
private:
    GLuint shader;
    
    glm::mat4 projection;
    
    std::map<char, Character> Characters;
    
    GLuint VAO, VBO;
};

#endif