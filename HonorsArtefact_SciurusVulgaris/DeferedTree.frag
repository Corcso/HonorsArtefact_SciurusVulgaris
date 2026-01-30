#version 450 

layout(binding = 0) uniform sampler2D colorBuffer;
layout(binding = 1) uniform sampler2D positionBuffer;
layout(binding = 2) uniform sampler2D normalBuffer;

layout(binding = 3) uniform LightBuffer{
    vec3 direction;
    vec3 color; 
    float intensity;
    mat4 viewMatrix;
    mat4 projMatrix;
} light;

layout(binding = 4) uniform sampler2D lightShadowMap;

layout(location = 0) in vec2 inTex;

layout(location = 0) out vec4 outColor;

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
    vec4 color = texture(colorBuffer, inTex);
    vec4 position = texture(positionBuffer, inTex);
    vec4 normal = texture(normalBuffer, inTex);

    if(color.a == 0) discard;

    //vec3 diffuseDirection = vec3(-0.707, -0.707, 0);

    float diffuseStrengthFront = dot(normalize(normal.xyz), normalize(-light.direction));
    float diffuseStrengthBack = dot(normalize(-normal.xyz), normalize(-light.direction)) * 0.5;
    if(color.y < 0.5) diffuseStrengthBack = 0;
    float diffuseStrength = max(diffuseStrengthFront, diffuseStrengthBack) * light.intensity;
    if(IsInShadow(position.xyz)) diffuseStrength = 0;
    diffuseStrength = max(diffuseStrength, 0.1);
    vec3 diffuseColor = light.color * diffuseStrength;

    // Return ambient + diffuse + specular
    outColor = vec4(color.rgb * diffuseColor, 1.0);
    //outColor = vec4(inNormal /0.5 + 0.5, 1);
}