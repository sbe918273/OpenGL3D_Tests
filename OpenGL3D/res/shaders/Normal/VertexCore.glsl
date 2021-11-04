#version 440

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
	vec2 texCoords;

	vec3 tangentLightPos;
	vec3 tangentViewPos;
	vec3 tangentFragPos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 tangent;
uniform vec3 bitangent;
uniform vec3 normal;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D normalMap;

void main()
{
	vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
	vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
	mat3 TBN = transpose(mat3(T, B, N));

	vs_out.tangentLightPos = TBN * lightPos;
	vs_out.tangentViewPos = TBN * viewPos;
	vs_out.tangentFragPos = TBN * vec3(model * vec4(aPos, 1.0));

	vs_out.texCoords = aTexCoords;

	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}