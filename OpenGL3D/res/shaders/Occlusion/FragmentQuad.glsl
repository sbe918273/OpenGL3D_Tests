#version 440

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float radius;
};

in vec2 texCoords;

out vec4 fragColor;

uniform PointLight pointLight;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gDiff;
uniform sampler2D gSpec;
uniform sampler2D ssaoTex;

uniform float shininess;

uniform mat4 view;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 diffColor, vec3 specColor);

void main()
{

	vec3 fragPos = texture(gPos, texCoords).rgb;
	vec3 norm = texture(gNormal, texCoords).rgb;
	vec3 diffColor = texture(gDiff, texCoords).rgb;
	vec3 specColor = texture(gSpec, texCoords).rgb;

	// point light
	vec3 result = calcPointLight(pointLight, norm, fragPos, diffColor, specColor);
	fragColor = vec4(result, 1.0);
	//fragColor = vec4(vec3(texture(ssaoTex, texCoords).r), 1.0);
	//fragColor = vec4(diffColor, 1.0);
	//fragColor = vec4(norm, 1.0);
	//fragColor = vec4(fragPos, 1.0);
	//fragColor = vec4(1 - gl_FragDepth);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 diffColor, vec3 specColor)
{
	vec3 lightPos = vec3(view * vec4(light.position, 1.0));
	vec3 viewDir = normalize(-fragPos);

	// ambient
	float occlusion = pow(texture(ssaoTex, texCoords).r, 4.0);
	vec3 ambient = occlusion * light.ambient * diffColor;

	//diffuse
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * diffColor; 

	// specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess); 
	vec3 specular = spec * light.specular * specColor;

	// attenuation
	float dist = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	// combine
	return attenuation * (ambient + diffuse + specular);
}
