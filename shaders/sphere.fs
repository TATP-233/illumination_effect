#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 sphereColor;

void main()
{
    // 基本环境光
    vec3 ambient = 0.3 * sphereColor;
    
    // 漫反射光
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(viewPos - FragPos); // 使用相机位置作为光源
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sphereColor;
    
    // 镜面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * vec3(1.0);
    
    // 最终颜色
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
} 