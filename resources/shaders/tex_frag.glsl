#version 330 core 

out vec4 color;

uniform sampler2D Texture0;
uniform vec3 MatSpec;
uniform vec3 MatDif;
uniform vec3 MatAmb;
uniform float Shine;
uniform float farPlane;

struct PortalLight {
    vec3 pos;
    vec3 dir;
    float innerCutoff;
    float outerCutoff;
    float intensity;
};
uniform PortalLight portalLights[4];

uniform vec3 pointLightPos;
uniform vec3 dirLightColor;
uniform vec3 viewPos;

uniform samplerCube depthMapPointLight;

uniform sampler2D depthMapPortal1;
uniform sampler2D depthMapPortal2;
uniform sampler2D depthMapPortal3;
uniform sampler2D depthMapPortal4;

in vec2 vTexCoord;
in vec3 fragNor;
in vec3 fragPos;

float PortalShadowCalculation(vec3 pos, int index) {
    vec3 lightDir = normalize(pos - fragPos);
    float bias = max(0.05 * (1.0 - dot(fragNor, lightDir)), 0.005);  
	vec3 shifted = (pos.xyz + vec3(1)) * 0.5;
	float shadow = 0.0;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
            float pcfDepth = 0.0f;
            if (index == 0) {
                pcfDepth = texture(depthMapPortal1, shifted.xy + vec2(x, y)).r;
            } else if (index == 1) {
                pcfDepth = texture(depthMapPortal2, shifted.xy + vec2(x, y)).r;
            } else if (index == 2) {
                pcfDepth = texture(depthMapPortal3, shifted.xy + vec2(x, y)).r;
            } else {
                pcfDepth = texture(depthMapPortal4, shifted.xy + vec2(x, y)).r;                
            }
			shadow += shifted.z - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	if (shifted.z > 1.0) {
		shadow = 0.0;
	}
	return shadow / 9.0;
}

float ShadowCalculation(vec3 pos, vec3 lightPos, int index)
{
	// get vector between fragment position and light position
    vec3 fragToLight = pos - lightPos;
    // use the light to fragment vector to sample from the depth map 
    float closestDepth = texture(depthMapPointLight, fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= farPlane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float bias = 0;//max(0.0005 * (1.0 - dot(fragNor, normalize(fragToLight))), 0.00001); 
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

vec3 LightingCalculation(vec3 lightPos, float intensity, int index) {
    vec3 normal = normalize(fragNor);

	vec3 dirLightDirNorm = normalize(lightPos - fragPos);

	vec3 texColor0 = vec3(texture(Texture0, vTexCoord));
	vec3 diffuse = MatDif * texColor0 * max(0, dot(normal, dirLightDirNorm)) * dirLightColor * intensity;

	vec3 ambient = MatAmb * texColor0 * dirLightColor * intensity;

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 H = normalize((dirLightDirNorm + viewDir) / 2.0);
	vec3 specular = MatSpec * pow(max(0, dot(H, normal)), Shine) * dirLightColor * intensity;

    float shadow = 0.0;
    if (index < 0) {
	    shadow = ShadowCalculation(fragPos, lightPos, index);
    } else {
        shadow = PortalShadowCalculation(lightPos, index);
    }

    return ambient + (1.0 - shadow) * (diffuse + specular);
}

void main()
{
	vec3 normal = normalize(fragNor);
    vec3 lighting = LightingCalculation(pointLightPos, 1.0, -1);
	for (int i = 0; i < 4; i++) {
        vec3 dirLightDirNorm = normalize(portalLights[i].pos - fragPos);
        float theta = dot(dirLightDirNorm, normalize(-portalLights[i].dir));
        float epsilon = portalLights[i].innerCutoff - portalLights[i].outerCutoff;
        float intensity = clamp((theta - portalLights[i].outerCutoff) / epsilon, 0.0, 1.0);
        //if (theta > portalLights[i].innerCutoff) {
            lighting += LightingCalculation(portalLights[i].pos, intensity * portalLights[i].intensity, i);
        //}
    }

	color = vec4(lighting, 1.0);
}
