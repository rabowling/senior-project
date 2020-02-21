#version 330 core 
uniform vec3 outlinecolor;
out vec4 color;

void main()
{
	color = vec4(outlinecolor, 1);
}
