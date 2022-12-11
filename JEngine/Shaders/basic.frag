#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragPos;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D textureAtlas;

void main() 
{
    vec4 color = texture(textureAtlas, fragPos) * vec4(fragColor, 1);
     if(color.a < .01f)
        discard;
    outColor = color;
}