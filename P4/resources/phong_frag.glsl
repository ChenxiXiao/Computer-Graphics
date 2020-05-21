#version 330 core

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;
uniform vec3 camera;
uniform vec3 lightSrc;

in vec4 vertPosition;
in vec3 fragNor;
layout(location = 0) out vec4 color;

void main()
{
    vec3 L = normalize(lightSrc - vertPosition.xyz);
    vec3 N = normalize(fragNor);
    vec3 eyedir = normalize(camera - vertPosition.xyz);
    vec3 H = normalize(L + eyedir);

    vec3 specularLight = MatSpec * pow(clamp(dot(N, H), 0, 1), shine);
    vec3 lightdiffuse = MatDif * clamp(dot(N, L), 0.0, 1.0);
    vec3 ambientLight = MatAmb;
    color = vec4(clamp(lightdiffuse + ambientLight + specularLight, 0.0, 1.0), 1.0);
}
