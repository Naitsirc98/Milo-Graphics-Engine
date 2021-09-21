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
    vec4 unused;
};

layout(std140, set = 0, binding = 1) uniform Environment {
    DirectionalLight u_DirLight;
    bool u_DirLightPresent;
    float u_MaxPrefilterLOD;
    float u_PrefilterLODBias;
    bool u_SkyboxPresent;
};

struct PointLight {
    vec4 position;
    vec4 color;
    float multiplier;
    float minRadius;
    float radius;
    float falloff;
    vec4 unused;
};

layout(std140, set = 0, binding = 2) uniform PointLights {
    PointLight u_PointLights[128];
    uint u_PointLightsCount;
};

layout(set = 0, binding = 3) buffer VisibleLightsBuffer {
    int u_VisibleLightIndices[];
};

layout(set = 0, binding = 4) uniform samplerCube u_IrradianceMap;
layout(set = 0, binding = 5) uniform samplerCube u_PrefilterMap;
layout(set = 0, binding = 6) uniform sampler2D u_BRDF;

// ========================================================

layout(std140, set = 1, binding = 0) uniform ShadowsDetails {
    mat4 u_LightMatrix[4];
    vec4 u_CascadeSplits;
    int u_TilesCountX;
    bool u_ShowCascades;
    bool u_SoftShadows;
    float u_LightSize;
    float u_MaxShadowDistance;
    float u_ShadowFade;
    bool u_CascadeFading;
    float u_CascadeTransitionFade;
    bool u_ShowLightComplexity;
    bool u_ShadowsEnabled;
};

layout(set = 1, binding = 1) uniform sampler2DArray u_ShadowMap;

// ========================================================

layout(std140, set = 2, binding = 0) uniform Material {

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

layout(set = 2, binding = 1) uniform sampler2D u_AlbedoMap;
layout(set = 2, binding = 2) uniform sampler2D u_EmissiveMap;
layout(set = 2, binding = 3) uniform sampler2D u_NormalMap;
layout(set = 2, binding = 4) uniform sampler2D u_MetallicMap;
layout(set = 2, binding = 5) uniform sampler2D u_RoughnessMap;
layout(set = 2, binding = 6) uniform sampler2D u_MetallicRoughnessMap;
layout(set = 2, binding = 7) uniform sampler2D u_OcclusionMap;

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
    mat3 cameraViewMatrix;
    vec4 shadowMapCoords[4];
    vec3 viewPosition;
} fragment;

layout(location = 0) out vec4 out_FragColor;

const ivec2 TILE_SIZE = ivec2(16, 16);
const uint MAX_POINT_LIGHTS = 128;

int GetLightBufferIndex(int i) {

    ivec2 tileID = ivec2(gl_FragCoord) / TILE_SIZE;
    uint index = tileID.y * u_TilesCountX + tileID.x;

    uint offset = index * 128;

    return u_VisibleLightIndices[offset + i];
}

int GetPointLightCount() {

    int result = 0;
    for (int i = 0; i < u_PointLightsCount; i++) {
        uint lightIndex = GetLightBufferIndex(i);
        if (lightIndex == -1)
        break;

        result++;
    }

    return result;
}

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

    vec3 L0 = vec3(0.0);

    for(int i = 0; i < u_PointLightsCount; ++i) {

        uint lightIndex = GetLightBufferIndex(i);
        if(lightIndex == -1) continue;

        PointLight light = u_PointLights[lightIndex];

        vec3 direction = light.position.xyz - fragment.position;

        vec3 L = normalize(direction);
        vec3 H = normalize(g_PBR.viewDir + L);
        float distance = length(direction);
        float attenuation = 1.0 / (distance * distance);

        L0 += calculateLighting(light.color.rgb, L, H, attenuation);
    }

    return L0;
}

/////////////////////////////////////////////
// PCSS
/////////////////////////////////////////////

float ShadowFade = 1.0;

float GetShadowBias()
{
    const float MINIMUM_SHADOW_BIAS = 0.002;
    float bias = max(MINIMUM_SHADOW_BIAS * (1.0 - dot(g_PBR.normal, u_DirLight.direction.xyz)), MINIMUM_SHADOW_BIAS);
    return bias;
}

float HardShadows_DirectionalLight(sampler2DArray shadowMap, uint cascade, vec3 shadowCoords)
{
    float bias = GetShadowBias();
    float shadowMapDepth = texture(shadowMap, vec3(shadowCoords.xy * 0.5 + 0.5, cascade)).x;
    return step(shadowCoords.z, shadowMapDepth + bias) * ShadowFade;
}

