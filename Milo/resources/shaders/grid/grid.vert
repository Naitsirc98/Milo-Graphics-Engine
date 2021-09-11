// Based on the grid shader from https://github.com/TheCherno/Hazel dev

#version 450 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

layout(std140, binding = 0) uniform UniformBuffer {
	mat4 u_ProjViewModel;
	float u_Scale;
	float u_Size;
};

layout (location = 0) out vec2 out_UV;

void main() {
	vec4 position = u_ProjViewModel * vec4(in_Position, 1.0);
	gl_Position = position;
	out_UV = in_UV;
}


