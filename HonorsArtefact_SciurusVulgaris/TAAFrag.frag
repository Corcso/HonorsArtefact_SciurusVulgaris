#version 450 

layout(binding = 0) uniform sampler2D renderInput; // Sampled Nearest
layout(binding = 1) uniform sampler2D historyInput; // Sampled Linear
layout(binding = 2) uniform sampler2D velocityInput; // Sampled Nearest

layout(binding = 3) uniform TAAInfo{
    vec2 currentJitter;
    vec2 inverseScreenSize;
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
    vec2 velocity = texture(velocityInput, inTex).xy;
    vec2 historyLocation = inTex - velocity;

    vec4 historyColor = texture(historyInput, historyLocation);

    // Apply clamping on the history color.
    vec4 NearColor0 = texture(renderInput, inTex + vec2(taaInfo.inverseScreenSize.x, 0));
    vec4 NearColor1 = texture(renderInput, inTex + vec2(0, taaInfo.inverseScreenSize.y));
    vec4 NearColor2 = texture(renderInput, inTex + vec2(-taaInfo.inverseScreenSize.x, 0));
    vec4 NearColor3 = texture(renderInput, inTex + vec2(0, -taaInfo.inverseScreenSize.y));
       
    vec4 BoxMin = min(renderColor, min(NearColor0, min(NearColor1, min(NearColor2, NearColor3))));
    vec4 BoxMax = max(renderColor, max(NearColor0, max(NearColor1, max(NearColor2, NearColor3))));;
    
    historyColor = clamp(historyColor, BoxMin, BoxMax);

    outColor = mix(renderColor, historyColor, WEIGHTING);
    return;
}