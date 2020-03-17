#version 330 core

layout(location = 0) in vec4 vertPos;

uniform mat4 lightProjection;
uniform mat4 lightView;
uniform mat4 M;

void main() {
    /* transform into light space */
    gl_Position = lightProjection * lightView * M * vertPos;
}