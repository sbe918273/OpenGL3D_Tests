#version 440
layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 brightColor;

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
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} fs_in;

uniform PointLight pointLights[4];

uniform Material material;

uniform vec3 viewPos;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos);

void main()
{
	vec3 norm = normalize(fs_in.normal);

	vec3 result = vec3(0.0f);

	// point lights
	for (int i = 0; i < 4; i++)
		result += calcPointLight(pointLights[i], norm, fs_in.fragPos);

	fragColor = vec4(result, 1.0);

    float brightness = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        brightColor = vec4(fragColor.rgb, 1.0);
    else
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos)
{
	vec3 viewDir = normalize(viewPos - light.position);

	// ambient
	vec3 diffuseColor = vec3(texture(material.diffuse, fs_in.texCoords));
	vec3 ambient = light.ambient * diffuseColor;

	//diffuse
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffuseColor; 

	// specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); 
	vec3 specular = spec * light.specular * vec3(texture(material.specular, fs_in.texCoords));

	// attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	// combine
	return attenuation * (ambient + diffuse + specular);
}
