#version 450 

// Triangle rendering shader which outputs to GBuffer
// But does not output world position as position, instead uses local model position. 
// This is used for virtual photogrammetry
// Also samples a texture.

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;


void main() {
    outColor = texture(texSampler, inTex);
    if(outColor.w <= 0) discard;
    outPosition = vec4(inPosition, 1);
    outNormal = vec4(normalize(inNormal), 1);
    if(!gl_FrontFacing) outNormal = vec4(normalize(-inNormal), 1);
}