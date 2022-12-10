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

struct Camera
{
    vec2 position;
    float zoom;
    float rotation;
};

layout(push_constant) uniform PushConstants
{
    Camera camera;
    vec2 resolution;
} pushConstants;

layout(std140, set = 0, binding = 0) readonly buffer InstanceBuffer
{
	InstanceData instances[];
} instanceBuffer;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragPos;

void HandleInstance(in InstanceData instance)
{
    vec2 pos = inPosition.xy * instance.scale + instance.position;
    pos *= vec2(1.0 + pushConstants.camera.zoom);
    pos = Rotate(pos, pushConstants.camera.rotation);
    float aspectFix = pushConstants.resolution.y / pushConstants.resolution.x;
    pos.x *= aspectFix;

    gl_Position = vec4(pos, 0.0, 1.0);
    fragPos = CalculateTextureCoordinates(instance.subTexture, inTexCoords);
    fragColor = instance.color.xyz;
}

void main() 
{
    HandleInstance(instanceBuffer.instances[gl_InstanceIndex]);
}