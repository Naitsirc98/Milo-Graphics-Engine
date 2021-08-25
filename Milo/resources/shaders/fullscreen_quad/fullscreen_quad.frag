#version 450 core

layout(set = 0, binding = 0) uniform sampler2D u_Texture;

layout(location = 0) in vec2 frag_TexCoords;

layout(location = 0) out vec4 out_FragColor;

void main() {
	out_FragColor = texture(u_Texture, frag_TexCoords);
}