#version 330 core 

out vec4 color;

uniform vec3 MatSpec;
uniform vec3 MatDif;
uniform vec3 MatAmb;
uniform float Shine;

uniform vec3 dirLightDir;
uniform vec3 dirLightColor;
uniform vec3 viewPos;

in vec2 vTexCoord;
in vec3 fragNor;
in vec3 fragPos;

void main()
{
	vec3 normal = normalize(fragNor);

    vec3 dirLightDirNorm = normalize(dirLightDir);
	vec3 diffuse = MatDif * max(0, dot(normal, dirLightDirNorm)) * dirLightColor;

	vec3 ambient = MatAmb * dirLightColor;

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 H = normalize((dirLightDirNorm + viewDir) / 2.0);
	vec3 specular = MatSpec * pow(max(0, dot(H, normal)), Shine) * dirLightColor;

	color = vec4(diffuse, 1.0);
}
