#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// 从文件加载着色器源代码
std::string loadShaderSource(const char* filePath) {
    std::string shaderCode;
    std::ifstream shaderFile;
    
    // 确保ifstream对象可以抛出异常
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // 打开文件
        shaderFile.open(filePath);
        std::stringstream shaderStream;
        // 读取文件缓冲区到数据流
        shaderStream << shaderFile.rdbuf();
        // 关闭文件
        shaderFile.close();
        // 将流转换为字符串
        shaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filePath << std::endl;
    }
    
    return shaderCode;
}

// 创建着色器程序
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    // 读取着色器源码
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    
    // 编译着色器
    GLuint vertex, fragment;
    GLint success;
    GLchar infoLog[512];
    
    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    
    // 检查顶点着色器编译错误
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // 片段着色器
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    
    // 检查片段着色器编译错误
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // 着色器程序
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    
    // 检查着色器程序链接错误
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    // 删除着色器，它们已链接到程序中，不再需要
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    return program;
} 