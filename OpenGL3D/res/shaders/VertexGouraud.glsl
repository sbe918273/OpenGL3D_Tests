#version 440

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec4 aColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{

	vec3 fragPos = vec3(model * vec4(aPos, 1.0f));
	gl_Position = projection * view * vec4(fragPos, 1.0f);

	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 norm = normalize(aNormal);

	float specularStrength = 0.5f;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	aColor = vec4(result, 1.0);
}