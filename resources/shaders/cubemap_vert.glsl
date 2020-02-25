#version 330 core
layout (location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec3 vertTex;

uniform mat4 M;

void main()
{
    gl_Position = M * vec4(vertPos, 1);
    vertNor;
    vertTex;
}