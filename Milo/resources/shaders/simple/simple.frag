#version 450 core

layout(push_constant) uniform PushConstants {
    mat4 u_MVP;
};

layout(std140, binding = 1) uniform MaterialUniformBuffer {
    vec4 color;
} u_Material;

layout(binding = 2) uniform sampler2D u_Texture;


layout(location = 0) in Fragment {
    vec2 uv;
} fragment;


layout(location = 0) out vec4 out_FragColor;


void main() {

    out_FragColor = u_Material.color * texture(u_Texture, fragment.uv);
}
