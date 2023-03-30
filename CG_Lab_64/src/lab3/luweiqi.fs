#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;
}; 

uniform Material material;

uniform sampler2D ourTexture;
uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    vec3 objectColor = vec3(texture(ourTexture, TexCoord));

    // 环境光
    vec3 ambient = lightColor * material.ambient;

    // 漫反射 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * material.diffuse);

    // 镜面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * (spec * material.specular); 

    // 发光
    vec3 emission = material.emission;

    vec3 result = (ambient + diffuse + specular + emission) * objectColor;

    FragColor = vec4(result, 1.0);
    
}