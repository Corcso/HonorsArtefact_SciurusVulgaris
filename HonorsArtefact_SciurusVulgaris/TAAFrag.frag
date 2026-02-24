#version 450 

layout(binding = 0) uniform sampler2D renderInput; // Sampled Nearest
layout(binding = 1) uniform sampler2D historyInput; // Sampled Linear

layout(binding = 2) uniform TAAInfo{
    vec2 currentJitter;
    bool enabled;
} taaInfo;

layout(location = 0) in vec2 inTex;

layout(location = 0) out vec4 outColor;

const float WEIGHTING = 0.9;

void main() {
    vec4 renderColor = texture(renderInput, inTex);
    // If no TAA don't do anything
    if(!taaInfo.enabled) {
        outColor = renderColor;
        return;
    }
    // If yes TAA
    vec4 historyColor = texture(historyInput, inTex);
    outColor = mix(renderColor, historyColor, WEIGHTING);
    return;
}