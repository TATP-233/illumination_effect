#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

class Light 
{
public:
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float intensity;
    glm::vec3 direction;
    
    Light(glm::vec3 pos = glm::vec3(0.0f, 1.0f, -1.0f)) 
        : position(pos), 
          ambient(glm::vec3(0.2f, 0.2f, 0.2f)), 
          diffuse(glm::vec3(0.5f, 0.5f, 0.5f)), 
          specular(glm::vec3(1.0f, 1.0f, 1.0f)),
          intensity(1.0f),
          direction(glm::vec3(0.0f, -1.0f, 0.0f))
    {
    }
    
    void setUniforms(Shader &shader) 
    {
        shader.setVec3("light.position", position);
        shader.setVec3("light.ambient", ambient * intensity);
        shader.setVec3("light.diffuse", diffuse * intensity);
        shader.setVec3("light.specular", specular * intensity);
    }
    
    void adjustIntensity(float amount) 
    {
        intensity += amount;
        if (intensity < 0.1f) intensity = 0.1f;
        if (intensity > 3.0f) intensity = 3.0f;
    }
    
    void move(glm::vec3 offset) 
    {
        position += offset;
    }
    
    glm::vec3 getDirection() const
    {
        return direction;
    }
    
    void rotate(float angle, const glm::vec3& axis, const glm::vec3& pivot)
    {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
        direction = glm::vec3(rotationMatrix * glm::vec4(direction, 0.0f));
        direction = glm::normalize(direction);
    }
};

#endif 