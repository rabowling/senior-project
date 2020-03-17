#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 lightingMatrix;

out vec3 fragNor;
out vec3 fragPos;
out vec2 vTexCoord;
out vec4 fragPosLighting;

void main()
{
	vTexCoord = vertTex;
	gl_Position = P * V * M * vertPos;
	fragNor = vec3(M * vec4(vertNor, 0.0));
	fragPos = vec3(M * vertPos);
    fragPosLighting = lightingMatrix * M * vertPos;
}