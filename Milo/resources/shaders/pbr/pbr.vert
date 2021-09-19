#version 450 core

layout(std140, set = 0, binding = 0) uniform CameraData {
	mat4 viewProjectionMatrix;
	mat4 viewMatrix;
	vec4 position;
} u_Camera;

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

layout(push_constant) uniform PushConstants {
	mat4 u_ModelMatrix;
};

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoords;
layout(location = 3) in vec3 in_Tangent;
layout(location = 4) in vec3 in_BiTangent;

layout(location = 0) out Fragment {
	vec3 position;
	vec3 normal;
	vec2 texCoords;
	mat3 cameraView;
	vec4 shadowMapCoords[4];
	vec3 viewPosition;
} fragment;

void main() {

	vec4 worldPos = u_ModelMatrix * vec4(in_Position, 1.0);

	fragment.position = worldPos.xyz;
	fragment.normal = mat3(u_ModelMatrix) * normalize(in_Normal);
	fragment.texCoords = vec2(in_TexCoords.x, -in_TexCoords.y);

	fragment.cameraView = mat3(u_Camera.viewMatrix);
	fragment.viewPosition = vec3(u_Camera.viewMatrix * vec4(fragment.position, 1.0));

	if(u_ShadowsEnabled) {
		fragment.shadowMapCoords[0] = u_LightMatrix[0] * vec4(fragment.position, 1.0);
		fragment.shadowMapCoords[1] = u_LightMatrix[1] * vec4(fragment.position, 1.0);
		fragment.shadowMapCoords[2] = u_LightMatrix[2] * vec4(fragment.position, 1.0);
		fragment.shadowMapCoords[3] = u_LightMatrix[3] * vec4(fragment.position, 1.0);
	}

	gl_Position = u_Camera.viewProjectionMatrix * worldPos;
}