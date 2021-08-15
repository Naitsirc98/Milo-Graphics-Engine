#version 450 core

layout(push_constants) uniform PushConstants {
    mat4 u_ModelMatrix;
};

layout(location = 0) in vec3 in_Position;

void main() {
    gl_Position = u_ModelMatrix * vec4(in_Position, 1.0);
}