// Penumbra

// this search area estimation comes from the following article:
// http://developer.download.nvidia.com/whitepapers/2008/PCSS_Integration.pdf
float SearchWidth(float uvLightSize, float receiverDistance)
{
    const float NEAR = 0.1;
    return uvLightSize * (receiverDistance - NEAR) / u_Camera.position.z;
}

float SearchRegionRadiusUV(float zWorld)
{
    const float light_zNear = 0.0; // 0.01 gives artifacts? maybe because of ortho proj?
    const float lightRadiusUV = 0.05;
    return lightRadiusUV * (zWorld - light_zNear) / zWorld;
}

const vec2 PoissonDistribution[64] = vec2[](
vec2(-0.884081, 0.124488),
vec2(-0.714377, 0.027940),
vec2(-0.747945, 0.227922),
vec2(-0.939609, 0.243634),
vec2(-0.985465, 0.045534),
vec2(-0.861367, -0.136222),
vec2(-0.881934, 0.396908),
vec2(-0.466938, 0.014526),
vec2(-0.558207, 0.212662),
vec2(-0.578447, -0.095822),
vec2(-0.740266, -0.095631),
vec2(-0.751681, 0.472604),
vec2(-0.553147, -0.243177),
vec2(-0.674762, -0.330730),
vec2(-0.402765, -0.122087),
vec2(-0.319776, -0.312166),
vec2(-0.413923, -0.439757),
vec2(-0.979153, -0.201245),
vec2(-0.865579, -0.288695),
vec2(-0.243704, -0.186378),
vec2(-0.294920, -0.055748),
vec2(-0.604452, -0.544251),
vec2(-0.418056, -0.587679),
vec2(-0.549156, -0.415877),
vec2(-0.238080, -0.611761),
vec2(-0.267004, -0.459702),
vec2(-0.100006, -0.229116),
vec2(-0.101928, -0.380382),
vec2(-0.681467, -0.700773),
vec2(-0.763488, -0.543386),
vec2(-0.549030, -0.750749),
vec2(-0.809045, -0.408738),
vec2(-0.388134, -0.773448),
vec2(-0.429392, -0.894892),
vec2(-0.131597, 0.065058),
vec2(-0.275002, 0.102922),
vec2(-0.106117, -0.068327),
vec2(-0.294586, -0.891515),
vec2(-0.629418, 0.379387),
vec2(-0.407257, 0.339748),
vec2(0.071650, -0.384284),
vec2(0.022018, -0.263793),
vec2(0.003879, -0.136073),
vec2(-0.137533, -0.767844),
vec2(-0.050874, -0.906068),
vec2(0.114133, -0.070053),
vec2(0.163314, -0.217231),
vec2(-0.100262, -0.587992),
vec2(-0.004942, 0.125368),
vec2(0.035302, -0.619310),
vec2(0.195646, -0.459022),
vec2(0.303969, -0.346362),
vec2(-0.678118, 0.685099),
vec2(-0.628418, 0.507978),
vec2(-0.508473, 0.458753),
vec2(0.032134, -0.782030),
vec2(0.122595, 0.280353),
vec2(-0.043643, 0.312119),
vec2(0.132993, 0.085170),
vec2(-0.192106, 0.285848),
vec2(0.183621, -0.713242),
vec2(0.265220, -0.596716),
vec2(-0.009628, -0.483058),
vec2(-0.018516, 0.435703)
);

const vec2 poissonDisk[16] = vec2[](
vec2(-0.94201624, -0.39906216),
vec2(0.94558609, -0.76890725),
vec2(-0.094184101, -0.92938870),
vec2(0.34495938, 0.29387760),
vec2(-0.91588581, 0.45771432),
vec2(-0.81544232, -0.87912464),
vec2(-0.38277543, 0.27676845),
vec2(0.97484398, 0.75648379),
vec2(0.44323325, -0.97511554),
vec2(0.53742981, -0.47373420),
vec2(-0.26496911, -0.41893023),
vec2(0.79197514, 0.19090188),
vec2(-0.24188840, 0.99706507),
vec2(-0.81409955, 0.91437590),
vec2(0.19984126, 0.78641367),
vec2(0.14383161, -0.14100790)
);

vec2 SamplePoisson(int index)
{
    return PoissonDistribution[index % 64];
}

