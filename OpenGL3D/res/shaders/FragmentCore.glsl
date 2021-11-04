#version 440
#define COUNT_POINT_LIGHT 4

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

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Spotlight
{
	vec3 position;
	vec3 direction;

	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	bool on;
};

in vec3 fragPos;
in vec3 normal;
in vec2 texCoords;

out vec4 fragColor;

uniform vec3 viewPos;

uniform Material material;

uniform DirLight dirLight;
uniform PointLight pointLights[COUNT_POINT_LIGHT];
uniform Spotlight spotlight;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{

	// properties
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(viewPos - fragPos);

	// directional lighthing
	vec3 result = calcDirLight(dirLight, norm, viewDir);

	for (int i = 0; i < COUNT_POINT_LIGHT; i++)
		result += calcPointLight(pointLights[i], norm, fragPos, viewDir); 

	// spotlight
	if (spotlight.on)
		result += calcSpotlight(spotlight, normal, fragPos, viewDir);

	fragColor = vec4(result, 1.0);
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	// ambient
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
	vec3 ambient = light.ambient * diffuseColor;
	
	//diffuse
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffuseColor; 
	
	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	vec3 specular = spec * light.specular * vec3(texture(material.specular, texCoords));

	// combine
	return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	// ambient
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
	vec3 ambient = light.ambient * diffuseColor;

	//diffuse
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffuseColor; 

	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	vec3 specular = spec * light.specular * vec3(texture(material.specular, texCoords));

	// attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	// combine
	return attenuation * (ambient + diffuse + specular);
}

vec3 calcSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	// ambient
	vec3 diffuseColor = vec3(texture(material.diffuse, texCoords));
	vec3 ambient = light.ambient * diffuseColor;

	//diffuse
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffuseColor; 

	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	vec3 specular = spec * light.specular * vec3(texture(material.specular, texCoords));

	// spotlight
	float theta = max(dot(lightDir, normalize(-light.direction)), 0.0);
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	// combine 
	return attenuation * intensity * (ambient + diffuse + specular);
}