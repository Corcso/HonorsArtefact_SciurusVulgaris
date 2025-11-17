#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec3 outNormal;

layout(binding = 0) uniform UniformBufferObject {
    mat4 world;
    mat4 view;
    mat4 proj;
} ubo;

//layout(set = 1, binding = 1) uniform LightInfo {
//    float intensity;
//} li;

void main() {

    gl_Position = ubo.proj * ubo.view * ubo.world * vec4(inPosition, 1.0);
   
    outWorldPos = (ubo.world * vec4(inPosition, 1.0)).xyz;
    outNormal = (ubo.world * vec4(inNormal, 0.0)).xyz;
    outColor = vec4(inColor, 1.0);

    gl_PointSize = (1.0 - (gl_Position.z / gl_Position.w)) * 40000.0;
}