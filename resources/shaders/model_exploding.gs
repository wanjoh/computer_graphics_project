#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
    vec3 aPos;
    vec3 Normal1;
    vec3 FragPos1;
} gs_in[];

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
uniform float time;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 0.3;
    vec3 direction = normal * time * magnitude;
    return position + vec4(vec3(direction.x, direction.y, 0.0), 0.0);
}

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

void main() {
    vec3 normal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoords = gs_in[0].texCoords;
    Normal = gs_in[0].Normal1;
    FragPos = gs_in[0].FragPos1;
    EmitVertex();

    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoords = gs_in[2].texCoords;
    Normal = gs_in[1].Normal1;
    FragPos = gs_in[1].FragPos1;
    EmitVertex();

    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoords = gs_in[2].texCoords;
    Normal = gs_in[2].Normal1;
    FragPos = gs_in[2].FragPos1;
    EmitVertex();
    EndPrimitive();
}