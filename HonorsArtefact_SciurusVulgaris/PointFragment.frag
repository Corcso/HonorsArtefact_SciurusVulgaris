#version 450 

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;


void main() {
    vec3 diffuseDirection = vec3(-0.707, -0.707, 0);

    float diffuseStrengthFront = dot(normalize(inNormal), normalize(-diffuseDirection));
    float diffuseStrengthBack = dot(normalize(-inNormal), normalize(-diffuseDirection)) * 0.5;
    if(inColor.y < 0.5) diffuseStrengthBack = 0;
    float diffuseStrength = max(diffuseStrengthFront, diffuseStrengthBack);

    // Return ambient + diffuse + specular
    outColor = vec4(inColor.rgb * max(diffuseStrength, 0.0), 1.0);
    //outColor = vec4(inNormal /0.5 + 0.5, 1);
}