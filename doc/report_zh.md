# 光照效果演示项目技术报告

## 1. 项目概述

本项目实现了一个基于OpenGL的三维场景渲染系统，重点展示了不同的光照效果和法线计算方法。通过本项目，可以直观地了解计算机图形学中的基本光照模型及其视觉效果，同时演示了顶点法线和面法线在渲染中的差异。

## 2. 光照模型实现

本项目实现了Phong光照模型，包含三个主要组成部分：环境光、漫反射和镜面反射。

### 2.1 环境光（Ambient Light）

环境光模拟了来自各个方向的间接光照，是场景中最基本的光照效果。在实现中，环境光的计算公式为：

```glsl
vec3 ambient = light.ambient * objectColor;
```

其中，`light.ambient`是光源的环境光强度，`objectColor`是物体表面的基本颜色。环境光不依赖于表面法线或观察方向，为整个场景提供基础亮度。

### 2.2 漫反射（Diffuse Reflection）

漫反射模拟了光线照射到粗糙表面时的散射效果，与表面法线和光源方向相关。实现公式为：

```glsl
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(light.position - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = light.diffuse * diff * objectColor;
```

其中：
- `Normal`是表面法线向量
- `light.position`是光源位置
- `FragPos`是当前片段在世界空间中的位置
- `diff`是漫反射系数，通过法线向量与光照方向的点积计算得出
- `light.diffuse`是光源的漫反射强度

### 2.3 镜面反射（Specular Reflection）

镜面反射模拟了光线在较光滑表面上的反射效果，产生高光。计算公式为：

```glsl
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
vec3 specular = light.specular * spec * objectColor;
```

其中：
- `viewPos`是观察者（相机）的位置
- `reflectDir`是光线反射方向
- `shininess`是材质的光泽度，控制高光的集中程度
- `light.specular`是光源的镜面反射强度

### 2.4 组合光照

最终的光照效果是三种光照组件的叠加：

```glsl
vec3 result = ambient + diffuse + specular;
```

在本项目中，用户可以通过按键1、2、3分别开关这三种光照组件，以便观察它们各自的效果。

## 3. 法向量计算原理与实现

本项目实现了两种法向量计算方法：顶点法线和面法线，并可通过按键N进行切换。

### 3.1 顶点法线计算

顶点法线通过对顶点所属的所有面的法线进行平均计算得出，这种方法可以产生平滑的曲面效果：

1. 首先，为每个面计算法线（通过面的三个顶点构成的两个向量的叉积）：
```cpp
glm::vec3 normal = glm::normalize(glm::cross(pos2 - pos1, pos3 - pos1));
```

2. 然后，将面法线累加到对应顶点：
```cpp
vertices[face.v1].Normal += normal;
vertices[face.v2].Normal += normal;
vertices[face.v3].Normal += normal;
```

3. 最后，对累加后的顶点法线进行归一化：
```cpp
for (auto& vertex : vertices) {
    if (glm::length(vertex.Normal) > 0) {
        vertex.Normal = glm::normalize(vertex.Normal);
    }
}
```

### 3.2 面法线计算

面法线是直接使用面的法线作为该面上所有片段的法线，这种方法会产生分面的效果，适合表现多面体：

```cpp
// 使用面法线
newNormals.resize(vertices.size(), glm::vec3(0.0f));
for (size_t i = 0; i < faces.size(); i++) {
    Face face = faces[i];
    glm::vec3 faceNormal = faceNormals[i];
    
    newNormals[face.v1] = faceNormal;
    newNormals[face.v2] = faceNormal;
    newNormals[face.v3] = faceNormal;
}
```

### 3.3 法线模式切换实现

在Model类中，通过toggleNormalMode函数实现法线模式的切换：

```cpp
void toggleNormalMode() {
    useVertexNormal = !useVertexNormal;
    updateNormals();
}
```

updateNormals函数根据当前模式更新顶点数据中的法线信息并重新上传到GPU。