float FindBlockerDistance_DirectionalLight(sampler2DArray shadowMap, uint cascade, vec3 shadowCoords, float uvLightSize)
{
    float bias = GetShadowBias();

    int numBlockerSearchSamples = 64;
    int blockers = 0;
    float avgBlockerDistance = 0;

    float searchWidth = SearchRegionRadiusUV(shadowCoords.z);
    for (int i = 0; i < numBlockerSearchSamples; i++)
    {
        float z = textureLod(shadowMap, vec3((shadowCoords.xy * 0.5 + 0.5) + SamplePoisson(i) * searchWidth, cascade), 0).r;
        if (z < (shadowCoords.z - bias))
        {
            blockers++;
            avgBlockerDistance += z;
        }
    }

    if (blockers > 0)
    return avgBlockerDistance / float(blockers);

    return -1;
}

float PCF_DirectionalLight(sampler2DArray shadowMap, uint cascade, vec3 shadowCoords, float uvRadius)
{
    float bias = GetShadowBias();
    int numPCFSamples = 64;

    float sum = 0;
    for (int i = 0; i < numPCFSamples; i++)
    {
        vec2 offset = SamplePoisson(i) * uvRadius;
        float z = textureLod(shadowMap, vec3((shadowCoords.xy * 0.5 + 0.5) + offset, cascade), 0).r;
        sum += step(shadowCoords.z - bias, z);
    }
    return sum / numPCFSamples;
}

float NV_PCF_DirectionalLight(sampler2DArray shadowMap, uint cascade, vec3 shadowCoords, float uvRadius)
{
    float bias = GetShadowBias();

    float sum = 0;
    for (int i = 0; i < 16; i++)
    {
        vec2 offset = poissonDisk[i] * uvRadius;
        float z = textureLod(shadowMap, vec3((shadowCoords.xy * 0.5 + 0.5) + offset, cascade), 0).r;
        sum += step(shadowCoords.z - bias, z);
    }
    return sum / 16.0f;
}

float PCSS_DirectionalLight(sampler2DArray shadowMap, uint cascade, vec3 shadowCoords, float uvLightSize)
{
    float blockerDistance = FindBlockerDistance_DirectionalLight(shadowMap, cascade, shadowCoords, uvLightSize);
    if (blockerDistance == -1) // No occlusion
    return 1.0f;

    float penumbraWidth = (shadowCoords.z - blockerDistance) / blockerDistance;

    float NEAR = 0.01; // Should this value be tweakable?
    float uvRadius = penumbraWidth * uvLightSize * NEAR / shadowCoords.z; // Do we need to divide by shadowCoords.z?
    uvRadius = min(uvRadius, 0.002f);
    return PCF_DirectionalLight(shadowMap, cascade, shadowCoords, uvRadius) * ShadowFade;
}

/////////////////////////////////////////////

uint cascadeIndex = 0;

