#version 450
#include "utils.shader"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

struct InstanceData
{
    vec2 position;
    vec2 scale;
    SubTexture subTexture;
    vec4 color;
};

layout(std140, set = 0, binding = 0) readonly buffer InstanceBuffer
{
	InstanceData instances[];
} instanceBuffer;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragPos;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(inPosition.xyz, 1.0);
    fragPos = inTexCoords;
    fragColor = colors[gl_VertexIndex];
}