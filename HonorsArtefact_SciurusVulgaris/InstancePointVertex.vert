#version 450

// Vertex shader for point tree rendering

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outVelocity;

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
    uint showLODView; // The below two shouldn't be here, but just for speed of development, they are.
	uint pointSize;
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
    
    newPos /= newPos.w;
    newPos.xy = (newPos.xy + vec2(1,1))/2.0f;
    
    return (newPos - oldPos).xy;
}

void main() {
    // Calculate instance ID based on total number of models loaded.
    uint instanceID = gl_InstanceIndex * instanceInfo.modelCount + instanceInfo.myModelNumber;

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

    // If out of LOD, place behind rasterized area. 
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

    // Size 1
	gl_PointSize = instanceInfo.pointSize;

    // World pos and normals
	outWorldPos = (transformation.worldMatrices[instanceID] * vec4(inPosition, 1.0)).xyz;
	outNormal = (transformation.worldMatrices[instanceID] * vec4(inNormal, 0.0)).xyz;

	// Vertices color
	outColor = vec4(inColor, 1.0);
	if(instanceInfo.showLODView > 0) outColor = vec4(debugColors[level % 8], 1.0);
}