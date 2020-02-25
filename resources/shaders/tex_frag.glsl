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

uniform samplerCube depthMap;

in vec2 vTexCoord;
in vec3 fragNor;
in vec3 fragPos;

float ShadowCalculation(vec3 pos)
{
	float shadow = 0.0;
	float bias = 0.15;
	int samples = 20;
	float viewDistance = length(viewPos - pos);
	float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;  
	vec3 fragToLight = pos - lightPos;
	float currentDepth = length(fragToLight);

	vec3 sampleOffsetDirections[20] = vec3[]
	(
   		vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   		vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   		vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   		vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   		vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);

	for (int i = 0; i < samples; i++)
	{
		float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= farPlane;
		if (currentDepth - bias > closestDepth)
		{
			shadow += 1.0;
		}
	}
	shadow /= float(samples);
	return shadow;
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

	float shadow = ShadowCalculation(fragPos);

	color = vec4(ambient + (1.0 - shadow) * (diffuse + specular), 1.0);
}
