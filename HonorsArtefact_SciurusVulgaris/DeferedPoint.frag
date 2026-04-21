#version 450 

// Point -> GBuffer

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inVelocity;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outVelocity;


void main() {
    outColor = inColor;
    outPosition = vec4(inWorldPos, 1);
    outVelocity = vec4(inVelocity, 0, 1);
    outNormal = vec4(normalize(inNormal), 1);
}