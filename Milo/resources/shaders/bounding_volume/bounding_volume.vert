#version 450 core

layout(push_constant) uniform PushConstants {
    mat4 u_ProjViewModelMatrix;
    vec4 u_Color;
};

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

void main() {
    gl_Position = u_ProjViewModelMatrix * vec4(in_Position, 1.0);
}
