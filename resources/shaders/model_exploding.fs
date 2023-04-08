#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float time;
in vec3 aPos1;

uniform sampler2D texture_diffuse1;
uniform float transparency;
uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform mat4 model;

void main()
{
    if (transparency <= 0.000001)
        discard;
    vec3 Position = vec3(model * vec4(aPos1, 1.0));
    vec3 I = normalize(Position - cameraPos);
    FragColor = vec4(vec3(texture(texture_diffuse1, TexCoords)), transparency);
}
