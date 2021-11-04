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
	vec3 fragPos;
	vec3 fragWorldPos;
	vec3 normal;
	vec2 texCoords;
} fs_in;

out vec4 fragColor;

uniform PointLight pointLight;

uniform Material material;
uniform samplerCube depthMap;

uniform mat4 view;

uniform float farPlane;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float calcShadow(PointLight light, vec3 fragPos, vec3 normal);

const vec3 sampleOffsetDirections[20] = vec3[]
(
	vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
	vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

void main()
{

	// properties
	vec3 norm = normalize(fs_in.normal);
	vec3 viewDir = normalize(-fs_in.fragPos);

	// directional lighthing
	vec3 result = calcPointLight(pointLight, norm, fs_in.fragPos, viewDir);
	fragColor = vec4(result, 1.0);
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
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); 
	vec3 specular = spec * light.specular * vec3(texture(material.specular, fs_in.texCoords));

	// attenuation
	float dist = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	float shadow = calcShadow(light, fs_in.fragWorldPos, normal);

	// combine
	
	return attenuation * (ambient + (1.0 - shadow) * (diffuse + specular));

	//return attenuation * (ambient + diffuse + specular);
	//return vec3(length(fs_in.fragWorldPos - light.position) / farPlane);
	//return vec3(texture(depthMap, fs_in.fragWorldPos - light.position).r);
}

float calcShadow(PointLight light, vec3 fragPos, vec3 normal)
{


	vec3 lightDir = fragPos - light.position;
	float currentDepth = length(lightDir) / farPlane;

	if (currentDepth > 1.0 || currentDepth < 0.0)
		return 0.0;

	// bias increases from 0.003 to 0.05 as angle between light 
	// and normal decreases from 0 to 90 degrees (i.e. light is more direct on surface)

	//const float factor = 1.0 - max(0, dot(normal, lightDir));
	//const float bias = max(0.05 * factor, 0.03);  

	float viewDistance = length(fs_in.fragPos);
	float diskRadius = viewDistance / 200.0;  

	float shadow  = 0.0;
	float bias    = 0.0005; 
	int samples = 20;
	float offset  = 0.1;
	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(depthMap, lightDir + sampleOffsetDirections[i] * diskRadius).r;
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}

	shadow /= float(samples);

	return shadow;
}
