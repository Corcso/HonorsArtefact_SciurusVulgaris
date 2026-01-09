#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec3 outNormal;

struct WCPMatrices{
    mat4 world;
    mat4 view;
    mat4 proj;
};

layout(binding = 0) readonly buffer UniformBufferObject {
    WCPMatrices matrices[40000];
} ubo;

void main() {

    gl_Position = ubo.matrices[gl_InstanceIndex].proj * ubo.matrices[gl_InstanceIndex].view * ubo.matrices[gl_InstanceIndex].world * vec4(inPosition, 1.0);
   
    outWorldPos = (ubo.matrices[gl_InstanceIndex].world * vec4(inPosition, 1.0)).xyz;
    outNormal = (ubo.matrices[gl_InstanceIndex].world * vec4(inNormal, 0.0)).xyz;
    outColor = vec4(inColor, 1.0);

    gl_PointSize = (1.0 - (gl_Position.z / gl_Position.w)) * 10000.0;
}