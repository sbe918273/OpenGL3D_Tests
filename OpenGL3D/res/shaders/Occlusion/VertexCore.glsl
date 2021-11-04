#version 440

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 fragPos;
out vec3 normal;
out vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	fragPos = vec3(view * model * vec4(aPos, 1.0f));
	gl_Position = projection * vec4(fragPos, 1.0f);
	normal = mat3(transpose(inverse(view * model))) * aNormal;
	texCoords = aTexCoords;
}