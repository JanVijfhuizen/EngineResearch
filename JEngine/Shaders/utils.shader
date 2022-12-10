#ifndef UTILS
#define UTILS
struct Transform
{
    vec2 position;
    float rotation;
    float scale;
};

struct SubTexture
{
    vec2 lTop;
    vec2 rBot;
};

vec2 Rotate(in vec2 pos, in float rotation)
{
    float newX = pos.x * cos(rotation) - pos.y * sin(rotation);
    float newY = pos.x * sin(rotation) + pos.y * cos(rotation);
    return vec2(newX, newY);
}

SubTexture GetSubTexture(in SubTexture subTexture, in uint index, in uint length)
{
    SubTexture ret = subTexture;
    float part = (ret.rBot.x - ret.lTop.x) / length;
    ret.lTop.x += part;
    ret.rBot.x = ret.lTop.x + part;
    return ret;
}

vec2 CalculateTextureCoordinates(in SubTexture subTexture, in vec2 texCoords)
{
    return subTexture.lTop + (subTexture.rBot - subTexture.lTop) * texCoords;
}

float GetAspectRatio(in vec2 resolution)
{
    return resolution.y / resolution.x;
}

vec4 CalculatePoint(in vec2 vertPosition, in vec2 camPosition, in vec2 resolution, in float pixelSize)
{
    float aspectFix = GetAspectRatio(resolution);
    vec2 worldPos = (vertPosition - camPosition) * pixelSize;
    vec4 pos = vec4(worldPos, 1, 1);
    pos.x *= aspectFix;
    return pos;
}

vec4 CalculatePositionUI(in vec2 position, in vec2 scale, in vec2 camPosition, in vec2 vertPosition, in vec2 resolution, in float pixelSize)
{
    float aspectFix = GetAspectRatio(resolution);
    vec2 localPos = vertPosition * scale;
    vec2 worldPos = (position - camPosition + vec2(localPos.x * aspectFix, localPos.y)) * pixelSize;
    vec4 pos = vec4(worldPos, 1, 1);
    return pos;
}

vec4 CalculatePosition(in vec2 position, in vec2 scale, in vec2 camPosition, in vec2 vertPosition, in vec2 resolution, in float pixelSize)
{
    float aspectFix = GetAspectRatio(resolution);
    vec2 localPos = vertPosition * scale;
    vec2 worldPos = (position - camPosition + localPos) * pixelSize;
    vec4 pos = vec4(worldPos, 1, 1);
    pos.x *= aspectFix;
    return pos;
}

vec4 CalculatePosition(in Transform transform, in vec2 camPosition, in vec2 vertPosition, in vec2 resolution, in float pixelSize)
{
    float aspectFix = GetAspectRatio(resolution);
    vec2 localPos = vertPosition * transform.scale;
    vec2 worldPos = (transform.position - camPosition + Rotate(localPos, transform.rotation)) * pixelSize;
    vec4 pos = vec4(worldPos, 1, 1);
    pos.x *= aspectFix;
    return pos;
}
#endif