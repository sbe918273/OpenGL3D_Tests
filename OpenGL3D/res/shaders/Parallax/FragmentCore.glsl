#version 440

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in VS_OUT {
	vec2 texCoords;

	vec3 tangentLightPos;
	vec3 tangentViewPos;
	vec3 tangentFragPos;
} fs_in;

out vec4 fragColor;

uniform PointLight pointLight;

uniform Material material;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform mat4 model;
uniform vec3 viewPos;

uniform bool mapNormals;

vec2 calcParallax(vec2 texCoords, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 lightPos, vec3 viewDir, vec2 texCoords);

void main()
{
	vec3 viewDir = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);

	vec3 norm;
	vec2 texCoords;
	if (mapNormals)
	{
		texCoords = calcParallax(fs_in.texCoords, viewDir);
		if(texCoords.x > 2.0 || texCoords.y > 2.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
			discard;
		vec3 normal = texture(normalMap, texCoords).rgb;
		norm = normalize(normal * 2.0 - 1.0);
	}
	else
	{
		texCoords = fs_in.texCoords;
		norm = vec3(0.0, 0.0, 1.0);
	}

	// point light
	vec3 result = calcPointLight(pointLight, norm, fs_in.tangentFragPos, fs_in.tangentLightPos, viewDir, texCoords);
	fragColor = vec4(result, 1.0);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 lightPos, vec3 viewDir, vec2 texCoords)
{
	// ambient
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
	vec3 ambient = light.ambient * diffuseColor;

	//diffuse
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffuseColor; 

	// specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); 
	vec3 specular = spec * light.specular * vec3(texture(material.specular, texCoords));

	// attenuation
	float dist = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	// combine
	return attenuation * (ambient + diffuse + specular);
}

vec2 calcParallax(vec2 texCoords, vec3 viewDir)
{
	const float minLayers = 8.0;
	const float maxLayers = 32.0;
	const float numLayers = mix(maxLayers, minLayers, max(viewDir.z, 0.0));  

	const float depthScale = 0.1;

	const float layerDepth = 1.0 / numLayers;
	const vec2 P = viewDir.xy * depthScale;
	const vec2 deltaTexCoords = P / numLayers;
	float currentLayerDepth = 0.0;

	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
		currentTexCoords -= deltaTexCoords;
		currentDepthMapValue = texture(depthMap, currentTexCoords).r;
		currentLayerDepth += layerDepth;
	}

	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;  
}