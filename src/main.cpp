#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "light.h"
#include "sphere.h"
#include "text_renderer.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// 窗口设置
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 摄像机设置 - 轨道相机模式
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 光照设置
Light light;

// 模型
Model* ourModel = nullptr;

// 圆柱体（表示光源）
Sphere* lightSphere = nullptr;

// 时间设置
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 渲染模式
enum InteractionMode {
    CAMERA,
    LIGHT
};
InteractionMode currentMode = CAMERA; // 默认为相机模式

// 材质属性
float shininess = 32.0f;

// 光照组件开关
bool enableAmbient = true;
bool enableDiffuse = true;
bool enableSpecular = true;

// 文本渲染器
TextRenderer* textRenderer = nullptr;

int main()
{
    // glfw初始化和配置
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw窗口创建
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Illumination Effect", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 保持鼠标指针可见
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // glew初始化
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // 配置全局OpenGL状态
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 初始化文本渲染器
    textRenderer = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    textRenderer->Load("fonts/MarkerFelt.ttc", 24);

    // 构建并编译着色器程序
    Shader modelShader("shaders/model.vs", "shaders/model.fs");
    Shader sphereShader("shaders/sphere.vs", "shaders/sphere.fs");

    // 加载模型
    ourModel = new Model("models/eight.uniform.obj");
    
    // 创建圆柱体（表示光源）
    lightSphere = new Sphere(0.5f);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 计算每帧的时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 处理输入
        processInput(window);

        // 渲染
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 视图/投影变换 - 用于所有着色器
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // 1. 首先渲染主模型
        modelShader.use();
        
        // 设置着色器uniform
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setFloat("shininess", shininess);
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        
        // 设置光照组件开关
        modelShader.setBool("enableAmbient", enableAmbient);
        modelShader.setBool("enableDiffuse", enableDiffuse);
        modelShader.setBool("enableSpecular", enableSpecular);

        // 世界变换
        glm::mat4 model = glm::mat4(1.0f);
        modelShader.setMat4("model", model);

        // 设置光照属性
        light.setUniforms(modelShader);

        // 渲染模型
        ourModel->Draw(modelShader);
        
        // 2. 然后渲染表示光源的圆柱体
        sphereShader.use();
        sphereShader.setMat4("projection", projection);
        sphereShader.setMat4("view", view);
        sphereShader.setVec3("viewPos", camera.Position);
        
        // 绘制圆柱体
        lightSphere->Draw(sphereShader, light.position, light.intensity);

        // 渲染状态文本
        std::string ambientStatus = "Ambient: " + std::string(enableAmbient ? "ON" : "OFF");
        std::string diffuseStatus = "Diffuse: " + std::string(enableDiffuse ? "ON" : "OFF");
        std::string specularStatus = "Specular: " + std::string(enableSpecular ? "ON" : "OFF");
        
        textRenderer->RenderText(ambientStatus, 25.0f, SCR_HEIGHT - 25.0f, 0.5f, 
                               glm::vec3(enableAmbient ? 0.0f : 1.0f, enableAmbient ? 1.0f : 0.0f, 0.0f));
        textRenderer->RenderText(diffuseStatus, 25.0f, SCR_HEIGHT - 50.0f, 0.5f, 
                               glm::vec3(enableDiffuse ? 0.0f : 1.0f, enableDiffuse ? 1.0f : 0.0f, 0.0f));
        textRenderer->RenderText(specularStatus, 25.0f, SCR_HEIGHT - 75.0f, 0.5f, 
                               glm::vec3(enableSpecular ? 0.0f : 1.0f, enableSpecular ? 1.0f : 0.0f, 0.0f));

        // 交换缓冲并查询IO事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理
    delete ourModel;
    delete lightSphere;
    delete textRenderer;
    
    glfwTerminate();
    return 0;
}

// 处理输入
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // 检测Shift键状态，按下时切换到CAMERA模式，松开时切换回LIGHT模式
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        currentMode = LIGHT;
    } else {
        currentMode = CAMERA;
    }
    
    // 光源移动
    float speed = 2.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        light.move(glm::vec3(0.0f, 0.0f, -speed));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        light.move(glm::vec3(0.0f, 0.0f, speed));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        light.move(glm::vec3(-speed, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        light.move(glm::vec3(speed, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        light.move(glm::vec3(0.0f, speed, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        light.move(glm::vec3(0.0f, -speed, 0.0f));

    // 切换法线模式
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        static float lastNormalToggle = 0.0f;
        float currentTime = static_cast<float>(glfwGetTime());
        
        if (currentTime - lastNormalToggle > 0.2f) {
            ourModel->toggleNormalMode();
            lastNormalToggle = currentTime;
        }
    }
    
    // 随机改变颜色
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        static float lastColorChange = 0.0f;
        float currentTime = static_cast<float>(glfwGetTime());
        
        if (currentTime - lastColorChange > 0.2f) {
            ourModel->randomColor();
            lastColorChange = currentTime;
        }
    }
    
    // 更改材质光泽度
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        shininess = std::min(shininess + 1.0f, 128.0f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        shininess = std::max(shininess - 1.0f, 1.0f);
        
    // 开关环境光（按键1）
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        static float lastKeyPress = 0.0f;
        float currentTime = static_cast<float>(glfwGetTime());
        
        if (currentTime - lastKeyPress > 0.2f) {
            enableAmbient = !enableAmbient;
            std::cout << "环境光: " << (enableAmbient ? "开启" : "关闭") << std::endl;
            lastKeyPress = currentTime;
        }
    }
    
    // 开关漫反射（按键2）
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        static float lastKeyPress = 0.0f;
        float currentTime = static_cast<float>(glfwGetTime());
        
        if (currentTime - lastKeyPress > 0.2f) {
            enableDiffuse = !enableDiffuse;
            std::cout << "漫反射: " << (enableDiffuse ? "开启" : "关闭") << std::endl;
            lastKeyPress = currentTime;
        }
    }
    
    // 开关镜面反射（按键3）
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        static float lastKeyPress = 0.0f;
        float currentTime = static_cast<float>(glfwGetTime());
        
        if (currentTime - lastKeyPress > 0.2f) {
            enableSpecular = !enableSpecular;
            std::cout << "镜面反射: " << (enableSpecular ? "开启" : "关闭") << std::endl;
            lastKeyPress = currentTime;
        }
    }
}

// 窗口大小改变时的回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 鼠标移动回调
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 颠倒，因为y坐标从底部到顶部

    lastX = xpos;
    lastY = ypos;

    // 处理鼠标左右键不同操作
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (currentMode == CAMERA) {
            camera.ProcessMouseMovement(xoffset, yoffset);
        } else if (currentMode == LIGHT) {
            // 旋转光源
            light.rotate(glm::radians(xoffset * 0.02f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f));
            light.rotate(glm::radians(yoffset * 0.02f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f));
        }
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (currentMode == CAMERA) {
            // 相机平移 - 使用新的专用函数
            camera.ProcessMousePan(xoffset, yoffset);
        } else if (currentMode == LIGHT) {
            // 光源平移
            float sensitivity = 0.01f;
            light.move(xoffset * sensitivity  * camera.Right + yoffset * sensitivity * camera.Up);
        }
    }
}

// 滚轮回调
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (currentMode == CAMERA) {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    } else if (currentMode == LIGHT) {
        light.adjustIntensity(static_cast<float>(yoffset * 0.1f));
    }
} 