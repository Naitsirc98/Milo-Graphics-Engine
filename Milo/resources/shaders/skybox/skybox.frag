#version 450 core

layout(std140, set = 0, binding = 0) uniform UniformBuffer {
    mat4 u_InverseProjViewMatrix;
    float u_TextureLOD;
    float u_Intensity;
};

layout(set = 0, binding = 1) uniform samplerCube u_Texture;

layout (location = 0) in vec3 in_Position;

layout(location = 0) out vec4 out_FragColor;

void main() {

    out_FragColor = textureLod(u_Texture, in_Position, 0);

    // HDR tonemapping
    out_FragColor /= (out_FragColor + vec4(1.0));
    // Gamma correct
    out_FragColor = pow(out_FragColor, vec4(1.0 / 2.2));
}
