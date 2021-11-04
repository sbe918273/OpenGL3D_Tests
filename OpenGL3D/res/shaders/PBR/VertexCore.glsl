#version 440
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
layout (location = 4) in vec3 aNormal;
layout (location = 5) in mat4 model;


uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
	vec3 worldPos;
	vec3 normal;
	mat3 TBN;
	vec2 texCoords;
} vs_out;

void main()
{
	mat3 normalMatrix = mat3(transpose(inverse(model)));

	vec3 tangent = normalize(normalMatrix * aTangent);
	vec3 bitangent = normalize(normalMatrix * aBitangent);
	vec3 normal = normalize(normalMatrix * aNormal);
	vs_out.TBN = mat3(tangent, bitangent, normal);

	vs_out.worldPos = vec3(model * vec4(aPos, 1.0));
	vs_out.texCoords = aTexCoords;
	gl_Position = projection * view * vec4(vs_out.worldPos, 1.0);
}