#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTex;

struct WCPMatrices{
    mat4 world;
    mat4 view;
    mat4 proj;
};

layout(binding = 0) readonly buffer UniformBufferObject {
    WCPMatrices matrices[400];
} ubo;

//layout(set = 1, binding = 1) uniform LightInfo {
//    float intensity;
//} li;

void main() {

    gl_Position = ubo.matrices[gl_InstanceIndex].proj * ubo.matrices[gl_InstanceIndex].view * ubo.matrices[gl_InstanceIndex].world * vec4(inPosition, 1.0);
   
    outWorldPos = (ubo.matrices[gl_InstanceIndex].world * vec4(inPosition, 1.0)).xyz;
    outNormal = normalize((ubo.matrices[gl_InstanceIndex].world * vec4(inNormal, 0.0)).xyz);
    outTex = inTex;
}