## 4. 渲染管线

本项目使用了标准的OpenGL渲染管线，主要包含以下步骤：

### 4.1 顶点着色器

顶点着色器负责将顶点坐标从模型空间变换到裁剪空间，并计算光照需要的数据：

```glsl
void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

注意这里对法线的处理使用了模型矩阵的逆转置矩阵，以保证非均匀缩放时法线仍然正确。

### 4.2 片段着色器

片段着色器实现了前面描述的Phong光照模型，根据环境光、漫反射和镜面反射计算最终颜色：

```glsl
void main()
{
    // 环境光
    vec3 ambient = light.ambient * objectColor * (enableAmbient ? 1.0 : 0.0);
    
    // 漫反射光
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * objectColor * (enableDiffuse ? 1.0 : 0.0);
    
    // 镜面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * objectColor * (enableSpecular ? 1.0 : 0.0);
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

### 4.3 渲染循环

主程序的渲染循环执行以下步骤：
1. 处理输入事件
2. 清除颜色和深度缓冲
3. 设置视图和投影矩阵
4. 渲染主模型（使用Phong光照）
5. 渲染表示光源的球体
6. 渲染界面文本（显示光照组件状态）
7. 交换缓冲并处理事件

## 5. 交互实现

项目的交互功能主要通过以下几个函数实现：

### 5.1 输入处理

processInput函数处理键盘输入，控制光源移动、模式切换和光照参数调整：

```cpp
void processInput(GLFWwindow *window)
{
    // 检测Shift键状态，控制交互模式
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        currentMode = LIGHT;
    } else {
        currentMode = CAMERA;
    }
    
    // 光源移动
    // WASD、QE控制光源位置
    
    // 光照组件开关
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        // 开关环境光
    }
    
    // 类似处理：2键、3键、上下箭头键等
}
```

### 5.2 鼠标交互

mouse_callback函数处理鼠标移动，实现相机和光源的旋转与平移：

```cpp
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    // 计算鼠标偏移量
    
    // 根据当前模式和鼠标按键处理不同操作
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (currentMode == CAMERA) {
            camera.ProcessMouseMovement(xoffset, yoffset);
        } else if (currentMode == LIGHT) {
            light.rotate(glm::radians(xoffset * 0.02f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f));
            light.rotate(glm::radians(yoffset * 0.02f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f));
        }
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // 处理鼠标右键操作
    }
}
```

### 5.3 滚轮交互

scroll_callback函数处理鼠标滚轮，控制相机缩放或光源强度：

```cpp
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (currentMode == CAMERA) {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    } else if (currentMode == LIGHT) {
        light.adjustIntensity(static_cast<float>(yoffset * 0.1f));
    }
}
```

## 6. 文本渲染

为了在界面上显示光照组件的状态，项目实现了基于FreeType的文本渲染系统：

```cpp
// 渲染状态文本
std::string ambientStatus = "Ambient: " + std::string(enableAmbient ? "ON" : "OFF");
std::string diffuseStatus = "Diffuse: " + std::string(enableDiffuse ? "ON" : "OFF");
std::string specularStatus = "Specular: " + std::string(enableSpecular ? "ON" : "OFF");

textRenderer->RenderText(ambientStatus, 25.0f, SCR_HEIGHT - 50.0f, 0.5f, 
                       glm::vec3(enableAmbient ? 0.0f : 1.0f, enableAmbient ? 1.0f : 0.0f, 0.0f));
```

文本渲染器将文本转换为包含字形的纹理四边形，并使用专用的着色器程序进行渲染。

## 7. 总结

本项目通过实现Phong光照模型和不同的法向量计算方法，演示了光照对3D渲染效果的影响。通过交互式控制，用户可以直观地了解不同光照组件和法线类型的视觉效果。项目还实现了文本渲染功能，以增强用户界面的信息展示。

这个项目展示了计算机图形学中基本光照和着色技术的实现方法，对于学习OpenGL和图形学理论有很好的参考价值。 