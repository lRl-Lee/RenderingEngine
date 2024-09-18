#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct PointLight
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

// uniform buffer
layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    vec4 ambientLightColor; // w is intensity
    PointLight PointLights[10];
    int numLights;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform Push
{
    vec4 position;
    vec4 color;
    float radius;
} push;

const float M_PI = 3.1415926538;

void main()
{
    float disSquare = dot(fragOffset, fragOffset);
    if(disSquare >= 1.0){
        discard;
    }
    float cosDis = 0.5 * (cos(disSquare * M_PI) + 1.0); // ranges from 1 -> 0
    outColor = vec4(push.color.xyz + 0.5 * cosDis, cosDis);
}