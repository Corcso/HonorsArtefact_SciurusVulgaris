#version 450 

// Triangle -> GBuffer with texture sampling
// Doesn't include velocity data

layout(binding = 1) uniform sampler2D colorTexture;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;


void main() {
    outColor = texture(colorTexture, inTex);
    if(outColor.a == 0) discard;
    outPosition = vec4(inWorldPos, 1);
    outNormal = vec4(normalize(inNormal), 1);
}