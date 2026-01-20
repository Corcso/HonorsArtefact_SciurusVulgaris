#version 450 

layout(binding = 0) uniform sampler2D colorBuffer;
layout(binding = 1) uniform sampler2D positionBuffer;
layout(binding = 2) uniform sampler2D normalBuffer;

layout(binding = 3) uniform LightBuffer{
    vec3 direction;
    vec3 color; 
    float intensity;
} light;

layout(location = 0) in vec2 inTex;

layout(location = 0) out vec4 outColor;

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
    diffuseStrength = max(diffuseStrength, 0.1);
    vec3 diffuseColor = light.color * diffuseStrength;

    // Return ambient + diffuse + specular
    outColor = vec4(color.rgb * diffuseColor, 1.0);
    //outColor = vec4(inNormal /0.5 + 0.5, 1);
}