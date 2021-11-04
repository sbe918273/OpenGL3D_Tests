#version 440
#define PI 3.14159265359

in vec2 texCoords;
out vec2 fragColor;

vec2 integrateBRDF(float nDotV, float roughness);

float distributionGGX(float nDotH, float roughness);
float geometrySchlickGGX(float nDotV, float roughness);

float radicalInverse_VdC(uint bits);
vec3 importanceSampleGGX(vec2 Xi, vec3 n, float roughness);
vec2 hammersley(uint i, uint N);

void main()
{
    vec2 integratedBDRF = integrateBRDF(texCoords.x, texCoords.y);
    fragColor = integratedBDRF;
}

vec2 integrateBRDF(float nDotV, float roughness)
{
	// unit view vector in x-z plane has required dot product with normal z axis chosen
	vec3 v = vec3(sqrt(1.0 - nDotV * nDotV), 0.0, nDotV);

	float scale = 0.0;
	float bias = 0.0;

	vec3 n = vec3(0.0, 0.0, 1.0);

	const uint sampleCount = 1024u;
	for (uint i = 0; i < sampleCount; i++)
	{
	    vec2 Xi = hammersley(i, sampleCount);
        vec3 h = importanceSampleGGX(Xi, n, roughness);
        vec3 l = normalize(2.0 * dot(v, h) * h - v);

        float nDotH = max(h.z, 0.0);
        float nDotL = max(l.z, 0.0);
        float vDotH = max(dot(v, h), 0.0);

		//float N = min(distributionGGX(nDotH, roughness), 1.0);
        float N = 1.0;
		float G = geometrySchlickGGX(nDotL, roughness) * 
		geometrySchlickGGX(nDotV, roughness);
		float bdrf = N * G * vDotH / max(4.0 * nDotV * nDotL, 0.001); //nDotV <-> vDotH
        //float bdrf = N * G * vDotH / max(4.0 * nDotH * nDotL, 0.001);

        float Fc = pow(1.0 - vDotH, 5.0);

        scale += bdrf * (1.0 - Fc);
        bias += bdrf * Fc;
	}

    scale /= sampleCount;
    bias /= sampleCount;

	return vec2(scale, bias);
}

float radicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec3 importanceSampleGGX(vec2 Xi, vec3 n, float roughness)
{
    // gives halfway vector closer to the normal the lesser the roughness 
    // and low discrepency sequence.
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    // dot product between normal and halfway vector decreases to 0 Xi.y and roughness increase to 1.
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 h = vec3(cos(phi) * sinTheta, cos(phi) * sinTheta, cosTheta);
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);
    mat3 TBN = mat3(tangent, bitangent, n);
	
    vec3 sampleVec = TBN * h;
    return normalize(sampleVec);
}  

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}  

float distributionGGX(float nDotH, float roughness)
{
	float a2 = roughness * roughness;
	float denom = nDotH * nDotH * (a2 -1.0) + 1.0;
	denom *= PI * denom;

	return a2 / denom;
}

float geometrySchlickGGX(float nDotV, float roughness)
{
    float k = (roughness * roughness) / 2.0;
	return nDotV / ((1.0 - k) * nDotV + k);
}
