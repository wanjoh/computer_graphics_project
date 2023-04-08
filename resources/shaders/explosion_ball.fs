#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 explosionColor;

void main()
{
    FragColor = vec4(explosionColor, 1.0);
}