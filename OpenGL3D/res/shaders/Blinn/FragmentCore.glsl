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

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} fs_in;

out vec4 fragColor;

uniform Material material;

uniform DirLight dirLight;
uniform PointLight pointLights[COUNT_POINT_LIGHT];
uniform Spotlight spotlight;

uniform mat4 view;
uniform bool blinn;
uniform float gamma;
uniform bool correctGamma;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{

	// properties
	vec3 norm = normalize(fs_in.normal);
	vec3 viewDir = normalize(-fs_in.fragPos);

	// directional lighthing
	vec3 result = calcDirLight(dirLight, norm, viewDir);

	// point lights
	for (int i = 0; i < COUNT_POINT_LIGHT; i++)
		result += calcPointLight(pointLights[i], norm, fs_in.fragPos, viewDir); 

	// spotlight
	if (spotlight.on)
		result += calcSpotlight(spotlight, norm, fs_in.fragPos, viewDir);

	fragColor = vec4(result, 1.0);

	// gamma correction
	if (correctGamma)
		fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
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
	if (blinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); 
	}
	else 
	{
		vec3 reflectDir = reflect(-lightDir, normal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	}
	vec3 specular = spec * light.specular * vec3(texture(material.specular, fs_in.texCoords));

	// combine
	return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	// ambient
	vec3 diffuseColor = vec3(texture(material.diffuse, fs_in.texCoords));
	vec3 ambient = light.ambient * diffuseColor;

	//diffuse (convert light pos to eye space)
	vec3 lightPos = vec3(view * vec4( light.position, 1.0));
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffuseColor; 

	// specular
	float spec;
	if (blinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); 
	}
	else 
	{
		vec3 reflectDir = reflect(-lightDir, normal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	}
	vec3 specular = spec * light.specular * vec3(texture(material.specular, fs_in.texCoords));

	// attenuation
	float dist = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	// combine
	return attenuation * (ambient + diffuse + specular);
}

vec3 calcSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	// ambient
	vec3 diffuseColor = vec3(texture(material.diffuse, fs_in.texCoords));
	vec3 ambient = light.ambient * diffuseColor;

	//diffuse (convert light pos to eye space)
	vec3 lightPos = vec3(view * vec4(light.position, 1.0));
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffuseColor; 

	// specular
	float spec;
	if (blinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); 
	}
	else 
	{
		vec3 reflectDir = reflect(-lightDir, normal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	}
	vec3 specular = spec * light.specular * vec3(texture(material.specular, fs_in.texCoords));

	// spotlight (convert light dir to eye space)
	float theta = max(dot(lightDir, -normalize(mat3(view) * light.direction)), 0.0);
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// attenuation
	float dist = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	// combine
	return attenuation * intensity * (ambient + diffuse + specular);
}