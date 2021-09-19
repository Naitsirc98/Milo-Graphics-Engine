#version 450 core

#define PI 3.1415926536

layout(std140, set = 0, binding = 0) uniform CameraData {
    mat4 viewProjectionMatrix;
    mat4 viewMatrix;
    vec4 position;
} u_Camera;

struct DirectionalLight {
    vec4 direction;
    vec4 color;
};

layout(std140, set = 0, binding = 1) uniform Environment {
    DirectionalLight u_DirLight;
    bool u_DirLightPresent;
    float u_MaxPrefilterLOD;
    float u_PrefilterLODBias;
    bool u_SkyboxPresent;
// TODO: point lights etc
};

layout(set = 0, binding = 2) uniform samplerCube u_IrradianceMap;
layout(set = 0, binding = 3) uniform samplerCube u_PrefilterMap;
layout(set = 0, binding = 4) uniform sampler2D u_BRDF;

// ========================================================

layout(std140, set = 1, binding = 0) uniform Material {

    vec4 albedo;
    vec4 emissiveColor;

    float alpha;
    float metallic;
    float roughness;
    float occlusion;
    float fresnel0;
    float normalScale;

    bool useNormalMap;

} u_Material;

layout(set = 1, binding = 1) uniform sampler2D u_AlbedoMap;
layout(set = 1, binding = 2) uniform sampler2D u_EmissiveMap;
layout(set = 1, binding = 3) uniform sampler2D u_NormalMap;
layout(set = 1, binding = 4) uniform sampler2D u_MetallicMap;
layout(set = 1, binding = 5) uniform sampler2D u_RoughnessMap;
layout(set = 1, binding = 6) uniform sampler2D u_OcclusionMap;

// ============================

struct PBRInfo {
    vec3 albedo;
    vec3 normal;
    vec3 viewDir;
    vec3 reflectDir;
    vec3 F0;

    float metallic;
    float roughness;
    float occlusion;
} g_PBR;

layout(location = 0) in Fragment {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
} fragment;

layout(location = 0) out vec4 out_FragColor;

float radicalInverseVanDerCorpus(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint N) {
    return vec2(float(i) / float(N), radicalInverseVanDerCorpus(i));
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;

    return normalize(sampleVec);
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calculateLighting(vec3 lightColor, vec3 L, vec3 H, float attenuation) {

    vec3 normal = g_PBR.normal;
    vec3 viewDir = g_PBR.viewDir;
    vec3 F0 = g_PBR.F0;
    vec3 albedo = g_PBR.albedo;
    float metallic = g_PBR.metallic;
    float roughness = g_PBR.roughness;

    vec3 radiance = lightColor * attenuation;

    float NDF = distributionGGX(normal, H, roughness);
    float G = geometrySmith(normal, viewDir, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, normal), 0.0), F0);

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0);
    vec3 specular = nominator / (denominator + 0.0001); // 0.001 to prevent division by zero.

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, L), 0.0);

    vec3 L0 = (kD * albedo / PI + specular) * radiance * NdotL;

    return L0;
}

vec3 computeDirLights() {

    if(!u_DirLightPresent) return vec3(0.0);

    vec3 L = normalize(u_DirLight.direction.xyz);
    vec3 H = normalize(g_PBR.viewDir + L);
    float distance = length(u_DirLight.direction.xyz);
    float attenuation = 1.0; // No attenuation in directional lights

    return calculateLighting(u_DirLight.color.rgb, L, H, attenuation);
}

vec3 computePointLights() {

    /*

    vec3 L0 = vec3(0.0);

    for(int i = 0; i < u_PointLightsCount; ++i) {

        Light light = u_PointLights[i];

        vec3 direction = light.position.xyz - fragment.position;

        vec3 L = normalize(direction);
        vec3 H = normalize(g_PBR.viewDir + L);
        float distance = length(direction);
        float attenuation = 1.0 / (distance * distance);

        L0 += calculateLighting(light.color.rgb, L, H, attenuation);
    }

    return L0;

    */

    return vec3(0);
}

vec3 reflectanceEquation() {
    vec3 dirLighting = computeDirLights();
    vec3 pointLighting = computePointLights();
    return dirLighting + pointLighting;
}

vec3 getDiffuseIBL() {
    return g_PBR.albedo * texture(u_IrradianceMap, g_PBR.normal).rgb;
}

vec3 getSpecularIBL(vec3 F, float angle) {
    float prefilterLOD = g_PBR.roughness * 4 + -0.25;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, g_PBR.reflectDir, prefilterLOD).rgb;
    vec2 brdf = texture(u_BRDF, vec2(angle, g_PBR.roughness)).rg;
    return prefilteredColor * (F * brdf.x + brdf.y);
}

vec3 getNormal(vec2 uv, vec3 position, vec3 normal) {

    vec3 tangentNormal = texture(u_NormalMap, uv).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(position);
    vec3 Q2 = dFdy(position);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N = normalize(normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec4 getAlbedo(vec2 uv) {
    return u_Material.albedo * pow(texture(u_AlbedoMap, uv), vec4(2.2));
}

float getMetallic(vec2 uv) {
    return texture(u_MetallicMap, uv).r * u_Material.metallic;
}

float getRoughness(vec2 uv) {
    return texture(u_RoughnessMap, uv).r * u_Material.roughness;
}

float getOcclusion(vec2 uv) {
    return texture(u_OcclusionMap, uv).r * u_Material.occlusion;
}

vec3 getF0(vec3 albedo, float metallic) {
    return mix(vec3(u_Material.fresnel0), albedo, metallic);
}

vec4 computeLighting() {

    vec2 texCoords = fragment.texCoords;

    g_PBR.albedo = getAlbedo(texCoords).rgb;
    g_PBR.metallic = getMetallic(texCoords);
    g_PBR.roughness = getRoughness(texCoords);
    g_PBR.occlusion = getOcclusion(texCoords);
    g_PBR.normal = u_Material.useNormalMap ? getNormal(texCoords, fragment.position, fragment.normal) : fragment.normal;
    g_PBR.F0 = getF0(g_PBR.albedo, g_PBR.metallic);

    g_PBR.viewDir = normalize(u_Camera.position.xyz - fragment.position);
    g_PBR.reflectDir = reflect(-g_PBR.viewDir, g_PBR.normal);

    float angle = max(dot(g_PBR.normal, g_PBR.viewDir), 0.0);

    vec3 L0 = vec3(0);//reflectanceEquation();
    vec3 F = fresnelSchlickRoughness(angle, g_PBR.F0, g_PBR.roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD = kD * (1.0 - g_PBR.metallic);

    vec3 ambient = (kD * getDiffuseIBL() + getSpecularIBL(F, angle)) * g_PBR.occlusion;

    //if(u_SkyboxPresent) {
    //    // If skybox is present, then apply Image Based Lighting (IBL)
    //    ambient = (kD * getDiffuseIBL() + getSpecularIBL(F, angle)) * g_PBR.occlusion;
    //} else {
    //    vec3 ambientColor = vec3(0.25);
    //    ambient = kD * ambientColor * g_PBR.albedo * g_PBR.occlusion;
    //}

    vec3 color = ambient + L0;

    // HDR tonemapping
    //color /= (color + vec3(1.0));

    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    return vec4(color, 1.0);
}

void main() {
    computeLighting();
    float prefilterLOD = g_PBR.roughness * 4 + -0.25;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, g_PBR.reflectDir, prefilterLOD).rgb;
    out_FragColor = vec4(g_PBR.normal, 1);
}