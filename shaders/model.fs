#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

struct Light {
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewPos;
uniform vec3 objectColor;
uniform Light light;
uniform float shininess;

// 光照组件开关
uniform bool enableAmbient;
uniform bool enableDiffuse;
uniform bool enableSpecular;

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