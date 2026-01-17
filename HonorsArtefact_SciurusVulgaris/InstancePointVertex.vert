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
    WCPMatrices matrices[400];
} ubo;

layout(binding = 1) uniform LODDataBuffer {
    vec4 cameraPosition;
    int maxVertexLevels[16];
    int maxLevel;
} levelOfDetailData;

void main() {

    

    outWorldPos = (ubo.matrices[gl_InstanceIndex].world * vec4(inPosition, 1.0)).xyz;
    
    int level =  clamp(int(floor(length(outWorldPos - levelOfDetailData.cameraPosition.xyz) / 10.0f)), 0, levelOfDetailData.maxLevel - 1);
    if(levelOfDetailData.maxVertexLevels[level] < gl_VertexIndex){
        gl_Position = vec4(0, 0, -2, 1);
        return;
    }
    

    gl_Position = ubo.matrices[gl_InstanceIndex].proj * ubo.matrices[gl_InstanceIndex].view * ubo.matrices[gl_InstanceIndex].world * vec4(inPosition, 1.0);

    outNormal = (ubo.matrices[gl_InstanceIndex].world * vec4(inNormal, 0.0)).xyz;
    outColor = vec4(inColor, 1.0);

    gl_PointSize = (1.0 - (gl_Position.z / gl_Position.w)) * 10000.0;
}