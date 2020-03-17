#version 330 core 

out vec4 color;

uniform sampler2D Texture0;
uniform vec3 MatSpec;
uniform vec3 MatDif;
uniform vec3 MatAmb;
uniform float Shine;
uniform float farPlane;

uniform vec3 lightPos;
uniform vec3 dirLightColor;
uniform vec3 viewPos;

uniform sampler2D depthMap;

in vec2 vTexCoord;
in vec3 fragNor;
in vec3 fragPos;
in vec4 fragPosLighting;

float ShadowCalculation(vec4 pos, vec3 lightDir)
{
	float bias = max(0.05 * (1.0 - dot(fragNor, lightDir)), 0.005);  
	vec3 shifted = (pos.xyz + vec3(1)) * 0.5;
	float shadow = 0.0;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float pcfDepth = texture(depthMap, shifted.xy + vec2(x, y)).r;
			shadow += shifted.z - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	if (shifted.z > 1.0) {
		shadow = 0.0;
	}
	return shadow / 9.0;
}

void main()
{
	vec3 normal = normalize(fragNor);

	vec3 dirLightDirNorm = normalize(lightPos - fragPos);

	vec3 texColor0 = vec3(texture(Texture0, vTexCoord));
	vec3 diffuse = MatDif * texColor0 * max(0, dot(normal, dirLightDirNorm)) * dirLightColor;

	vec3 ambient = MatAmb * texColor0 * dirLightColor;

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 H = normalize((dirLightDirNorm + viewDir) / 2.0);
	vec3 specular = MatSpec * pow(max(0, dot(H, normal)), Shine) * dirLightColor;

	float shadow = ShadowCalculation(fragPosLighting, dirLightDirNorm);

	color = vec4(ambient + (1.0 - shadow) * (diffuse + specular), 1.0);
}
