#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure;

void main()
{
    const float gamma = 2.2;
    vec4 hdrColor = texture(hdrBuffer, TexCoords).rgba;
    vec4 result = vec4(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec4(1.0 / gamma));
    FragColor = result;
}