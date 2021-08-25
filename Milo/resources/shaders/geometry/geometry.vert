#version 450 core

layout(push_constant) uniform PushConstants {
    mat4 u_ModelMatrix;
};

layout(std140, set = 0, binding = 0) uniform CameraUniformBuffer {
    mat4 view;
    mat4 proj;
    mat4 projView;
} u_Camera;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoords;


layout(location = 0) out Fragment {
    vec2 uv;
} fragment;

void main() {
    fragment.uv = vec2(in_TexCoords.x, -in_TexCoords.y);
    gl_Position = u_Camera.projView * u_ModelMatrix * vec4(in_Position, 1.0);
}
