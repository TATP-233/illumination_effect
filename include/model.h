#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <random>

#include "shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

struct Face {
    unsigned int v1, v2, v3;
};

class Model 
{
public:
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<glm::vec3> faceNormals;
    std::vector<unsigned int> indices;
    glm::vec3 modelColor;
    bool useVertexNormal;
    
    unsigned int VAO;
    
    Model(const char* path)
    {
        loadModel(path);
        setupMesh();
        useVertexNormal = true;
        randomColor();
    }
    
    void Draw(Shader &shader) 
    {
        shader.setVec3("objectColor", modelColor);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    void randomColor() 
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0, 1.0);
        
        modelColor = glm::vec3(dis(gen), dis(gen), dis(gen));
    }
    
    void toggleNormalMode() 
    {
        useVertexNormal = !useVertexNormal;
        updateNormals();
    }
    
private:
    unsigned int VBO, EBO;
    
    void loadModel(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cout << "Failed to open file: " << path << std::endl;
            return;
        }
        
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> tempNormals;
        
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            
            if (prefix == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                positions.push_back(glm::vec3(x, y, z));
                
                Vertex vertex;
                vertex.Position = glm::vec3(x, y, z);
                vertex.Normal = glm::vec3(0.0f); // 初始化法线为0
                vertices.push_back(vertex);
            }
            else if (prefix == "f") {
                Face face;
                iss >> face.v1 >> face.v2 >> face.v3;
                
                // OBJ文件索引从1开始，需要减1
                face.v1 -= 1;
                face.v2 -= 1;
                face.v3 -= 1;
                faces.push_back(face);
                
                // 添加到索引数组中用于绘制
                indices.push_back(face.v1);
                indices.push_back(face.v2);
                indices.push_back(face.v3);
                
                // 计算面法线
                glm::vec3 pos1 = positions[face.v1];
                glm::vec3 pos2 = positions[face.v2];
                glm::vec3 pos3 = positions[face.v3];
                
                glm::vec3 normal = glm::normalize(glm::cross(pos2 - pos1, pos3 - pos1));
                faceNormals.push_back(normal);
                
                // 将面法线累加到顶点法线上，后续会归一化
                vertices[face.v1].Normal += normal;
                vertices[face.v2].Normal += normal;
                vertices[face.v3].Normal += normal;
            }
        }
        
        // 归一化顶点法线
        for (auto& vertex : vertices) {
            if (glm::length(vertex.Normal) > 0) {
                vertex.Normal = glm::normalize(vertex.Normal);
            }
        }
        
        file.close();
    }
    
    void updateNormals()
    {
        std::vector<glm::vec3> newNormals;
        
        if (useVertexNormal) {
            // 使用之前计算好的顶点法线
            for (const auto& vertex : vertices) {
                newNormals.push_back(vertex.Normal);
            }
        } else {
            // 使用面法线
            newNormals.resize(vertices.size(), glm::vec3(0.0f));
            for (size_t i = 0; i < faces.size(); i++) {
                Face face = faces[i];
                glm::vec3 faceNormal = faceNormals[i];
                
                newNormals[face.v1] = faceNormal;
                newNormals[face.v2] = faceNormal;
                newNormals[face.v3] = faceNormal;
            }
        }
        
        // 更新VBO中的法线数据
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        
        // 先更新顶点数据
        std::vector<float> data;
        for (size_t i = 0; i < vertices.size(); i++) {
            // 位置
            data.push_back(vertices[i].Position.x);
            data.push_back(vertices[i].Position.y);
            data.push_back(vertices[i].Position.z);
            
            // 法线
            data.push_back(newNormals[i].x);
            data.push_back(newNormals[i].y);
            data.push_back(newNormals[i].z);
        }
        
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    }
    
    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        // 顶点数据
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        
        std::vector<float> data;
        for (const auto& vertex : vertices) {
            // 位置
            data.push_back(vertex.Position.x);
            data.push_back(vertex.Position.y);
            data.push_back(vertex.Position.z);
            
            // 法线
            data.push_back(vertex.Normal.x);
            data.push_back(vertex.Normal.y);
            data.push_back(vertex.Normal.z);
        }
        
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
        
        // 索引数据
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        // 设置顶点属性指针
        // 位置属性
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        
        // 法线属性
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
    }
};

#endif 