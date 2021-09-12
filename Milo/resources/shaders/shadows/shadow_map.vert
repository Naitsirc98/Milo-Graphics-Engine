#version 450 core

#define MAX_SHADOW_CASCADES 4

layout(std140, binding = 0) uniform ShadowData {
    mat4 u_ViewProjectionMatrix[MAX_SHADOW_CASCADES];
};

layout(push_constant) uniform PushConstants {
    mat4 u_ModelMatrix;
    uint u_CascadeIndex;
};

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

void main() {
    gl_Position = u_ViewProjectionMatrix[u_CascadeIndex] * u_ModelMatrix * vec4(in_Position, 1.0);
}
