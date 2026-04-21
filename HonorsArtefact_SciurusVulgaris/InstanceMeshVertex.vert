#version 450

// Instanced triangle vertex shader

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTex;

struct VPMatrices{
    mat4 view;
    mat4 proj;
};

layout(binding = 0) readonly buffer UniformBufferObject {
    mat4 worldMatrices[1024000]; // Up to 1,024,000 instances
} transformation;

layout(binding = 2) uniform VPMatricesObject {
	VPMatrices thisFrame;
	VPMatrices lastFrame;
} camProjMatrices;

void main() {

    gl_Position = camProjMatrices.thisFrame.proj * camProjMatrices.thisFrame.view * transformation.worldMatrices[gl_InstanceIndex] * vec4(inPosition, 1.0);
   
    outWorldPos = (transformation.worldMatrices[gl_InstanceIndex] * vec4(inPosition, 1.0)).xyz;
    outNormal = normalize((transformation.worldMatrices[gl_InstanceIndex] * vec4(inNormal, 0.0)).xyz);
    outTex = inTex;
}