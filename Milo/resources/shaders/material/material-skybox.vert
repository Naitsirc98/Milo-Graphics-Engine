#version 450 core

layout(std140, set = 0, binding = 0) uniform UniformBuffer {
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
    float u_TextureLOD;
    float u_Intensity;
};

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

layout(location = 0) out vec3 out_Position;

void main() {

    out_Position = in_Position;

    mat4 rotatedView = mat4(mat3(u_ViewMatrix));
    vec4 clipPosition = u_ProjectionMatrix * rotatedView * vec4(in_Position, 1.0);

    gl_Position = clipPosition.xyww;
}