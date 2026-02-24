#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTex;
layout(location = 3) out vec2 outVelocity;

layout(binding = 0) uniform UniformBufferObject {
    mat4 world;
    mat4 view;
    mat4 proj;
    mat4 worldLast;
    mat4 viewLast;
    mat4 projLast;
} ubo;

layout(binding = 2) uniform TAAInfo {
	vec2 currentJitter;
    vec2 inverseScreenSize;
	bool enabled;
} taaInfo; 

// (Lee, 2021) Used for TAA
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

    gl_Position = ubo.proj * ubo.view * ubo.world * vec4(inPosition, 1.0);
    vec4 lastScreenSpacePosition = ubo.projLast * ubo.viewLast * ubo.worldLast * vec4(inPosition, 1.0);
    outVelocity = CalcVelocity(gl_Position, lastScreenSpacePosition);

    // Apply TAA Jitter (if enabled)
    if(taaInfo.enabled) gl_Position += vec4(taaInfo.currentJitter * gl_Position.w, 0, 0);

    outWorldPos = (ubo.world * vec4(inPosition, 1.0)).xyz;
    outNormal = normalize((ubo.world * vec4(inNormal, 0.0)).xyz);
    outTex = inTex;
}