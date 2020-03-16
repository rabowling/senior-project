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
	// get vector between fragment position and light position
    vec3 fragToLight = pos - lightPos;
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= farPlane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float bias = 0;//max(0.0005 * (1.0 - dot(fragNor, normalize(fragToLight))), 0.00001); 
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

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
