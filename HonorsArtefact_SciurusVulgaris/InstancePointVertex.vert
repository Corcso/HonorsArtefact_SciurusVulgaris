#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outVelocity;

//struct WCPMatrices{
//    mat4 world;
//    mat4 view;
//    mat4 proj;
//};

struct VPMatrices{
    mat4 view;
    mat4 proj;
};

layout(binding = 1) readonly buffer UniformBufferObject {
    mat4 worldMatrices[1024000];
} transformation;

layout(binding = 6) uniform VPMatricesObject {
	VPMatrices thisFrame;
	VPMatrices lastFrame;
} camProjMatrices;

layout(binding = 2) uniform InstancingInfo {
    uint instanceCount;
    uint instanceStride;
    uint modelCount;
	uint myModelNumber;
} instanceInfo; 

layout(binding = 4) uniform LODDataBuffer {
    vec4 cameraPosition;
    int maxVertexLevels[16];
    int maxLevel;
    float continousStart, continousShallowness, continousDecay;
	int lodType;
} levelOfDetailData;

layout(binding = 5) uniform TAAInfo {
	vec2 currentJitter;
    vec2 inverseScreenSize;
	bool enabled;
    bool logarithmicColorSpace;
} taaInfo; 

const vec3[] debugColors = {vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0), vec3(0.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0)};

// (Lee, 2021)
vec2 CalcVelocity(vec4 newPos, vec4 oldPos)
{
    oldPos /= oldPos.w;
    oldPos.xy = (oldPos.xy + vec2(1,1))/2.0f;
    //oldPos.y = 1 - oldPos.y;
    
    newPos /= newPos.w;
    newPos.xy = (newPos.xy + vec2(1,1))/2.0f;
    //newPos.y = 1 - newPos.y;
    
    return (newPos - oldPos).xy;
}

void main() {
    uint instanceID = gl_InstanceIndex;

    // Calculate LOD
    vec3 instancePosition = (transformation.worldMatrices[instanceID] * vec4(0, 0, 0, 1)).xyz;
    vec3 cameraPosition = levelOfDetailData.cameraPosition.xyz; 

    float distanceToInstance = length(instancePosition - cameraPosition);
    uint level = 0;
    uint pointsMaxToDraw = 0;
    if(levelOfDetailData.lodType == 0){ // RANDOM_LEVELS
        level =  clamp(uint(floor(distanceToInstance / 10.0f)), 0, levelOfDetailData.maxLevel - 1);
        pointsMaxToDraw = uint(levelOfDetailData.maxVertexLevels[level]);
    }
    else if(levelOfDetailData.lodType == 1){ // CONTINUOUS
        level = uint(floor(distanceToInstance / levelOfDetailData.continousShallowness));
        pointsMaxToDraw = uint((levelOfDetailData.continousStart * pow(levelOfDetailData.continousDecay, -distanceToInstance * (1.0f / levelOfDetailData.continousShallowness))));
    }

    // For mesh shader parity, round up to the nearest 128
    pointsMaxToDraw = max(uint(ceil(float(pointsMaxToDraw) / 128.0f) * 128.0f), 128);

    if(pointsMaxToDraw <= gl_VertexIndex){
        gl_Position = vec4(0, 0, -2, 1);
        return;
    }

	// Vertices position
	vec4 screenSpacePosition =  camProjMatrices.thisFrame.proj * camProjMatrices.thisFrame.view * transformation.worldMatrices[instanceID] * vec4(inPosition, 1.0); 
	vec4 lastScreenSpacePosition =  camProjMatrices.lastFrame.proj * camProjMatrices.lastFrame.view * transformation.worldMatrices[instanceID] * vec4(inPosition, 1.0); 
	outVelocity = CalcVelocity(screenSpacePosition, lastScreenSpacePosition);
	// Apply TAA Jitter
	if(taaInfo.enabled) screenSpacePosition += vec4(taaInfo.currentJitter * screenSpacePosition.w * 1.2f, 0, 0);
	gl_Position = screenSpacePosition;

	gl_PointSize = 1;

	outWorldPos = (transformation.worldMatrices[instanceID] * vec4(inPosition, 1.0)).xyz;

	outNormal = (transformation.worldMatrices[instanceID] * vec4(inNormal, 0.0)).xyz;

	// Vertices color
	outColor = vec4(inColor, 1.0);
	//outColor[gl_LocalInvocationID.x] = vec4(debugColors[level % 8], 1.0);

    // =============

//    outWorldPos = (ubo.matrices[gl_InstanceIndex].world * vec4(inPosition, 1.0)).xyz;
//    
//    int level =  clamp(int(floor(length(outWorldPos - levelOfDetailData.cameraPosition.xyz) / 10.0f)), 0, levelOfDetailData.maxLevel - 1);
//    if(levelOfDetailData.maxVertexLevels[level] < gl_VertexIndex){
//        gl_Position = vec4(0, 0, -2, 1);
//        return;
//    }
//    
//
//    gl_Position = ubo.matrices[gl_InstanceIndex].proj * ubo.matrices[gl_InstanceIndex].view * ubo.matrices[gl_InstanceIndex].world * vec4(inPosition, 1.0);
//
//    outNormal = (ubo.matrices[gl_InstanceIndex].world * vec4(inNormal, 0.0)).xyz;
//    outColor = vec4(inColor, 1.0);
//
//    gl_PointSize = (1.0 - (gl_Position.z / gl_Position.w)) * 10000.0;
}