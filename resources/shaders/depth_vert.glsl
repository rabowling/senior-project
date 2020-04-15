#version  330 core

layout(location = 0) in vec4 vertPos;

uniform mat4 LP;
uniform mat4 LV;
uniform mat4 M;

void main() {

  /* transform into light space */
  gl_Position = LP * LV * M * vertPos;

}