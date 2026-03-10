#version 450 

layout(binding = 1) uniform sampler2D colorTexture;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTex;
layout(location = 3) in vec2 inVelocity;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(colorTexture, inTex);
}