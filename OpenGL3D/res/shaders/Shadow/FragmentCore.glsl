#version 440

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec4 fragPosLightSpace;
} fs_in;

out vec4 fragColor;

uniform Material material;
uniform sampler2D shadowMap;

uniform DirLight dirLight;

uniform mat4 view;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
float calcShadow(vec4 fragPosLightSpace, DirLight light, vec3 normal);

void main()
{

	// properties
	vec3 norm = normalize(fs_in.normal);
	vec3 viewDir = normalize(-fs_in.fragPos);

	// directional lighthing
	vec3 result = calcDirLight(dirLight, norm, viewDir);
	fragColor = vec4(result, 1.0);
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	// ambient
	vec3 diffuseColor = vec3(texture(material.diffuse, fs_in.texCoords));
	vec3 ambient = light.ambient * diffuseColor;
	
	//diffuse (convert light dir to eye space)
	vec3 lightDir = -normalize(mat3(view) * light.direction);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffuseColor; 
	
	// specular
	float spec;
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); 

	vec3 specular = spec * light.specular * vec3(texture(material.specular, fs_in.texCoords));

	float shadow = calcShadow(fs_in.fragPosLightSpace, dirLight, normal);
	
	return ambient + (1.0f - shadow) * (diffuse + specular);
}

float calcShadow(vec4 fragPosLightSpace, DirLight light, vec3 normal)
{
	// perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// [-1, 1] => [0, 1]
	projCoords = projCoords * 0.5 + 0.5;

	const float currentDepth = projCoords.z;

	if (currentDepth > 1.0 || currentDepth < 0.0)
		return 0.0;

	// bias increases from 0.005 to 0.05 as angle between light 
	// and normal decreases from 0 to 90 degrees (i.e. light is more direct on surface)

	const float factor = 1.0 - max(0, dot(normal, light.direction));
	const float bias = max(0.005 * factor, 0.003);  

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	const int upperSample = 3;
	const int lowerSample = -3;

	for (int x = lowerSample; x <= upperSample; x++)
	{
		for (int y = lowerSample; y <= upperSample; y++)
		{
			float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
		}
	}

	shadow /= ((upperSample + 2) * (-lowerSample + 2));
	return shadow;
}
