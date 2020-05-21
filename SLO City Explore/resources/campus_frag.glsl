#version 330 core
in vec3 fragNor;

in vec4 fragCol;

out vec4 color;

void main()
{
    vec3 normal = normalize(fragNor);
    color = fragCol;
   
}

