#version 440

in vec2 texCoords;

out float fragColor;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;

const vec2 noiseScale = vec2(800.0 / 4.0, 600.0 / 4.0);

void main()
{

	vec3 fragPos = texture(gPos, texCoords).xyz;
	vec3 norm = texture(gNormal, texCoords).xyz;
	vec3 randomVec = texture(texNoise, noiseScale * texCoords).xyz;

	// tangent is on triangle's surface as component projected onto normal is subtracted.
	vec3 tangent   = normalize(randomVec - norm * dot(randomVec, norm));
	vec3 bitangent = cross(norm, tangent);
	mat3 TBN = mat3(tangent, bitangent, norm);

	float occlusion = 0.0;
	const float radius = 0.5;
	const float bias = 0.025;
	for (int i = 0; i < 64; i++)
	{
		vec3 samplePos = TBN * samples[i]; // tangent to view space
		samplePos = fragPos + samplePos * radius;

		vec4 offset = vec4(samplePos, 1.0);
		offset = projection * offset;        // from view to clip-space
		offset.xyz /= offset.w;              // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // [0.0, 1.0] (i.e. ndc)

		float sampleDepth = texture(gPos, offset.xy).z; 
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck; 
	}

	occlusion = 1.0 - (occlusion / 64);
	fragColor = occlusion;  
}