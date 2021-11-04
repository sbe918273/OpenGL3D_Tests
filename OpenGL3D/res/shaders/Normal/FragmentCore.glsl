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

uniform mat4 model;
uniform vec3 viewPos;

uniform bool mapNormals;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 lightPos, vec3 viewDir);

void main()
{

	vec3 norm;
	if (mapNormals)
	{
		vec3 normal = texture(normalMap, fs_in.texCoords).rgb;
		norm = normalize(normal * 2.0 - 1.0);
	}
	else
		norm = vec3(0.0, 0.0, 1.0);

	vec3 viewDir = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);

	// point light
	vec3 result = calcPointLight(pointLight, norm, fs_in.tangentFragPos, fs_in.tangentLightPos, viewDir);
	fragColor = vec4(result, 1.0);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 lightPos, vec3 viewDir)
{
	// ambient
	vec3 diffuseColor = vec3(texture(material.diffuse, fs_in.texCoords));
	vec3 ambient = light.ambient * diffuseColor;

	//diffuse
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffuseColor; 

	// specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); 
	vec3 specular = spec * light.specular * vec3(texture(material.specular, fs_in.texCoords));

	// attenuation
	float dist = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	// combine
	return attenuation * (ambient + diffuse + specular);
}
