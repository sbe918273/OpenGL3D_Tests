#version 440

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
	vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;

void main()
{
	mat4 viewModel = view * model;
	gl_Position = viewModel * vec4(aPos, 1.0);
	vs_out.normal = mat3(transpose(inverse(viewModel))) * aNormal;
}