float computeCascadeShadows() {

    const uint SHADOW_MAP_CASCADE_COUNT = 4;
    for (uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; i++) {
        if (fragment.viewPosition.z < u_CascadeSplits[i])
        cascadeIndex = i + 1;
    }

    float shadowDistance = u_MaxShadowDistance;
    float transitionDistance = u_ShadowFade;
    float distance = length(fragment.viewPosition);
    ShadowFade = distance - (shadowDistance - transitionDistance);
    ShadowFade /= transitionDistance;
    ShadowFade = clamp(1.0 - ShadowFade, 0.0, 1.0);
    float shadowAmount = 1.0;

    bool fadeCascades = u_CascadeFading;
    if (fadeCascades) {
        float cascadeTransitionFade = u_CascadeTransitionFade;

        float c0 = smoothstep(u_CascadeSplits[0] + cascadeTransitionFade * 0.5f, u_CascadeSplits[0] - cascadeTransitionFade * 0.5f, fragment.viewPosition.z);
        float c1 = smoothstep(u_CascadeSplits[1] + cascadeTransitionFade * 0.5f, u_CascadeSplits[1] - cascadeTransitionFade * 0.5f, fragment.viewPosition.z);
        float c2 = smoothstep(u_CascadeSplits[2] + cascadeTransitionFade * 0.5f, u_CascadeSplits[2] - cascadeTransitionFade * 0.5f, fragment.viewPosition.z);
        if (c0 > 0.0 && c0 < 1.0) {
            // Sample 0 & 1
            vec3 shadowMapCoords = (fragment.shadowMapCoords[0].xyz / fragment.shadowMapCoords[0].w);
            float shadowAmount0 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMap, 0, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMap, 0, shadowMapCoords);
            shadowMapCoords = (fragment.shadowMapCoords[1].xyz / fragment.shadowMapCoords[1].w);
            float shadowAmount1 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMap, 1, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMap, 1, shadowMapCoords);

            shadowAmount = mix(shadowAmount0, shadowAmount1, c0);

        } else if (c1 > 0.0 && c1 < 1.0) {
            // Sample 1 & 2
            vec3 shadowMapCoords = (fragment.shadowMapCoords[1].xyz / fragment.shadowMapCoords[1].w);
            float shadowAmount1 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMap, 1, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMap, 1, shadowMapCoords);
            shadowMapCoords = (fragment.shadowMapCoords[2].xyz / fragment.shadowMapCoords[2].w);
            float shadowAmount2 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMap, 2, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMap, 2, shadowMapCoords);

            shadowAmount = mix(shadowAmount1, shadowAmount2, c1);

        }  else if (c2 > 0.0 && c2 < 1.0) {
            // Sample 2 & 3
            vec3 shadowMapCoords = (fragment.shadowMapCoords[2].xyz / fragment.shadowMapCoords[2].w);
            float shadowAmount2 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMap, 2, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMap, 2, shadowMapCoords);
            shadowMapCoords = (fragment.shadowMapCoords[3].xyz / fragment.shadowMapCoords[3].w);
            float shadowAmount3 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMap, 3, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMap, 3, shadowMapCoords);

            shadowAmount = mix(shadowAmount2, shadowAmount3, c2);
        }  else {
            vec3 shadowMapCoords = (fragment.shadowMapCoords[cascadeIndex].xyz / fragment.shadowMapCoords[cascadeIndex].w);
            shadowAmount = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMap, cascadeIndex, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMap, cascadeIndex, shadowMapCoords);
        }
    }  else {
        vec3 shadowMapCoords = (fragment.shadowMapCoords[cascadeIndex].xyz / fragment.shadowMapCoords[cascadeIndex].w);
        shadowAmount = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMap, cascadeIndex, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMap, cascadeIndex, shadowMapCoords);
    }

    return shadowAmount;
}

vec3 reflectanceEquation() {
    vec3 dirLighting = computeDirLights();
    vec3 pointLighting = computePointLights();

    if(u_ShadowsEnabled) {
        dirLighting *= computeCascadeShadows();
    }

    return dirLighting + pointLighting;
}

vec3 getDiffuseIBL() {
    return g_PBR.albedo * texture(u_IrradianceMap, g_PBR.normal).rgb;
}

vec3 getSpecularIBL(vec3 F, float angle) {
    float prefilterLOD = g_PBR.roughness;
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
    if(u_Material.useCombinedMetallicRoughnessMap) {
        return texture(u_MetallicRoughnessMap, uv).b * u_Material.metallic;
    }
    return texture(u_MetallicMap, uv).r * u_Material.metallic;
}

float getRoughness(vec2 uv) {
    if(u_Material.useCombinedMetallicRoughnessMap) {
        return texture(u_MetallicRoughnessMap, uv).g * u_Material.roughness;
    }
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

    vec3 L0 = reflectanceEquation();
    vec3 F = fresnelSchlickRoughness(angle, g_PBR.F0, g_PBR.roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD = kD * (1.0 - g_PBR.metallic);

    vec3 ambient = vec3(0);

    if(u_SkyboxPresent) {
        // If skybox is present, then apply Image Based Lighting (IBL)
        ambient = (kD * getDiffuseIBL() + getSpecularIBL(F, angle)) * g_PBR.occlusion;
    } else {
        vec3 ambientColor = vec3(0.25);
        ambient = kD * ambientColor * g_PBR.albedo * g_PBR.occlusion;
    }

    vec3 color = ambient + L0;

    // HDR tonemapping
    //color /= (color + vec3(1.0));

    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    return vec4(color, 1.0);
}

void main() {
    vec4 color = computeLighting();
    vec4 emissive = u_Material.emissiveColor * texture(u_EmissiveMap, fragment.texCoords);
    out_FragColor = color + emissive;

    if (u_ShowCascades) {
        switch (cascadeIndex) {
            case 0:
            out_FragColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
            break;
            case 1:
            out_FragColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
            break;
            case 2:
            out_FragColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
            break;
            case 3:
            out_FragColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
            break;
        }
    }
}