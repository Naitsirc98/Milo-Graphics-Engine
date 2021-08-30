#version 450 core

layout(std140, set = 0, binding = 0) uniform UniformBuffer {
    mat4 u_InverseProjViewMatrix;
    float u_TextureLOD;
    float u_Intensity;
};

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_UV;

layout(location = 0) out vec3 out_Position;

void main() {

    vec4 position = vec4(in_Position.xy, 1.0, 1.0);

    out_Position = (u_InverseViewProjMatrix * position).xyz;

    gl_Position = position;
}