# Illumination Effect Demo Technical Report

## 1. Project Overview

This project implements a 3D scene rendering system based on OpenGL, focusing on demonstrating different lighting effects and normal calculation methods. Through this project, users can intuitively understand the basic lighting models in computer graphics and their visual effects, while also observing the differences between vertex normals and face normals in rendering.

## 2. Lighting Model Implementation

This project implements the Phong lighting model, which includes three main components: ambient light, diffuse reflection, and specular reflection.

### 2.1 Ambient Light

Ambient light simulates indirect lighting from all directions and is the most basic lighting effect in a scene. In the implementation, the ambient light calculation formula is:

```glsl
vec3 ambient = light.ambient * objectColor;
```

Where `light.ambient` is the ambient light intensity of the light source, and `objectColor` is the base color of the object's surface. Ambient light does not depend on surface normals or viewing direction, providing basic brightness for the entire scene.

### 2.2 Diffuse Reflection

Diffuse reflection simulates the scattering effect when light hits a rough surface, related to the surface normal and light direction. The implementation formula is:

```glsl
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(light.position - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = light.diffuse * diff * objectColor;
```

Where:
- `Normal` is the surface normal vector
- `light.position` is the light source position
- `FragPos` is the current fragment's position in world space
- `diff` is the diffuse coefficient, calculated as the dot product of the normal vector and light direction
- `light.diffuse` is the diffuse intensity of the light source

### 2.3 Specular Reflection

Specular reflection simulates the reflection effect of light on a relatively smooth surface, producing highlights. The calculation formula is:

```glsl
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
vec3 specular = light.specular * spec * objectColor;
```

Where:
- `viewPos` is the observer's (camera's) position
- `reflectDir` is the light reflection direction
- `shininess` is the material's shininess, controlling the concentration of highlights
- `light.specular` is the specular reflection intensity of the light source

### 2.4 Combined Lighting

The final lighting effect is the sum of the three lighting components:

```glsl
vec3 result = ambient + diffuse + specular;
```

In this project, users can toggle these three lighting components individually using keys 1, 2, and 3 to observe their respective effects.

## 3. Normal Vector Calculation Principles and Implementation

This project implements two methods for normal vector calculation: vertex normals and face normals, which can be toggled with the N key.

### 3.1 Vertex Normal Calculation

Vertex normals are calculated by averaging the normals of all faces that the vertex belongs to, producing a smooth surface effect:

1. First, calculate the normal for each face (using the cross product of two vectors formed by the face's three vertices):
```cpp
glm::vec3 normal = glm::normalize(glm::cross(pos2 - pos1, pos3 - pos1));
```

2. Then, accumulate face normals to the corresponding vertices:
```cpp
vertices[face.v1].Normal += normal;
vertices[face.v2].Normal += normal;
vertices[face.v3].Normal += normal;
```

3. Finally, normalize the accumulated vertex normals:
```cpp
for (auto& vertex : vertices) {
    if (glm::length(vertex.Normal) > 0) {
        vertex.Normal = glm::normalize(vertex.Normal);
    }
}
```

### 3.2 Face Normal Calculation

Face normals directly use the normal of the face for all fragments on that face, producing a faceted effect suitable for representing polyhedra:

```cpp
// Use face normals
newNormals.resize(vertices.size(), glm::vec3(0.0f));
for (size_t i = 0; i < faces.size(); i++) {
    Face face = faces[i];
    glm::vec3 faceNormal = faceNormals[i];
    
    newNormals[face.v1] = faceNormal;
    newNormals[face.v2] = faceNormal;
    newNormals[face.v3] = faceNormal;
}
```

### 3.3 Normal Mode Switching Implementation

In the Model class, the toggleNormalMode function implements switching between normal modes:

```cpp
void toggleNormalMode() {
    useVertexNormal = !useVertexNormal;
    updateNormals();
}
```

The updateNormals function updates the normal information in the vertex data according to the current mode and reuploads it to the GPU.

## 4. Rendering Pipeline

This project uses the standard OpenGL rendering pipeline, which primarily includes the following steps:

### 4.1 Vertex Shader

The vertex shader is responsible for transforming vertex coordinates from model space to clip space, and calculating data needed for lighting:

```glsl
void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

Note that the normal is processed using the inverse transpose of the model matrix to ensure normals remain correct during non-uniform scaling.

### 4.2 Fragment Shader

The fragment shader implements the Phong lighting model described earlier, calculating the final color based on ambient, diffuse, and specular reflection:

```glsl
void main()
{
    // Ambient light
    vec3 ambient = light.ambient * objectColor * (enableAmbient ? 1.0 : 0.0);
    
    // Diffuse light
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * objectColor * (enableDiffuse ? 1.0 : 0.0);
    
    // Specular light
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * objectColor * (enableSpecular ? 1.0 : 0.0);
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

### 4.3 Rendering Loop

The main program's rendering loop performs the following steps:
1. Process input events
2. Clear color and depth buffers
3. Set view and projection matrices
4. Render the main model (using Phong lighting)
5. Render the sphere representing the light source
6. Render interface text (displaying lighting component status)
7. Swap buffers and process events

## 5. Interaction Implementation

The project's interactive features are primarily implemented through the following functions:

### 5.1 Input Processing

The processInput function handles keyboard input, controlling light source movement, mode switching, and lighting parameter adjustments:

```cpp
void processInput(GLFWwindow *window)
{
    // Detect Shift key state to control interaction mode
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        currentMode = LIGHT;
    } else {
        currentMode = CAMERA;
    }
    
    // Light source movement
    // WASD, QE control light position
    
    // Lighting component toggles
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        // Toggle ambient light
    }
    
    // Similar processing for keys 2, 3, up/down arrows, etc.
}
```

### 5.2 Mouse Interaction

The mouse_callback function handles mouse movement, implementing camera and light source rotation and translation:

```cpp
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    // Calculate mouse offset
    
    // Process different operations based on current mode and mouse button
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (currentMode == CAMERA) {
            camera.ProcessMouseMovement(xoffset, yoffset);
        } else if (currentMode == LIGHT) {
            light.rotate(glm::radians(xoffset * 0.02f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f));
            light.rotate(glm::radians(yoffset * 0.02f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f));
        }
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // Process right mouse button operations
    }
}
```

### 5.3 Scroll Wheel Interaction

The scroll_callback function handles mouse wheel scrolling, controlling camera zoom or light intensity:

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

## 6. Text Rendering

To display the status of lighting components on the interface, the project implements a FreeType-based text rendering system:

```cpp
// Render status text
std::string ambientStatus = "Ambient: " + std::string(enableAmbient ? "ON" : "OFF");
std::string diffuseStatus = "Diffuse: " + std::string(enableDiffuse ? "ON" : "OFF");
std::string specularStatus = "Specular: " + std::string(enableSpecular ? "ON" : "OFF");

textRenderer->RenderText(ambientStatus, 25.0f, SCR_HEIGHT - 50.0f, 0.5f, 
                       glm::vec3(enableAmbient ? 0.0f : 1.0f, enableAmbient ? 1.0f : 0.0f, 0.0f));
```

The text renderer converts text into textured quads containing glyphs and renders them using a dedicated shader program.

## 7. Conclusion

This project demonstrates the impact of lighting on 3D rendering effects by implementing the Phong lighting model and different normal vector calculation methods. Through interactive controls, users can intuitively understand the visual effects of different lighting components and normal types. The project also implements text rendering functionality to enhance the user interface's information display.

This project showcases the implementation methods of basic lighting and shading techniques in computer graphics, providing valuable reference for learning OpenGL and graphics theory. 