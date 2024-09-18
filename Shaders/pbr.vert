#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 texcoord;

layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec2 fragTexcoord;
layout(location = 3) out mat3 tangentBasis;

struct PointLight
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(set = 1, binding = 0) uniform GameObjectBufferData {
    mat4 modelMatrix;
    mat4 normalMatrix;
} gameObject;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main()
{
    vec3 normalWorldSpace = normalize(mat3(gameObject.normalMatrix)*normal);
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
    // Output
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * gameObject.modelMatrix * vec4(position, 1.0);
    fragPosWorld = positionWorld.xyz;
    fragTexcoord = vec2(texcoord.x, 1- texcoord.y);
    tangentBasis = mat3(gameObject.modelMatrix) * mat3(tangent, bitangent, normal);
}