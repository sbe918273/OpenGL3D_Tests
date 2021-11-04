#version 440

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vs_out.fragPos = vec3(model * vec4(aPos, 1.0f));
	vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
	vs_out.texCoords = aTexCoords;
	gl_Position = projection * view *  vec4(vs_out.fragPos, 1.0f);
}