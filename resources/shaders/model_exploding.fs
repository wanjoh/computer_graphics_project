#version 330 core
out vec4 FragColor;

struct PointLight {
    vec3 position;
    vec3 intensity;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    vec3 direction;
    vec3 intensity;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;

    float shininess;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform float transparency;
uniform DirLight dirLight;
uniform PointLight explosionLight;
uniform Material material;

uniform vec3 viewPosition;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);


void main()
{
    if (transparency <= 0.000001)
        discard;
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 result = 2*CalcDirLight(dirLight, normal, viewDir);
    result += CalcPointLight(explosionLight, normal, FragPos, viewDir);
    FragColor = vec4(result, transparency);
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
    return (ambient*light.intensity.x + diffuse*light.intensity.y + specular*light.intensity.z);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    // vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
    ambient *= attenuation * light.intensity.x;
    diffuse *= attenuation * light.intensity.y;
    specular *= attenuation * light.intensity.z;
    return (ambient + diffuse + specular);
}