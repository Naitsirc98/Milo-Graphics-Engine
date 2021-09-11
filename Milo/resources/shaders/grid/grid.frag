#version 450 core

layout(location = 0) out vec4 color;

layout (push_constant) uniform Settings
{
	layout (offset = 64) float Scale;
	float Size;
} u_Settings;

layout (location = 0) in vec2 v_TexCoord;

float grid(vec2 st, float res)
{
	vec2 grid = fract(st);
	return step(res, grid.x) * step(res, grid.y);
}

void main()
{
	float x = grid(v_TexCoord * u_Settings.Scale, u_Settings.Size);
	color = vec4(vec3(0.2), 0.5) * (1.0 - x);
	unused0 = vec4(0.0);
	unused1 = vec4(0.0);
	if (color.a == 0.0)
	discard;
}