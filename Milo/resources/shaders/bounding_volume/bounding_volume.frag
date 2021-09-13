#version 450 core

layout(push_constant) uniform PushConstants {
    mat4 u_ProjViewModelMatrix;
    vec4 u_Color;
};

layout(location = 0) out vec4 out_FragColor;

void main() {
    out_FragColor = u_Color;
}
