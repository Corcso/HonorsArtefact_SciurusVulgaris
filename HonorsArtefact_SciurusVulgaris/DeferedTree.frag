#version 450 

// GBuffer Paint Shader

layout(binding = 0) uniform sampler2D colorBuffer;
layout(binding = 1) uniform sampler2D positionBuffer;
layout(binding = 2) uniform sampler2D normalBuffer;
layout(binding = 5) uniform sampler2D depthBuffer;

layout(binding = 3) uniform LightBuffer{
    vec3 direction;
    vec3 color; 
    float intensity;
    mat4 viewMatrix;
    mat4 projMatrix;
    vec3 ambientColor; 
    float ambientIntensity;
    uint shadowEnabled;
} light;

layout(binding = 4) uniform sampler2D lightShadowMap;

layout(location = 0) in vec2 inTex;

layout(location = 0) out vec4 outColor;

// Calculate if a pixel is in shadow using the shadow map.
bool IsInShadow(vec3 positionOfPixel) {
    vec4 projectedPosition = light.projMatrix * light.viewMatrix * vec4(positionOfPixel, 1.0);
    projectedPosition = projectedPosition / projectedPosition.w;
    float pixelDepthValue = projectedPosition.z;

    // Convert to UV Space from NDC
    vec2 projectedUVPosition = projectedPosition.xy;
    projectedUVPosition /= vec2(2.0f, 2.0f);
    projectedUVPosition += vec2(0.5f, 0.5f);

    if(projectedUVPosition.x < 0 || projectedUVPosition.x > 1) return false;
    if(projectedUVPosition.y < 0 || projectedUVPosition.y > 1) return false;

    float lightDepthValue = texture(lightShadowMap, projectedUVPosition).x;

    float bias = 0.001;
    lightDepthValue += bias;

    if(pixelDepthValue > lightDepthValue) return true;

    return false;
}

void main() {
    // Get all GBuffer values. 
    vec4 color = texture(colorBuffer, inTex);
    vec4 position = texture(positionBuffer, inTex);
    vec4 normal = texture(normalBuffer, inTex);
    float depth = (texture(depthBuffer, inTex).r - 0.99998) / (1.0f - 0.99998);  // Setup for an unused fog effect. 

    if(color.a == 0) discard;

    float diffuseStrengthFront = dot(normalize(normal.xyz), normalize(-light.direction));
    float diffuseStrengthBack = dot(normalize(-normal.xyz), normalize(-light.direction)) * 0.5; // Use for subsurface scattering
    if(color.y < 0.5) diffuseStrengthBack = 0; // If the pixel is over a certain greenness consider it a leaf. You could use a trunk/leaf buffer here too. 
    float diffuseStrength = max(diffuseStrengthFront, diffuseStrengthBack) * light.intensity;
    if(IsInShadow(position.xyz) && light.shadowEnabled > 0.0) diffuseStrength = 0;

    vec3 diffuseColor = light.color * diffuseStrength;
    vec3 ambientColor = light.ambientColor * light.ambientIntensity;
    vec3 fogColor = vec3(0.3f, 0.3f, 0.32f); // Setup for an unused fog effect. 

    // Return ambient + diffuse + specular
    outColor = vec4(mix(color.rgb * (diffuseColor + ambientColor), fogColor, /*depth*/0.0f), /*1.0f - depth*/1.0f); // Don't blend fog color, not used
    //outColor = vec4(inNormal /0.5 + 0.5, 1);
    //outColor = vec4(depth, depth, depth , 1.0);
}