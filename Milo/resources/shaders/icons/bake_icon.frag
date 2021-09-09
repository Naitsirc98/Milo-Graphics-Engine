#version 450 core

layout(std140, set = 0, binding = 0) uniform MaterialUniformBuffer {

    vec4 albedo;
    vec4 emissiveColor;

    float alpha;
    float metallic;
    float roughness;
    float occlusion;
    float fresnel0;
    float normalScale;

    bool useNormalMap;
    bool useCombinedMetallicRoughnessMap;

} u_Material;

layout(set = 0, binding = 1) uniform sampler2D u_AlbedoMap;
layout(set = 0, binding = 2) uniform sampler2D u_MetallicMap;
layout(set = 0, binding = 3) uniform sampler2D u_RoughnessMap;
layout(set = 0, binding = 4) uniform sampler2D u_MetallicRoughnessMap;
layout(set = 0, binding = 5) uniform sampler2D u_OcclusionMap;
layout(set = 0, binding = 6) uniform sampler2D u_EmissiveMap;
layout(set = 0, binding = 7) uniform sampler2D u_NormalMap;

layout(location = 0) in Fragment {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
} fragment;

layout(location = 0) out vec4 out_FragColor;

void main() {

    vec4 textureColor = texture(u_AlbedoMap, fragment.texCoords);
    vec4 albedo = u_Material.albedo;

    // TODO

    out_FragColor = textureColor * albedo;
}
