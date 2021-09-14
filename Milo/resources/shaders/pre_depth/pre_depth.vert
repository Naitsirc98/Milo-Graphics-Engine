#version 450 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

layout(std140, binding = 0) uniform Camera {
    mat4 u_ProjMatrix;
    mat4 u_ViewMatrix;
    mat4 u_ProjViewMatrix;
};

layout(push_constant) uniform PushConstants {
    mat4 u_ModelMatrix;
};

layout(location = 0) out float out_LinearDepth;

void main() {
    vec4 worldPosition = u_ModelMatrix * vec4(in_Position, 1.0);
    out_LinearDepth = -(u_ViewMatrix * worldPosition).z;
    gl_Position = u_ProjViewMatrix * worldPosition;
}