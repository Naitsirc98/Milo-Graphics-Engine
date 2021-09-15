#version 450 core

layout(std140, binding = 0) uniform Camera {
    mat4 u_ProjMatrix;
    mat4 u_ViewMatrix;
    mat4 u_ProjViewMatrix;
};

void main() {

}