#version 450 core

layout(location = 0) in float in_LinearDepth;

layout(location = 0) out vec4 out_LinearDepth;

void main() {
    out_LinearDepth = vec4(in_LinearDepth, 0.0, 0.0, 1.0);
}