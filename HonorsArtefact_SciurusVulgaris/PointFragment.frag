#version 450 

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;


void main() {
     // Totals for ambient, diffuse and specular light
    vec3 ambientTotal = vec3(0, 0, 0);
    vec3 diffuseTotal = vec3(0, 0, 0);
    vec3 specularTotal = vec3(0, 0, 0);

    // Return ambient + diffuse + specular
    outColor = inColor;
    //outColor = vec4(inNormal /0.5 + 0.5, 1);
}