#version 450 core

const float PI = 3.141592;
const float Epsilon = 0.00001;

const int DirLightsCount = 1;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

layout(std140, set = 0, binding = 0) uniform CameraData {
    mat4 viewProjection;
    mat4 view;
    vec3 position;
} u_Camera;

layout(std140, set = 0, binding = 1) uniform RendererData {
    mat4 u_LightMatrix[4];
    vec4 u_CascadeSplits;
    vec2 u_InvFullResolution;
    vec2 u_FullResolution;
    int u_TilesCountX;
    bool u_ShowCascades;
    bool u_SoftShadows;
    float u_LightSize;
    float u_MaxShadowDistance;
    float u_ShadowFade;
    bool u_CascadeFading;
    float u_CascadeTransitionFade;
    bool u_ShowLightComplexity;
};

struct DirectionalLight {
    vec4 direction;
    vec3 radiance;
    float multiplier;
    vec4 _unused;
};

struct PointLight {
    vec3 position;
    float multiplier;
    vec3 radiance;
    float minRadius;
    float radius;
    float fallOff;
    float lightSize;
    bool castsShadows;
};

layout(std140, set = 0, binding = 2) uniform SceneData {
    DirectionalLight u_DirLight;
    float u_EnvironmentMapIntensity;
    uint u_PointLightsCount;
    PointLight u_pointLights[1024];
};

layout(std430, set = 0, binding = 3) readonly buffer VisibleLightIndicesBuffer {
    int indices[];
} visibleLightIndicesBuffer;

// ====================================================================== Environment

layout(set = 1, binding = 0) uniform samplerCube u_EnvRadianceTex;
layout(set = 1, binding = 1) uniform samplerCube u_EnvIrradianceTex;
layout(set = 1, binding = 2) uniform samplerCube u_PrefilterMap;
layout(set = 1, binding = 3) uniform sampler2D u_BRDFLUTTexture;

// ====================================================================== Shadows

// Shadow maps, 4 cascades
layout(set = 2, binding = 0) uniform sampler2DArray u_ShadowMapTexture;

// ======================================================================== MATERIAL

layout(std140, set = 3, binding = 0) uniform Material {
    vec4 albedo;
    vec4 emissive;
    float alpha;
    float metalness;
    float roughness;
    float occlusion;
    float fresnel0;
    float normalScale;
    bool useNormalMap;
} u_Material;

// PBR texture inputs
layout(set = 3, binding = 1) uniform sampler2D u_AlbedoTexture;
layout(set = 3, binding = 2) uniform sampler2D u_EmissiveTexture;
layout(set = 3, binding = 3) uniform sampler2D u_NormalTexture;
layout(set = 3, binding = 4) uniform sampler2D u_MetalnessTexture;
layout(set = 3, binding = 5) uniform sampler2D u_RoughnessTexture;
layout(set = 3, binding = 6) uniform sampler2D u_OcclussionTexture;

layout(location = 0) in FragmentData {
    vec3 worldPosition;
    vec3 normal;
    vec2 texCoord;
    mat3 worldNormals;
    mat3 worldTransform;
    vec3 biTangent;

    mat3 cameraView;

    vec4 shadowMapCoords[4];
    vec3 viewPosition;
} fragment;

layout(location = 0) out vec4 out_FragColor;

struct PBRParameters {
    vec3 albedo;
    float roughness;
    float metalness;
    float occlusion;

    vec3 normal;
    vec3 view;
    float NdotV;
    vec3 reflectDir;
    vec3 F0;
};

