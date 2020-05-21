#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec3 vertCol;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 fragNor;
out vec4 fragCol;

void main()
{ 
    gl_Position = P * V * M * vertPos;
    fragNor = (M * vec4(vertNor, 0.0)).xyz;
    fragCol = vec4(vertCol,1.0);
   
}
