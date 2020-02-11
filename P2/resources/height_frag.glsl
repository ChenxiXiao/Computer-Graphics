#version 330 core
in vec3 fragNor;
out vec4 color;

void main()
{
    vec3 normal = normalize(fragNor);
    // Map normal in the range [-1, 1] to color in range [0, 1];

    color = vec4(0.5, 0.1, 0.05, 1.0);
}

