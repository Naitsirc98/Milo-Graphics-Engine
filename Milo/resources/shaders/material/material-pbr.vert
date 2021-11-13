#version 450 core

layout(std140, set = 0, binding = 0) uniform CameraData {
	mat4 viewProjectionMatrix;
	mat4 viewMatrix;
	vec4 position;
} u_Camera;

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
	vec3 viewPosition;
} fragment;

void main() {

	vec4 worldPos = vec4(in_Position, 1.0);

	fragment.position = worldPos.xyz;
	fragment.normal = normalize(in_Normal);
	fragment.texCoords = vec2(in_TexCoords.x, -in_TexCoords.y);

	fragment.cameraView = mat3(u_Camera.viewMatrix);
	fragment.viewPosition = vec3(u_Camera.viewMatrix * vec4(fragment.position, 1.0));

	gl_Position = u_Camera.viewProjectionMatrix * worldPos;
}