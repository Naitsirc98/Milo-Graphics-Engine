#version 450 core

layout(std140, binding = 0) uniform UniformBuffer {
	mat4 u_ProjViewModel;
	float u_Scale;
	float u_Size;
};

layout(location = 0) in vec2 in_UV;

layout(location = 0) out vec4 out_FragColor;

float grid(vec2 st, float res) {
	vec2 grid = fract(st);
	return step(res, grid.x) * step(res, grid.y);
}

void main() {
	float x = grid(in_UV * u_Scale, u_Size);
	out_FragColor = vec4(vec3(0.1), 0.25) * (1.0 - x);
	if(out_FragColor.a == 0.0) discard;
}