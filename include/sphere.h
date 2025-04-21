#ifndef SPHERE_H
#define SPHERE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "shader.h"

class Sphere {
public:
    unsigned int VAO, VBO, EBO;
    float radius;
    int sectorCount;
    int stackCount;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Sphere(float radius = 0.1f, int sectors = 36, int stacks = 18)
        : radius(radius), sectorCount(sectors), stackCount(stacks) {
        setupSphere();
    }

    ~Sphere() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    // 绘制球体，基于光源的位置和强度
    void Draw(Shader &shader, const glm::vec3 &position, float intensity) {
        shader.use();

        // 设置颜色 - 基于光源强度
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 0.8f) * intensity;
        shader.setVec3("sphereColor", color);

        // 创建模型矩阵
        glm::mat4 model = glm::mat4(1.0f);
        
        // 移动到光源位置
        model = glm::translate(model, position);
        
        // 应用固定缩放，不再随光照强度变化
        model = glm::scale(model, glm::vec3(radius));
        
        // 设置模型矩阵
        shader.setMat4("model", model);
        
        // 绘制球体
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    void setupSphere() {
        // 生成球体的顶点和索引
        generateVertices();
        generateIndices();

        // 创建和配置顶点数组对象和相关缓冲区
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // 顶点数据
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

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

    void generateVertices() {
        vertices.clear();
        
        float x, y, z, xy;
        float nx, ny, nz;
        float sectorStep = 2 * M_PI / sectorCount;
        float stackStep = M_PI / stackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i) {
            stackAngle = M_PI / 2 - i * stackStep;  // 从北极到南极
            xy = radius * cos(stackAngle);
            z = radius * sin(stackAngle);

            for (int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;  // 从0到2pi

                // 顶点位置
                x = xy * cos(sectorAngle);
                y = xy * sin(sectorAngle);
                
                // 法线向量 (归一化的位置向量)
                nx = x / radius;
                ny = y / radius;
                nz = z / radius;

                // 添加顶点和法线
                vertices.push_back(x);
                vertices.push_back(z);  // 交换y和z，使北极指向+y方向
                vertices.push_back(y);
                vertices.push_back(nx);
                vertices.push_back(nz);  // 同样交换法线的y和z
                vertices.push_back(ny);
            }
        }
    }

    void generateIndices() {
        indices.clear();
        int k1, k2;

        for (int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                // 对每个堆栈，除了第一个，添加2个三角形
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                // 对每个堆栈，除了最后一个，添加2个三角形
                if (i != (stackCount - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }
    }
};

#endif 