#version 440

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (location = 3) in mat4 instanceMatrix;

out vec3 fragPos;
out vec3 normal;
out vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool instanced;

void main()
{
	mat4 aModel;

	if (instanced)
		aModel = instanceMatrix;
	else
		aModel = model;

	fragPos = vec3(aModel * vec4(aPos, 1.0f));
	gl_Position = projection * view * vec4(fragPos, 1.0f);
	normal = mat3(transpose(inverse(aModel))) * aNormal;
	texCoords = aTexCoords;
}