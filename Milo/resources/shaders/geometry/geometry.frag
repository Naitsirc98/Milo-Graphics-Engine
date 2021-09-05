#version 450 core

#define MAX_MATERIAL_TEXTURE_COUNT 1

layout(std140, set = 1, binding = 0) uniform MaterialUniformBuffer {
    vec4 color;
} u_Material;

layout(set = 1, binding = 1) uniform sampler2D u_MaterialTextures[MAX_MATERIAL_TEXTURE_COUNT];


layout(location = 0) in Fragment {
    vec2 uv;
} fragment;


layout(location = 0) out vec4 out_FragColor;


void main() {

    out_FragColor = u_Material.color * texture(u_MaterialTextures[0], fragment.uv);

    // HDR tonemapping
    out_FragColor /= (out_FragColor + vec4(1.0));
    // Gamma correct
    out_FragColor = pow(out_FragColor, vec4(1.0 / 2.2));
}
