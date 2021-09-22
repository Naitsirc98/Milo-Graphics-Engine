#version 450 core

layout(std140, binding = 0) uniform Camera {
    mat4 u_ProjMatrix;
    mat4 u_ViewMatrix;
    mat4 u_ProjViewMatrix;
};

layout(location = 0) in float in_LinearDepth;

layout(location = 0) out vec4 out_FragColor;
layout(location = 1) out vec4 out_DebugFragColor;

float zNear = u_ProjMatrix[3][2];
float zFar = u_ProjMatrix[2][2];

float linearizeDepth(float depth) {
    return zNear * zFar / (zFar + depth * (zNear - zFar));
}

void main() {
    out_FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    out_DebugFragColor = vec4(vec3(linearizeDepth(gl_FragCoord.z) / zFar), 1.0);
}