#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// 默认相机值
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SENSITIVITY =  0.15f;
const float ZOOM        =  45.0f;
const float DISTANCE    =  5.0f;

// 轨道相机类
class Camera
{
public:
    // 相机属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::vec3 Target;    // 视点中心
    
    // 欧拉角
    float Yaw;
    float Pitch;
    float Distance;      // 到目标的距离
    
    // 相机选项
    float MouseSensitivity;
    float Zoom;

    // 构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, DISTANCE), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        Target = glm::vec3(0.0f, 0.0f, 0.0f);  // 默认目标点是原点
        Distance = DISTANCE;
        updateCameraVectors();
    }
    
    // 获取视图矩阵
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Target, Up);
    }

    // 处理鼠标移动 - 轨道旋转模式
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // 确保角度在合理范围内
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // 更新相机位置和方向
        updateCameraVectors();
    }
    
    // 处理相机平移 - 平移目标点
    void ProcessMousePan(float xoffset, float yoffset)
    {
        float sensitivity = 0.01f;
        
        // 平移目标点和位置
        glm::vec3 offset = Right * (-xoffset * sensitivity) - Up * (yoffset * sensitivity);
        Target += offset;
        Position += offset;
    }

    // 处理鼠标滚轮 - 调整与目标点的距离
    void ProcessMouseScroll(float yoffset)
    {
        // 调整缩放
        Zoom -= yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 90.0f)
            Zoom = 90.0f;
            
        // 调整距离
        Distance -= yoffset * 0.3f;
        if (Distance < 1.0f)
            Distance = 1.0f;
        if (Distance > 20.0f)
            Distance = 20.0f;
            
        // 更新相机位置
        updateCameraVectors();
    }

private:
    // 根据更新的欧拉角和距离更新相机位置与向量
    void updateCameraVectors()
    {
        // 计算新的前向量和位置
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        
        // 计算新的相机位置 - 轨道相机模式
        Position = Target - Front * Distance;
        
        // 重新计算右向量和上向量
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif 