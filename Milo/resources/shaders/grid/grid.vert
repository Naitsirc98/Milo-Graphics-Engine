#version 450 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoords;

layout(std140, binding = 0) uniform Camera {
	mat4 projView;
} u_Camera;

layout(push_constant) uniform PushConstants {
	mat4 u_ModelMatrix;
};

layout (location = 0) out vec2 out_UV;

void main() {
	vec4 position = u_Camera.projView * u_ModelMatrix * vec4(in_Position, 1.0);
	gl_Position = position;
	out_UV = in_UV;
}


