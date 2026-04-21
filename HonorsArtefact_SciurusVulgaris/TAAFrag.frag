#version 450 

// TAA Shader Taken from (Lee, 2021; Riñón, 2022)

layout(binding = 0) uniform sampler2D renderInput; // Sampled Nearest
layout(binding = 1) uniform sampler2D historyInput; // Sampled Linear
layout(binding = 2) uniform sampler2D velocityInput; // Sampled Nearest

layout(binding = 3) uniform TAAInfo{
    vec2 currentJitter;
    vec2 inverseScreenSize;
    bool enabled;
    bool logarithmicColorSpace;
} taaInfo;

layout(location = 0) in vec2 inTex;

layout(location = 0) out vec4 outColor;

const float WEIGHTING = 0.9;

bool EpsilonEqual(float a, float b){
    return (abs(a - b) < 0.1f);
}

// Logarithmic Colour Space Blending & Clamping (Riñón, 2022)
vec4 AdjustToLog(vec4 color){
    if(!taaInfo.logarithmicColorSpace) return color;
    return vec4(
        color.x > 0 ? log(color.x) : -10,
        color.y > 0 ? log(color.y) : -10,
        color.z > 0 ? log(color.z) : -10, 1
    );
}

vec4 AdjustToNorm(vec4 color){
    if(!taaInfo.logarithmicColorSpace) return color;
    return vec4(
       exp(color.x),
       exp(color.y),
       exp(color.z), 1
    );
}

void main() {
    vec4 renderColor = texture(renderInput, inTex);
    // If no TAA don't do anything
    if(!taaInfo.enabled) {
        outColor = renderColor;
        return;
    }
    renderColor = AdjustToLog(renderColor);
    // If yes TAA
    vec2 velocity = texture(velocityInput, inTex).xy;
    vec2 historyLocation = inTex - velocity;

    float type = texture(velocityInput, inTex).z; // Stored in Z component of velocity

    if(historyLocation.x > 1.0f || historyLocation.x < 0.0f || historyLocation.y > 1.0f || historyLocation.y < 0.0f) {
        outColor = AdjustToNorm(renderColor);
        return;
    }

    vec4 historyColor = AdjustToLog(texture(historyInput, historyLocation));

    // Apply clamping on the history color.
    vec4 NearColor0 = AdjustToLog(texture(renderInput, inTex + vec2(taaInfo.inverseScreenSize.x, 0)));
    vec4 NearColor1 = AdjustToLog(texture(renderInput, inTex + vec2(0, taaInfo.inverseScreenSize.y)));
    vec4 NearColor2 = AdjustToLog(texture(renderInput, inTex + vec2(-taaInfo.inverseScreenSize.x, 0)));
    vec4 NearColor3 = AdjustToLog(texture(renderInput, inTex + vec2(0, -taaInfo.inverseScreenSize.y)));
       
    vec4 BoxMin = min(renderColor, min(NearColor0, min(NearColor1, min(NearColor2, NearColor3))));
    vec4 BoxMax = max(renderColor, max(NearColor0, max(NearColor1, max(NearColor2, NearColor3))));;
    
    historyColor = clamp(historyColor, BoxMin, BoxMax);

    outColor = AdjustToNorm(mix(renderColor, historyColor, WEIGHTING));
    return;
}