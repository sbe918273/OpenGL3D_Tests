#version 440
#define COUNT_POINT_LIGHT 4
#define PI 3.14159265359

struct Light {
	vec3 pos;
	vec3 color;
};

struct Material {
	sampler2D albedoMap;
	sampler2D metallicMap;
	sampler2D normalMap;
	sampler2D roughnessMap;
	float ao;
};

uniform Material material;
uniform Light lights[COUNT_POINT_LIGHT];
uniform vec3 camPos;

uniform bool showNormal;
uniform bool normalMapping;

float distributionGGX(float nDotH, float roughness);
float geometrySchlickGGX(float nDotV, float roughness);
vec3 fresnelSchlick(float vDotH, vec3 F0);

in VS_OUT {
	vec3 worldPos;
	vec3 normal;
	mat3 TBN;
	vec2 texCoords;
} fs_in;

out vec4 fragColor;

void main()
{
	vec3 albedo = texture(material.albedoMap, fs_in.texCoords).rgb;
	float metallic = texture(material.metallicMap, fs_in.texCoords).r;
	float roughness = texture(material.roughnessMap, fs_in.texCoords).r;

	vec3 texNorm = texture(material.normalMap, fs_in.texCoords).xyz;
	vec3 n = normalMapping ? normalize(fs_in.TBN * (2.0 * texNorm - 1.0)) : fs_in.TBN[2];

	vec3 v = normalize(camPos - fs_in.worldPos);

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < COUNT_POINT_LIGHT; i++)
	{
		// radiance
		vec3 l = lights[i].pos - fs_in.worldPos;
		float dist = length(l);
		float attenuation = 1.0 / (dist * dist);
		vec3 radiance = attenuation * lights[i].color;

		// cook-torrence brdf
		// a) specular 
		l /= dist;
		vec3 h = normalize(v + l);
		
		float nDotH = max(dot(n, h), 0.0);
		float N = distributionGGX(nDotH, roughness);

		float nDotL = max(dot(n, l), 0.0);
		float nDotV = max(dot(n, v), 0.0);
		float G = geometrySchlickGGX(nDotL, roughness) * 
		geometrySchlickGGX(nDotV, roughness);
		
		float vDotH = max(dot(v, h), 0.0);
		vec3 F = fresnelSchlick(vDotH, F0);

		vec3 specular = N * G * F /  max((4.0 * nDotV * nDotL), 0.001);

		// b) diffuse
		vec3 kD = vec3(1.0) - F;
		kD *= 1.0 - metallic;
		vec3 diffuse = kD * albedo / PI;

		// combine
		Lo += (diffuse + specular) * radiance * nDotL;
	}

	// ambient
	vec3 ambient = 0.03 * albedo * material.ao;
	vec3 color = ambient + Lo;
	
	const float exposure = 1.0;
	color = vec3(1.0) - exp(-color * exposure);
    color = pow(color, vec3(1.0/2.2));  
   
    fragColor = showNormal ? vec4(n, 1.0) : vec4(color, 1.0);
}

float distributionGGX(float nDotH, float roughness)
{
	float a2 = roughness * roughness;
	float denom = nDotH * nDotH * (a2 -1.0) + 1.0;
	denom *= PI * denom;

	return a2 / denom;
}

float geometrySchlickGGX(float nDotV, float roughness)
{
	float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
	return nDotV / ((1.0 - k) * nDotV + k);
}

vec3 fresnelSchlick(float vDotH, vec3 F0)
{
	return F0 + (1.0 - F0) * pow((1.0 - vDotH), 5);
}