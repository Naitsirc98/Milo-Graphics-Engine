// References upon which this is based:
// - Unreal Engine 4 PBR notes (https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf)
// - Frostbite's SIGGRAPH 2014 paper (https://seblagarde.wordpress.com/2015/07/14/siggraph-2014-moving-frostbite-to-physically-based-rendering/)
// - Michał Siejak's PBR project (https://github.com/Nadrin)
// - Sparky engine (https://github.com/TheCherno/Sparky)
// - Hazel engine (https://github.com/TheCherno/Hazel.git) (dev)
#version 450 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoords;
layout(location = 3) in vec3 in_Tangent;
layout(location = 4) in vec3 in_BiTangent;

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

layout(push_constant) uniform PushConstants {
    mat4 u_ModelMatrix;
};

layout(location = 0) out VertexOutput {
    vec3 worldPosition;
    vec3 normal;
    vec2 texCoords;
    mat3 worldNormals;
    mat3 worldTransform;
    vec3 biTangent;

    mat3 cameraView;

    vec4 shadowMapCoords[4];
    vec3 viewPosition;
} vertex;

void main() {

    vertex.worldPosition = vec3(u_ModelMatrix * vec4(in_Position, 1.0));
    vertex.normal = mat3(u_ModelMatrix) * in_Normal;
    vertex.texCoords = vec2(in_TexCoords.x, 1.0 - in_TexCoords.y);
    vertex.worldNormals = mat3(u_ModelMatrix) * mat3(in_Tangent, in_BiTangent, in_Normal);
    vertex.worldTransform = mat3(u_ModelMatrix);
    vertex.biTangent = in_BiTangent;

    vertex.cameraView = mat3(u_Camera.view);

    vertex.shadowMapCoords[0] = u_LightMatrix[0] * vec4(vertex.worldPosition, 1.0);
    vertex.shadowMapCoords[1] = u_LightMatrix[1] * vec4(vertex.worldPosition, 1.0);
    vertex.shadowMapCoords[2] = u_LightMatrix[2] * vec4(vertex.worldPosition, 1.0);
    vertex.shadowMapCoords[3] = u_LightMatrix[3] * vec4(vertex.worldPosition, 1.0);
    vertex.viewPosition = vec3(u_Camera.view * vec4(vertex.worldPosition, 1.0));

    gl_Position = u_Camera.viewProjection * u_ModelMatrix * vec4(in_Position, 1.0);
}

