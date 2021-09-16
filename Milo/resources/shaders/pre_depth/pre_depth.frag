#version 450 core

layout(std140, binding = 0) uniform Camera {
    mat4 u_ProjMatrix;
    mat4 u_ViewMatrix;
    mat4 u_ProjViewMatrix;
};

layout(location = 0) in float in_LinearDepth;

layout(location = 0) out vec4 out_FragColor;

void main() {
    out_FragColor = vec4(vec3(in_LinearDepth), 1.0);
}