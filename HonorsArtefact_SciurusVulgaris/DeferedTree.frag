#version 450 

layout(binding = 0) uniform sampler2D colorBuffer;
layout(binding = 1) uniform sampler2D positionBuffer;
layout(binding = 2) uniform sampler2D normalBuffer;

layout(location = 0) in vec2 inTex;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(colorBuffer, inTex);
    vec4 position = texture(positionBuffer, inTex);
    vec4 normal = texture(normalBuffer, inTex);

    if(color.a == 0) discard;

    vec3 diffuseDirection = vec3(-0.707, -0.707, 0);

    float diffuseStrengthFront = dot(normalize(normal.xyz), normalize(-diffuseDirection));
    float diffuseStrengthBack = dot(normalize(-normal.xyz), normalize(-diffuseDirection)) * 0.5;
    if(color.y < 0.5) diffuseStrengthBack = 0;
    float diffuseStrength = max(diffuseStrengthFront, diffuseStrengthBack);

    // Return ambient + diffuse + specular
    outColor = vec4(color.rgb * max(diffuseStrength, 0.1), 1.0);
    //outColor = vec4(inNormal /0.5 + 0.5, 1);
}