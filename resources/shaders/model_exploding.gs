#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
    vec3 aPos;
} gs_in[];

out vec2 TexCoords;
out vec3 aPos1;

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

    aPos1 = vec3(gs_in[0].aPos);
    gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoords = gs_in[0].texCoords;
    EmitVertex();

    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoords = gs_in[2].texCoords;
    aPos1 = vec3(gs_in[0].aPos);
    EmitVertex();

    aPos1 = vec3(gs_in[2].aPos);
    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoords = gs_in[2].texCoords;
    EmitVertex();
    EndPrimitive();
}