PBRParameters m_Params;

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float ndfGGX(float cosLh, float roughness) {
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k) {
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(NdotV, k);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// ---------------------------------------------------------------------------------------------------
// The following code (from Unreal Engine 4's paper) shows how to filter the environment map
// for different roughnesses. This is mean to be computed offline and stored in cube map mips,
// so turning this on online will cause poor performance
float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N) {
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, float Roughness, vec3 N) {
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt((1 - Xi.y) / (1 + (a * a - 1) * Xi.y));
    float SinTheta = sqrt(1 - CosTheta * CosTheta);
    vec3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 TangentX = normalize(cross(UpVector, N));
    vec3 TangentY = cross(N, TangentX);
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float TotalWeight = 0.0;

vec3 PrefilterEnvMap(float Roughness, vec3 R) {
    vec3 N = R;
    vec3 V = R;
    vec3 PrefilteredColor = vec3(0.0);
    int NumSamples = 1024;
    for (int i = 0; i < NumSamples; i++) {
        vec2 Xi = Hammersley(i, NumSamples);
        vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
        vec3 L = 2 * dot(V, H) * H - V;
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        if (NoL > 0)
        {
            //PrefilteredColor += texture(u_EnvRadianceTex, L).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}

// ---------------------------------------------------------------------------------------------------

vec3 RotateVectorAboutY(float angle, vec3 vec) {
    angle = radians(angle);
    mat3x3 rotationMatrix = { vec3(cos(angle),0.0,sin(angle)),
    vec3(0.0,1.0,0.0),
    vec3(-sin(angle),0.0,cos(angle)) };
    return rotationMatrix * vec;
}

vec3 CalculateDirLights(vec3 F0) {
    vec3 Li = u_DirLight.direction.xyz;
    vec3 Lradiance = u_DirLight.radiance * u_DirLight.multiplier;
    vec3 Lh = normalize(Li + m_Params.view);

    // Calculate angles between surface normal and various light vectors.
    float cosLi = max(0.0, dot(m_Params.normal, Li));
    float cosLh = max(0.0, dot(m_Params.normal, Lh));

    vec3 F = fresnelSchlickRoughness(F0, max(0.0, dot(Lh, m_Params.view)), m_Params.roughness);
    float D = ndfGGX(cosLh, m_Params.roughness);
    float G = gaSchlickGGX(cosLi, m_Params.NdotV, m_Params.roughness);

    vec3 kd = (1.0 - F) * (1.0 - m_Params.metalness);
    vec3 diffuseBRDF = kd * m_Params.albedo;

    // Cook-Torrance
    vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);
    specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
    return (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
}

const int TILE_SIZE = 16;
const ivec2 TILE = ivec2(TILE_SIZE, TILE_SIZE);

int GetLightBufferIndex(int i) {
    ivec2 tileID = ivec2(gl_FragCoord) / TILE;
    uint index = tileID.y * u_TilesCountX + tileID.x;
    uint offset = index * 1024;
    return visibleLightIndicesBuffer.indices[offset + i];
}

int GetPointLightCount() {
    int result = 0;
    for(int i = 0; i < u_PointLightsCount; ++i) {
        uint lightIndex = GetLightBufferIndex(i);
        if(lightIndex == -1) break;
        ++result;
    }

    return result;
}

vec3 CalculatePointLights(in vec3 F0) {
    vec3 result = vec3(0.0);
    for (int i = 0; i < u_PointLightsCount; i++) {
        uint lightIndex = GetLightBufferIndex(i);
        if (lightIndex == -1)
        break;

        PointLight light = u_pointLights[lightIndex];
        vec3 Li = normalize(light.position - fragment.worldPosition);
        float lightDistance = length(light.position - fragment.worldPosition);
        vec3 Lh = normalize(Li + m_Params.view);

        float attenuation = clamp(1.0 - (lightDistance * lightDistance) / (light.radius * light.radius), 0.0, 1.0);
        attenuation *= mix(attenuation, 1.0, light.fallOff);

        vec3 Lradiance = light.radiance * light.multiplier * attenuation;

        // Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.0, dot(m_Params.normal, Li));
        float cosLh = max(0.0, dot(m_Params.normal, Lh));

        vec3 F = fresnelSchlickRoughness(F0, max(0.0, dot(Lh, m_Params.view)), m_Params.roughness);
        float D = ndfGGX(cosLh, m_Params.roughness);
        float G = gaSchlickGGX(cosLi, m_Params.NdotV, m_Params.roughness);

        vec3 kd = (1.0 - F) * (1.0 - m_Params.metalness);
        vec3 diffuseBRDF = kd * m_Params.albedo;

        // Cook-Torrance
        vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);
        specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
        result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }
    return result;
}

float Convert_sRGB_FromLinear(float theLinearValue) {
    return theLinearValue <= 0.0031308f
    ? theLinearValue * 12.92f
    : pow(theLinearValue, 1.0f / 2.4f) * 1.055f - 0.055f;
}

vec3 getDiffuseIBL() {
    return m_Params.albedo * texture(u_EnvIrradianceTex, m_Params.normal).rgb;
}

vec3 getSpecularIBL(vec3 F) {
    float prefilterLOD = m_Params.roughness * 4 + -0.25;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, reflect(-m_Params.view, m_Params.normal), prefilterLOD).rgb;
    vec2 brdf = texture(u_BRDFLUTTexture, vec2(m_Params.NdotV, m_Params.roughness)).rg;
    return prefilteredColor * (F * brdf.x + brdf.y);
}

float TotalWeight = 0.0;

vec3 PrefilterEnvMap(float Roughness, vec3 R) {
    vec3 N = R;
    vec3 V = R;
    vec3 PrefilteredColor = vec3(0.0);
    int NumSamples = 1024;
    for (int i = 0; i < NumSamples; i++) {
        vec2 Xi = Hammersley(i, NumSamples);
        vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
        vec3 L = 2 * dot(V, H) * H - V;
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        if (NoL > 0) {
            //PrefilteredColor += texture(u_EnvRadianceTex, L).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}

vec3 IBL(vec3 F0, vec3 Lr) {

    vec3 F = fresnelSchlickRoughness(m_Params.F0, m_Params.NdotV, m_Params.roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD = kD * (1.0 - m_Params.metalness);

    return PrefilterEnvMap(m_Params.roughness, m_Params.reflectDir);

    //return (kD * getDiffuseIBL() + getSpecularIBL(F)) * m_Params.occlusion;
}

vec4 getAlbedo(vec2 uv) {
    return u_Material.albedo * pow(texture(u_AlbedoTexture, uv), vec4(2.2));
}

float getMetalness(vec2 uv) {
    return texture(u_MetalnessTexture, uv).r * u_Material.metalness;
}

float getRoughness(vec2 uv) {
    return texture(u_RoughnessTexture, uv).r * u_Material.roughness;
}

float getOcclusion(vec2 uv) {
    return texture(u_OcclussionTexture, uv).r * u_Material.occlusion;
}

vec3 getF0(vec3 albedo, float metalness) {
    return mix(vec3(u_Material.fresnel0), albedo, metalness);
}

void main() {
    // Standard PBR inputs
    vec2 texCoord = fragment.texCoord;

    m_Params.albedo = getAlbedo(texCoord).rgb;
    m_Params.metalness = getMetalness(texCoord);
    m_Params.roughness = max(getRoughness(texCoord), 0.05);
    m_Params.occlusion = getOcclusion(texCoord);
    m_Params.F0 = getF0(m_Params.albedo, m_Params.metalness);

    // Normals (either from vertex or map)
    m_Params.normal = normalize(fragment.normal);
    if (u_Material.useNormalMap)
    {
        m_Params.normal = normalize(texture(u_NormalTexture, fragment.texCoord).rgb * 2.0f - 1.0f);
        m_Params.normal = normalize(fragment.worldNormals * m_Params.normal);
    }

    m_Params.view = normalize(u_Camera.position - fragment.worldPosition);
    m_Params.reflectDir = reflect(-m_Params.view, m_Params.normal);
    m_Params.NdotV = max(dot(m_Params.normal, m_Params.view), 0.0);

    // Specular reflection vector
    vec3 Lr = 2.0 * m_Params.NdotV * m_Params.normal - m_Params.view;

    // Fresnel reflectance, metals use albedo
    vec3 F0 = mix(Fdielectric, m_Params.albedo, m_Params.metalness);

    vec3 lightContribution = vec3(0);//CalculateDirLights(F0);
    //lightContribution += CalculatePointLights(F0);
    lightContribution += m_Params.albedo * u_Material.emissive.rgb;
    vec3 iblContribution = IBL(F0, m_Params.reflectDir);

    out_FragColor = vec4(lightContribution + iblContribution, 1.0);

    /*

    uint cascadeIndex = 0;

    const uint SHADOW_MAP_CASCADE_COUNT = 4;
    for (uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; i++) {
        if (fragment.viewPosition.z < u_CascadeSplits[i])
        cascadeIndex = i + 1;
    }

    float shadowDistance = u_MaxShadowDistance;//u_CascadeSplits[3];
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
        if (c0 > 0.0 && c0 < 1.0){
            // Sample 0 & 1
            vec3 shadowMapCoords = (fragment.shadowMapCoords[0].xyz / fragment.shadowMapCoords[0].w);
            float shadowAmount0 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMapTexture, 0, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMapTexture, 0, shadowMapCoords);
            shadowMapCoords = (fragment.shadowMapCoords[1].xyz / fragment.shadowMapCoords[1].w);
            float shadowAmount1 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMapTexture, 1, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMapTexture, 1, shadowMapCoords);

            shadowAmount = mix(shadowAmount0, shadowAmount1, c0);

        }  else if (c1 > 0.0 && c1 < 1.0) {
            // Sample 1 & 2
            vec3 shadowMapCoords = (fragment.shadowMapCoords[1].xyz / fragment.shadowMapCoords[1].w);
            float shadowAmount1 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMapTexture, 1, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMapTexture, 1, shadowMapCoords);
            shadowMapCoords = (fragment.shadowMapCoords[2].xyz / fragment.shadowMapCoords[2].w);
            float shadowAmount2 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMapTexture, 2, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMapTexture, 2, shadowMapCoords);

            shadowAmount = mix(shadowAmount1, shadowAmount2, c1);

        } else if (c2 > 0.0 && c2 < 1.0) {
            // Sample 2 & 3
            vec3 shadowMapCoords = (fragment.shadowMapCoords[2].xyz / fragment.shadowMapCoords[2].w);
            float shadowAmount2 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMapTexture, 2, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMapTexture, 2, shadowMapCoords);
            shadowMapCoords = (fragment.shadowMapCoords[3].xyz / fragment.shadowMapCoords[3].w);
            float shadowAmount3 = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMapTexture, 3, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMapTexture, 3, shadowMapCoords);

            shadowAmount = mix(shadowAmount2, shadowAmount3, c2);

        } else  {
            vec3 shadowMapCoords = (fragment.shadowMapCoords[cascadeIndex].xyz / fragment.shadowMapCoords[cascadeIndex].w);
            shadowAmount = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMapTexture, cascadeIndex, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMapTexture, cascadeIndex, shadowMapCoords);
        }
    } else {
        vec3 shadowMapCoords = (fragment.shadowMapCoords[cascadeIndex].xyz / fragment.shadowMapCoords[cascadeIndex].w);
        shadowAmount = u_SoftShadows ? PCSS_DirectionalLight(u_ShadowMapTexture, cascadeIndex, shadowMapCoords, u_LightSize) : HardShadows_DirectionalLight(u_ShadowMapTexture, cascadeIndex, shadowMapCoords);
    }

    vec3 lightContribution = CalculateDirLights(F0) * shadowAmount;
    lightContribution += CalculatePointLights(F0);
    lightContribution += m_Params.albedo * u_u_Material.emissive.rgb;
    vec3 iblContribution = IBL(F0, Lr) * u_EnvironmentMapIntensity;

    out_FragColor = vec4(iblContribution + lightContribution, 1.0);

    if (u_ShowLightComplexity) {
        int pointLightCount = GetPointLightCount();
        float value = float(pointLightCount);
        out_FragColor.rgb = (out_FragColor.rgb * 0.2) + GetGradient(value);
    }

    out_FragColor.a = alpha;

    // (shading-only)
    // color.rgb = vec3(1.0) * shadowAmount + 0.2f;

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

    */
}